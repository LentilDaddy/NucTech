void init_hist() {
    // Create the ROOT file in RECREATE mode
    TFile *f = new TFile("C6F14_30MeV.root", "RECREATE");
    
    // Create the initial histogram
    TH1F *h_final = new TH1F("h_final", "Final Histogram", 3, 0, 3);
    
    // Write the histogram to the file and close
    h_final->Write();
    f->Close();
    
    // Clean up pointer
    delete f;
}