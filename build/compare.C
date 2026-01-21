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
// #include <omp.h>

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


    // #pragma omp parallel
    // {
    //     int tid = omp_get_thread_num();
    //     #pragma omp critical
    //     std::cout << "Hello from thread " << tid << std::endl;
    // }

    //===============================
    // Define TChains for each medium
    //===============================

    std::vector<std::tuple<std::string, TChain*, double>> chains;

    std::vector<std::string> mediums = {"SF6", "C3F8", "CF4", "UF6", "vacuum"};
    // std::vector<std::string> mediums = {"SF6", "C3F8"};
    std::vector<std::string> energies = {"20MeV", "25MeV", "30MeV", "35MeV", "40MeV", "45MeV", "50MeV"};

    //vector pairs fr energy and foil thicknesses:

    std::vector<std::pair<std::string, double>> energyFoilPairs = {
        {"20MeV", 3},
        {"25MeV", 3},
        {"30MeV", 6},
        {"35MeV", 10},
        {"40MeV", 10},
        {"45MeV", 12},
        {"50MeV", 12}
    };

    // std::vector<std::string> energies = {"30MeV"};

    // create one TChain per (medium, energy) and add matching files immediately
    for (const auto &m : mediums) {
        for (const auto &pair : energyFoilPairs) {
            std::string e = pair.first;
            double foilThickness = pair.second;
            std::string label = m + "_" + e; //does this mean it has to be in this order?
            TChain *ch = new TChain("IndividualHits");
            ch->Add(TString::Format("%s_*%s_*.root", m.c_str(), e.c_str()).Data());
            chains.push_back(std::make_tuple(label, ch, foilThickness));
        }
    }
    

    //===============================
    // Decorations and setup
    //===============================
    HistogramDecoration Decoration = {
       1, kBlack, "Depth (cm)", "#photons in range 15-22 MeV", nullptr, 0., 100., 0., 0.
    }; //do not use this range

    // HistogramDecoration photonDecoration = {
    //    1, kBlack, "Depth (cm)", "Photon Energy (MeV)", nullptr, 0., 100., 0., 0.
    // };

    int colors[] = {kRed, kSpring+5, kBlack, kMagenta+2, kViolet-2, kBlue-7,
                    kAzure-1, kCyan, kTeal+10, kGreen+3, kYellow, kGray,
                    kOrange-7, kPink+7};
    int nColors = sizeof(colors)/sizeof(int);

    TLegend *legend = new TLegend(0.7,0.7,0.9,0.9);
    TCanvas *c1 = new TCanvas("c1", "Compare Energy Deposition", 900, 700);
    c1->cd();

    std::vector<TH1D*> histos;
    // std::vector<TH2D*> h2_histos;
    std::vector<double> usefulPhotonIntegrals;
    // double globalMax = 0.0;

