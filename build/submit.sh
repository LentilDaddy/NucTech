#!/usr/bin/env bash

# 1. Dynamic Environment Setup
# Use geant4-config to find data paths if the variables aren't already set
if ! command -v geant4-config &> /dev/null; then
    echo "Geant4 not found in PATH. Please load the module first."
    exit 1
fi

# 2. Safety: Only set these if they aren't already exported by a module
export ROOT_TTREE_MAX_SIZE=500000000000

# 3. Execution
root -l -b -q init_hist.C

for batch in {0..1}; do
  start=$((batch * 100 + 1))
  end=$(((batch + 1) * 100))
  
  for i in $(seq $start $end); do
    # Using 'time' or 'nice' can also help manage resources on shared clusters
    ./nucTech run.mac out_$i.root &
  done
  wait
  
  root -l -b -q combine_batch.C
  wait
  
  # Clean up safely
  rm out_*.root
done