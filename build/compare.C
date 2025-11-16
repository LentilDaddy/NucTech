
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


struct EnergyResult {
    std::string energy;
    double foilThickness;
    double usefulPhotons;
};


void compare()
{
    gROOT->SetBatch(kTRUE);


    std::vector<std::pair<std::string, TChain*>> chains;

    // std::vector<std::string> energies = {"20MeV", "22MeV", "25MeV", "28MeV", "30MeV", "35MeV", "40MeV", "45MeV", "50MeV"};
    std::vector<std::string> energies = {"25MeV", "50MeV"};

    std::vector<std::string> foilThicknesses = {"1mm","2mm","3mm","4mm","5mm","6mm","7mm","8mm","9mm","10mm", "11mm", "12mm", "13mm", "14mm", "15mm", "16mm", "17mm", "18mm", "19mm", "20mm"};

    std::vector<EnergyResult> results;

    for (const auto &m : energies) {
        for (const auto &e : foilThicknesses) {
            std::string label = e + "_" + m; //does this mean it has to be in this order?
            TChain *ch = new TChain("IndividualHits");
            ch->Add(TString::Format("*_%s*_*%s*.root", e.c_str(), m.c_str()).Data());
            chains.push_back({label, ch});
            // std::cout << "Added chain for " << label << std::endl;
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
        size_t start = label.find_last_of('_', pos); //find underscore before "mm"
        foilThickness = std::stod(label.substr(start + 1, pos - start - 1));
    } else continue;

    std::string energy = label.substr(label.find("_") + 1);

    if (!t || t->GetEntries() == 0) {
        std::cout << "Warning: Chain " << label << " is empty!" << std::endl;
        continue;
    }

    TH1D *h = new TH1D(TString::Format("h_thread_%zu", i),
                       "Photon Depth", 100, Decoration.xMin, Decoration.xMax);
    h->SetDirectory(nullptr);

    float_t z, kineticE;
    Int_t pdg;

    // Branch optimization
    t->SetBranchStatus("*", 0);
    t->SetBranchStatus("HitZ", 1);
    t->SetBranchStatus("HitPDG", 1);
    t->SetBranchStatus("HitKineticEnergy", 1);
    t->SetBranchAddress("HitZ", &z);
    t->SetBranchAddress("HitPDG", &pdg);
    t->SetBranchAddress("HitKineticEnergy", &kineticE);

    // t->SetCacheSize(50 * 1024 * 1024);
    // t->AddBranchToCache("HitZ");
    // t->AddBranchToCache("HitPDG");
    // t->AddBranchToCache("HitKineticEnergy");

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

    results.push_back({energy, foilThickness, integral / 1e6});
}


    //===============================
    // Scatter plot of useful photons vs beam energy
    //===============================
    TGraph *g20MeV = new TGraph();
    TGraph *g22MeV = new TGraph();
    TGraph *g25MeV = new TGraph();
    TGraph *g28MeV = new TGraph();
    TGraph *g30MeV = new TGraph();
    TGraph *g35MeV = new TGraph();
    TGraph *g40MeV = new TGraph();
    TGraph *g45MeV = new TGraph();
    TGraph *g50MeV = new TGraph();

    for (const auto& r : results) {
        if (r.energy == "20MeV")      g20MeV->SetPoint(g20MeV->GetN(), r.foilThickness, r.usefulPhotons);
        else if (r.energy == "22MeV") g22MeV->SetPoint(g22MeV->GetN(), r.foilThickness, r.usefulPhotons);
        else if (r.energy == "25MeV") g25MeV->SetPoint(g25MeV->GetN(), r.foilThickness, r.usefulPhotons);
        else if (r.energy == "28MeV") g28MeV->SetPoint(g28MeV->GetN(), r.foilThickness, r.usefulPhotons);
        else if (r.energy == "30MeV") g30MeV->SetPoint(g30MeV->GetN(), r.foilThickness, r.usefulPhotons);
        else if (r.energy == "35MeV") g35MeV->SetPoint(g35MeV->GetN(), r.foilThickness, r.usefulPhotons);
        else if (r.energy == "40MeV") g40MeV->SetPoint(g40MeV->GetN(), r.foilThickness, r.usefulPhotons);
        else if (r.energy == "45MeV") g45MeV->SetPoint(g45MeV->GetN(), r.foilThickness, r.usefulPhotons);
        else if (r.energy == "50MeV") g50MeV->SetPoint(g50MeV->GetN(), r.foilThickness, r.usefulPhotons);
    }
    //need to change this to have differenet energies now instead of different media.

    // Style graphs
    g20MeV->SetMarkerColor(kBlue);   g20MeV->SetMarkerStyle(21);
    g22MeV->SetMarkerColor(kCyan);   g22MeV->SetMarkerStyle(22);
    g25MeV->SetMarkerColor(kGreen);  g25MeV->SetMarkerStyle(23);
    g28MeV->SetMarkerColor(kYellow); g28MeV->SetMarkerStyle(24);
    g30MeV->SetMarkerColor(kOrange); g30MeV->SetMarkerStyle(25);
    g35MeV->SetMarkerColor(kRed);    g35MeV->SetMarkerStyle(26);
    g40MeV->SetMarkerColor(kMagenta);g40MeV->SetMarkerStyle(27);
    g45MeV->SetMarkerColor(kBlack);  g45MeV->SetMarkerStyle(28);
    g50MeV->SetMarkerColor(kGray);   g50MeV->SetMarkerStyle(29);

    double YMax = -1e9;
    for (auto g : {g20MeV, g22MeV, g25MeV, g28MeV, g30MeV, g35MeV, g40MeV, g45MeV, g50MeV}) {
        int n = g->GetN(); //number of points in each graph
        for (int i = 0; i < n; ++i) {
            double x, y;
            g->GetPoint(i, x, y);
            if (y > YMax) YMax = y;
        }
    }

    //rest should be serial
    TMultiGraph *mg = new TMultiGraph();
    mg->Add(g20MeV, "P");
    mg->Add(g22MeV, "P");
    mg->Add(g25MeV, "P");
    mg->Add(g28MeV, "P");
    mg->Add(g30MeV, "P");
    mg->Add(g35MeV, "P");
    mg->Add(g40MeV, "P");
    mg->Add(g45MeV, "P");
    mg->Add(g50MeV, "P");

    mg->SetMaximum(YMax * 1.1);


    //legend on the top right:
    TLegend *scatterLegend = new TLegend(0.7, 0.7, 0.9, 0.9);
    scatterLegend->AddEntry(g20MeV, "20MeV", "p");
    scatterLegend->AddEntry(g22MeV, "22MeV", "p");
    scatterLegend->AddEntry(g25MeV, "25MeV", "p");
    scatterLegend->AddEntry(g28MeV, "28MeV", "p");
    scatterLegend->AddEntry(g30MeV, "30MeV", "p");
    scatterLegend->AddEntry(g35MeV, "35MeV", "p");
    scatterLegend->AddEntry(g40MeV, "40MeV", "p");
    scatterLegend->AddEntry(g45MeV, "45MeV", "p");
    scatterLegend->AddEntry(g50MeV, "50MeV", "p");

    TCanvas *c3 = new TCanvas("c3", "#Useful Photons vs Foil Thickness", 600, 500);
    mg->SetTitle("Total #Useful photons (15-22MeV) in Detector per Primary Electron vs Foil Thickness;Foil Thickness (mm);#Useful photons per Primary Electron");
    mg->Draw("AP");
    mg->GetXaxis()->SetLimits(0, 21);
    scatterLegend->Draw();
    c3->Update();
    c3->SaveAs("Photons_FoilThickness_latest.png");
}

int main() {
    compare();
    return 0;
}
