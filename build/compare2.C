// compare2.C
// Usage (from shell):
/*root -l 'compare2.C("50mmW_20MeV_SF6_1milleven
ts.root","50mmW_22MeV_SF6_1milleven
ts.root","50mmW_25MeV_SF6_1milleven
ts.root","50mmW_28MeV_SF6_1milleven
ts.root","50mmW_30MeV_SF6_1milleven
ts.root","50mmW_35MeV_SF6_1milleven
ts.root","50mmW_40MeV_SF6_1milleven
ts.root","50mmW_45MeV_SF6_1milleven
ts.root","50mmW_50MeV_SF6_1milleven
ts.root","50mmCu_20MeV_SF6_1milleven
ts.root","50mmCu_22MeV_SF6_1milleven
ts.root","50mmCu_25MeV_SF6_1milleven
ts.root","50mmCu_28MeV_SF6_1milleven
ts.root","50mmCu_30MeV_SF6_1milleven
ts.root","50mmCu_35MeV_SF6_1milleven
ts.root","50mmCu_40MeV_SF6_1milleven
ts.root","50mmCu_45MeV_SF6_1milleven
ts.root","50mmCu_50MeV_SF6_1milleven
ts.root")'*/   

#include <filesystem>
#include <regex>
#include <string>
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

// void compare() // No arguments needed
// {
//     // Automatically find all .root files in the current directory
//     std::vector<std::string> files;
//     for (const auto& entry : fs::directory_iterator(".")) {
//         if (entry.is_regular_file() && entry.path().extension() == ".root") {
//             files.push_back(entry.path().string());
//         }
//     }

//     if (files.empty()) {
//         std::cout << "No .root files found in current directory!" << std::endl;
//         return;
//     }

