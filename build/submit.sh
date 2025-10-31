#!/usr/bin/env bash
export G4LEVELGAMMADATA=/shared/physics/physdata/nuclear/JLab/js3096/NucTech/PhotonEvaporation6.1
export G4RADIOACTIVEDATA=/shared/physics/physdata/nuclear/JLab/js3096/NucTech/RadioactiveDecay6.1.2
export G4LEDATA=/opt/york/easybuild/software/Geant4/11.1.3-GCC-12.3.0/share/Geant4/data/G4EMLOW8.2  
export ROOT_TTREE_MAX_SIZE=500000000000  # e.g., 500 GB

for i in {1..100}
do
  ./nucTech run.mac out_$i.root &
done

wait
# hadd -f1 out.root out_*.root
# rm out_*.root

# Merge all ROOT output files safely using TChain (avoids hadd limits)
# root -l -b -q -e '{ TChain c("IndividualHits"); c.Add("out_*.root"); c.Merge("out.root"); }'
root -l -b -q -e '{ TChain c("IndividualHits"); c.Add("out_*.root"); }'


# rm out_*.root
