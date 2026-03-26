#include <iostream>
#include <vector>
#include <string>

#include "TFile.h"
#include "TH1.h"
#include "TGraph.h"
#include "TMultiGraph.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TAxis.h"

void plot_reactions() {
    // Define the materials and energies
    std::vector<std::string> materials = {"C3F8", "CF4", "HF", "SF6", "UF6"};
    std::vector<double> energies = {20.0, 25.0, 30.0};
    
    // Define distinct colors and marker styles for each material
    std::vector<int> colors = {kRed, kBlue, kGreen+2, kMagenta, kOrange+7};
    std::vector<int> markers = {kFullCircle, kFullSquare, kFullTriangleUp, kFullTriangleDown, kFullDiamond};

    // Create a canvas
    TCanvas *c1 = new TCanvas("c1", "18F Reactions vs Beam Energy", 800, 600);
    c1->SetGrid();

    // Create a MultiGraph and a Legend
    TMultiGraph *mg = new TMultiGraph();
    // mg->SetTitle("^{19}F(#gamma, n)^{18}F Reactions vs Beam Energy;Beam Energy (MeV);^{19}F(#gamma, n)^{18}F Reactions");
    mg->SetTitle("^{18}F Activity vs Beam Energy;Beam Energy (MeV);Activity (#frac{MBq}{mA#bullet h})");
    
    TLegend *leg = new TLegend(0.15, 0.65, 0.35, 0.85); // Adjust position as needed
    leg->SetBorderSize(1);
    leg->SetFillColor(0);
    leg->SetTextSize(0.04);

    // Loop over each material to create a separate TGraph
    for (size_t i = 0; i < materials.size(); ++i) {
        TGraph *graph = new TGraph();
        graph->SetMarkerStyle(markers[i]);
        graph->SetMarkerSize(1.5);
        graph->SetMarkerColor(colors[i]);
        graph->SetLineColor(colors[i]);
        
        int point_idx = 0;
        
        // Loop over each energy for the current material
        for (size_t j = 0; j < energies.size(); ++j) {
            // Construct the file name
            int energy_int = static_cast<int>(energies[j]);
            std::string filename = materials[i] + "_" + std::to_string(energy_int) + "MeV.root";
            
            // Open the ROOT file
            TFile *file = TFile::Open(filename.c_str(), "READ");
            if (!file || file->IsZombie()) {
                std::cerr << "Warning: Could not open file " << filename << std::endl;
                continue;
            }
            
            // Get the histogram
            TH1 *h_final = (TH1*)file->Get("h_final");
            if (!h_final) {
                std::cerr << "Warning: Could not find 'h_final' in " << filename << std::endl;
                file->Close();
                continue;
            }
            
            // Extract counts (assuming it's a single bin histogram, we can take bin 1 or Integral)
            double counts = h_final->GetBinContent(2);
            if (energies[j] == 20){
                counts = 4.7e-3 * counts;
            }else if (energies[j] == 25){
                counts = 5.4e-3 * counts;
            }else if (energies[j] == 30){
                counts = 6.3e-3 * counts;
            }
            
            // Add the point to the graph
            graph->SetPoint(point_idx, energies[j], counts);
            point_idx++;
            
            file->Close();
        }
        
        // Add the graph to the multigraph and legend if it has points
        if (graph->GetN() > 0) {
            mg->Add(graph, "P"); // "P" means draw points only
            leg->AddEntry(graph, materials[i].c_str(), "p");
        }
    }

    // Draw the multigraph
    // "A" draws axes, "P" draws markers
    mg->Draw("AP");

    // Force the X-axis range from 19 to 31 MeV
    // Note: Axes properties in TMultiGraph can only be modified *after* calling Draw()
    mg->GetXaxis()->SetLimits(19.0, 31.0);
    
    // TMultiGraph automatically adjusts the Y-axis to fit the highest point.
    // However, it's often good practice to start the Y-axis at 0 for counts.
    mg->SetMinimum(0.0);

    // Draw the legend
    leg->Draw();

    // Update the canvas
    c1->Update();
}
