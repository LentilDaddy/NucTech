#include "NucTechPrimaryGeneratorAction.hh"

#include "G4GeneralParticleSource.hh"
#include "G4Run.hh"
#include "G4RunManager.hh"

NucTechPrimaryGeneratorAction::NucTechPrimaryGeneratorAction()
  : fGPS(std::make_unique<G4GeneralParticleSource>()) 
{}

NucTechPrimaryGeneratorAction::~NucTechPrimaryGeneratorAction() = default;

void NucTechPrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
  fGPS->GeneratePrimaryVertex(anEvent);

  // The following is to display the progress bar
  const G4int evtNb = anEvent->GetEventID();

  const G4int totNbOfEvt = G4RunManager::GetRunManager()->GetCurrentRun()
                           ->GetNumberOfEventToBeProcessed();

  if (evtNb == 0)
  {
    G4cout << "\n" << G4endl;
    G4cout << "Simulating " << totNbOfEvt << " events." << G4endl;
  } 

  if (totNbOfEvt > ProgressBarLimit)
  {
    if (percentageFactor*(evtNb) % totNbOfEvt == 0)
    {
      ShowProgressBar(evtNb, totNbOfEvt);
    }

    else if (evtNb == totNbOfEvt - 1)
    {
      ShowProgressBar(totNbOfEvt, totNbOfEvt);
    }
  }
}

void NucTechPrimaryGeneratorAction::ShowProgressBar(
  const int64_t evtID, 
  const G4int totEvt)
{
  const G4double progress = static_cast<G4double>(evtID) / totEvt;
  const auto pos = static_cast<G4int>(BAR_WIDTH * progress);
  
  G4cout << "[";

  for (G4int i = 0; i < BAR_WIDTH; i++)
  {
    if (i < pos)
    {
      G4cout << "=";
    }
    else if (i == pos)
    {
      G4cout << ">";
    } 
    else
    {
      G4cout << " ";
    }
  }

  G4cout << "] " << static_cast<G4int>(percentageFactor * progress) << "% \r" 
         << std::flush;
}