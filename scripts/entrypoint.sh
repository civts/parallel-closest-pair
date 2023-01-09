#! /usr/bin/env bash

# This script is intended to be run by the CI, on the cluster.

NICKNAME=$(cat ./tag)
NICKNAME_NICE=$(cut -d '_' -f 1 <<<"$NICKNAME")
TRIGGER_INFO=$(cat ./run_trigger | tail -n +2)

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

echo "Starting 🏃"

echo "Checking that the home directory is not full 🏨"
ensure_home_is_not_full

TARGET_PARALLEL_SCRIPT="./parallel.sh"
FINALIZE_SCRIPT="$(pwd)/finalize.sh"
N_CPUS=8
N_NODES=2
N_PROCESSES=$((N_NODES * N_CPUS))
MEM_GB=2
MAX_MINUTES=05
INPUT_FILE=1M.txt
OUTPUT_DIR=outputs/
OUTPUT_PATH=$(pwd)/$OUTPUT_DIR

cat >$TARGET_PARALLEL_SCRIPT <<EOL
#! /usr/bin/env bash

#PBS -l select=${N_NODES}:ncpus=${N_CPUS}:mem=${MEM_GB}gb

# Set maximum execution time
#PBS -l walltime=0:${MAX_MINUTES}:00

# Select execution queue
#PBS -q short_cpuQ

# Load MPI environment
module load mpich-3.2

EXECUTABLE=$(pwd)/parallel_closest_points
INPUT_PATH=$(pwd)/data/${INPUT_FILE}
OUTPUT_PATH=$OUTPUT_PATH

mkdir \$OUTPUT_PATH

echo "Running the mpiexec command"

# Run the job
mpiexec -n ${N_PROCESSES} \$EXECUTABLE "\$INPUT_PATH" "\$OUTPUT_PATH"

echo "Unloading MPI module"
# Unload MPI environment
module unload mpich-3.2

echo "All done in $TARGET_PARALLEL_SCRIPT"

EOL

chmod +x $TARGET_PARALLEL_SCRIPT

echo "Submitting the job"
# Execute the script
JOB_ID=$(qsub $TARGET_PARALLEL_SCRIPT)
JOB_NUMBER=$(echo $JOB_ID | sed 's/\..*//')
if [ $? ]; then
  echo "Launched 🚀"
else
  ./notify_on_telegram.sh \
    "There was a problem submitting job $NICKNAME to the queue 🌩.

    It was tirggered by:
    $TRIGGER_INFO" \
    --loud
  exit 1
fi

START=$(date)
INTERVAL_SECONDS=10
MAX_CHECKS=$(((MAX_MINUTES + 2) * (60 / INTERVAL_SECONDS)))

cat >$FINALIZE_SCRIPT <<EOL
#! /usr/bin/env bash

cd $(pwd)

I=0
EXIT_CODE="R"
while [[ "\$EXIT_CODE" != "F" && \$I -le $MAX_CHECKS ]]; do
  sleep $INTERVAL_SECONDS
  I=\$((I + 1))
  JOB_RESULT_QSTAT=\$(qstat $JOB_ID -H | tail -n 1)
  EXIT_CODE=\$(echo \$JOB_RESULT_QSTAT | awk '{print \$10;}')
done

TIME_AVAILABLE=\$(echo \$JOB_RESULT_QSTAT | awk '{print \$9;}')
TIME_ELAPSED=\$(echo \$JOB_RESULT_QSTAT | awk '{print \$11;}')
JOB_INFO="The job id was $JOB_ID
Input: $INPUT_FILE
It was submitted $START

It was tirggered by:
$TRIGGER_INFO"

# Ensure the output diretory exists, even if empty
mkdir -p $OUTPUT_DIR || true

zip -r outputs.zip \
  "$OUTPUT_DIR" \
  "$TARGET_PARALLEL_SCRIPT.e$JOB_NUMBER" \
  "$TARGET_PARALLEL_SCRIPT.o$JOB_NUMBER"

case \$EXIT_CODE in
  "F")
    if [[ "\$TIME_AVAILABLE" == "\$TIME_ELAPSED" ]]; then
      MESSAGE="timed out 🦖💥\nThe time limit was \$TIME_AVAILABLE minutes."
    else
      OUT_FILE=${OUTPUT_DIR}-1.txt
      if [[ -f "\$OUT_FILE" ]]; then
        MIN_DIST=\$( cat "\$OUT_FILE" | tail -n 1)
        MESSAGE="finished successfully 🧸
 
\$MIN_DIST"
      else
        MESSAGE="Failed for an unknown reason 🤐💥
 
It ran for \$TIME_ELAPSED minutes.
The time limit was \$TIME_AVAILABLE minutes."
      fi
      echo $MESSAGE
    fi;;
  *)
    MESSAGE="finished with an unknown status: \$EXIT_CODE 👾
 
It ran for \$TIME_ELAPSED minutes.
The time limit was \$TIME_AVAILABLE minutes" ;;
esac

$(pwd)/notify_on_telegram.sh \
"Job $NICKNAME_NICE \$MESSAGE
\$JOB_INFO" --file outputs.zip

for A in 'BOT_TOKEN' 'TELEGRAM_CHAT_ID'; do 
  sed -i "s/\$A=.*/\$A=redacted/g" $(pwd)/notify_on_telegram.sh
done

rm outputs.zip || true
EOL

chmod +x $FINALIZE_SCRIPT

./notify_on_telegram.sh \
  "A new job started on the cluster 🌠
Codename: $NICKNAME_NICE
Job_id: $JOB_ID
Input: $INPUT_FILE

Triggered by:
$TRIGGER_INFO"

echo -n $JOB_ID >job_id

echo "Starting the finalize script 🕰"
nohup $FINALIZE_SCRIPT 1>"$(pwd)/finalize_stdout.log" 2>"$(pwd)/finalize_stderr.log" &
disown

echo "Farewell 👋"
