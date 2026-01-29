
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


struct EnergyResult {
    std::string energyLabel;
    double energy;
    double foilThickness;
    double NumReactions;
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

    std::vector<std::string> energies = {"20MeV", "25MeV", "30MeV", "35MeV", "40MeV", "45MeV", "50MeV"};
    // std::vector<std::string> energies = {"30MeV"};
    // std::vector<std::string> energies = {"25MeV", "50MeV"};
    // std::vector<std::string> energies = {"40MeV"};

    std::vector<std::string> foilThicknesses = {"1mm","2mm","3mm","4mm","5mm","6mm","7mm","8mm","9mm","10mm", "11mm", "12mm", "13mm", "14mm", "15mm", "16mm", "17mm", "18mm", "19mm", "20mm", "21mm", "22mm"};
    // std::vector<std::string> foilThicknesses = {"10mm"}; //test

    std::vector<EnergyResult> results;

for (const auto &m : energies) {
    for (const auto &e : foilThicknesses) {
        std::string label = e + "_" + m;
        
        // Main Chain
        TChain *ch = new TChain("IndividualHits");
        ch->Add(TString::Format("*_%s*_*%s*.root", e.c_str(), m.c_str()).Data());

        // Friend Chain
        TChain *chEnergySpectrum = new TChain("EnergySpectrum");
        chEnergySpectrum->Add(TString::Format("*_%s*_*%s*.root", e.c_str(), m.c_str()).Data());

        // Link them
        ch->AddFriend(chEnergySpectrum);

        chains.push_back({label, ch});
    }
}


    TLegend *legend = new TLegend(0.7,0.7,0.9,0.9);
    TCanvas *c1 = new TCanvas("c1", "Compare Energy Deposition", 900, 700);
    c1->cd();

    std::vector<TH1D*> histos;
    std::vector<double> usefulPhotonIntegrals;

for (size_t i = 0; i < chains.size(); i++) {
    TChain *t = chains[i].second;
    std::string label = chains[i].first;

    double integral = 0.0; // Initialize to 0

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

    Float_t vacuumLength = 10.0; //cm
    Float_t steel = 0.05; //cm

    TH1D *h = new TH1D(
        TString::Format("h_%zu", i),
        TString::Format("Depth Distribution of Electrons"),
        1, 1, 2
    ); 
    h->SetDirectory(nullptr);

    // float_t z, kineticE;
    float_t z;
    Int_t reactionCount;

    // Branch optimization
    t->SetBranchStatus("*", 0);
    t->SetBranchStatus("HitZ", 1);
    t->SetBranchStatus("ReactionCount", 1);
    // t->SetBranchStatus("HitPDG", 1);
    // t->SetBranchStatus("HitKineticEnergy", 1);
    t->SetBranchAddress("HitZ", &z);
    t->SetBranchAddress("ReactionCount", &reactionCount);
    // t->SetBranchAddress("HitPDG", &pdg);
    // t->SetBranchAddress("HitKineticEnergy", &kineticE);

    Long64_t nentries = t->GetEntries();
    for (Long64_t j = 0; j < nentries; ++j) {
        t->GetEntry(j);
        if (z >= (foilThickness/10 + vacuumLength + steel) && z <= (foilThickness/10 + vacuumLength + steel + 200)) 
            h->Fill(reactionCount);

    }

    integral = h->Integral(); //since the x range is already adjusted

    histos.push_back(h);
    legend->AddEntry(h, label.c_str(), "l");


    results.push_back({energyLabel, energy, foilThickness, integral / 1e6}); //need to divide by 2e6 for 40 and 45MeV
}

    //===============================
    // Scatter plot of useful photons vs beam energy
    //===============================
    TGraph *g20MeV = new TGraph();
    TGraph *g25MeV = new TGraph();
    TGraph *g30MeV = new TGraph();
    TGraph *g35MeV = new TGraph();
    TGraph *g40MeV = new TGraph();
    TGraph *g45MeV = new TGraph();
    TGraph *g50MeV = new TGraph();


    //this needs to be converted to double for the eneryg!!!!!
    for (const auto& r : results) {
        if (r.energyLabel == "20MeV")      g20MeV->SetPoint(g20MeV->GetN(), r.foilThickness, r.NumReactions);
        else if (r.energyLabel == "25MeV") g25MeV->SetPoint(g25MeV->GetN(), r.foilThickness, r.NumReactions);
        else if (r.energyLabel == "30MeV") g30MeV->SetPoint(g30MeV->GetN(), r.foilThickness, r.NumReactions);
        else if (r.energyLabel == "35MeV") g35MeV->SetPoint(g35MeV->GetN(), r.foilThickness, r.NumReactions);
        else if (r.energyLabel == "40MeV") g40MeV->SetPoint(g40MeV->GetN(), r.foilThickness, r.NumReactions);
        else if (r.energyLabel == "45MeV") g45MeV->SetPoint(g45MeV->GetN(), r.foilThickness, r.NumReactions);
        else if (r.energyLabel == "50MeV") g50MeV->SetPoint(g50MeV->GetN(), r.foilThickness, r.NumReactions);
    }
    //change this to accessible colour scheme ROOT



