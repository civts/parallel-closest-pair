#! /usr/bin/env bash

# This script is intended to be run by the CI, on the cluster.

# This is for runnig the program multiple times to benchmark it

# It assumes that the current directory contains:
# - this script
# - the data directory, with the input files
# - the binary of the program, called parallel_closest_points
# - the script notify_on_telegram.sh with the secrets populated
# - the file with the info on the trigger called run_trigger

BENCHMARK_NAME={{BENCHMARK_NAME}}

MEM_GB=1
MAX_MINUTES=02
GITHUB_COMMIT_SHA=$(cat run_trigger | grep "commit hash" | sed 's/.*hash=//')
GITHUB_COMMIT_AUTHOR=$(cat run_trigger | grep "commit author" | sed 's/.*author=//')

if (($# != 1)); then
  echo >&2 "This script takes exactly one argument: the path to the data directory"
  exit 1
fi

DATA_DIR_PATH=$(echo $1 | sed 's/\/$//')

if [[ ! -d $DATA_DIR_PATH ]]; then
  echo >&2 "Can't find data diretory at $DATA_DIR_PATH"
  exit 1
fi

mkdir runs

ensure_home_is_not_full() {
  MAX_HOME_SIZE=10
  #We keep a 1GB buffer
  HOME_LIMIT=$(($MAX_HOME_SIZE - 1))
  HOME_DIR_SIZE=$(du -s -BG /home/{{UNITN_BASE}} | awk '{print $1;}' | sed 's/G//g')
  if [ $HOME_DIR_SIZE -ge $HOME_LIMIT ]; then
    ./notify_on_telegram.sh \
      "We are getting close to the maximum dimensions of /home. Currently at ${HOME_DIR_SIZE}GB. Closing here 🥺" \
      --loud
    exit 1
  fi
}

I=0
TOTAL_RUNS=1
for RUN_INDEX in {1..$TOTAL_RUNS}; do #Run the experiment 8 times so that we can average the results
  for INPUT_FILE in $DATA_DIR_PATH/*.txt; do
    ensure_home_is_not_full
    for N_CPUS in 1 2 4 8 16; do
      for N_NODES in 1 2 4 8 16; do
        for STRATEGY in "pack" "scatter" "pack:excl" "scatter:excl"; do
          I=$((I + 1))
        done # STRATEGY for loop
      done   # N_NODES for loop
    done     # N_CPUS for loop
  done       # INPUT_FILE for loop
done         # For 8 times

./notify_on_telegram.sh "Starting the benchmark $BENCHMARK_NAME ($I jobs) 🛩"

for RUN_INDEX in {1..$TOTAL_RUNS}; do #Run the experiment 8 times so that we can average the results
  for INPUT_FILE in $DATA_DIR_PATH/*.txt; do
    ensure_home_is_not_full
    for N_CPUS in 1 2 4 8 16; do
      for N_NODES in 1 2 4 8 16; do
        for STRATEGY in "pack" "scatter" "pack:excl" "scatter:excl"; do

          # Create run directory
          INPUT_NAME_CLEAN=$(echo $INPUT_FILE | sed 's/.*\///' | sed 's/\.txt//')
          RUN_DIRECTORY_NAME="${INPUT_NAME_CLEAN}_${N_CPUS}_${N_NODES}_${STRATEGY}"
          RUN_DIRECTORY="$(pwd)/runs/$RUN_DIRECTORY_NAME"
          OUTPUT_DIR_FOR_RUN="$RUN_DIRECTORY/output"
          EXECUTABLE_FOR_RUN="$RUN_DIRECTORY/parallel_closest_points"
          N_PROCESSES=$((N_NODES * N_CPUS))

          if [ $N_PROCESSES -gt 64 ]; then
            MAX_MINUTES=05
          fi

          mkdir -p "$RUN_DIRECTORY/data" "$OUTPUT_DIR_FOR_RUN"

          # "copy" (link) the files to the run directory
          INPUT_FILE_FOR_RUN="$RUN_DIRECTORY/data/input.txt"
          ln -s "$(pwd)/$INPUT_FILE" "$INPUT_FILE_FOR_RUN"
          ln -s "$(pwd)/parallel_closest_points" "$EXECUTABLE_FOR_RUN"

          # Create the PBS script to run the program
          RUN_SCRIPT="$RUN_DIRECTORY/run.sh"

          cat >$RUN_SCRIPT <<EOL
#! /usr/bin/env bash

#PBS -l select=$N_NODES:ncpus=$N_CPUS:mem=${MEM_GB}gb

# Set maximum execution time
#PBS -l walltime=0:$MAX_MINUTES:00

# Set the node placement strategy
#PBS -l place=$STRATEGY

# Select execution queue
#PBS -q short_cpuQ

# Load MPI environment
module load mpich-3.2

echo "Running the mpiexec command"

# Run the job
mpiexec -n $N_PROCESSES $EXECUTABLE_FOR_RUN $INPUT_FILE_FOR_RUN $OUTPUT_DIR_FOR_RUN

echo "Unloading MPI module"
# Unload MPI environment
module unload mpich-3.2

echo "All done for $RUN_SCRIPT"

EOL

          chmod +x $RUN_SCRIPT

          # Run the job
          SUBMITTED_ON=$(date)
          JOB_ID=$(qsub $RUN_SCRIPT)

          # Wait for it to complete
          INTERVAL_SECONDS=10
          MAX_CHECKS=$(((MAX_MINUTES + 2) * (60 / INTERVAL_SECONDS)))

          I=0
          RUN_STATUS="R"
          while [[ "$RUN_STATUS" != "F" && $I -le $MAX_CHECKS ]]; do
            sleep $INTERVAL_SECONDS
            I=$((I + 1))
            JOB_RESULT_QSTAT=$(qstat $JOB_ID -H | tail -n 1)
            RUN_STATUS=$(echo $JOB_RESULT_QSTAT | awk '{print $10;}')
          done

          # Collect output
          TIME_AVAILABLE=$(echo $JOB_RESULT_QSTAT | awk '{print $9;}')
          TIME_ELAPSED=$(echo $JOB_RESULT_QSTAT | awk '{print $11;}')
          case $RUN_STATUS in
          "F")
            if [[ "$TIME_AVAILABLE" == "$TIME_ELAPSED" ]]; then
              STATUS_MESSAGE="Timed out 🦖💥"
            else
              OUT_FILE="$OUTPUT_DIR_FOR_RUN/-1.txt"
              if [[ -f "$OUT_FILE" ]]; then
                MIN_DIST=$(cat "$OUT_FILE" | tail -n 1)
                STATUS_MESSAGE="Finished successfully 🧸"
              else
                STATUS_MESSAGE="Failed for an unknown reason 🤐💥. It ran for $TIME_ELAPSED minutes"
              fi
            fi
            ;;
          *)
            STATUS_MESSAGE="Finished with an unknown status: $EXIT_CODE 👾. It ran for $TIME_ELAPSED minutes"
            ;;
          esac

          TIME_AVAILABLE=$(echo $JOB_RESULT_QSTAT | awk '{print $9;}')
          TIME_ELAPSED=$(echo $JOB_RESULT_QSTAT | awk '{print $11;}')

          READING_TIME=$(grep "Reading time" $OUTPUT_DIR_FOR_RUN/0.txt | awk '{print $3;}')
          SCATTER_TIME=$(grep "Scatter time" $OUTPUT_DIR_FOR_RUN/0.txt | awk '{print $3;}')
          TOTAL_TIME=$(grep "Total time" $OUTPUT_DIR_FOR_RUN/0.txt | awk '{print $3;}')
          FINAL_DISTANCE=$(grep "Final distance" $OUTPUT_DIR_FOR_RUN/0.txt | awk '{print $3;}')

          # Send output
          RESULTS_GOOGLE_FORM_ID={{RESULTS_GOOGLE_FORM_ID}}
          curl -X POST --silent \
            -F "comment=Output from process 0" \
            -F "entry.39232730=$(cat $OUTPUT_DIR_FOR_RUN/0.txt)" \
            -F "comment=Run Status" \
            -F "entry.366340186=$STATUS_MESSAGE" \
            -F "comment=Github commit SHA" \
            -F "entry.557446610=$GITHUB_COMMIT_SHA" \
            -F "comment=Reading time" \
            -F "entry.826027480=$READING_TIME" \
            -F "comment=Cluster job ID" \
            -F "entry.841155711=$JOB_ID" \
            -F "comment=Submitted on" \
            -F "entry.883078977=$SUBMITTED_ON" \
            -F "comment=Scattering time" \
            -F "entry.1088844820=$SCATTER_TIME" \
            -F "comment=Minimum Distance" \
            -F "entry.1239478360=$FINAL_DISTANCE" \
            -F "comment=Github commit author" \
            -F "entry.1812592966=$GITHUB_COMMIT_AUTHOR" \
            -F "comment=Input size" \
            -F "entry.1930476245=$INPUT_NAME_CLEAN" \
            -F "comment=Number of CPUs" \
            -F "entry.663311192=$N_CPUS" \
            -F "comment=Number of Nodes" \
            -F "entry.763677391=$N_NODES" \
            -F "comment=Placing strategy" \
            -F "entry.1527262616=$STRATEGY" \
            -F "comment=GB of memory" \
            -F "entry.1762526935=$MEM_GB" \
            -F "comment=Maximum duration (minutes)" \
            -F "entry.1014747154=$MAX_MINUTES" \
            -F "comment=Total run time (seconds)" \
            -F "entry.133395210=$TOTAL_TIME" \
            -F "comment=Benchmark name" \
            -F "entry.1067054614=$BENCHMARK_NAME" \
            "https://docs.google.com/forms/d/e/$RESULTS_GOOGLE_FORM_ID/formResponse" &>/dev/null

        done # STRATEGY for loop
      done   # N_NODES for loop
    done     # N_CPUS for loop
  done       # INPUT_FILE for loop
  ./notify_on_telegram.sh "[$BENCHMARK_NAME] finished run $RUN_INDEX/$TOTAL_RUNS 🛤"
done # For 8 times

./notify_on_telegram.sh "Test benchmark $BENCHMARK_NAME finished! 📅" --loud
for A in 'BOT_TOKEN' 'TELEGRAM_CHAT_ID'; do
  sed -i "s/\$A=.*/\$A=redacted/g" ./notify_on_telegram.sh
done
sed -i "s/RESULTS_GOOGLE_FORM_ID=.*/RESULTS_GOOGLE_FORM_ID=redacted/g" ./benchmark.sh
