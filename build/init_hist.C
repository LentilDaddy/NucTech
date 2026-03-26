void init_hist() {
    // Create the ROOT file in RECREATE mode
    TFile *f = new TFile("SF6.root", "RECREATE");
    
    // Create the initial histogram
    TH1F *h_final = new TH1F("h_final", "Final Histogram", 400, 0, 200);
    
    // Write the histogram to the file and close
    h_final->Write();
    f->Close();
    
    // Clean up pointer
    delete f;
}