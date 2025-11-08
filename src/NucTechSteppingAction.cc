#include "NucTechSteppingAction.hh"

#include "G4AnalysisManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4VProcess.hh"

#include "G4Run.hh"
#include "G4RunManager.hh"

#include "G4Electron.hh"
#include "G4Gamma.hh"


// NucTechSteppingAction::NucTechSteppingAction(G4double foilThickness)
//     : fV_hitEdep(), fV_hitPos(), fV_hitTime(),
//     fFoilThickness(foilThickness), fV_hitPDG(), fV_KineticEnergy(), fV_hitParentID(),
//     fV_hitMomentum()
// {}

NucTechSteppingAction::NucTechSteppingAction()
    : fV_hitEdep(), fV_hitPos(), fV_hitPDG(), fV_KineticEnergy()
{}

void NucTechSteppingAction::BeginOfEventAction() {
  fV_hitEdep.clear();
  fV_hitPos.clear();
  // fV_hitMomentum.clear();
  // fV_hitTime.clear();
  fV_hitPDG.clear();
  fV_KineticEnergy.clear();
  // fV_hitParentID.clear();
}

void NucTechSteppingAction::EndOfEventAction() {
  // Check at least one phantom hit happened during the event
  if (fV_hitEdep.empty()) //why only for hitEdep?
    return;

  G4AnalysisManager *mgr = G4AnalysisManager::Instance();

  // Store the total nergy deposited in the event
  const G4double Edep_event =
      std::accumulate(fV_hitEdep.begin(), fV_hitEdep.end(), 0.);

// Fill the total event energy into the first ntuple ("EnergySpectrum")
  mgr->FillNtupleDColumn(1, 0, Edep_event / MeV);
  // mgr->FillNtupleDColumn(1, 1, fEdepDetector2 / MeV); // Assuming column 1 in ntuple 1
  // mgr->FillNtupleDColumn(1, 1, fFoilThickness / mm); // Assuming column 2 is for thickness
  // mgr->FillNtupleIColumn(1, 2, fPrimariesDetector1.size());
  mgr->AddNtupleRow(1);


  // Then record the individual hit energy and coordinates of this event
  for (std::size_t i = 0; i < fV_hitEdep.size(); i++) {
    auto energy = fV_hitEdep[i] / MeV;
    auto position = fV_hitPos[i];
    // auto time = fV_hitTime[i] / ns;
    auto kinEnergy = fV_KineticEnergy[i] / MeV;
    auto momentum = fV_hitMomentum[i];

    // mgr->FillNtupleDColumn(2, 0, energy);
    // mgr->FillNtupleDColumn(2, 1, position.x() / cm);
    // mgr->FillNtupleDColumn(2, 2, position.y() / cm);
    mgr->FillNtupleDColumn(2, 0, position.z() / cm);
    // mgr->FillNtupleDColumn(2, 2, momentum.x() / (MeV));
    // mgr->FillNtupleDColumn(2, 3, momentum.y() / (MeV));
    // mgr->FillNtupleDColumn(2, 2, momentum.z() / (MeV));
    // mgr->FillNtupleDColumn(2, 3, time / ns);
    mgr->FillNtupleIColumn(2, 1, fV_hitPDG[i]); // Assuming column 5 is for PDG code
    mgr->FillNtupleDColumn(2, 2, kinEnergy);
    // mgr->FillNtupleIColumn(2, 6, fV_hitParentID[i]); // Assuming column 6 is for Parent ID
    mgr->AddNtupleRow(2);
  }
}

void NucTechSteppingAction::UserSteppingAction(const G4Step *step) {
  G4StepPoint *postStepPoint = step->GetPostStepPoint();
  // G4StepPoint *preStepPoint = step->GetPreStepPoint();

  // To only record the interactions inside the phantom
  if (postStepPoint->GetPhysicalVolume() == nullptr)
    return;
  //auto volName = postStepPoint->GetPhysicalVolume()->GetName();



G4TouchableHandle touchable = postStepPoint->GetTouchableHandle();

if (!postStepPoint->GetPhysicalVolume()) return; // extra safety

G4int depth = touchable->GetHistoryDepth();
G4String currentName = touchable->GetVolume(0)->GetName();
G4String motherName = (depth > 1 && touchable->GetVolume(1))
                        ? touchable->GetVolume(1)->GetName()
                        : "";

if (currentName != "Detector1" &&
    currentName != "Detector2" &&
    motherName != "Detector1"){
    return;
    }

  // if (postStepPoint->GetPhysicalVolume()->GetName() != "Detector1" &&
  //     postStepPoint->GetPhysicalVolume()->GetName() != "Detector2")
  //   return;



  // Store the energy deposited in the phantom
  const G4double Edep = step->GetTotalEnergyDeposit();

  if (Edep < 0.) { //changed from <= to < to include 0 energy deposits
    return;
  }

  fV_hitEdep.push_back(Edep);

  // Store the hit position
  const G4ThreeVector hitPos = postStepPoint->GetPosition();
  fV_hitPos.push_back(hitPos);

  // const G4ThreeVector hitMomentum = postStepPoint->GetMomentum();
  // fV_hitMomentum.push_back(hitMomentum);

    // Store the hit time
  // const G4double hitTime = step->GetPostStepPoint()->GetGlobalTime();
  // fV_hitTime.push_back(hitTime);

  const G4float kinEnergy = step->GetPostStepPoint()->GetKineticEnergy();
  fV_KineticEnergy.push_back(kinEnergy);

    // Store PDG code
  const G4Track* track = step->GetTrack();
  const G4ParticleDefinition* pd = track->GetDefinition();
  // int pdgCode = track->GetDefinition()->GetPDGEncoding();
  int particleType = 2; // default: other
  // int ParentID = track->GetParentID();

  // fV_hitPDG.push_back(pdgCode);

  if (pd == G4Electron::ElectronDefinition()) {
    particleType = 0;
  } else if (pd == G4Gamma::GammaDefinition()) {
    particleType = 1;
  }
  fV_hitPDG.push_back(particleType);
  // fV_hitParentID.push_back(ParentID);

}
