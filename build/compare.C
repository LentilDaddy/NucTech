
//root -l -b -q 'compare.C++("15.5mmCu_30MeV_UF6_1millevents.root")'


#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TGraph.h>
#include <TMultiGraph.h>
#include <TStyle.h>
#include <TString.h>
#include <TLatex.h>
#include <regex>
#include <iostream>
#include <vector>
#include <string>

#include <filesystem>
#include <cmath>

namespace fs = std::filesystem;


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


void compare() // No arguments needed
{
    // Automatically find all .root files in the current directory
    std::vector<std::string> files;
    for (const auto& entry : fs::directory_iterator(".")) {
        if (entry.is_regular_file() && entry.path().extension() == ".root") {
            files.push_back(entry.path().string());
        }
    }


    if (files.empty()) {
        std::cout << "No .root files found in current directory!" << std::endl;
        return;
    }


// void compare(const char* file1 = "", const char* file2 = "", const char* file3 = "",
//    const char* file4 = "", const char* file5 = "", const char* file6 = "", const char* file7 = "",
//    const char* file8 = "", const char* file9 = "", const char* file10 = "", const char* file11 = "",
//    const char* file12 = "", const char* file13 = "", const char* file14 = "")
// {
//     gROOT->SetBatch(kTRUE);
//    // Collect input filenames
//    std::vector<std::string> files;
//    if (strlen(file1)) files.push_back(file1);
//    if (strlen(file2)) files.push_back(file2);
//    if (strlen(file3)) files.push_back(file3);
//    if (strlen(file4)) files.push_back(file4);
//    if (strlen(file5)) files.push_back(file5);
//    if (strlen(file6)) files.push_back(file6);
//    if (strlen(file7)) files.push_back(file7);
//    if (strlen(file8)) files.push_back(file8);
//    if (strlen(file9)) files.push_back(file9);
//    if (strlen(file10)) files.push_back(file10);
//    if (strlen(file11)) files.push_back(file11);
//    if (strlen(file12)) files.push_back(file12);
//    if (strlen(file13)) files.push_back(file13);
//    if (strlen(file14)) files.push_back(file14);


//    if (files.empty()) {
//        std::cout << "No input files provided!" << std::endl;
//        return;
//    }


   // Base decoration
   HistogramDecoration logDecoration = {
       1,              // line width
       kBlack,         // placeholder color (overridden)
       "Depth (cm)",
       "Energy deposited (MeV)",
       nullptr,
       0., 100.,        // x-axis range
       0., 0.          // y-axis auto
   };


//should this be a histogram at all
   HistogramDecoration Decoration = {
       1,              // line width
       kBlack,         // placeholder color (overridden)
       "Depth (cm)",
       "#photons in range 15-22 MeV",
       nullptr,
       0., 100.,        // x-axis range
       0., 0.          // y-axis auto
   };


   HistogramDecoration photonDecoration = {
       1,              // line width
       kBlack,         // placeholder color (overridden)
       "Depth (cm)",
       "Photon Energy (MeV)",
       nullptr,
       0., 100.,        // x-axis range
       0., 0.          // y-axis auto
   };




   // Fixed colors
   int colors[] = {kRed, kSpring+5, kBlack, kMagenta+2, kViolet-2, kBlue-7, kAzure-1,
       kCyan, kTeal+10, kGreen+3, kYellow, kGray, kOrange-7, kPink+7};
   int nColors = sizeof(colors)/sizeof(int);


   TLegend *legend = new TLegend(0.7,0.7,0.9,0.9);
   TCanvas *c1 = new TCanvas("c1", "Compare Energy Deposition", 900, 700);
   c1->cd();


   // First loop: build histograms and find global max
   std::vector<TH1D*> histos;
   double globalMax = 0.0;
   std::vector<double> usefulPhotonIntegrals;
//    TH2D *h2_photonEnergy_depth = nullptr;
    // store all 2D histograms
    std::vector<TH2D*> h2_histos;

   for (size_t i=0; i<files.size(); i++) {
       TFile *f = TFile::Open(files[i].c_str());
       if (!f || f->IsZombie()) {
           std::cout << "Error: could not open " << files[i] << std::endl;
           continue;
       }


       TTree *t = (TTree*)f->Get("IndividualHits");
       if (!t) {
           std::cout << "Error: tree IndividualHits not found in " << files[i] << std::endl;
           continue;
       }




       // Disable all branches first
       t->SetBranchStatus("*", 0);


       // Enable only the branches you actually need
       t->SetBranchStatus("HitZ", 1);
       t->SetBranchStatus("HitEdep", 1);
       t->SetBranchStatus("HitPDG", 1);
       t->SetBranchStatus("HitParentID", 1);
       t->SetBranchStatus("HitKineticEnergy", 1);




       // Unique histogram name
       TString hname = TString::Format("h_%zu", i);
    //    TString hname_numPhotons = TString::Format("h_numPhotons_%zu", i);
    //    TString h2_name = TString::Format("h2_photonEnergy_depth");
        TString h2_name = TString::Format("h2_photonEnergy_depth_%zu", i);




       // Fill histogram with HitZ, weighted by Edep
       TString drawCmd_numPhotons = TString::Format("HitZ >> %s(100, %g, %g)",
                                         hname.Data(), Decoration.xMin, Decoration.xMax);


       // TString drawCmd = TString::Format("HitZ >> %s(300, %g, %g)",
       //                                   hname.Data(), logDecoration.xMin, logDecoration.xMax);


    //    TString drawCmd_photonEnergy = TString::Format("HitKineticEnergy:HitZ >> %s(300, %g, %g, 200, 0, 50)",
    //                                    h2_name.Data(), photonDecoration.xMin, photonDecoration.xMax);

    TString drawCmd_photonEnergy = TString::Format(
        "HitKineticEnergy:HitZ >> %s(100, %g, %g, 200, 0, 50)",
        h2_name.Data(), photonDecoration.xMin, photonDecoration.xMax
    );
// "yvar:xvar >> hist2d(nx, xmin, xmax, ny, ymin, ymax)"


       // TString weight = "HitEdep>0 ? HitEdep : 0";
       TString weight_numPhotons = "HitPDG==1 && HitKineticEnergy>15 && HitKineticEnergy<22 ? 1 : 0";
       TString weight_photon = "HitPDG==1 && HitKineticEnergy>0 ? 1 : 0";


       //need to fill with 1 not with HitPDG


       //condition ? value_if_true : value if false


//tree->Draw("yvar:xvar >> hist2d(nx, xmin, xmax, ny, ymin, ymax)", "weight_expression", "goff");




       // t->Draw(drawCmd, weight, "goff");
       t->Draw(drawCmd_numPhotons, weight_numPhotons, "goff");
       t->Draw(drawCmd_photonEnergy, weight_photon, "goff");


      


       TH1D *h = (TH1D*)gDirectory->Get(hname);
       if (!h) {
           std::cout << "Error: could not retrieve histogram from " << files[i] << std::endl;
           continue;
       }


//    h2_photonEnergy_depth = (TH2D*)gDirectory->Get("h2_photonEnergy_depth");
//    if (!h2_photonEnergy_depth) {
//        std::cout << "Error: could not retrieve h2_photonEnergy_depth from gDirectory for " << files[i] << std::endl;
//        continue;
//    }

    TH2D *h2 = (TH2D*)gDirectory->Get(h2_name);
    if (!h2) {
        std::cout << "Error: could not retrieve " << h2_name << " from " << files[i] << std::endl;
        continue;
    }

    h2->SetTitle(TString::Format("Photon Energy vs Depth: %s", files[i].c_str()));
    h2_histos.push_back(h2);
       histos.push_back(h);
       double thisMax = h->GetMaximum();
       if (thisMax > globalMax) globalMax = thisMax;
   }


   // Second loop: style and draw
   bool firstDraw = true;
   for (size_t i=0; i<histos.size(); i++) {
       TH1D *h = histos[i];
       int thisColor = colors[i % nColors];

       float usefulPhotons=-1.0;
        usefulPhotons = h->Integral(0,100); //integral over all bins
        usefulPhotonIntegrals.push_back(usefulPhotons);


       h->SetLineColor(thisColor);
       h->SetLineWidth(Decoration.lineWidth);
       h->GetXaxis()->SetTitle(Decoration.xTitle);
       h->GetYaxis()->SetTitle(Decoration.yTitle);
       if (Decoration.title) h->SetTitle(Decoration.title);


       // Apply global y-axis range (padding included)
       h->GetYaxis()->SetRangeUser(0.5, globalMax * 1.2);


       if (firstDraw) {
           gPad->SetLogy();  // logarithmic y-axis
           // gPad->SetLogy(0);  // linear y-axis
           h->Draw("C");
           firstDraw = false;
       } else {
           h->Draw("C same");
       }


       legend->AddEntry(h, files[i].c_str(), "l");
   }
   c1->SaveAs("compare_energy_deposition.png");


   legend->Draw();


//    h2_photonEnergy_depth->SetTitle("Photon Energy vs Depth;Depth (cm);Photon Energy (MeV)");
//    h2_photonEnergy_depth->GetXaxis()->SetTitle(photonDecoration.xTitle);
//    h2_photonEnergy_depth->GetYaxis()->SetTitle(photonDecoration.yTitle);
//    TCanvas *c3 = new TCanvas("c3", "Photon Energy vs Depth", 800, 600);
//    c3->cd();
//    // gStyle->SetOptStat(0);
//    h2_photonEnergy_depth->Draw("COLZ");


// Draw all 2D histograms on one canvas
int n2D = h2_histos.size();
if (n2D > 0) {
    int nCols = std::ceil(std::sqrt(n2D));  // square-ish layout
    int nRows = std::ceil((double)n2D / nCols);

    TCanvas *c2 = new TCanvas("c2", "Photon Energy vs Depth (All Files)", 1200, 800);
    c2->Divide(nCols, nRows);

    for (int i = 0; i < n2D; ++i) {
        c2->cd(i+1);
        gPad->SetRightMargin(0.15);
        gPad->SetLogz();  // optional
        h2_histos[i]->GetXaxis()->SetTitle(photonDecoration.xTitle);
        h2_histos[i]->GetYaxis()->SetTitle(photonDecoration.yTitle);
        h2_histos[i]->Draw("COLZ");

        // Label with filename
        TLatex label;
        label.SetNDC();
        label.SetTextSize(0.04);
        label.DrawLatex(0.15, 0.93, files[i].c_str());
    }

    c2->Update();
    c2->SaveAs("photon_energy_depth_all.png");
}


//we want the integral of useful photons against beam energy


/*Obtain integral of h OR num entries over 1 m. Save it for different files. Plot scatter graph of integral
against beam energy for different media so that each medium is plotted in a different colour on the same graph.
Both the medium and the beam energy are found inside the filenames.*/


    TGraph *gScatter = new TGraph();
    gScatter->SetTitle("#Useful Photons (15-22MeV) of foil vs Beam Energy;Energy (MeV);#Useful Photons");
    // Define colors for materials
    int colorSF6 = kBlue;
    int colorC3F8 = kRed;
    int colorCF4 = kBlack;
    int colorPF5 = kMagenta;
    int colorUF6 = kOrange;
    int colorVacuum = kGreen;

    TGraph *gSF6 = new TGraph();
    TGraph *gC3F8 = new TGraph();
    TGraph *gCF4 = new TGraph();
    TGraph *gPF5 = new TGraph();
    TGraph *gUF6 = new TGraph();
    TGraph *gVacuum = new TGraph();
    int idxSF6 = 0, idxC3F8 = 0, idxCF4 = 0, idxPF5 = 0, idxUF6 = 0, idxVacuum = 0;

//might not need to open the files again here since we just need to extract the energy from the names.
for (size_t i=0; i<files.size(); i++) {

    double beamEnergy = 0.0;
    std::regex energyRegex(R"((\d+(?:\.\d+)?)\s*MeV)");
    std::smatch match;

    if (std::regex_search(files[i], match, energyRegex)) {
        beamEnergy = std::stod(match[1].str());
    }
    float usefulPhotons = usefulPhotonIntegrals[i];
    gScatter->SetPoint(i, beamEnergy, usefulPhotons);

    if (files[i].find("SF6") != std::string::npos) {
        gSF6->SetPoint(idxSF6++, beamEnergy, usefulPhotons);
    } else if (files[i].find("C3F8") != std::string::npos) {
        gC3F8->SetPoint(idxC3F8++, beamEnergy, usefulPhotons);
    } else if (files[i].find("CF4") != std::string::npos) {
        gCF4->SetPoint(idxCF4++, beamEnergy, usefulPhotons);
    } else if (files[i].find("PF5") != std::string::npos) {
        gPF5->SetPoint(idxPF5++, beamEnergy, usefulPhotons);
    } else if (files[i].find("UF6") != std::string::npos) {
        gUF6->SetPoint(idxUF6++, beamEnergy, usefulPhotons);
    } else if (files[i].find("Vacuum") != std::string::npos) {
        gVacuum->SetPoint(idxVacuum++, beamEnergy, usefulPhotons);
    }
}

    gSF6->SetMarkerColor(colorSF6);
    gPF5->SetMarkerColor(colorPF5);
    gC3F8->SetMarkerColor(colorC3F8);
    gCF4->SetMarkerColor(colorCF4);
    gUF6->SetMarkerColor(colorUF6);
    gVacuum->SetMarkerColor(colorVacuum);
    gSF6->SetMarkerStyle(21);
    gPF5->SetMarkerStyle(22);
    gC3F8->SetMarkerStyle(23);
    gCF4->SetMarkerStyle(24);
    gUF6->SetMarkerStyle(25);
    gVacuum->SetMarkerStyle(26);
    // gW->SetMarkerStyle(21);
    // gCu->SetMarkerColor(colorCu);
    // gCu->SetMarkerStyle(22);

double YMax = -1e9;

for (auto g : {gSF6, gC3F8, gCF4, gPF5, gUF6, gVacuum}) {
    int n = g->GetN();
    for (int i = 0; i < n; ++i) {
        double x, y;
        g->GetPoint(i, x, y);
        // if (y < yMin) yMin = y;
        if (y > YMax) YMax = y;
    }
}



    TMultiGraph *mg = new TMultiGraph();
    mg->Add(gSF6, "P");
    mg->Add(gC3F8, "P");
    mg->Add(gCF4, "P");
    mg->Add(gPF5, "P");
    mg->Add(gUF6, "P");
    mg->Add(gVacuum, "P");

    mg->SetMaximum(YMax * 1.1);


    // Create a new legend for the scatter plot
    TLegend *scatterLegend = new TLegend(0.1, 0.78, 0.3, 0.88); //top left corner?
    scatterLegend->AddEntry(gSF6, "SF6", "p"); 
    scatterLegend->AddEntry(gC3F8, "C3F8", "p");
    scatterLegend->AddEntry(gCF4, "CF4", "p");
    scatterLegend->AddEntry(gPF5, "PF5", "p");
    scatterLegend->AddEntry(gUF6, "UF6", "p");
    scatterLegend->AddEntry(gVacuum, "Vacuum", "p");


    // Draw scatter plot on a new canvas
    TCanvas *c3 = new TCanvas("c3", "#Useful Photons vs Beam Energy", 600, 500);
    mg->SetTitle("#Useful photons (15-22MeV) vs beam energy;Beam energy (MeV);I#Useful photons");
    mg->Draw("AP");
    mg->GetXaxis()->SetLimits(19, 51);//-- set your desired range
    scatterLegend->Draw();
    c3->Update();
    c3->SaveAs("scatter.png");
}
