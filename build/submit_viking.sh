#!/usr/bin/env bash
#SBATCH --job-name=NucTechRun
#SBATCH --time=10:00:00         # Max run time (4 hours)
#SBATCH --ntasks=32               # Use all 96 threads
#SBATCH --cpus-per-task=1         # Each simulation gets 1 thread
#SBATCH --mem=180G                # Request enough RAM (check sinfo for node max)
#SBATCH --nodes=1                 # Keep it on one node for simplicity
#SBATCH --output=sim_%j.log     # Save terminal output to a log file

# 1. Load the modules inside the script so the compute node has them
module purge
module load Geant4/11.3.0-GCC-13.2.0
module load ROOT/6.32.20-foss-2023b

export ROOT_TTREE_MAX_SIZE=500000000000

# Initialize final histogram using an external macro
root -l -b -q init_hist.C

# Loop through batches 0 to 9 
for batch in {0..1}; do
  start=$((batch * 100 + 1))
  end=$(((batch + 1) * 100))
  

  for i in $(seq $start $end); do
    ./nucTech run.mac out_$i.root &
  done
  wait
  
  # Combine batch and add to final histogram
  root -l -b -q combine_batch.C
  wait

  rm out_*.root
done
