#include "NucTechEventAction.hh"
#include "NucTechVSteppingAction.hh"

NucTechEventAction::NucTechEventAction
(NucTechVSteppingAction* onePhotonSteppingAction)
: fpNucTechVSteppingAction(onePhotonSteppingAction)
{}

NucTechEventAction::~NucTechEventAction() = default;

void NucTechEventAction::BeginOfEventAction(const G4Event* /* event*/)
{
  fpNucTechVSteppingAction->BeginOfEventAction();
}

void NucTechEventAction::EndOfEventAction(const G4Event* /* event*/)
{
  fpNucTechVSteppingAction->EndOfEventAction();
}
