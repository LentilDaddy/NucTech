# 1. Dynamic Environment Setup
# Use geant4-config to find data paths if the variables aren't already set
if ! command -v geant4-config &> /dev/null; then
    echo "Geant4 not found in PATH. Please load the module first."
    exit 1
fi

# 2. Safety: Only set these if they aren't already exported by a module
export ROOT_TTREE_MAX_SIZE=500000000000

# Initialize final histogram using an external macro
root -l -b -q init_Photonhist.C

# Loop through batches
for batch in {0..1}; do
  start=$((batch * 10 + 1))
  end=$(((batch + 1) * 10))
  
  # Run simulations SEQUENTIALLY to avoid crashing the node
  for i in $(seq $start $end); do
    ./nucTech run.mac radiator_$i.root &
  done
  wait
  
  # Combine batch and add to final histogram
  root -l -b -q save_spectra_to_file.C
  wait

  rm radiator_*.root
done
