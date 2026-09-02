void save_spectra_to_file() {
    // 1. Setup the Chain
    TChain *chain = new TChain("IndividualHits");
    chain->Add("radiator_*.root");

    // 2. Create the 2D Histogram (Thickness in cm, Energy in MeV)
    TH2F *htemp = new TH2F("htemp", "Energy vs Thickness", 1, 0.14, 1.14, 100, 0, 50);
    
    std::cout << "Filling 2D histogram from chain..." << std::endl;
    chain->Draw("HitKineticEnergy:HitZ >> htemp", "HitPDG == 1", "goff");

    // 3. Create a new ROOT file to store the 1D projections
    TFile *outFile = new TFile("Tungsten_30MeV_spectra.root", "RECREATE");

    // 4. Loop through thicknesses 1mm to 30mm

    // int bin = htemp->GetXaxis()->FindBin(0.14+0.01); // Find the bin corresponding to 1mm (0.1cm) thickness
    // TH1D *proj = htemp->ProjectionY(Form("proj_%dcm", 1), bin, bin); //1cm after the tungsten
    // if (proj->GetEntries() > 0) {
    //     // Save this specific 1D histogram into the file
    //     proj->Write();
    // } 
        

    for (int i = 1; i <= 6; i++) {
        if (i == 1) {
            double thickness_cm = (double)i - 0.01; //to make sure the correct bin is selected
            int bin = htemp->GetXaxis()->FindBin(thickness_cm);
            
            // Project the slice
            TH1D *proj = htemp->ProjectionY(Form("proj_%dcm", i), bin, bin);
            
            if (proj->GetEntries() > 0) {
                // Save this specific 1D histogram into the file
                proj->Write(); 
                std::cout << "Saved projection for " << i << "mm to all_spectra.root" << std::endl;
            }
            // No delete proj here, outFile->Write() will handle the memory
        }
        else {
            continue;
        }

    }

    // 5. Cleanup
    outFile->Close();
}
