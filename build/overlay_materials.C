#include <TFile.h>
#include <TH1F.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TString.h>
#include <vector>
#include <TStyle.h>

void overlay_materials() {
    std::vector<TString> materials = {"C3F8", "CF4", "HF", "SF6", "UF6"};
    int colors[] = {kP10Red, kP10Cyan, kP10Blue, kP10Green, kP10Orange}; // Simplified palette for compatibility
    
    TCanvas *c1 = new TCanvas("c1", "Material Comparison", 800, 600);
    gStyle->SetOptStat(0);
    
    TLegend *leg = new TLegend(0.7, 0.7, 0.9, 0.9);
    leg->SetHeader("Materials", "C");

    // --- Step 1: Find the Global Maximum ---
    double globalMax = 0;
    std::vector<TH1F*> hists; // Store histograms temporarily

    for (int i = 0; i < materials.size(); ++i) {
        TString fileName = materials[i] + "_25MeV.root";
        TFile *f = TFile::Open(fileName);
        if (!f || f->IsZombie()) continue;

        TH1F *h = (TH1F*)f->Get("h_final");
        if (!h) continue;

        h->SetDirectory(0); 
        h->Rebin(10);
        
        // Track the highest point found so far
        if (h->GetMaximum() > globalMax) {
            globalMax = h->GetMaximum();
        }

        hists.push_back(h);
        f->Close();
    }

    // --- Step 2: Draw the Histograms ---
    for (int i = 0; i < hists.size(); ++i) {
        TH1F *h = hists[i];
        
        h->SetLineColor(colors[i]);
        h->SetLineWidth(1);
        h->SetFillStyle(0);
        
        // Apply the 1.05x buffer to the FIRST histogram drawn
        if (i == 0) {
            h->SetMaximum(globalMax * 1.05);
            h->SetTitle("Target Comparison (same density) at 25 MeV;Depth (cm);Reactions");
            h->Draw("HIST");
        } else {
            h->Draw("HIST SAME");
        }

        leg->AddEntry(h, materials[i], "l");
    }

    leg->Draw();
    c1->Update();
    c1->SaveAs("Material_Overlay.pdf");
}