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

    std::vector<std::pair<std::string, TChain*>> chains;

    // std::vector<std::string> mediums = {"SF6", "C3F8", "CF4", "PF5", "UF6", "vacuum"};
    std::vector<std::string> mediums = {"SF6","C3F8","CF4","UF6"};
    // std::vector<std::string> energies = {"20MeV", "22MeV", "25MeV", "28MeV", "30MeV", "35MeV", "40MeV", "45MeV", "50MeV"};
    std::vector<std::string> energies = {"25MeV"};

    double foilThickness = 10; //10mm for example
    

    // create one TChain per (medium, energy) and add matching files immediately
    for (const auto &m : mediums) {
        for (const auto &e : energies) {
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
       1, kBlack, "Radius (cm)", "#photons in range 15-22 MeV", nullptr, 0., 110., 0., 0.
    }; // lineWidth, lineColor, xTitle, yTitle, title, xMin, xMax, yMin, yMax

    HistogramDecoration photonDecoration = {
       1, kBlack, "Radius (cm)", "Photon Energy (MeV)", nullptr, 0., 110., 0., 0.
    };

    int colors[] = {kRed, kSpring+5, kBlack, kMagenta+2, kViolet-2, kBlue-7,
                    kAzure-1, kCyan, kTeal+10, kGreen+3, kYellow, kGray,
                    kOrange-7, kPink+7};
    int nColors = sizeof(colors)/sizeof(int);

    TLegend *legend = new TLegend(0.7,0.7,0.9,0.9);
    TCanvas *c1 = new TCanvas("c1", "Compare Energy Deposition", 900, 700);
    c1->cd();

    std::vector<TH1D*> histos;
    std::vector<TH2D*> h2_histos;
    std::vector<double> usefulPhotonIntegrals100;
    std::vector<double> usefulPhotonIntegrals10;
    std::vector<double> usefulPhotonIntegrals20;
    std::vector<double> usefulPhotonIntegrals30;
    std::vector<double> usefulPhotonIntegrals40;
    std::vector<double> usefulPhotonIntegrals50;
    std::vector<double> usefulPhotonIntegrals60;
    std::vector<double> usefulPhotonIntegrals70;
    std::vector<double> usefulPhotonIntegrals80;
    std::vector<double> usefulPhotonIntegrals90;
    
    // double globalMax = 0.0;

for (size_t i = 0; i < chains.size(); i++) {
    TChain *t = chains[i].second;
    std::string label = chains[i].first;

    if (!t || t->GetEntries() == 0) {
        std::cout << "Warning: Chain " << label << " is empty!" << std::endl;
        continue;
    }

    TH1D *h = new TH1D(TString::Format("h_thread_%zu", i),
                       "Photon Depth", 1100, Decoration.xMin, Decoration.xMax);
                       //
    TH2D *h2 = new TH2D(TString::Format("h2_thread_%zu", i),
                        "Photon Energy vs Depth", 1000, photonDecoration.xMin, photonDecoration.xMax, 1000, 0, 50);

    // Disable global ROOT directory writing for safety
    h->SetDirectory(nullptr);
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
        if (pdg == 1 && kineticE >= 15 && kineticE <= 22 && z > foilThickness)
            h->Fill(r);
        if (pdg == 1 && kineticE >= 0)
            h2->Fill(r, kineticE);
    }

    double integral100 = h->Integral(0, 10); //repeat over different lengths NOT includiing foil part
    double integral10 = h->Integral(0, 1); //repeat over different lengths
    double integral20 = h->Integral(0, 2); //repeat over different lengths
    double integral30 = h->Integral(0, 3); //repeat over different lengths
    double integral40 = h->Integral(0, 4); //repeat over different lengths
    double integral50 = h->Integral(0, 5); //repeat over different lengths
    double integral60 = h->Integral(0, 6); //repeat over different lengths
    double integral70 = h->Integral(0, 7); //repeat over different lengths
    double integral80 = h->Integral(0, 8); //repeat over different lengths
    double integral90 = h->Integral(0, 9); //repeat over different lengths

    {
        h->SetLineColor(colors[i % nColors]);
        h->SetLineWidth(Decoration.lineWidth);
        h->GetXaxis()->SetTitle(Decoration.xTitle);
        h->GetYaxis()->SetTitle(Decoration.yTitle);

        histos.push_back(h);
        h2_histos.push_back(h2);
        usefulPhotonIntegrals100.push_back(integral100);
        usefulPhotonIntegrals10.push_back(integral10);
        usefulPhotonIntegrals20.push_back(integral20);
        usefulPhotonIntegrals30.push_back(integral30);
        usefulPhotonIntegrals40.push_back(integral40);
        usefulPhotonIntegrals50.push_back(integral50);
        usefulPhotonIntegrals60.push_back(integral60);
        usefulPhotonIntegrals70.push_back(integral70);
        usefulPhotonIntegrals80.push_back(integral80);
        usefulPhotonIntegrals90.push_back(integral90);
        legend->AddEntry(h, label.c_str(), "l");
    }
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
        std::string label = chains[i].first;
        double beamEnergy = 0.0;


        std::regex energyRegex(R"((\d+(?:\.\d+)?)\s*MeV)");
        std::smatch match;
        if (std::regex_search(label, match, energyRegex)) {
            beamEnergy = std::stod(match[1].str());
        } else {
            continue;

        }


        double usefulPhotons100 = (i < usefulPhotonIntegrals100.size()) ? usefulPhotonIntegrals100[i] : 0; //make sure index is valid
        double usefulPhotons10 = (i < usefulPhotonIntegrals10.size()) ? usefulPhotonIntegrals10[i] : 0; //make sure index is valid
        double usefulPhotons20 = (i < usefulPhotonIntegrals20.size()) ? usefulPhotonIntegrals20[i] : 0; //make sure index is valid
        double usefulPhotons30 = (i < usefulPhotonIntegrals30.size()) ? usefulPhotonIntegrals30[i] : 0; //make sure index is valid
        double usefulPhotons40 = (i < usefulPhotonIntegrals40.size()) ? usefulPhotonIntegrals40[i] : 0; //make sure index is valid
        double usefulPhotons50 = (i < usefulPhotonIntegrals50.size()) ? usefulPhotonIntegrals50[i] : 0; //make sure index is valid
        double usefulPhotons60 = (i < usefulPhotonIntegrals60.size()) ? usefulPhotonIntegrals60[i] : 0; //make sure index is valid
        double usefulPhotons70 = (i < usefulPhotonIntegrals70.size()) ? usefulPhotonIntegrals70[i] : 0; //make sure index is valid
        double usefulPhotons80 = (i < usefulPhotonIntegrals80.size()) ? usefulPhotonIntegrals80[i] : 0; //make sure index is valid
        double usefulPhotons90 = (i < usefulPhotonIntegrals90.size()) ? usefulPhotonIntegrals90[i] : 0; //make sure index is valid
        
        std::cout << "Label: " << label << ", Useful Photons at 10cm: " << usefulPhotons10 << std::endl;
        usefulPhotons100 = usefulPhotons100 / 1e6; 
        usefulPhotons10 = usefulPhotons10 / 1e6; 
        usefulPhotons20 = usefulPhotons20 / 1e6; 
        usefulPhotons30 = usefulPhotons30 / 1e6; 
        usefulPhotons40 = usefulPhotons40 / 1e6; 
        usefulPhotons50 = usefulPhotons50 / 1e6; 
        usefulPhotons60 = usefulPhotons60 / 1e6; 
        usefulPhotons70 = usefulPhotons70 / 1e6; 
        usefulPhotons80 = usefulPhotons80 / 1e6; 
        usefulPhotons90 = usefulPhotons90 / 1e6;

        // std::cout << "Label: " << label << ", Useful Photons at 10cm per primary electron: " << usefulPhotons10 << std::endl;
        if (label.find("SF6") != std::string::npos){ //swap beam energy for lengths
            gSF6->SetPoint(idxSF6++, 10, usefulPhotons10);
            gSF6->SetPoint(idxSF6++, 20, usefulPhotons20);
            gSF6->SetPoint(idxSF6++, 30, usefulPhotons30);
            gSF6->SetPoint(idxSF6++, 40, usefulPhotons40);
            gSF6->SetPoint(idxSF6++, 50, usefulPhotons50);
            gSF6->SetPoint(idxSF6++, 60, usefulPhotons60);
            gSF6->SetPoint(idxSF6++, 70, usefulPhotons70);
            gSF6->SetPoint(idxSF6++, 80, usefulPhotons80);
            gSF6->SetPoint(idxSF6++, 90, usefulPhotons90);
            gSF6->SetPoint(idxSF6++, 100, usefulPhotons100);
        }
        else if (label.find("C3F8") != std::string::npos){
            gC3F8->SetPoint(idxC3F8++, 10, usefulPhotons10);
            gC3F8->SetPoint(idxC3F8++, 20, usefulPhotons20);
            gC3F8->SetPoint(idxC3F8++, 30, usefulPhotons30);
            gC3F8->SetPoint(idxC3F8++, 40, usefulPhotons40);
            gC3F8->SetPoint(idxC3F8++, 50, usefulPhotons50);
            gC3F8->SetPoint(idxC3F8++, 60, usefulPhotons60);
            gC3F8->SetPoint(idxC3F8++, 70, usefulPhotons70);
            gC3F8->SetPoint(idxC3F8++, 80, usefulPhotons80);
            gC3F8->SetPoint(idxC3F8++, 90, usefulPhotons90);
            gC3F8->SetPoint(idxC3F8++, 100, usefulPhotons100);
        }
        else if (label.find("CF4") != std::string::npos){
            gCF4->SetPoint(idxCF4++, 10, usefulPhotons10);
            gCF4->SetPoint(idxCF4++, 20, usefulPhotons20);
            gCF4->SetPoint(idxCF4++, 30, usefulPhotons30);
            gCF4->SetPoint(idxCF4++, 40, usefulPhotons40);
            gCF4->SetPoint(idxCF4++, 50, usefulPhotons50);
            gCF4->SetPoint(idxCF4++, 60, usefulPhotons60);
            gCF4->SetPoint(idxCF4++, 70, usefulPhotons70);
            gCF4->SetPoint(idxCF4++, 80, usefulPhotons80);
            gCF4->SetPoint(idxCF4++, 90, usefulPhotons90);
            gCF4->SetPoint(idxCF4++, 100, usefulPhotons100);
        }
        else if (label.find("PF5") != std::string::npos){
            gPF5->SetPoint(idxPF5++, 10, usefulPhotons10);
            gPF5->SetPoint(idxPF5++, 20, usefulPhotons20);
            gPF5->SetPoint(idxPF5++, 30, usefulPhotons30);
            gPF5->SetPoint(idxPF5++, 40, usefulPhotons40);
            gPF5->SetPoint(idxPF5++, 50, usefulPhotons50);
            gPF5->SetPoint(idxPF5++, 100, usefulPhotons100);
        }
        else if (label.find("UF6") != std::string::npos){
            gUF6->SetPoint(idxUF6++, 10, usefulPhotons10);
            gUF6->SetPoint(idxUF6++, 20, usefulPhotons20);
            gUF6->SetPoint(idxUF6++, 30, usefulPhotons30);
            gUF6->SetPoint(idxUF6++, 40, usefulPhotons40);
            gUF6->SetPoint(idxUF6++, 50, usefulPhotons50);
            gUF6->SetPoint(idxUF6++, 60, usefulPhotons60);
            gUF6->SetPoint(idxUF6++, 70, usefulPhotons70);
            gUF6->SetPoint(idxUF6++, 80, usefulPhotons80);
            gUF6->SetPoint(idxUF6++, 90, usefulPhotons90);
            gUF6->SetPoint(idxUF6++, 100, usefulPhotons100);
        }
        else if (label.find("vacuum") != std::string::npos){
            gVacuum->SetPoint(idxVacuum++, 10, usefulPhotons10);
            gVacuum->SetPoint(idxVacuum++, 20, usefulPhotons20);
            gVacuum->SetPoint(idxVacuum++, 30, usefulPhotons30);
            gVacuum->SetPoint(idxVacuum++, 40, usefulPhotons40);
            gVacuum->SetPoint(idxVacuum++, 50, usefulPhotons50);
            gVacuum->SetPoint(idxVacuum++, 60, usefulPhotons60);
            gVacuum->SetPoint(idxVacuum++, 70, usefulPhotons70);
            gVacuum->SetPoint(idxVacuum++, 80, usefulPhotons80);
            gVacuum->SetPoint(idxVacuum++, 90, usefulPhotons90);
            gVacuum->SetPoint(idxVacuum++, 100, usefulPhotons100);
        }
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
    scatterLegend->AddEntry(gC3F8, "C3F8_1.32g/cm3", "p");
    scatterLegend->AddEntry(gCF4, "CF4_1.603g/cm3", "p");
    scatterLegend->AddEntry(gPF5, "PF5", "p");
    scatterLegend->AddEntry(gUF6, "UF6_3.630g/cm3", "p");
    scatterLegend->AddEntry(gVacuum, "vacuum", "p");

    TCanvas *c3 = new TCanvas("c3", "#Useful Photons vs Radius [25MeV beam]", 600, 500);
    mg->SetTitle("#Useful photons (15-22MeV) per Primary Electron vs Radius;Detector Radius (cm);#Useful photons per Primary Electron");
    mg->Draw("AP");
    mg->GetXaxis()->SetLimits(0, 110);
    scatterLegend->Draw();
    c3->Update();
    c3->SaveAs("Photons_Radius.png");
}

int main() {
    compare();
    return 0;
}
