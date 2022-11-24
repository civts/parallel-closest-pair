#!/bin/bash
#PBS -l select=1:ncpus=4:mem=2gb

# max exec time
#PBS -l walltime=0:05:00

# execution queue
#PBS -q short_cpuQ
module load mpich-3.2
mpirun.actual -n 4 ./project/parallel-closest-pair/parallel/main ./project/parallel-closest-pair/data/test_dataset.txt ./project/parallel-closest-pair/parallel/outputs/