    // int colors[] = {kP10Red, kP10Cyan, kP10Ash, kP10Green, kP10Orange, kP10Brown, kP10Gray};
    int colors[] = {kRed, kCyan, kMagenta, kGreen+2, kOrange+7, kBlue, kGray+2};
    int nColors = sizeof(colors)/sizeof(int);

    // Style graphs
    g20MeV->SetMarkerColor(colors[0]);   g20MeV->SetMarkerStyle(21);
    g25MeV->SetMarkerColor(colors[1]);  g25MeV->SetMarkerStyle(23);
    g30MeV->SetMarkerColor(colors[2]); g30MeV->SetMarkerStyle(25);
    g35MeV->SetMarkerColor(colors[3]);    g35MeV->SetMarkerStyle(26);
    g40MeV->SetMarkerColor(colors[4]); g40MeV->SetMarkerStyle(27);
    g45MeV->SetMarkerColor(colors[5]);  g45MeV->SetMarkerStyle(28);
    g50MeV->SetMarkerColor(colors[6]);   g50MeV->SetMarkerStyle(29);


    double YMax = -1e9;
    for (auto g : {g20MeV, g25MeV, g30MeV, g35MeV, g40MeV, g45MeV, g50MeV}) {
        int n = g->GetN(); //number of points in each graph
        for (int i = 0; i < n; ++i) {
            double x, y;
            g->GetPoint(i, x, y);
            if (y > YMax) YMax = y;
        }
    }

    TMultiGraph *mg = new TMultiGraph();
    mg->Add(g20MeV, "P");
    mg->Add(g25MeV, "P");
    mg->Add(g30MeV, "P");
    mg->Add(g35MeV, "P");
    mg->Add(g40MeV, "P");
    mg->Add(g45MeV, "P");
    mg->Add(g50MeV, "P");

    mg->SetMaximum(YMax * 1.1);

    TLegend *scatterLegend = new TLegend(0.75, 0.75, 0.95, 0.95);
    scatterLegend->AddEntry(g20MeV, "20MeV", "p");
    scatterLegend->AddEntry(g25MeV, "25MeV", "p");
    scatterLegend->AddEntry(g30MeV, "30MeV", "p");
    scatterLegend->AddEntry(g35MeV, "35MeV", "p");
    scatterLegend->AddEntry(g40MeV, "40MeV", "p");
    scatterLegend->AddEntry(g45MeV, "45MeV", "p");
    scatterLegend->AddEntry(g50MeV, "50MeV", "p");

    TCanvas *c3 = new TCanvas("c3", "#Useful Photons vs Foil Thickness", 600, 500);
    // c3->SetRightMargin(0.15);
    c3->SetLeftMargin(0.15);


    mg->SetTitle("Total Reactions in Detector per Primary Electron vs Foil Thickness");
    mg->GetXaxis()->SetTitle("Foil Thickness (mm)");
    mg->GetYaxis()->SetTitle("#Reactions per Primary Electron");
    mg->GetXaxis()->SetTitleSize(0.05);
    mg->GetYaxis()->SetTitleSize(0.05);
    mg->GetXaxis()->SetLabelSize(0.04);
    mg->GetYaxis()->SetLabelSize(0.04);
    mg->Draw("AP");
    mg->GetXaxis()->SetLimits(0, 24);


    // c3->Update();
    c3->cd();
    scatterLegend->Draw();
    c3->Update();
    c3->SaveAs("Reactions_FoilThickness_latest.png");



    /*Cost Graph different canvas AND save as different file*/



    TMultiGraph *mg_Price = new TMultiGraph();

    TGraph *g20MeV_Price = new TGraph();
    TGraph *g25MeV_Price = new TGraph();
    TGraph *g30MeV_Price = new TGraph();
    TGraph *g35MeV_Price = new TGraph();
    TGraph *g40MeV_Price = new TGraph();
    TGraph *g45MeV_Price = new TGraph();
    TGraph *g50MeV_Price = new TGraph();



