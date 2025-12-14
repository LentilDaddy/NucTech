#include "NucTechRunAction.hh"

#include "G4AnalysisManager.hh"
#include "G4Run.hh"

NucTechRunAction::NucTechRunAction(const G4String& outFileName) 
: fOutputFileName(outFileName)
{}

NucTechRunAction::~NucTechRunAction() = default;

void NucTechRunAction::BeginOfRunAction(const G4Run* /*run*/)
{
  G4AnalysisManager* mgr = G4AnalysisManager::Instance();
  
  mgr->SetVerboseLevel(0);
  mgr->SetNtupleMerging(true);

  const G4bool fileOpen = mgr->OpenFile(fOutputFileName);

  if (fileOpen)
  {
    if (IsMaster())
    {
      G4cout << "Opening: " << mgr->GetFileName() << G4endl;
    }
  }
  
  else
  {
    std::string errorMessage = "Could not open the output file: " + mgr->GetFileName();
    G4Exception(
        "NucTechRunAction::BeginOfRunAction",
        "FileOpenError",
        FatalException,
        errorMessage.c_str());
  }

  mgr->SetFirstNtupleId(1); //nTuple is a root tree. This part only defines the structure, it doesn't fill the variables.
  // mgr->CreateNtuple("EnergySpectrum", "NucTech");
  // mgr->CreateNtupleFColumn("EventEdep"); //total energy deposited PER EVENT
  // // mgr->CreateNtupleDColumn("EventEdepDetector2"); // Add this branch
  // // mgr->CreateNtupleDColumn("FoilThickness");      // Add this branch
  // // mgr->CreateNtupleIColumn("PrimariesDetector1"); // Add this branch
  // mgr->FinishNtuple();
  
  mgr->CreateNtuple("IndividualHits", "NucTech");
  // mgr->CreateNtupleDColumn("HitEdep");

  mgr->CreateNtupleFColumn("HitZ");
  // mgr->CreateNtupleFColumn("HitX"); //positions of each hit
  // mgr->CreateNtupleFColumn("HitY");
  // mgr->CreateNtupleDColumn("HitPx");
  // mgr->CreateNtupleDColumn("HitPy");
  // mgr->CreateNtupleDColumn("HitPz");
  // mgr->CreateNtupleDColumn("HitTime"); // <-- Add this line
  mgr->CreateNtupleIColumn("HitPDG"); // <-- Add this line for PDG code
  mgr->CreateNtupleFColumn("HitKineticEnergy");
  // mgr->CreateNtupleIColumn("HitParentID"); // <-- Add this line for Parent ID
  
  
  mgr->FinishNtuple();

  if (IsMaster())
  {
    fTimer.Start();
  } 
}

void NucTechRunAction::EndOfRunAction(const G4Run* /*run*/)
{
  if (IsMaster())
  {
    fTimer.Stop();
    G4cout << "\nReal simulation time = "
           << fTimer.GetRealElapsed() << "s \n" << G4endl;
  }

  G4AnalysisManager* mgr = G4AnalysisManager::Instance();
  mgr->Write();
  mgr->CloseFile();
  mgr->Clear();
}
