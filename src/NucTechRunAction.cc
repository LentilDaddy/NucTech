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
  
  mgr->SetVerboseLevel(1); // Set to 1 temporarily to see confirmation in console
  mgr->SetNtupleMerging(true);

  // --- STEP 1: DEFINE STRUCTURE FIRST ---
  // We define the Ntuple before opening the file so the manager 
  // knows exactly what the "schema" of the ROOT file looks like.
  mgr->SetFirstNtupleId(1); 
  mgr->CreateNtuple("EnergySpectrum", "NucTech");
  mgr->CreateNtupleIColumn("ReactionCount"); 
  mgr->FinishNtuple();

  // --- STEP 2: OPEN THE FILE ---
  G4bool fileOpen = mgr->OpenFile(fOutputFileName);

  if (fileOpen)
  {
    if (IsMaster())
    {
      G4cout << "### RunAction: Opening output file: " << mgr->GetFileName() << G4endl;
    }
  }
  else
  {
    std::string errorMessage = "Could not open the output file: " + fOutputFileName;
    G4Exception(
        "NucTechRunAction::BeginOfRunAction",
        "FileOpenError",
        FatalException,
        errorMessage.c_str());
  }

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
  
  // Write() commits the data in the buffers to the file
  mgr->Write();
  
  // CloseFile() ensures the ROOT file is finalized and readable
  mgr->CloseFile();
  
  // Clear() prevents memory leaks and issues if multiple runs occur
  mgr->Clear();
}