for (size_t i = 0; i < chains.size(); i++) {
    auto [label, t, foilThickness] = chains[i];

    double integral = 0.0; // Initialize to 0

    if (!t || t->GetEntries() == 0) {
        std::cout << "Warning: Chain " << label << " is empty!" << std::endl;
    } else {


        Float_t vacuumLength = 10.0; //cm
        Float_t steel = 0.05; //cm

    // TH1D *h = new TH1D(
    //     TString::Format("h_%zu", i),
    //     TString::Format("Depth Distribution of Electrons"),
    //     200, foilThickness/10 + vacuumLength+0.05, foilThickness/10 + vacuumLength + 0.05 + 20
    // ); //1mm bin size

    TH1D *h = new TH1D(
        TString::Format("h_%zu", i),
        TString::Format("Depth Distribution of Electrons"),
        2, 0, 2
    ); 

        // Disable global ROOT directory writing for safety
        h->SetDirectory(nullptr);

        Float_t z, kineticE;
        Int_t pdg, reactionCount;
        t->SetBranchAddress("HitZ", &z);
        t->SetBranchAddress("HitPDG", &pdg);
        t->SetBranchAddress("HitKineticEnergy", &kineticE);
        t->SetBranchAddress("ReactionCount", &reactionCount);

        Long64_t nentries = t->GetEntries();
        for (Long64_t j = 0; j < nentries; ++j) {
            t->GetEntry(j);
            if (z >= (foilThickness/10 + vacuumLength + steel) && z <= (foilThickness/10 + vacuumLength + steel + 20))
                h->Fill(reactionCount);

        }

        integral = h->Integral(); //since the x range is already adjusted

        h->SetLineColor(colors[i % nColors]);
        h->SetLineWidth(Decoration.lineWidth);
        h->GetXaxis()->SetTitle(Decoration.xTitle);
        h->GetYaxis()->SetTitle(Decoration.yTitle);

        histos.push_back(h);
        legend->AddEntry(h, label.c_str(), "l");
    }

    // usefulPhotonIntegrals.push_back(integral / 10e6); // Always push, even if 0
    usefulPhotonIntegrals.push_back(integral / 1e5); // now pushing back reaction count
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

    int idxSF6=0, idxC3F8=0, idxCF4=0, idxPF5=0, idxUF6=0, idxVacuum=0;

    for (size_t i=0; i<chains.size(); i++) {
        // std::string label = chains[i].first;
        auto [label, _, __] = chains[i]; 
        double beamEnergy = 0.0;


        std::regex energyRegex(R"((\d+(?:\.\d+)?)\s*MeV)");
        std::smatch match;
        if (std::regex_search(label, match, energyRegex)) {
            beamEnergy = std::stod(match[1].str());
        } else {
            continue;

        }

        double usefulPhotons = (i < usefulPhotonIntegrals.size()) ? usefulPhotonIntegrals[i] : 0; // what does this mean

        if (label.find("SF6") != std::string::npos)
            gSF6->SetPoint(idxSF6++, beamEnergy, usefulPhotons);
        else if (label.find("C3F8") != std::string::npos)
            gC3F8->SetPoint(idxC3F8++, beamEnergy, usefulPhotons);
        else if (label.find("CF4") != std::string::npos)
            gCF4->SetPoint(idxCF4++, beamEnergy, usefulPhotons);
        else if (label.find("PF5") != std::string::npos)
            gPF5->SetPoint(idxPF5++, beamEnergy, usefulPhotons);
        else if (label.find("UF6") != std::string::npos)
            gUF6->SetPoint(idxUF6++, beamEnergy, usefulPhotons);
        else if (label.find("vacuum") != std::string::npos)
            gVacuum->SetPoint(idxVacuum++, beamEnergy, usefulPhotons);
    }

    // Style graphs
    // gSF6->SetMarkerColor(kP10Red);   gSF6->SetMarkerStyle(21);
    // gC3F8->SetMarkerColor(kP10Cyan);   gC3F8->SetMarkerStyle(23);
    // gCF4->SetMarkerColor(kP10Ash);  gCF4->SetMarkerStyle(24);
    // gPF5->SetMarkerColor(kP10Green); ;gPF5->SetMarkerStyle(22);
    // gUF6->SetMarkerColor(kP10Orange); gUF6->SetMarkerStyle(25);
    // gVacuum->SetMarkerColor(kP10Brown) ;gVacuum->SetMarkerStyle(26);

    gSF6->SetMarkerColor(kRed);   gSF6->SetMarkerStyle(21);
    gC3F8->SetMarkerColor(kCyan);   gC3F8->SetMarkerStyle(23);
    gCF4->SetMarkerColor(kBlue);  gCF4->SetMarkerStyle(24);
    gPF5->SetMarkerColor(kGreen); gPF5->SetMarkerStyle(22);
    gUF6->SetMarkerColor(kOrange); gUF6->SetMarkerStyle(25);
    gVacuum->SetMarkerColor(kPink) ;gVacuum->SetMarkerStyle(26);

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
    scatterLegend->AddEntry(gSF6, "SF6", "p");
    scatterLegend->AddEntry(gC3F8, "C3F8", "p");
    scatterLegend->AddEntry(gCF4, "CF4", "p");
    scatterLegend->AddEntry(gPF5, "PF5", "p");
    scatterLegend->AddEntry(gUF6, "UF6", "p");
    scatterLegend->AddEntry(gVacuum, "Vacuum", "p");

    TCanvas *c3 = new TCanvas("c3", "#Useful Photons vs Beam Energy", 600, 500);
    mg->SetTitle("#19F(gamma, 18F)n reactions vs beam energy;Beam energy (MeV);#19F(gamma, 18F)n reactions");
    mg->Draw("AP");
    mg->GetXaxis()->SetLimits(19, 51);
    scatterLegend->Draw();
    c3->Update();
    c3->SaveAs("scatter.png");
}

int main() {
    compare();
    return 0;
}
