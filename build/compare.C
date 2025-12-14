
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


// struct EnergyResult {
//     std::string energyLabel;
//     double energy;
//     double foilThickness;
//     double stoppedPrimaries;
// };

struct EnergyResult {
    std::string BfieldLabel;
    double vacuumLength;
    double Bfield;
    double electrons;
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

    std::vector<EnergyResult> results;

    std::vector<std::string> vacuumLengths = {"1cm", "2cm", "3cm", "4cm", "5cm", "10cm", "15cm"};

    std::vector<std::string> BFields = {"0.0T", "0.1T", "1.0T"};;

    for (const auto &m : vacuumLengths) {
        for (const auto &e : BFields) {
            std::string label = m + "_" + e; //does this mean it has to be in this order?
            TChain *ch = new TChain("IndividualHits");
            ch->Add(TString::Format("*_%sVacuum*%s*.root", m.c_str(), e.c_str()).Data());
            chains.push_back({label, ch});
        }
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

    // Parse label: vacuumLength_Bfield (e.g., "15cm_0.1T")
    size_t lastUnderscore = label.rfind("_");
    std::string vacuumLengthStr = label.substr(0, lastUnderscore);
    std::string BfieldStr = label.substr(lastUnderscore + 1);

    // Extract numeric values
    auto posCm = vacuumLengthStr.find("cm");
    double vacuumLength = std::stod(vacuumLengthStr.substr(0, posCm));
    
    auto posT = BfieldStr.find("T");
    double Bfield = std::stod(BfieldStr.substr(0, posT));

    if (!t || t->GetEntries() == 0) {
        std::cout << "Warning: Chain " << label << " is empty!" << std::endl;
        continue;
    }

    TH1D *h = new TH1D(
        TString::Format("h_%zu", i),
        TString::Format("Depth Distribution of Electrons"),
        2000, 0, 20
    );
    h->SetDirectory(nullptr);

    float_t z, kineticE, r;
    Int_t pdg, parentID;

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
        if (pdg == 0) {
            h->Fill(z);
        }
    }

    double foilThickness = 40.0;
    double integral = h->Integral(foilThickness/10 + vacuumLength, foilThickness/10 + vacuumLength + 0.05);
    
    std::cout << "Integral for " << label << ": " << integral << std::endl;

    histos.push_back(h);
    legend->AddEntry(h, label.c_str(), "l");

    results.push_back({BfieldStr, vacuumLength, Bfield, integral});
}

    // //===============================
    // // Scatter plot of useful photons vs beam energy
    // //===============================

    TGraph *gScatter_0T = new TGraph();
    TGraph *gScatter_01T = new TGraph();
    TGraph *gScatter_1T = new TGraph();


    for (const auto& r : results) {
        if (r.BfieldLabel == "0.0T") {
            gScatter_0T->SetPoint(gScatter_0T->GetN(), r.vacuumLength, r.electrons);
        } else if (r.BfieldLabel == "0.1T") {
            gScatter_01T->SetPoint(gScatter_01T->GetN(), r.vacuumLength, r.electrons);
        } else if (r.BfieldLabel == "1.0T") {
            gScatter_1T->SetPoint(gScatter_1T->GetN(), r.vacuumLength, r.electrons);
        }
    }  


    // // Style graphs
    gScatter_0T->SetMarkerColor(kRed);
    gScatter_0T->SetMarkerStyle(21);

    gScatter_01T->SetMarkerColor(kBlue);
    gScatter_01T->SetMarkerStyle(23);

    gScatter_1T->SetMarkerColor(kGreen+2);
    gScatter_1T->SetMarkerStyle(22);

    // Find maximum y value and add 10% padding
    double maxY_0T = (gScatter_0T->GetN() > 0) ? gScatter_0T->GetHistogram()->GetMaximum() : 0;
    double maxY_01T = (gScatter_01T->GetN() > 0) ? gScatter_01T->GetHistogram()->GetMaximum() : 0;
    double maxY_1T = (gScatter_1T->GetN() > 0) ? gScatter_1T->GetHistogram()->GetMaximum() : 0;
    double YMax = std::max(maxY_0T, maxY_01T, maxY_1T) * 1.1;  // Add 10% padding

    TMultiGraph *mg = new TMultiGraph();
    mg->Add(gScatter_0T, "P");
    mg->Add(gScatter_01T, "P");
    mg->Add(gScatter_1T, "P");

    //Fix y axis range to 100

    mg->SetMinimum(0);
    mg->SetMaximum(YMax);
 

    TCanvas *c3 = new TCanvas("c3", "#Electrons entering Stainless Steel Layer vs Bfield region Length", 600, 500);
    // c3->SetRightMargin(0.15);
    c3->SetLeftMargin(0.15);


    mg->SetTitle("#Electrons entering Stainless Steel Layer against length of B field region");
    mg->GetXaxis()->SetTitle("Length (cm)");
    mg->GetYaxis()->SetTitle("#Electrons entering Stainless Steel Layer");
    mg->GetXaxis()->SetTitleSize(0.05);
    mg->GetYaxis()->SetTitleSize(0.05);
    mg->GetXaxis()->SetLabelSize(0.04);
    mg->GetYaxis()->SetLabelSize(0.04);

    mg->Draw("AP");
    mg->GetXaxis()->SetLimits(0, 20); //this was too low
    mg->GetYaxis()->SetRangeUser(0, YMax);   // << FIXED
    TLegend *lgb = new TLegend(0.65, 0.75, 0.9, 0.9);
    lgb->SetBorderSize(0);
    lgb->SetFillStyle(0);
    lgb->AddEntry(gScatter_0T, "B = 0.0 T", "P");
    lgb->AddEntry(gScatter_01T, "B = 0.1 T", "P");
    lgb->Draw();

    c3->cd();
    c3->Update();
    c3->SaveAs("ElectronsSteel.png");


}




int main() {
    compare();
    return 0;
}
