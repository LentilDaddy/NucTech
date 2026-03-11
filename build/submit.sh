#!/usr/bin/env bash
export G4LEVELGAMMADATA=/shared/physics/physdata/nuclear/JLab/js3096/NucTech/PhotonEvaporation6.1
export G4RADIOACTIVEDATA=/shared/physics/physdata/nuclear/JLab/js3096/NucTech/RadioactiveDecay6.1.2
export G4LEDATA=/opt/york/easybuild/software/Geant4/11.1.3-GCC-12.3.0/share/Geant4/data/G4EMLOW8.2  
export ROOT_TTREE_MAX_SIZE=500000000000

# Initialize final histogram
root -l -b -q <<'EOF'
TFile *f = new TFile("SF6.root", "RECREATE");
TH1F *h_final = new TH1F("h_final", "Final Histogram", 3, 0, 3);
f->Close();
EOF

# Loop through 10 batches of 100 simulations each (1-1000)
for batch in {0..9}; do
  start=$((batch * 100 + 1))
  end=$(((batch + 1) * 100))
  
  # echo "Processing simulations $start to $end..."
  
  # Run simulations in parallel
  for i in $(seq $start $end); do
    ./nucTech run.mac out_$i.root &
  done
  wait
  
  # Combine batch and add to final histogram
  root -l -b -q <<EOF
TChain *myChain = new TChain("EnergySpectrum");
myChain->Add("out_*.root");
myChain->Draw("ReactionCount >> h_batch(3, 0, 3)");

TFile *finalFile = TFile::Open("SF6.root", "UPDATE");
TH1F *h_final = (TH1F*)finalFile->Get("h_final");
h_final->Add(h_batch);
finalFile->cd();
h_final->Write("h_final", TObject::kOverwrite);
finalFile->Close();
EOF
  
  wait
  rm out_*.root
done

# echo "Complete! Final histogram saved to SF6.root"