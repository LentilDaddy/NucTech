#include "NucTechSteppingAction.hh"

#include "G4AnalysisManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4VProcess.hh"

#include "G4Run.hh"
#include "G4RunManager.hh"

#include "G4Electron.hh"
#include "G4Gamma.hh"
#include "G4Neutron.hh"
// #include "G4LowEGammaNuclearModel.hh"
// #include "G4EmExtraPhysics.hh"
// #include "G4PhotoNuclearCrossSection.hh"

// NucTechSteppingAction::NucTechSteppingAction(G4double foilThickness)
//     : fV_hitEdep(), fV_hitPos(), fV_hitTime(),
//     fFoilThickness(foilThickness), fV_hitPDG(), fV_KineticEnergy(), fV_hitParentID(),
//     fV_hitMomentum()
// {}

NucTechSteppingAction::NucTechSteppingAction()
    : fV_hitEdep(), fV_hitPos(), fV_hitPDG(), fV_KineticEnergy(), fV_hitParentID(), fReactionCount(0)
{}

void NucTechSteppingAction::BeginOfEventAction() {
  fV_hitEdep.clear();
  fV_hitPos.clear();
  // fV_hitMomentum.clear();
  // fV_hitTime.clear();
  fV_hitPDG.clear();
  fV_KineticEnergy.clear();
  fV_hitParentID.clear();
  fReactionCount = 0;
}

void NucTechSteppingAction::EndOfEventAction() {
  // Check at least one phantom hit happened during the event
  if (fV_hitEdep.empty()) //why only for hitEdep?
    return;

  G4AnalysisManager *mgr = G4AnalysisManager::Instance();

  // Store the total nergy deposited in the event
  const G4float Edep_event =
      std::accumulate(fV_hitEdep.begin(), fV_hitEdep.end(), 0.);


  // Then record the individual hit energy and coordinates of this event
  for (std::size_t i = 0; i < fV_hitEdep.size(); i++) {
    // auto energy = fV_hitEdep[i] / MeV;
    auto position = fV_hitPos[i];
    G4float z = static_cast<G4float>(position.z() / cm);
    // G4float x = static_cast<G4float>(position.x() / cm);
    // G4float y = static_cast<G4float>(position.y() / cm);
    G4float r = static_cast<G4float>(position.perp() / cm);

    // auto time = fV_hitTime[i] / ns;
    auto kinEnergy = fV_KineticEnergy[i] / MeV;
    // auto momentum = fV_hitMomentum[i];

    // mgr->FillNtupleDColumn(2, 0, energy);
    // mgr->FillNtupleDColumn(2, 1, position.x() / cm);
    // mgr->FillNtupleDColumn(2, 2, position.y() / cm);
    // mgr->FillNtupleFColumn(2, 0, position.z() / cm);
    mgr->FillNtupleFColumn(1, 0, z);
    // mgr->FillNtupleFColumn(1, 1, x);
    // mgr->FillNtupleFColumn(1, 2, y);
    mgr->FillNtupleFColumn(1, 1, r);
    // mgr->FillNtupleDColumn(2, 2, momentum.x() / (MeV));
    // mgr->FillNtupleDColumn(2, 3, momentum.y() / (MeV));
    // mgr->FillNtupleDColumn(2, 2, momentum.z() / (MeV));
    // mgr->FillNtupleDColumn(2, 3, time / ns);
    mgr->FillNtupleIColumn(1, 2, fV_hitPDG[i]); // Assuming column 5 is for PDG code
    mgr->FillNtupleFColumn(1, 3, kinEnergy);
    mgr->FillNtupleIColumn(1, 4, fV_hitParentID[i]); // Assuming column 6 is for Parent ID
    mgr->FillNtupleIColumn(1, 5, fReactionCount);
    mgr->AddNtupleRow(1);
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
    motherName != "Detector1" && currentName != "vacuumLayer"
  && currentName != "stainlessSteel"){
    return;
    }

// if (currentName != "Detector1" &&
//     currentName != "Detector2" &&
//     motherName != "Detector1"){
//     return;
//     }


  // if (postStepPoint->GetPhysicalVolume()->GetName() != "Detector1" &&
  //     postStepPoint->GetPhysicalVolume()->GetName() != "Detector2")
  //   return;



  // Store the energy deposited in the phantom
  const G4float Edep = step->GetTotalEnergyDeposit();

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
  int ParentID = track->GetParentID();

  // fV_hitPDG.push_back(pdgCode);

  if (pd == G4Electron::ElectronDefinition()) {
    particleType = 0;
  } else if (pd == G4Gamma::GammaDefinition()) {
    particleType = 1;
  }
  fV_hitPDG.push_back(particleType);
  fV_hitParentID.push_back(ParentID);


  CheckPhotonuclearReaction(step);

}

void NucTechSteppingAction::CheckPhotonuclearReaction(const G4Step* step) {
  const G4VProcess* process = step->GetPostStepPoint()->GetProcessDefinedStep();
  if (!process) return;
  
  G4String processName = process->GetProcessName();
  if (processName.find("photonNuclear") != std::string::npos && 
      processName.find("PhotoNuclear") != std::string::npos) {
      return;
  }
    
  // CHECK 1: Verify parent track is a gamma
  const G4Track* track = step->GetTrack();
  if (track->GetDefinition() != G4Gamma::GammaDefinition()) {
      return;
  }

  // CHECK 2 & 3: Look for reaction products (neutron + 18F residual)
  const std::vector<const G4Track*>* secondaries = step->GetSecondaryInCurrentStep();
  if (!secondaries || secondaries->empty()) {
      return;
  }

  bool hasNeutron = false;
  bool hasFluorine18 = false;

  for (const auto* secondary : *secondaries) {
    G4int Z = secondary->GetDefinition()->GetAtomicNumber();
    G4int A = secondary->GetDefinition()->GetBaryonNumber();
    
    // Check for neutron (Z=0, A=1)
    if (secondary->GetDefinition() == G4Neutron::NeutronDefinition()) {
        hasNeutron = true;
    }
    
    // Check for 18F nucleus (Z=9, A=18)
    if (Z == 9 && A == 18) {
        hasFluorine18 = true;
    }
  }
  // If we have both products, the reaction occurred (implies target was 19F)
  if (hasNeutron && hasFluorine18) {
    //print all particle types produced in the reaction
    std::cout << "Photonuclear reaction particles A and Z:" << std::endl;
    for (const auto* secondary : *secondaries) {
        G4int Z = secondary->GetDefinition()->GetAtomicNumber();
        G4int A = secondary->GetDefinition()->GetBaryonNumber();
        std::cout << "Particle: A=" << A << ", Z=" << Z << std::endl;
    }

      fReactionCount++;
      std::cout << "Count incremented to: " << fReactionCount << std::endl;
  }
  else {
    std::cout << "Photonuclear reaction did not produce both neutron and 18F." << std::endl;
    std::cout << "Count incremented to: " << fReactionCount << std::endl;
    return;
  }
}