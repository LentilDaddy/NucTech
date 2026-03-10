#!/usr/bin/env bash
export G4LEVELGAMMADATA=/shared/physics/physdata/nuclear/JLab/js3096/NucTech/PhotonEvaporation6.1
export G4RADIOACTIVEDATA=/shared/physics/physdata/nuclear/JLab/js3096/NucTech/RadioactiveDecay6.1.2
export G4LEDATA=/opt/york/easybuild/software/Geant4/11.1.3-GCC-12.3.0/share/Geant4/data/G4EMLOW8.2  
export ROOT_TTREE_MAX_SIZE=500000000000  # e.g., 500 GB

for i in {1..100}
do
  ./nucTech run.mac radiator_$i.root &
done

wait

scp radiator_*.root hadron8:/old_disk/js3096/NucTech/projections2
wait

rm radiator_*.root

wait

for i in {101..200}
do
  ./nucTech run.mac radiator_$i.root &
done

wait

scp radiator_*.root hadron8:/old_disk/js3096/NucTech/projections2
wait

rm radiator_*.root

wait

for i in {201..300}
do
  ./nucTech run.mac radiator_$i.root &
done

wait

scp radiator_*.root hadron8:/old_disk/js3096/NucTech/projections2
wait

rm radiator_*.root

wait

for i in {301..400}
do
  ./nucTech run.mac radiator_$i.root &
done

wait

scp radiator_*.root hadron8:/old_disk/js3096/NucTech/projections2
wait

rm radiator_*.root

wait

for i in {401..500}
do
  ./nucTech run.mac radiator_$i.root &
done

wait

scp radiator_*.root hadron8:/old_disk/js3096/NucTech/projections2
wait

rm radiator_*.root

wait

for i in {501..600}
do
  ./nucTech run.mac radiator_$i.root &
done

wait

scp radiator_*.root hadron8:/old_disk/js3096/NucTech/projections2
wait

rm radiator_*.root

wait

for i in {601..700}
do
  ./nucTech run.mac radiator_$i.root &
done

wait

scp radiator_*.root hadron8:/old_disk/js3096/NucTech/projections2
wait

rm radiator_*.root

wait

for i in {701..800}
do
  ./nucTech run.mac radiator_$i.root &
done

wait

scp radiator_*.root hadron8:/old_disk/js3096/NucTech/projections2
wait

rm radiator_*.root

wait

for i in {801..900}
do
  ./nucTech run.mac radiator_$i.root &
done

wait

scp radiator_*.root hadron8:/old_disk/js3096/NucTech/projections2
wait

rm radiator_*.root

wait

for i in {901..1000}
do
  ./nucTech run.mac radiator_$i.root &
done

wait

scp radiator_*.root hadron8:/old_disk/js3096/NucTech/projections2
wait

rm radiator_*.root

wait