#!/usr/bin/env bash
export G4LEVELGAMMADATA=/shared/physics/physdata/nuclear/JLab/js3096/NucTech/PhotonEvaporation6.1
export G4RADIOACTIVEDATA=/shared/physics/physdata/nuclear/JLab/js3096/NucTech/RadioactiveDecay6.1.2
export G4LEDATA=/opt/york/easybuild/software/Geant4/11.1.3-GCC-12.3.0/share/Geant4/data/G4EMLOW8.2  
export ROOT_TTREE_MAX_SIZE=500000000000

# Initialize final histogram using an external macro
root -l -b -q init_hist.C

# Loop through batches
for batch in {0..9}; do
  start=$((batch * 100 + 1))
  end=$(((batch + 1) * 100))
  
  # Run simulations
  for i in $(seq $start $end); do
    ./nucTech run.mac out_$i.root &
  done
  wait
  
  # Combine batch and add to final histogram using an external macro
  root -l -b -q combine_batch.C
  wait
  
  rm out_*.root
done