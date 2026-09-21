#include <TCanvas.h>
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TRandom.h>
#include <TStyle.h>
#include <TString.h>
#include <TTree.h>
#include <TTreeReader.h>

#include <iomanip>
#include <iostream>

// Struct to encapsulate histogram decoration parameters
struct HistogramDecoration
{
  int         lineWidth      = -1;
  int         lineColor      = -1;
  const char *xTitle        = nullptr;
  const char *yTitle        = nullptr;
  const char *zTitle        = nullptr;
  double      xRangeMin      = 0.;
  double      xRangeMax      = 0.;
  double      yRangeMin      = 0.;
  double      yRangeMax      = 0.;
};

// Function to show the progress bar
void PrintProgressBar(
  const int progress, 
  const int nHits, 
  const int barWidth = 50);

void DecorateHistogram(TH1 *hist, const HistogramDecoration &decoration);

void processSim(
  const TString inFileName   = "out.root",
  const TString outFileName  = "energyDistribution.png",
  const int     nEvts        = -1,
  const double  E_threshold  = 0.,
  const double  E_resolution = 0.)
{
  // Open the input file
  TFile *file_input = TFile::Open(inFileName);
  if (!file_input || file_input->IsZombie())
  {
    std::cerr << "Error: Unable to open file " 
              << inFileName << std::endl;
    return;
  }

  // Process the energy spectrum TTree
  TTree *tree1 = (TTree*)file_input->Get("EnergySpectrum");
  if (!tree1)
  {
    std::cerr << "Error: TTree 'EnergySpectrum' not found in file " 
              << inFileName << std::endl;
    return;
  }

  TTreeReader reader1(tree1);
  TTreeReaderValue<double> EdepEvent(reader1, "EventEdep");
  const int nEvents = tree1->GetEntries();
  std::cout << nEvents << " primary particles found." << std::endl;

  constexpr int spectrumBins = 50;
  constexpr double spectrumMin  = 0.0;

  // Scan the TTree to find the maximum EdepEvent during the simulation
  double spectrumMax = -1.0;
  while (reader1.Next()) 
  {
    if (*EdepEvent > spectrumMax)
        spectrumMax = *EdepEvent;
  }

  spectrumMax *= 1.1;

  reader1.Restart();

  TH1D *spectrum = new TH1D("spectrum", "",
                            spectrumBins, spectrumMin, spectrumMax);

  // Count detected particles for efficiency
  int count = 0;
  double resolutionFactor = E_resolution / (100. * 2.355);

  while (reader1.Next())
  {
    double Edep_meas = gRandom->Gaus(
      *EdepEvent, resolutionFactor * (*EdepEvent));

    if (Edep_meas > E_threshold)
    {
      ++count;
      spectrum->Fill(Edep_meas);
    }
  }

  if (nEvts > 0)
  {
    double efficiency = static_cast<double>(count) * 100. / nEvts;
    std::cout << "Detection efficiency = " 
              << std::setprecision(3) << efficiency << "%" 
              << std::endl;
  }

  // Process the individual hits TTree
  TTree *tree2 = (TTree*)file_input->Get("IndividualHits");
  if (!tree2)
  {
    std::cerr << "Error: TTree 'IndividualHits' not found in file " 
              << inFileName << std::endl;
    return;
  }

  TTreeReader reader2(tree2);
  TTreeReaderValue<double> hitEdep(reader2, "HitEdep");
  TTreeReaderValue<double> hitX(reader2, "HitX");
  TTreeReaderValue<double> hitY(reader2, "HitY");
  TTreeReaderValue<double> hitZ(reader2, "HitZ");

  const int nHits = tree2->GetEntries();
  std::cout  << "Processing " << nHits << " hits" << std::endl;

  constexpr int zBins = 300;
  constexpr int xyBins = 300;
  constexpr int xzBins = 300;
  constexpr double xRange = 15.0;
  constexpr double yRange = 15.0;
  constexpr double zMax = 30.0;

  TH1D *EdepZ_distr = new TH1D("EdepZ_distr", "",
                               zBins, 0.0, zMax);

  TH2D *EdepXY_distr = new TH2D("EdepXY_distr", "",
                                xyBins, -xRange, xRange, 
                                xyBins, -yRange, yRange);

  TH2D *EdepZY_distr = new TH2D("EdepZY_distr", "",
                                zBins, 0.0, zMax, 
                                xyBins, -yRange, yRange);

  int progressCount = 0;

  while (reader2.Next())
  {
    if (progressCount % 10000 == 0 || progressCount == nHits - 1)
    {
      PrintProgressBar(progressCount, nHits);
    }

    progressCount++;

    EdepZ_distr->Fill(*hitZ, *hitEdep);
    EdepXY_distr->Fill(*hitX, *hitY, *hitEdep);
    EdepZY_distr->Fill(*hitZ, *hitY, *hitEdep);
  }

  PrintProgressBar(nHits, nHits);

  // Apply decorations
  HistogramDecoration spectrumDecoration = 
  {
    2, 
    kBlack, 
    "Energy deposited (MeV)", 
    "Count", 
    nullptr, 
    0., 
    spectrumMax, 
    0., 
    0.
  };

  HistogramDecoration depthDecoration = 
  {
    2, 
    kRed, 
    "Depth (cm)", 
    "Energy deposited (MeV)", 
    nullptr, 
    0., 
    zMax, 
    0., 
    0.
  };

  HistogramDecoration xyDecoration = 
  {
    -1, 
    -1, 
    "X (cm)", 
    "Y (cm)", 
    "Energy deposited (MeV)", 
    -xRange, 
    xRange, 
    -yRange, 
    yRange
  };

  HistogramDecoration zyDecoration = 
  {
    -1, 
    -1, 
    "Depth (cm)", 
    "Y (cm)", 
    "Energy deposited (MeV)", 
    0., 
    zMax, 
    -xRange, 
    xRange
  };

  // Apply decorations to histograms
  DecorateHistogram(spectrum, spectrumDecoration);
  DecorateHistogram(EdepZ_distr, depthDecoration);
  DecorateHistogram(EdepXY_distr, xyDecoration);
  DecorateHistogram(EdepZY_distr, zyDecoration);

  // Plot histograms
  gStyle->SetOptStat(0);

  TCanvas *C1 = new TCanvas("C1", "Histograms", 900, 825);

  C1->Divide(2, 2);  // Divide canvas into 4 pads
  gStyle->SetPalette(kRainBow);
  gStyle->SetNumberContours(100);

  // Pad 1 (Energy spectrum)
  C1->cd(1);
  gPad->SetLeftMargin(0.15);
  gPad->SetRightMargin(0.05);
  spectrum->Draw("HIST");

  // Pad 2 (Energy deposited vs. phantom depth)
  C1->cd(2);
  gPad->SetLeftMargin(0.15);
  gPad->SetRightMargin(0.05);
  EdepZ_distr->Draw("HIST");

  // Pad 3 (Energy distribution in the XY plane)
  C1->cd(3);
  gPad->SetLeftMargin(0.15);
  gPad->SetRightMargin(0.20); 
  EdepXY_distr->Draw("COLZ");

  // Pad 4 (Energy distribution in the ZY plane)
  C1->cd(4);
  gPad->SetLeftMargin(0.15);
  gPad->SetRightMargin(0.20);
  EdepZY_distr->Draw("COLZ");

  // Update canvas and save output
  C1->Update();
  C1->Print(outFileName);
}

