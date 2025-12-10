
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
    // HistogramDecoration Decoration = {
    //    1, kBlack, "Depth (cm)", "#photons in range 15-22 MeV", nullptr, 0., 100., 0., 0.
    // };

    TLegend *legend = new TLegend(0.7,0.7,0.9,0.9);
    TCanvas *c1 = new TCanvas("c1", "Compare Energy Deposition", 900, 700);
    c1->cd();

    std::vector<TH1D*> histos;
    std::vector<double> StopppedPrimariesIntegrals;

for (size_t i = 0; i < chains.size(); i++) {
    TChain *t = chains[i].second;

    std::string label = chains[i].first;

    std::string energyLabel   = label.substr(0, label.find("_"));
    std::string foilLabel     = label.substr(label.find("_") + 1);

    auto posE = energyLabel.find("MeV");
    double energy = std::stod(energyLabel.substr(0, posE));
    auto posF = foilLabel.find("mm");
    double foilThickness = std::stod(foilLabel.substr(0, posF));


    if (!t || t->GetEntries() == 0) {
        std::cout << "Warning: Chain " << label << " is empty!" << std::endl;
        continue;
    }

    TH1D *h = new TH1D(
        TString::Format("h_%zu", i),
        TString::Format("Depth Distribution of Stopped Primary Electrons"),
        2000, 0, 20      // depth range in cm
    );

        // TH1D *h = new TH1D(TString::Format("h_%zu", i),

        //                "Photon Depth", 100, Decoration.xMin, Decoration.xMax);

    h->SetDirectory(nullptr);

    float_t z, kineticE, r;
    Int_t pdg, parentID;

    // Branch optimization
    t->SetBranchStatus("*", 0);
    t->SetBranchStatus("HitZ", 1);
    t->SetBranchStatus("HitPDG", 1);
    t->SetBranchStatus("HitKineticEnergy", 1);
    t->SetBranchStatus("HitParentID", 1);
    t->SetBranchAddress("HitZ", &z);
    t->SetBranchAddress("HitR", &r);
    t->SetBranchAddress("HitPDG", &pdg);
    t->SetBranchAddress("HitKineticEnergy", &kineticE);
    t->SetBranchAddress("HitParentID", &parentID);


    Long64_t nentries = t->GetEntries();
    for (Long64_t j = 0; j < nentries; ++j) {
        t->GetEntry(j);
        if (pdg == 0 && parentID==0 && kineticE ==0 || pdg == 0 && parentID==0 && r==9.0) //to obtain all stopped and deflected primary electrons. Probably not the best method
            h->Fill(z);
    }


    std::cout<<"foil thickness is "<<foilThickness<<" mm"<<std::endl;
    // double integral = h->Integral(0, foilThickness/10);
    double integral = h->Integral(foilThickness/10, 15+foilThickness/10); //changed to 15 cm + foil thickness to ignore initial build up region
    std::cout << "Integral for " << label << ": " << integral << std::endl;

    // h->SetLineColor(colors[i % nColors]); //colours dont matter here
    // h->SetLineWidth(Decoration.lineWidth);
    // h->GetXaxis()->SetTitle(Decoration.xTitle);
    // h->GetYaxis()->SetTitle(Decoration.yTitle);

    histos.push_back(h);
    legend->AddEntry(h, label.c_str(), "l");


    /*this part gives the percentage of primary electrons that stopped in the foil*/
    results.push_back({label, energy, foilThickness, 100 * integral / 1e6}); //temprary change to test

    // results.push_back({energyLabel, energy, foilThickness, 100* integral / 1e6}); //need to divide by 2e6 for 40 and 45MeV
}

    // //===============================
    // // Scatter plot of useful photons vs beam energy
    // //===============================

    TGraph *gPrimary = new TGraph();


    //this needs to be converted to double for the eneryg!!!!!
    for (const auto& r : results) {
        gPrimary->SetPoint(gPrimary->GetN(), 1, r.stoppedPrimaries); //we will need stoppedPrimaries1, 2, 3 etc
        gPrimary->SetPoint(gPrimary->GetN(), 2, r.stoppedPrimaries);
        gPrimary->SetPoint(gPrimary->GetN(), 3, r.stoppedPrimaries);
        gPrimary->SetPoint(gPrimary->GetN(), 4, r.stoppedPrimaries);
        gPrimary->SetPoint(gPrimary->GetN(), 5, r.stoppedPrimaries);
        gPrimary->SetPoint(gPrimary->GetN(), 10, r.stoppedPrimaries);
        gPrimary->SetPoint(gPrimary->GetN(), 15, r.stoppedPrimaries);
    }  
    
        //change this to accessible colour scheme ROOT

    // // Style graphs
    gPrimary->SetMarkerColor(kRed);
    gPrimary->SetMarkerStyle(21);

    //fix gPrimary y axis range to 100

    gPrimary->SetMaximum(100);

    double YMax = 100;

    TMultiGraph *mg = new TMultiGraph();
    mg->Add(gPrimary, "P");

    //Fix y axis range to 100

    mg->SetMinimum(0);
    mg->SetMaximum(YMax);
    // mg->SetMaximum(YMax);


    TCanvas *c3 = new TCanvas("c3", "Percentage Primary Electrons Stopped vs Beam Energy", 600, 500);
    // c3->SetRightMargin(0.15);
    c3->SetLeftMargin(0.15);


    mg->SetTitle("% Primary Electrons Deflected/stopped inside B field region against length of region");
    mg->GetXaxis()->SetTitle("Length (cm)");
    mg->GetYaxis()->SetTitle("% Primary Electrons Deflected/stopped");
    mg->GetXaxis()->SetTitleSize(0.05);
    mg->GetYaxis()->SetTitleSize(0.05);
    mg->GetXaxis()->SetLabelSize(0.04);
    mg->GetYaxis()->SetLabelSize(0.04);

    mg->Draw("AP");
    mg->GetXaxis()->SetLimits(0, 20); //this was too low
    mg->GetYaxis()->SetRangeUser(0, 100);   // << FIXED

    c3->cd();
    c3->Update();
    c3->SaveAs("PrimariesDeflected.png");



    }



int main() {
    compare();
    return 0;
}
