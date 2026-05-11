void save_spectra_to_file() {
    // 1. Setup the Chain
    TChain *chain = new TChain("IndividualHits");
    chain->Add("radiator_*.root");

    
    // 2. Create the 2D Histogram (Thickness in cm, Energy in MeV)
    TH2F *htemp = new TH2F("htemp", "Energy vs Thickness", 3, 0, 1.5, 100, 0, 50);
    
    std::cout << "Filling 2D histogram from chain..." << std::endl;
    chain->Draw("HitKineticEnergy:HitZ >> htemp", "HitPDG == 1", "goff");

    TFile *finalFile = TFile::Open("30MeV_spectra.root", "UPDATE");

    // 4. Loop through thicknesses 1mm to 30mm
    for (int i = 1; i <= 3; i++) {
            double thickness_cm = (double)i * 5 * (1/10.0) - 0.01; //to make sure the correct bin is selected
            int bin = htemp->GetXaxis()->FindBin(thickness_cm);
            
            // Project the slice
            TH1D *proj = htemp->ProjectionY(Form("proj_%dmm", i*5), bin, bin);
            
            if (proj->GetEntries() > 0) {
                // Save this specific 1D histogram into the file
                proj->Write(); 
                // std::cout << "Saved projection for " << i << "mm to all_spectra.root" << std::endl;
            }
            // No delete proj here, outFile->Write() will handle the memory
    }

    // 5. Cleanup
    finalFile->Close();
    // std::cout << "Done! File 'all_spectra.root' is ready." << std::endl;
}
