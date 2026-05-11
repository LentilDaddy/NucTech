void init_Photonhist() {
    // Create the ROOT file in RECREATE mode
    TFile *f = new TFile("30MeV_spectra.root", "RECREATE");
    
    // Create the initial histogram
    // TH1F *h_final = new TH1F("h_final", "Final Histogram", 1, 0, 0.14);
    TH2F *htemp = new TH2F("htemp", "Energy vs Thickness", 3, 0, 1.5, 100, 0, 50);
    
    // Write the histogram to the file and close
    htemp->Write();
    f->Close();
    
    // Clean up pointer
    delete f;
}
