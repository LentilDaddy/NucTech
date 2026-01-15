
#include <TSystem.h>
#include <TPluginManager.h> 
#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>
#include <TChain.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TGraph.h>
#include <TMultiGraph.h>
#include <TStyle.h>
#include <TString.h>
#include <TLatex.h>
#include <iostream>
#include <vector>
#include <string>
#include <regex>
#include <cmath>

struct HistogramDecoration {
   int lineWidth;
   int lineColor;
   const char* xTitle;
   const char* yTitle;
   const char* title;
   double xMin;
   double xMax;
   double yMin;
   double yMax;
};

void compare()
{
    gROOT->SetBatch(kTRUE);

    //===============================
    // Define TChains for each medium
    //===============================

    std::vector<std::pair<std::string, TChain*>> chains;

    // std::vector<std::string> mediums = {"SF6", "C3F8", "CF4", "PF5", "UF6", "vacuum"};
    std::vector<std::string> mediums = {"vacuum"};
    // std::vector<std::string> energies = {"20MeV", "22MeV", "30MeV", "28MeV", "30MeV", "35MeV", "40MeV", "45MeV", "50MeV"};
    std::vector<std::string> energies = {"12GeV"};

    double foilThickness = 2; //2cm
    

    // create one TChain per (medium, energy) and add matching files immediately
    for (const auto &m : mediums) {
        for (const auto &e : energies) {
            std::string label = m + "_" + e; //does this mean it has to be in this order?
            TChain *ch = new TChain("IndividualHits");
            ch->Add(TString::Format("%s_*%s*.root", m.c_str(), e.c_str()).Data());
            chains.push_back({label, ch});
        }
    }

    TLegend *legend = new TLegend(0.7,0.7,0.9,0.9);
    TCanvas *c1 = new TCanvas("c1", "Compare Energy Deposition", 900, 700);
    c1->cd();

    // std::vector<TH2D*> h2_histos;

TH2D *h2 = new TH2D("h2", "Energy vs Radius", 20, -1, 1, 100, 0, 12000);

    

for (size_t i = 0; i < chains.size(); i++) {
    TChain *t = chains[i].second;
    std::string label = chains[i].first;

    if (!t || t->GetEntries() == 0) {
        std::cout << "Warning: Chain " << label << " is empty!" << std::endl;
        continue;
    }

    // Disable global ROOT directory writing for safety
    h2->SetDirectory(nullptr);

    Float_t z, kineticE, r;
    Int_t pdg;
    t->SetBranchAddress("HitZ", &z);
    t->SetBranchAddress("HitR", &r);
    t->SetBranchAddress("HitPDG", &pdg);
    t->SetBranchAddress("HitKineticEnergy", &kineticE);

    Long64_t nentries = t->GetEntries();
    for (Long64_t j = 0; j < nentries; ++j) {
        t->GetEntry(j);
        // if (pdg == 1 && kineticE >= 15 && kineticE <= 22 && z > foilThickness/10)
        //     h->Fill(r);
        if (pdg == 1 && kineticE >= 0 && z == foilThickness + 5000) //foilthickness and 500 are in cm! 
            h2->Fill(r, kineticE);
    }

    {
        // h2_histos.push_back(h2);
        //now draw and save the histogram as a png
        h2->SetTitle("Energy vs Radius at 50m from copper foil");
        h2->GetXaxis()->SetTitle("Radius (cm)");
        h2->GetYaxis()->SetTitle("Photon Energy (MeV)");
        // h2->SetStats(0);
        h2->Draw("COLZ");
        c1->SaveAs("Energy_vs_Radius.png");
    }
}

}

int main() {
    compare();
    return 0;
}
