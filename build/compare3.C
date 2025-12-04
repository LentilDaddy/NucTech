
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
#include <TLine.h>
#include <TGraph.h>
#include <TGaxis.h>
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



// int kP10Cyan = TColor::GetColor("#92dadd");
// int kP10Ash = TColor::GetColor("#717581");
// int kP10Green = TColor::GetColor("#b9ac70");
// int kP10Orange = TColor::GetColor("#e76300");
// int kP10Brown = TColor::GetColor("#a96b59");
// int kP10Violet = TColor::GetColor("#832db6");
// int kP10Gray = TColor::GetColor("#94a4a2");
// int kP10Red = TColor::GetColor("#bd1f01");
// int kP10Yellow = TColor::GetColor("#ffa90e");
// int kP10Blue = TColor::GetColor("#3f90da");




void compare3()
{
    gROOT->SetBatch(kTRUE);

    // define mediums:
    // std::vector<std::string> Media = {"SF6", "PF5", "C3F8", "CF4"};
    std::vector<std::string> Media = {"SF6"};

    // create chains
    std::vector<std::pair<std::string, TChain*>> chains;
    for (const auto &medium : Media) {
        TChain *ch = new TChain("IndividualHits");
        ch->Add(TString::Format("%s*.root", medium.c_str()));
        chains.push_back({medium, ch});
    }

    // create main canvas
    TCanvas *c = new TCanvas("c", "Primary Electron Spread", 1400, 1400);
    int N = chains.size();
    int rows = std::ceil(std::sqrt(N));
    int cols = std::ceil((double)N / rows);
    c->Divide(cols, rows);

    for (size_t i = 0; i < chains.size(); i++) {
        TChain *t = chains[i].second;
        std::string label = chains[i].first;

        if (!t || t->GetEntries() == 0) {
            std::cout << "Warning: Chain " << label << " is empty!" << std::endl;
            continue;
        }

        TH2D *h = new TH2D(
            TString::Format("h2_%s", label.c_str()),
            TString::Format("Primary Electron Spread - %s", label.c_str()),
            2000, 0.0, 20.0,      // depth range
            1000, 0.0, 10.0       // radius range
        );
        h->SetDirectory(nullptr);

        float_t z, kineticE, r;
        int pdg;

        t->SetBranchStatus("*", 0);
        // t->SetBranchStatus("HitX", 1);
        // t->SetBranchStatus("HitY", 1);
        t->SetBranchStatus("HitZ", 1);
        t->SetBranchStatus("HitR", 1);
        t->SetBranchStatus("HitPDG", 1);
        t->SetBranchStatus("HitParentID", 1);
        t->SetBranchStatus("HitKineticEnergy", 1);
        // t->SetBranchAddress("HitX", &x);
        // t->SetBranchAddress("HitY", &y);
        t->SetBranchAddress("HitZ", &z);
        t->SetBranchAddress("HitR", &r);
        t->SetBranchAddress("HitPDG", &pdg);
        t->SetBranchAddress("HitParentID", &parentID);
        t->SetBranchAddress("HitKineticEnergy", &kineticE);

        Long64_t nentries = t->GetEntries();
        for (Long64_t j = 0; j < nentries; ++j) {
            t->GetEntry(j);
            if (pdg == 0 && )
                h->Fill(z, r);
        }

        c->cd(i+1);
        gPad->SetRightMargin(0.12);
        gPad->SetLogz();
        h->GetXaxis()->SetTitle("Depth (cm)");
        h->GetYaxis()->SetTitle("Radius (cm)");
        h->Draw("COLZ");
    }

    c->SaveAs("PrimarySpread.png");
    // std::cout << "Saved: PhotonEnergy_Compare.png\n";
}

int main() {
    compare3();
    return 0;
}