void compare2(const char* file1 = "", const char* file2 = "", const char* file3 = "", 
    const char* file4 = "", const char* file5 = "", const char* file6 = "", const char* file7 = "", 
    const char* file8 = "", const char* file9 = "", const char* file10 = "", const char* file11 = "",
    const char* file12 = "", const char* file13 = "", const char* file14 = "", const char* file15 = "",
    const char* file16 = "", const char* file17 = "", const char* file18 = "")
{
    // Collect input filenames
    std::vector<std::string> files;
    if (strlen(file1)) files.push_back(file1);
    if (strlen(file2)) files.push_back(file2);
    if (strlen(file3)) files.push_back(file3);
    if (strlen(file4)) files.push_back(file4);
    if (strlen(file5)) files.push_back(file5);
    if (strlen(file6)) files.push_back(file6);
    if (strlen(file7)) files.push_back(file7);
    if (strlen(file8)) files.push_back(file8);
    if (strlen(file9)) files.push_back(file9);
    if (strlen(file10)) files.push_back(file10);
    if (strlen(file11)) files.push_back(file11);
    if (strlen(file12)) files.push_back(file12);
    if (strlen(file13)) files.push_back(file13);
    if (strlen(file14)) files.push_back(file14);
    if (strlen(file15)) files.push_back(file15);
    if (strlen(file16)) files.push_back(file16);
    if (strlen(file17)) files.push_back(file17);
    if (strlen(file18)) files.push_back(file18);

    if (files.empty()) {
        std::cout << "No input files provided!" << std::endl;
        return;
    }


//should this be a histogram at all
    HistogramDecoration Decoration = {
        1,              // line width
        kBlack,         // placeholder color (overridden)
        "Depth (cm)", 
        "#primary electrons through foil", 
        nullptr, 
        0., 5.0,        // x-axis range
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

    std::vector<double> idealThicknesses;


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
        t->SetBranchStatus("HitPz", 1);
    


        // Unique histogram name
        TString hname = TString::Format("h_%zu", i);

        TString drawCmd = TString::Format("HitZ >> %s(80, %g, %g)", 
                                          hname.Data(), Decoration.xMin, Decoration.xMax);


        TString weight= "HitPDG==0 && HitParentID==0 && HitPz>0 && HitEdep>0 ? 1 : 0";


        t->Draw(drawCmd, weight, "goff");
        

        TH1D *h = (TH1D*)gDirectory->Get(hname);
        if (!h) {
            std::cout << "Error: could not retrieve histogram from " << files[i] << std::endl;
            continue;
        }


        histos.push_back(h);
        double thisMax = h->GetMaximum();
        if (thisMax > globalMax) globalMax = thisMax;
    }

    // Second loop: style and draw
    bool firstDraw = true;
    for (size_t i=0; i<histos.size(); i++) {
        TH1D *h = histos[i];
        int thisColor = colors[i % nColors];

        double idealThickness = -1.0;
        double ymin = 0.0;



        //int numEntries = h->GetEntries();
        // double thisMax = h->GetMaximum();
        // int Events = globalMax;
        double threshold = 0.1 * h->GetMaximum(); //the marker just plots at 0... or clsoe to 0 on the x axis for the log scale. Why?
        
        int nbins = h->GetNbinsX();
        for (int bin=1; bin<= nbins; ++bin){
            double content = h->GetBinContent(bin);
            if (content<=threshold){
                idealThickness = h->GetXaxis()->GetBinCenter(bin);
                break;
            }
        }
        std::cout<<idealThickness<<std::endl;
        idealThicknesses.push_back(idealThickness);
 
        h->SetLineColor(thisColor);
        h->SetLineWidth(Decoration.lineWidth);
        h->GetXaxis()->SetTitle(Decoration.xTitle);
        h->GetYaxis()->SetTitle(Decoration.yTitle);
        TLine *marker = new TLine(idealThickness, ymin, idealThickness, globalMax);
        marker->SetLineColor(thisColor);
        marker->SetLineWidth(2);
        marker->SetLineStyle(2);
        
        if (Decoration.title) h->SetTitle(Decoration.title);

        // Apply global y-axis range (padding included)
        h->GetYaxis()->SetRangeUser(ymin, globalMax * 1.2);

        if (firstDraw) {
            // gPad->SetLogy();  // logarithmic y-axis
            h->Draw("C");
            marker->Draw("same");
            firstDraw = false;
        } else {
            h->Draw("C same");
            marker->Draw("same");
        }

        legend->AddEntry(h, files[i].c_str(), "l");
    }

    legend->Draw();


    // --- Scatter plot: ideal thickness vs beam energy ---
    TGraph *gScatter = new TGraph();
    gScatter->SetTitle("Ideal thickness of foil vs Beam Energy;Energy (MeV);Thickness (mm)");
    // Define colors for materials
    int colorW = kBlue;
    int colorCu = kRed;

    TGraph *gW = new TGraph();
    TGraph *gCu = new TGraph();
    int idxW = 0, idxCu = 0;

//might not need to open the files again here since we just need to extract the energy from the names.
    for (size_t i=0; i<files.size(); i++) {

    double beamEnergy = 0.0;
    std::regex energyRegex(R"((\d+(?:\.\d+)?)\s*MeV)");
    std::smatch match;

    if (std::regex_search(files[i], match, energyRegex)) {
        beamEnergy = std::stod(match[1].str());
    }
        double idealThickness = idealThicknesses[i];
        gScatter->SetPoint(i, beamEnergy, idealThickness);

    // Set marker color based on material in filename
    if (files[i].find("W") != std::string::npos) {
        gW->SetPoint(idxW++, beamEnergy, idealThickness);
    } else if (files[i].find("Cu") != std::string::npos) {
        gCu->SetPoint(idxCu++, beamEnergy, idealThickness);
    }

        //gScatter->SetMarkerColor(markerColor);
        // gScatter->SetMarkerStyle(20 + i);
    }

    gW->SetMarkerColor(colorW);
    gW->SetMarkerStyle(21);
    gCu->SetMarkerColor(colorCu);
    gCu->SetMarkerStyle(22);

    TMultiGraph *mg = new TMultiGraph();
    mg->Add(gW, "P");
    mg->Add(gCu, "P");

    // Create a new legend for the scatter plot
    TLegend *scatterLegend = new TLegend(0.7,0.7,0.9,0.9);
    scatterLegend->AddEntry(gW, "W", "p");
    scatterLegend->AddEntry(gCu, "Cu", "p");


    // Draw scatter plot on a new canvas
    TCanvas *c2 = new TCanvas("c2", "Ideal thickness vs beamEnergy", 600, 500);
    mg->SetTitle("Ideal foil thickness vs beam energy;Beam energy (MeV);Ideal thickness (cm)");
    mg->Draw("A");
    mg->GetXaxis()->SetLimits(19, 51);//-- set your desired range
    scatterLegend->Draw();
    c2->Update();

}