// Function to apply uniform decorations to histograms
void DecorateHistogram(TH1 *hist, const HistogramDecoration &decoration)
{
  // Apply line width and color for 1D histograms
  if (decoration.lineWidth > 0)
  {
    hist->SetLineWidth(decoration.lineWidth);
  }

  if (decoration.lineColor > 0)
  {
    hist->SetLineColor(decoration.lineColor);
  }

  // Set axis ranges if specified
  if (decoration.xRangeMax > decoration.xRangeMin)
  {
    hist->GetXaxis()->SetRangeUser(decoration.xRangeMin,
                                   decoration.xRangeMax);
  }

  if (decoration.yRangeMax > decoration.yRangeMin)
  {
    hist->GetYaxis()->SetRangeUser(decoration.yRangeMin,
                                   decoration.yRangeMax);
  }

  // Set axis titles
  hist->GetXaxis()->SetTitle(decoration.xTitle);
  hist->GetYaxis()->SetTitle(decoration.yTitle);

  if (decoration.zTitle)
  {
    hist->GetZaxis()->SetTitle(decoration.zTitle);
  }

  // Center axis titles
  hist->GetXaxis()->CenterTitle();
  hist->GetYaxis()->CenterTitle();

  if (decoration.zTitle)
  {
    hist->GetZaxis()->CenterTitle();
  }

  // Set title and label sizes
  hist->GetXaxis()->SetTitleOffset(0.9);
  hist->GetYaxis()->SetTitleOffset(1.5);
  hist->GetXaxis()->SetLabelSize(0.04);
  hist->GetYaxis()->SetLabelSize(0.04);
  hist->GetXaxis()->SetTitleSize(0.05);
  hist->GetYaxis()->SetTitleSize(0.05);

  if (decoration.zTitle)
  {
    hist->GetZaxis()->SetTitleSize(0.05);
    hist->GetZaxis()->SetTitleOffset(1.3);
  }
}

void PrintProgressBar(const int progress, const int total, const int barWidth) 
{
  const float percentage = (float)progress / total;
  const int pos = barWidth * percentage;

  std::cout << "[";

  for (int i = 0; i < barWidth; ++i) 
  {
    if (i < pos)
    {
      std::cout << "=";
    }

    else if (i == pos)
    {
      std::cout << ">";
    }

    else 
    {
      std::cout << " ";
    }
  }
  
  std::cout << "] " << int(percentage * 100.) << "%\r";
  std::cout.flush();
}