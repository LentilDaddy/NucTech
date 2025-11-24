
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


struct EnergyResult {
    std::string energyLabel;
    double energy;
    double foilThickness;
    double stoppedPrimaries;
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




void compare()
{
    gROOT->SetBatch(kTRUE);


    std::vector<std::pair<std::string, TChain*>> chains;

    // Replace the separate energy and foil thickness vectors with specific pairs
    std::vector<std::pair<std::string, std::string>> energyFoilPairs = {
        {"20MeV", "4mm"},
        {"25MeV", "6mm"},
        {"30MeV", "9mm"},
        {"35MeV", "12mm"},
        {"40MeV", "11mm"},
        {"45MeV", "11mm"},
        {"50MeV", "13mm"}
    };

    std::vector<EnergyResult> results;

    for (const auto &pair : energyFoilPairs) {
        std::string energy = pair.first;
        std::string foil = pair.second;
        std::string label = energy + "_" + foil;
        TChain *ch = new TChain("IndividualHits");
        ch->Add(TString::Format("*_%s*_*%s*.root", foil.c_str(), energy.c_str()).Data());
        chains.push_back({label, ch});
    }
    

    //===============================
    // Decorations and setup
    //===============================
    HistogramDecoration Decoration = {
       1, kP10Brown, "Depth (cm)", "#photons in range 15-22 MeV", nullptr, 0., 100., 0., 0.
    };

    int colors[] = {kP10Red, kP10Cyan, kP10Ash, kP10Green, kP10Orange, kP10Brown, kP10Violet, kP10Gray,
                    kP10Yellow, kP10Blue};
    int nColors = sizeof(colors)/sizeof(int);

    TLegend *legend = new TLegend(0.7,0.7,0.9,0.9);
    TCanvas *c1 = new TCanvas("c1", "Compare Energy Deposition", 900, 700);
    c1->cd();

    std::vector<TH1D*> histos;
    std::vector<double> usefulPhotonIntegrals;

for (size_t i = 0; i < chains.size(); i++) {
    TChain *t = chains[i].second;
    std::string label = chains[i].first;

    // Extract foil thickness
    double foilThickness = 0.0;
    auto pos_MeV = label.find("mm");
    if (pos_MeV != std::string::npos) {
        size_t start = label.find_last_of('_', pos_MeV); //find underscore before "mm"
        foilThickness = std::stod(label.substr(start + 1, pos_MeV - start - 1));
    } else continue;

    std::string energyLabel = label.substr(label.find("_") + 1);

    // Extract foil thickness
    double energy = 0.0;
    auto pos = label.find("MeV");
    if (pos != std::string::npos) {
        size_t start = label.find_last_of('_', pos); 
        energy = std::stod(label.substr(start + 1, pos - start - 1));
    } else continue;

    if (!t || t->GetEntries() == 0) {
        std::cout << "Warning: Chain " << label << " is empty!" << std::endl;
        continue;
    }

    h->SetDirectory(nullptr);

    float_t z, kineticE;
    Int_t pdg, parentID;

    // Branch optimization
    t->SetBranchStatus("*", 0);
    t->SetBranchStatus("HitZ", 1);
    t->SetBranchStatus("HitPDG", 1);
    t->SetBranchStatus("HitKineticEnergy", 1);
    t->SetBranchStatus("HitParentID", 1);
    t->SetBranchAddress("HitZ", &z);
    t->SetBranchAddress("HitPDG", &pdg);
    t->SetBranchAddress("HitKineticEnergy", &kineticE);
    t->SetBranchAddress("HitParentID", &parentID);

    // t->SetCacheSize(50 * 1024 * 1024);
    // t->AddBranchToCache("HitZ");
    // t->AddBranchToCache("HitPDG");
    // t->AddBranchToCache("HitKineticEnergy");

    Long64_t nentries = t->GetEntries();
    for (Long64_t j = 0; j < nentries; ++j) {
        t->GetEntry(j);
        if (pdg == 0 && parentID == 0 && kineticE ==0) //to obtain all stopped primary electrons. Need a key for PRIMARY
            h->Fill(z);
    }

    double integral = h->Integral(0, foilThickness/10);

    h->SetLineColor(colors[i % nColors]); //colours dont matter here
    h->SetLineWidth(Decoration.lineWidth);
    h->GetXaxis()->SetTitle(Decoration.xTitle);
    h->GetYaxis()->SetTitle(Decoration.yTitle);

    histos.push_back(h);
    legend->AddEntry(h, label.c_str(), "l");

    // //if energyLabel is 40MeV or 45MeV, divide integral by 2e6
    // if (energyLabel == "40MeV" || energyLabel == "45MeV") {
    //     integral /= 2.0;
    // }

    /*this part gives the percentage of primary electrons that stopped in the foil*/
    results.push_back({energyLabel, energy, foilThickness, 100* integral / 1e6}); //need to divide by 2e6 for 40 and 45MeV
}

    // //===============================
    // // Scatter plot of useful photons vs beam energy
    // //===============================

    TGraph *gPrimary = new TGraph();


    //this needs to be converted to double for the eneryg!!!!!
    for (const auto& r : results) {
        if (r.energyLabel == "20MeV_4mm" || r.energyLabel == "25MeV_6mm" || r.energyLabel == "30MeV_9mm" ||
            r.energyLabel == "35MeV_12mm" || r.energyLabel == "40MeV_11mm" || r.energyLabel == "45MeV_11mm" ||
            r.energyLabel == "50MeV_13mm") {
    gPrimary->SetPoint(gPrimary->GetN(), r.energy, r.stoppedPrimaries);
    }      

    }
    //change this to accessible colour scheme ROOT

    // // Style graphs
    gPrimary->SetMarkerColor(kP10Red);   gPrimary>SetMarkerStyle(21);



    double YMax = -1e9;
    for (auto g : {gPrimary}) {
        int n = g->GetN(); //number of points in each graph
        for (int i = 0; i < n; ++i) {
            double x, y;
            g->GetPoint(i, x, y);
            if (y > YMax) YMax = y;
        }
    }

    TMultiGraph *mg = new TMultiGraph();
    mg->Add(gPrimary, "P");

    mg->SetMaximum(YMax * 1.1);


    TCanvas *c3 = new TCanvas("c3", "Percentage Primary Electrons Stopped vs Beam Energy", 600, 500);
    // c3->SetRightMargin(0.15);
    c3->SetLeftMargin(0.15);


    mg->SetTitle("Percentage of Primary Electrons Stopped for each Beam Energy and Corresponding Foil Thickness");
    mg->GetXaxis()->SetTitle("Beam Energy (MeV)");
    mg->GetYaxis()->SetTitle("Percentage Primary Electrons Stopped");
    mg->GetXaxis()->SetTitleSize(0.05);
    mg->GetYaxis()->SetTitleSize(0.05);
    mg->GetXaxis()->SetLabelSize(0.04);
    mg->GetYaxis()->SetLabelSize(0.04);
    mg->Draw("AP");
    mg->GetXaxis()->SetLimits(0, 24);

    c3->cd();
    c3->Update();
    c3->SaveAs("PrimariesStopped.png");


}

int main() {
    compare();
    return 0;
}
