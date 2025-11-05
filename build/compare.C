//root -l -b -q 'compare.C++()'

//currently plotting points at beam energies that don't exist for SF6
//also points potentially not in order? They are randomly placed
//no UF6 points. Maybe file name pattern inconsistent?
//only need to plot scatter.png
//what label is being used?
//why does it take so long to run
//is label being redefined
//problem is only with scatter.png plot 

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

void compare()
{
    gROOT->SetBatch(kTRUE);

    //===============================
    // Define TChains for each medium
    //===============================

    std::vector<std::pair<std::string, TChain*>> chains;

    std::vector<std::string> mediums = {"SF6", "C3F8", "CF4", "PF5", "UF6", "Vacuum"};
    std::vector<std::string> energies = {"20MeV", "22MeV", "25MeV", "28MeV", "30MeV", "35MeV", "40MeV", "45MeV", "50MeV"};

    // create one TChain per (medium, energy) and add matching files immediately
    for (const auto &m : mediums) {
        for (const auto &e : energies) {
            std::string label = m + "_" + e; //does this mean it has to be in this order?
            TChain *ch = new TChain("IndividualHits");
            // pattern: MEDIUM_*ENERGY_*.root (matches your previous usage)
            ch->Add(TString::Format("%s_*%s_*.root", m.c_str(), e.c_str()).Data());
            chains.push_back({label, ch});
        }
    }


    // std::vector<std::pair<std::string, TChain*>> chains;

    // chains.push_back({"SF6",    new TChain("IndividualHits")});

    // chains.push_back({"C3F8",   new TChain("IndividualHits")});
    // chains.push_back({"CF4",    new TChain("IndividualHits")});
    // chains.push_back({"PF5",    new TChain("IndividualHits")});
    // chains.push_back({"UF6",    new TChain("IndividualHits")});
    // chains.push_back({"Vacuum", new TChain("IndividualHits")});

    // // Add files to each chain (adjust the wildcard patterns as needed)
    // chains[0].second->Add("SF6_*20MeV_*.root");
    // chains[1].second->Add("SF6_*22MeV_*.root");
    // chains[2].second->Add("SF6_*25MeV_*.root");
    // chains[3].second->Add("SF6_*28MeV_*.root");
    // chains[4].second->Add("SF6_*30MeV_*.root");
    // chains[5].second->Add("SF6_*35MeV_*.root");
    // chains[6].second->Add("SF6_*40MeV_*.root");
    // chains[7].second->Add("SF6_*45MeV_*.root");
    // chains[8].second->Add("SF6_*50MeV_*.root");
    // chains[9].second->Add("C3F8_*20MeV_*.root");
    // chains[10].second->Add("C3F8_*22MeV_*.root");
    // chains[11].second->Add("C3F8_*25MeV_*.root");
    // chains[12].second->Add("C3F8_*28MeV_*.root");
    // chains[13].second->Add("C3F8_*30MeV_*.root");
    // chains[14].second->Add("C3F8_*35MeV_*.root");
    // chains[15].second->Add("C3F8_*40MeV_*.root");
    // chains[16].second->Add("C3F8_*45MeV_*.root");
    // chains[17].second->Add("C3F8_*50MeV_*.root");
    // chains[18].second->Add("CF4_*20MeV_*.root");
    // chains[19].second->Add("CF4_*22MeV_*.root");
    // chains[20].second->Add("CF4_*25MeV_*.root");
    // chains[21].second->Add("CF4_*28MeV_*.root");
    // chains[22].second->Add("CF4_*30MeV_*.root");
    // chains[23].second->Add("CF4_*35MeV_*.root");
    // chains[24].second->Add("CF4_*40MeV_*.root");
    // chains[25].second->Add("CF4_*45MeV_*.root");
    // chains[26].second->Add("CF4_*50MeV_*.root");
    // chains[27].second->Add("PF5_*20MeV_*.root");
    // chains[28].second->Add("PF5_*22MeV_*.root");
    // chains[29].second->Add("PF5_*25MeV_*.root");
    // chains[30].second->Add("PF5_*28MeV_*.root");
    // chains[31].second->Add("PF5_*30MeV_*.root");
    // chains[32].second->Add("PF5_*35MeV_*.root");
    // chains[33].second->Add("PF5_*40MeV_*.root");
    // chains[34].second->Add("PF5_*45MeV_*.root");
    // chains[35].second->Add("PF5_*50MeV_*.root");
    // chains[36].second->Add("UF6_*20MeV_*.root");
    // chains[37].second->Add("UF6_*22MeV_*.root");
    // chains[38].second->Add("UF6_*25MeV_*.root");
    // chains[39].second->Add("UF6_*28MeV_*.root");
    // chains[40].second->Add("UF6_*30MeV_*.root");
    // chains[41].second->Add("UF6_*35MeV_*.root");
    // chains[42].second->Add("UF6_*40MeV_*.root");
    // chains[43].second->Add("UF6_*45MeV_*.root");
    // chains[44].second->Add("UF6_*50MeV_*.root");
    // chains[45].second->Add("Vacuum_*20MeV_*.root");
    
    // chains[1].second->Add("C3F8_*.root");
    // chains[2].second->Add("CF4_*.root");
    // chains[3].second->Add("PF5_*.root");
    // chains[4].second->Add("UF6_*.root");
    // chains[5].second->Add("Vacuum_*.root");

    //===============================
    // Decorations and setup
    //===============================
    HistogramDecoration Decoration = {
       1, kBlack, "Depth (cm)", "#photons in range 15-22 MeV", nullptr, 0., 100., 0., 0.
    };

    HistogramDecoration photonDecoration = {
       1, kBlack, "Depth (cm)", "Photon Energy (MeV)", nullptr, 0., 100., 0., 0.
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
    std::vector<double> usefulPhotonIntegrals;
    double globalMax = 0.0;

    //===============================
    // Loop over each TChain
    //===============================
    for (size_t i=0; i<chains.size(); i++) { //what is chains.size()? Number of chains ?
        TChain *t = chains[i].second;
        std::string label = chains[i].first;

        if (!t || t->GetEntries() == 0) {
            std::cout << "Warning: Chain " << label << " is empty!" << std::endl;
            continue;
        }

        t->SetBranchStatus("*", 0);
        t->SetBranchStatus("HitZ", 1);
        t->SetBranchStatus("HitEdep", 1);
        t->SetBranchStatus("HitPDG", 1);
        t->SetBranchStatus("HitParentID", 1);
        t->SetBranchStatus("HitKineticEnergy", 1);

        TString hname = TString::Format("h_%zu", i);
        TString h2name = TString::Format("h2_photonEnergy_depth_%zu", i);

        TString drawCmd_numPhotons = TString::Format("HitZ >> %s(100, %g, %g)",
                                     hname.Data(), Decoration.xMin, Decoration.xMax);
        TString drawCmd_photonEnergy = TString::Format(
            "HitKineticEnergy:HitZ >> %s(100, %g, %g, 200, 0, 50)",
            h2name.Data(), photonDecoration.xMin, photonDecoration.xMax);

        TString weight_numPhotons = "HitPDG==1 && HitKineticEnergy>15 && HitKineticEnergy<22 ? 1 : 0";
        TString weight_photon = "HitPDG==1 && HitKineticEnergy>0 ? 1 : 0";

        // Fill histograms from chain
        t->Draw(drawCmd_numPhotons, weight_numPhotons, "goff");
        t->Draw(drawCmd_photonEnergy, weight_photon, "goff");

        TH1D *h = (TH1D*)gDirectory->Get(hname);
        TH2D *h2 = (TH2D*)gDirectory->Get(h2name);
        if (!h || !h2) {
            std::cout << "Error: could not retrieve histograms for " << label << std::endl;
            continue;
        }

        h->SetLineColor(colors[i % nColors]);
        h->SetLineWidth(Decoration.lineWidth);
        h->GetXaxis()->SetTitle(Decoration.xTitle);
        h->GetYaxis()->SetTitle(Decoration.yTitle);

        double thisMax = h->GetMaximum();
        if (thisMax > globalMax) globalMax = thisMax;

        histos.push_back(h);
        h2_histos.push_back(h2);
        usefulPhotonIntegrals.push_back(h->Integral(0,100));

        legend->AddEntry(h, label.c_str(), "l");
    }

    // //===============================
    // // Draw 1D comparison
    // //===============================
    // bool firstDraw = true;
    // for (size_t i=0; i<histos.size(); i++) {
    //     TH1D *h = histos[i];
    //     h->GetYaxis()->SetRangeUser(0.5, globalMax * 1.2);
    //     if (firstDraw) {
    //         gPad->SetLogy();
    //         h->Draw("C");
    //         firstDraw = false;
    //     } else {
    //         h->Draw("C same");
    //     }
    // }

    // legend->Draw();
    // c1->SaveAs("compare_energy_deposition.png");

    // //===============================
    // // Draw all 2D histograms
    // //===============================
    // int n2D = h2_histos.size();
    // if (n2D > 0) {
    //     int nCols = std::ceil(std::sqrt(n2D));
    //     int nRows = std::ceil((double)n2D / nCols);

    //     TCanvas *c2 = new TCanvas("c2", "Photon Energy vs Depth (All Chains)", 1200, 800);
    //     c2->Divide(nCols, nRows);

    //     for (int i = 0; i < n2D; ++i) {
    //         c2->cd(i+1);
    //         gPad->SetRightMargin(0.15);
    //         gPad->SetLogz();
    //         h2_histos[i]->GetXaxis()->SetTitle(photonDecoration.xTitle);
    //         h2_histos[i]->GetYaxis()->SetTitle(photonDecoration.yTitle);
    //         h2_histos[i]->Draw("COLZ");

    //         TLatex label;
    //         label.SetNDC();
    //         label.SetTextSize(0.04);
    //         label.DrawLatex(0.15, 0.93, chains[i].first.c_str());
    //     }

    //     c2->Update();
    //     c2->SaveAs("photon_energy_depth_all.png");
    // }

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

        // For simplicity here, assume beam energy encoded in pattern name like "30MeV"
        std::regex energyRegex(R"((\d+(?:\.\d+)?)\s*MeV)");
        std::smatch match;
        if (std::regex_search(label, match, energyRegex)) {
            beamEnergy = std::stod(match[1].str());
        } else {
            //beamEnergy = (i+1)*10; // fallback dummy value
            // return; // Skip if no energy found
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
        else if (label.find("Vacuum") != std::string::npos)
            gVacuum->SetPoint(idxVacuum++, beamEnergy, usefulPhotons);
    }

    // Style graphs
    gSF6->SetMarkerColor(kBlue);   gSF6->SetMarkerStyle(21);
    gC3F8->SetMarkerColor(kRed);   gC3F8->SetMarkerStyle(23);
    gCF4->SetMarkerColor(kBlack);  gCF4->SetMarkerStyle(24);
    gPF5->SetMarkerColor(kMagenta);gPF5->SetMarkerStyle(22);
    gUF6->SetMarkerColor(kOrange); gUF6->SetMarkerStyle(25);
    gVacuum->SetMarkerColor(kGreen);gVacuum->SetMarkerStyle(26);

    // Find Y max for plot
    double YMax = -1e9;
    for (auto g : {gSF6, gC3F8, gCF4, gPF5, gUF6, gVacuum}) {
        int n = g->GetN();
        for (int i = 0; i < n; ++i) {
            double x, y;
            g->GetPoint(i, x, y);
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

    TLegend *scatterLegend = new TLegend(0.1, 0.78, 0.3, 0.88);
    scatterLegend->AddEntry(gSF6, "SF6", "p");
    scatterLegend->AddEntry(gC3F8, "C3F8", "p");
    scatterLegend->AddEntry(gCF4, "CF4", "p");
    scatterLegend->AddEntry(gPF5, "PF5", "p");
    scatterLegend->AddEntry(gUF6, "UF6", "p");
    scatterLegend->AddEntry(gVacuum, "Vacuum", "p");

    TCanvas *c3 = new TCanvas("c3", "#Useful Photons vs Beam Energy", 600, 500);
    mg->SetTitle("#Useful photons (15-22MeV) vs beam energy;Beam energy (MeV);#Useful photons");
    mg->Draw("AP");
    mg->GetXaxis()->SetLimits(19, 51);
    scatterLegend->Draw();
    c3->Update();
    c3->SaveAs("scatter.png");
}