    for (const auto& r : results) {
        double denom = (r.NumReactions > 0.0) ? r.NumReactions : 1e-12;
        double cost = (r.energy * r.energy) / denom;
        if (r.energyLabel == "20MeV")      g20MeV_Price->SetPoint(g20MeV_Price->GetN(), r.foilThickness, cost);
        else if (r.energyLabel == "25MeV") g25MeV_Price->SetPoint(g25MeV_Price->GetN(), r.foilThickness, cost);
        else if (r.energyLabel == "30MeV") g30MeV_Price->SetPoint(g30MeV_Price->GetN(), r.foilThickness, cost);
        else if (r.energyLabel == "35MeV") g35MeV_Price->SetPoint(g35MeV_Price->GetN(), r.foilThickness, cost);
        else if (r.energyLabel == "40MeV") g40MeV_Price->SetPoint(g40MeV_Price->GetN(), r.foilThickness, cost);
        else if (r.energyLabel == "45MeV") g45MeV_Price->SetPoint(g45MeV_Price->GetN(), r.foilThickness, cost);
        else if (r.energyLabel == "50MeV") g50MeV_Price->SetPoint(g50MeV_Price->GetN(), r.foilThickness, cost);
    }


    double YRightMax = -1e9;
    double YRightMin = 1e9;
    for (auto g : {g20MeV_Price, g25MeV_Price, g30MeV_Price, g35MeV_Price, g40MeV_Price, g45MeV_Price, g50MeV_Price}) {
        int n = g->GetN(); //number of points in each graph
        for (int i = 0; i < n; ++i) {
            double x, y;
            g->GetPoint(i, x, y);
            if (y > YRightMax) YRightMax = y;
            if (y < YRightMin) YRightMin = y;
        }
    }


    // Style graphs
    g20MeV_Price->SetMarkerColor(colors[0]);   g20MeV_Price->SetMarkerStyle(21);
    g25MeV_Price->SetMarkerColor(colors[1]);  g25MeV_Price->SetMarkerStyle(23);
    g30MeV_Price->SetMarkerColor(colors[2]); g30MeV_Price->SetMarkerStyle(25);
    g35MeV_Price->SetMarkerColor(colors[3]);    g35MeV_Price->SetMarkerStyle(26);
    g40MeV_Price->SetMarkerColor(colors[4]);g40MeV_Price->SetMarkerStyle(27);
    g45MeV_Price->SetMarkerColor(colors[5]);  g45MeV_Price->SetMarkerStyle(28);
    g50MeV_Price->SetMarkerColor(colors[6]);   g50MeV_Price->SetMarkerStyle(29);

    mg_Price->Add(g20MeV_Price, "P");
    mg_Price->Add(g25MeV_Price, "P");
    mg_Price->Add(g30MeV_Price, "P");
    mg_Price->Add(g35MeV_Price, "P");
    mg_Price->Add(g40MeV_Price, "P");
    mg_Price->Add(g45MeV_Price, "P");
    mg_Price->Add(g50MeV_Price, "P");

    mg_Price->SetMaximum(YRightMax * 1.1);


    // TLegend *scatterLegend_Price = new TLegend(0.15, 0.7, 0.35, 0.9);
    //price legend shoudl be n top right
    TLegend *scatterLegend_Price = new TLegend(0.05, 0.75, 0.25, 0.95);
    scatterLegend_Price->AddEntry(g20MeV_Price, "20MeV", "p");
    scatterLegend_Price->AddEntry(g25MeV_Price, "25MeV", "p");
    scatterLegend_Price->AddEntry(g30MeV_Price, "30MeV", "p");
    scatterLegend_Price->AddEntry(g35MeV_Price, "35MeV", "p");
    scatterLegend_Price->AddEntry(g40MeV_Price, "40MeV", "p");
    scatterLegend_Price->AddEntry(g45MeV_Price, "45MeV", "p");
    scatterLegend_Price->AddEntry(g50MeV_Price, "50MeV", "p");

    TCanvas *c4 = new TCanvas("c4", "Relative Cost per Reaction", 600, 500);
    c4->SetLeftMargin(0.15);


    mg_Price->SetTitle("Relative Cost per Reaction vs Foil Thickness");
    mg_Price->GetXaxis()->SetTitle("Foil Thickness (mm)");
    mg_Price->GetYaxis()->SetTitle("Cost per Reaction");
    mg_Price->GetXaxis()->SetTitleSize(0.05);
    mg_Price->GetYaxis()->SetTitleSize(0.05);
    mg_Price->GetXaxis()->SetLabelSize(0.04);
    mg_Price->GetYaxis()->SetLabelSize(0.04);
    mg_Price->Draw("AP");
    mg_Price->GetXaxis()->SetLimits(0, 24);


    // c3->Update();
    c4->cd();
    scatterLegend_Price->Draw();
    c4->Update();
    c4->SaveAs("CostPerReaction.png");

}

int main() {
    compare();
    return 0;
}
