#include "NucTechActionInitialization.hh"
#include "NucTechEventAction.hh"
#include "NucTechPrimaryGeneratorAction.hh"
#include "NucTechRunAction.hh"
#include "NucTechSteppingAction.hh"
#include "NucTechDetectorConstruction.hh" // <-- Add this line
#include "G4RunManager.hh"

#include <memory>

NucTechActionInitialization::NucTechActionInitialization(
  const G4String& outFileName)
 : fOutFileName(outFileName)
{}

NucTechActionInitialization::~NucTechActionInitialization() = default;

void NucTechActionInitialization::BuildForMaster() const
{
    // Use unique_ptr to manage memory safely
    auto runAction = std::make_unique<NucTechRunAction>(fOutFileName);

    // Transfer ownership to SetUserAction
    SetUserAction(runAction.release());
}

void NucTechActionInitialization::Build() const
{

    // Get the detector construction instance
    const NucTechDetectorConstruction* detector =
        static_cast<const NucTechDetectorConstruction*>(
            G4RunManager::GetRunManager()->GetUserDetectorConstruction());

    // Get foil thickness from detector construction
    // G4double foilThickness = detector->GetFoilThickness();

    // Use unique_ptr for steppingAction and eventAction
    // auto steppingAction = std::make_unique<NucTechSteppingAction>(foilThickness);
    auto eventAction = std::make_unique<NucTechEventAction>(steppingAction.get());

    // Transfer ownership to SetUserAction
    SetUserAction(std::make_unique<NucTechPrimaryGeneratorAction>().release());
    SetUserAction(std::make_unique<NucTechRunAction>(fOutFileName).release());
    SetUserAction(eventAction.release());
    SetUserAction(steppingAction.release());
}