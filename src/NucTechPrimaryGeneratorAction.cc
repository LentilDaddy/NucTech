#include "NucTechPrimaryGeneratorAction.hh"
#include "G4GeneralParticleSource.hh"
#include "G4Run.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"

// ROOT Includes
#include "TFile.h"
#include "TH1D.h"

NucTechPrimaryGeneratorAction::NucTechPrimaryGeneratorAction()
  : fGPS(std::make_unique<G4GeneralParticleSource>()),
    fEnergyHist(nullptr) // Initialize to null
{
  // 1. Get thickness from Environment Variable
  // Default to 10 if the variable isn't set
  G4String thickness = "10"; 
  char* envThickness = std::getenv("RADIATOR_THICKNESS");
  
  if (envThickness) {
      thickness = G4String(envThickness);
  }
  
  G4String histName = "proj_" + thickness + "mm";
  G4cout << "--- Generator initializing for thickness: " << thickness << "mm ---" << G4endl;

  // 2. Open the ROOT file
  TFile* inputFile = TFile::Open("all_spectra.root");

  if (inputFile && !inputFile->IsZombie()) {
    fEnergyHist = static_cast<TH1D*>(inputFile->Get(histName));

    if (fEnergyHist) {
      fEnergyHist->SetDirectory(0);
      G4cout << "Successfully loaded spectrum: " << histName << G4endl;
    } else {
      G4Exception("NucTechPrimaryGeneratorAction", "001", FatalException, 
                  ("Could not find " + histName + " in all_spectra.root").c_str());
    }
    inputFile->Close();
  } else {
    G4Exception("NucTechPrimaryGeneratorAction", "002", FatalException, 
                "Could not open all_spectra.root");
  }
}
NucTechPrimaryGeneratorAction::~NucTechPrimaryGeneratorAction() {
  if (fEnergyHist) delete fEnergyHist;
}

void NucTechPrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
  // 3. Sample Energy from ROOT Histogram
  if (fEnergyHist) {
    // GetRandom() uses the histogram weights to pick a value
    G4double sampledEnergy = fEnergyHist->GetRandom() * MeV;
    
    // Force the GPS to use this specific energy for this event
    fGPS->GetCurrentSource()->GetEneDist()->SetMonoEnergy(sampledEnergy);
  }

  // 4. Generate the particle (position and direction still come from beam.mac)
  fGPS->GeneratePrimaryVertex(anEvent);

  // --- Progress Bar Logic ---
  const G4int evtNb = anEvent->GetEventID();
  const G4int totNbOfEvt = G4RunManager::GetRunManager()->GetCurrentRun()
                           ->GetNumberOfEventToBeProcessed();

  if (evtNb == 0) {
    G4cout << "\nSimulating " << totNbOfEvt << " events." << G4endl;
  } 

  if (totNbOfEvt > ProgressBarLimit) {
    if (percentageFactor*(evtNb) % totNbOfEvt == 0) {
      ShowProgressBar(evtNb, totNbOfEvt);
    }
    else if (evtNb == totNbOfEvt - 1) {
      ShowProgressBar(totNbOfEvt, totNbOfEvt);
    }
  }
}

void NucTechPrimaryGeneratorAction::ShowProgressBar(const int64_t evtID, const G4int totEvt)
{
  const G4double progress = static_cast<G4double>(evtID) / totEvt;
  const auto pos = static_cast<G4int>(BAR_WIDTH * progress);
  G4cout << "[";
  for (G4int i = 0; i < BAR_WIDTH; i++) {
    if (i < pos) G4cout << "=";
    else if (i == pos) G4cout << ">"; 
    else G4cout << " ";
  }
  G4cout << "] " << static_cast<G4int>(percentageFactor * progress) << "% \r" << std::flush;
}