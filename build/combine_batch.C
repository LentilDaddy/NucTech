void combine_batch() {
    // Chain all out_*.root files from the current batch
    TChain *myChain = new TChain("EnergySpectrum");
    myChain->Add("out_*.root");

    // Create a temporary batch histogram in memory
    TH1F *h_batch = new TH1F("h_batch", "Batch Histogram", 3, 0, 3);
    
    // Draw ReactionCount into h_batch. 
    // "goff" turns off graphics rendering, which is safer/faster for background scripts.
    myChain->Draw("ReactionCount >> h_batch", "", "goff");

    // Open the main file in UPDATE mode
    TFile *finalFile = TFile::Open("H2O_25MeV_VolumeA.root", "UPDATE");
    
    // Retrieve the existing final histogram
    TH1F *h_final = (TH1F*)finalFile->Get("h_final");
    
    // Add the batch to the final histogram and overwrite the previous version
    if (h_final) {
        h_final->Add(h_batch);
        finalFile->cd();
        h_final->Write("h_final", TObject::kOverwrite);
    } else {
        std::cerr << "Error: Could not find h_final in SF6.root" << std::endl;
    }
    
    // Cleanup and close
    finalFile->Close();
    delete finalFile;
    delete myChain;
    delete h_batch;
}