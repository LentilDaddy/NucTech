//root -l -b -q 'compare.C++()'
//compiled version in Makefile

//why does it take so long to run
//figure out how to create tchains only for exisiting files
//batch processing?
//ask for help with profiling in ROOT 
//in parallel on Viking instead

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


struct MediumResult {
    std::string medium;
    double foilThickness;
    double usefulPhotons;
};


void compare()
{
    gROOT->SetBatch(kTRUE);


    //===============================
    // Define TChains for each medium
    //===============================

    std::vector<std::pair<std::string, TChain*>> chains;

    // std::vector<std::string> mediums = {"SF6", "C3F8", "CF4", "PF5", "UF6", "Vacuum"};
    std::vector<std::string> mediums = {"SF6"};
    // std::vector<std::string> energies = {"20MeV", "22MeV", "25MeV", "28MeV", "30MeV", "35MeV", "40MeV", "45MeV", "50MeV"};
    // std::vector<std::string> energies = {"25MeV"};
    std::vector<std::string> foilThicknesses = {"8mm","9mm","10mm", "11mm", "12mm", "13mm", "14mm", "15mm", "16mm", "17mm", "18mm", "19mm", "20mm"};

    std::vector<MediumResult> results;

    // create one TChain per (medium, energy) and add matching files immediately
    for (const auto &m : mediums) {
        for (const auto &e : foilThicknesses) {
            std::string label = m + "_" + e; //does this mean it has to be in this order?
            TChain *ch = new TChain("IndividualHits");
            ch->Add(TString::Format("%s_*%s*.root", m.c_str(), e.c_str()).Data());
            chains.push_back({label, ch});
        }
    }
    

    //===============================
    // Decorations and setup
    //===============================
    HistogramDecoration Decoration = {
       1, kBlack, "Depth (cm)", "#photons in range 15-22 MeV", nullptr, 0., 100., 0., 0.
    };

    int colors[] = {kRed, kSpring+5, kBlack, kMagenta+2, kViolet-2, kBlue-7,
                    kAzure-1, kCyan, kTeal+10, kGreen+3, kYellow, kGray,
                    kOrange-7, kPink+7};
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
    auto pos = label.find("mm");
    if (pos != std::string::npos) {
        size_t start = label.find_last_of('_', pos);
        foilThickness = std::stod(label.substr(start + 1, pos - start - 1));
    } else continue;

    // Extract medium from label
    std::string medium = label.substr(0, label.find("_"));

    if (!t || t->GetEntries() == 0) {
        std::cout << "Warning: Chain " << label << " is empty!" << std::endl;
        continue;
    }

    TH1D *h = new TH1D(TString::Format("h_thread_%zu", i),
                       "Photon Depth", 100, Decoration.xMin, Decoration.xMax);
    h->SetDirectory(nullptr);

    Float_t z, kineticE;
    Int_t pdg;

    // Branch optimization
    t->SetBranchStatus("*", 0);
    t->SetBranchStatus("HitZ", 1);
    t->SetBranchStatus("HitPDG", 1);
    t->SetBranchStatus("HitKineticEnergy", 1);
    t->SetBranchAddress("HitZ", &z);
    t->SetBranchAddress("HitPDG", &pdg);
    t->SetBranchAddress("HitKineticEnergy", &kineticE);

    t->SetCacheSize(50 * 1024 * 1024);
    t->AddBranchToCache("HitZ");
    t->AddBranchToCache("HitPDG");
    t->AddBranchToCache("HitKineticEnergy");

    Long64_t nentries = t->GetEntries();
    for (Long64_t j = 0; j < nentries; ++j) {
        t->GetEntry(j);
        if (pdg == 1 && kineticE >= 15 && kineticE <= 22)
            h->Fill(z);
    }

    double integral = h->Integral(foilThickness/10, 20+foilThickness/10);

    h->SetLineColor(colors[i % nColors]);
    h->SetLineWidth(Decoration.lineWidth);
    h->GetXaxis()->SetTitle(Decoration.xTitle);
    h->GetYaxis()->SetTitle(Decoration.yTitle);

    histos.push_back(h);
    legend->AddEntry(h, label.c_str(), "l");

    results.push_back({medium, foilThickness, integral / 1e6});
}


    //===============================
    // Scatter plot of useful photons vs beam energy
    //===============================
    TGraph *gSF6 = new TGraph();
    TGraph *gC3F8 = new TGraph();
    TGraph *gCF4 = new TGraph();
    TGraph *gPF5 = new TGraph();
    TGraph *gUF6 = new TGraph();
    TGraph *gVacuum = new TGraph();

    for (const auto& r : results) {
        if (r.medium == "SF6")      gSF6->SetPoint(gSF6->GetN(), r.foilThickness, r.usefulPhotons);
        else if (r.medium == "C3F8") gC3F8->SetPoint(gC3F8->GetN(), r.foilThickness, r.usefulPhotons);
        else if (r.medium == "CF4")  gCF4->SetPoint(gCF4->GetN(), r.foilThickness, r.usefulPhotons);
        else if (r.medium == "PF5")  gPF5->SetPoint(gPF5->GetN(), r.foilThickness, r.usefulPhotons);
        else if (r.medium == "UF6")  gUF6->SetPoint(gUF6->GetN(), r.foilThickness, r.usefulPhotons);
        else if (r.medium == "Vacuum") gVacuum->SetPoint(gVacuum->GetN(), r.foilThickness, r.usefulPhotons);
    }

    // Style graphs
    gSF6->SetMarkerColor(kBlue);   gSF6->SetMarkerStyle(21);
    gC3F8->SetMarkerColor(kRed);   gC3F8->SetMarkerStyle(23);
    gCF4->SetMarkerColor(kBlack);  gCF4->SetMarkerStyle(24);
    gPF5->SetMarkerColor(kMagenta);gPF5->SetMarkerStyle(22);
    gUF6->SetMarkerColor(kOrange); gUF6->SetMarkerStyle(25);
    gVacuum->SetMarkerColor(kGreen);gVacuum->SetMarkerStyle(26);

    double YMax = -1e9;
    for (auto g : {gSF6, gC3F8, gCF4, gPF5, gUF6, gVacuum}) {
        int n = g->GetN(); //number of points in each graph
        for (int i = 0; i < n; ++i) {
            double x, y;
            g->GetPoint(i, x, y);
            if (y > YMax) YMax = y;
        }
    }

    //rest should be serial
    TMultiGraph *mg = new TMultiGraph();
    mg->Add(gSF6, "P");
    mg->Add(gC3F8, "P");
    mg->Add(gCF4, "P");
    mg->Add(gPF5, "P");
    mg->Add(gUF6, "P");
    mg->Add(gVacuum, "P");
    mg->SetMaximum(YMax * 1.1);

    TLegend *scatterLegend = new TLegend(0.1, 0.78, 0.3, 0.88);
    scatterLegend->AddEntry(gSF6, "SF6_1.339g/cm3", "p");
    scatterLegend->AddEntry(gC3F8, "C3F8_1.352g/cm3", "p");
    scatterLegend->AddEntry(gCF4, "CF4_1.603g/cm3", "p");
    scatterLegend->AddEntry(gPF5, "PF5", "p");
    scatterLegend->AddEntry(gUF6, "UF6_3.630g/cm3", "p");
    scatterLegend->AddEntry(gVacuum, "Vacuum", "p");

    TCanvas *c3 = new TCanvas("c3", "#Useful Photons vs Foil Thickness", 600, 500);
    mg->SetTitle("Total #Useful photons (15-22MeV) in Detector per Primary Electron with 25MeV Beam vs Foil Thickness;Foil Thickness (mm);#Useful photons per Primary Electron");
    mg->Draw("AP");
    mg->GetXaxis()->SetLimits(7, 21);
    scatterLegend->Draw();
    c3->Update();
    c3->SaveAs("Photons_FoilThickness_latest.png");
}

int main() {
    compare();
    return 0;
}
