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

ensure_home_is_not_full

TARGET_PARALLEL_SCRIPT="./parallel.sh"
FINALIZE_SCRIPT="$(pwd)/finalize.sh"
N_CPUS=4
N_NODES=2
N_PROCESSES=$((N_NODES * N_CPUS))
MEM_GB=2
MAX_MINUTES=05
INPUT_FILE=5k.txt
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
INPUT_PATH=$(pwd)/../../code/data/${INPUT_FILE}
OUTPUT_PATH=$OUTPUT_PATH

mkdir \$OUTPUT_PATH

# Run the job
mpiexec -n ${N_PROCESSES} \$EXECUTABLE "\$INPUT_PATH" "\$OUTPUT_PATH" ${FINALIZE_SCRIPT}

# Unload MPI environment
module unload mpich-3.2

EOL
chmod +x $TARGET_PARALLEL_SCRIPT

echo "Submitting the job"
# Execute the script
JOB_ID=$(qsub $TARGET_PARALLEL_SCRIPT)
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

cat >$FINALIZE_SCRIPT <<EOL
#! /usr/bin/env bash

cd $(pwd)

echo "Job finished correctly, all is good"

zip -r outputs.zip $OUTPUT_DIR

$(pwd)/notify_on_telegram.sh \
"Job $NICKNAME_NICE finished 🧸.
Its job id was $JOB_ID.
Input: $INPUT_FILE.
It was submitted $START.

It was tirggered by:
$TRIGGER_INFO" \
--file outputs.zip

for A in 'BOT_TOKEN' 'TELEGRAM_CHAT_ID'; do 
  sed -i "s/$A=.*/$A=redacted/g" $FINALIZE_SCRIPT
done

rm outputs.zip
EOL

chmod +x $FINALIZE_SCRIPT

./notify_on_telegram.sh \
  "A new job started on the cluster 🌠
Codename: $NICKNAME_NICE
Job_id: $JOB_ID
Input: $INPUT_FILE.

Triggered by:
$TRIGGER_INFO
  "

echo -n $JOB_ID >job_id
