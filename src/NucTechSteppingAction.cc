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

// NucTechSteppingAction::NucTechSteppingAction()
//     : fV_hitEdep(), fV_hitPos(), fV_hitPDG(), fV_KineticEnergy(), fV_hitParentID(), HitReactionCount(0)
// {}

std::atomic<long long> NucTechSteppingAction::sTotalPhotonuclearSteps{0};
std::atomic<long long> NucTechSteppingAction::sTotalTaggedReactions{0};
std::atomic<long long> NucTechSteppingAction::sEventsWithPhotonuclear{0};
std::atomic<long long> NucTechSteppingAction::sEventsWithTaggedReaction{0};
std::atomic<long long> NucTechSteppingAction::sEventsWithAnyF18{0};
std::atomic<long long> NucTechSteppingAction::sEventsWithPhotonuclearAndAnyF18{0};
std::atomic<long long> NucTechSteppingAction::sChannelNO15{0};
std::atomic<long long> NucTechSteppingAction::sChannelC12Alpha{0};
std::atomic<long long> NucTechSteppingAction::sChannelPN15{0};
std::atomic<long long> NucTechSteppingAction::sChannelThreeAlpha{0};
std::atomic<long long> NucTechSteppingAction::sChannelOther{0};
std::atomic<int> NucTechSteppingAction::sPrintedPhotonuclearEvents{0};

namespace {
constexpr int kMaxPhotonuclearEventPrints = 15;
}

NucTechSteppingAction::NucTechSteppingAction()
    : HitReactionCount(0),
  fPhotonuclearStepsThisEvent(0),
  fHasO15ThisEvent(false)
{}

void NucTechSteppingAction::ResetDiagnostics()
{
  sTotalPhotonuclearSteps.store(0, std::memory_order_relaxed);
  sTotalTaggedReactions.store(0, std::memory_order_relaxed);
  sEventsWithPhotonuclear.store(0, std::memory_order_relaxed);
  sEventsWithTaggedReaction.store(0, std::memory_order_relaxed);
  sEventsWithAnyF18.store(0, std::memory_order_relaxed);
  sEventsWithPhotonuclearAndAnyF18.store(0, std::memory_order_relaxed);
  sChannelNO15.store(0, std::memory_order_relaxed);
  sChannelC12Alpha.store(0, std::memory_order_relaxed);
  sChannelPN15.store(0, std::memory_order_relaxed);
  sChannelThreeAlpha.store(0, std::memory_order_relaxed);
  sChannelOther.store(0, std::memory_order_relaxed);
  sPrintedPhotonuclearEvents.store(0, std::memory_order_relaxed);
}

long long NucTechSteppingAction::GetTotalPhotonuclearSteps()
{
  return sTotalPhotonuclearSteps.load(std::memory_order_relaxed);
}

long long NucTechSteppingAction::GetTotalTaggedReactions()
{
  return sTotalTaggedReactions.load(std::memory_order_relaxed);
}

long long NucTechSteppingAction::GetEventsWithPhotonuclear()
{
  return sEventsWithPhotonuclear.load(std::memory_order_relaxed);
}

long long NucTechSteppingAction::GetEventsWithTaggedReaction()
{
  return sEventsWithTaggedReaction.load(std::memory_order_relaxed);
}

long long NucTechSteppingAction::GetEventsWithAnyF18()
{
  return sEventsWithAnyF18.load(std::memory_order_relaxed);
}

long long NucTechSteppingAction::GetEventsWithPhotonuclearAndAnyF18()
{
  return sEventsWithPhotonuclearAndAnyF18.load(std::memory_order_relaxed);
}

long long NucTechSteppingAction::GetChannelNO15()
{
  return sChannelNO15.load(std::memory_order_relaxed);
}

long long NucTechSteppingAction::GetChannelC12Alpha()
{
  return sChannelC12Alpha.load(std::memory_order_relaxed);
}

long long NucTechSteppingAction::GetChannelPN15()
{
  return sChannelPN15.load(std::memory_order_relaxed);
}

long long NucTechSteppingAction::GetChannelThreeAlpha()
{
  return sChannelThreeAlpha.load(std::memory_order_relaxed);
}

long long NucTechSteppingAction::GetChannelOther()
{
  return sChannelOther.load(std::memory_order_relaxed);
}


void NucTechSteppingAction::BeginOfEventAction() {
  // fV_hitEdep.clear();
  // fV_hitPos.clear();
  // // fV_hitMomentum.clear();
  // // fV_hitTime.clear();
  // fV_hitPDG.clear();
  // fV_KineticEnergy.clear();
  // fV_hitParentID.clear();
  HitReactionCount = 0;
  fPhotonuclearStepsThisEvent = 0;
  fHasO15ThisEvent = false;
}

void NucTechSteppingAction::EndOfEventAction() {
  sTotalPhotonuclearSteps.fetch_add(fPhotonuclearStepsThisEvent, std::memory_order_relaxed);
  sTotalTaggedReactions.fetch_add(HitReactionCount, std::memory_order_relaxed);

  if (fPhotonuclearStepsThisEvent > 0)
  {
    sEventsWithPhotonuclear.fetch_add(1, std::memory_order_relaxed);
  }

  if (HitReactionCount > 0)
  {
    sEventsWithTaggedReaction.fetch_add(1, std::memory_order_relaxed);
  }

  if (fHasO15ThisEvent)
  {
    sEventsWithAnyF18.fetch_add(1, std::memory_order_relaxed);
    if (fPhotonuclearStepsThisEvent > 0)
    {
      sEventsWithPhotonuclearAndAnyF18.fetch_add(1, std::memory_order_relaxed);
    }
  }

  // Check at least one phantom hit happened during the event
  // if (fV_hitEdep.empty()) //why only for hitEdep?
  //   return;

  if (HitReactionCount < 1)
    return;

  G4AnalysisManager *mgr = G4AnalysisManager::Instance();

  // // Store the total nergy deposited in the event
  // const G4float Edep_event =
  //     std::accumulate(fV_hitEdep.begin(), fV_hitEdep.end(), 0.);

// Fill Ntuple 1 (EnergySpectrum)
  mgr->FillNtupleIColumn(1, 0, HitReactionCount); 
  mgr->AddNtupleRow(1);

  // // Then record the individual hit energy and coordinates of this event
  // for (std::size_t i = 0; i < fV_hitEdep.size(); i++) {
  //   // auto energy = fV_hitEdep[i] / MeV;
  //   auto position = fV_hitPos[i];
  //   G4float z = static_cast<G4float>(position.z() / cm);
  //   // G4float x = static_cast<G4float>(position.x() / cm);
  //   // G4float y = static_cast<G4float>(position.y() / cm);
  //   // G4float r = static_cast<G4float>(position.perp() / cm);

  //   // auto time = fV_hitTime[i] / ns;
  //   // auto kinEnergy = fV_KineticEnergy[i] / MeV;
  //   // auto momentum = fV_hitMomentum[i];

  //   // mgr->FillNtupleDColumn(2, 0, energy);
  //   // mgr->FillNtupleDColumn(2, 1, position.x() / cm);
  //   // mgr->FillNtupleDColumn(2, 2, position.y() / cm);
  //   // mgr->FillNtupleFColumn(2, 0, position.z() / cm);
  //   mgr->FillNtupleFColumn(2, 0, z); // Using ID 2
  //   // mgr->FillNtupleFColumn(1, 1, x);
  //   // mgr->FillNtupleFColumn(1, 2, y);
  //   // mgr->FillNtupleFColumn(1, 1, r);
  //   // mgr->FillNtupleDColumn(2, 2, momentum.x() / (MeV));
  //   // mgr->FillNtupleDColumn(2, 3, momentum.y() / (MeV));
  //   // mgr->FillNtupleDColumn(2, 2, momentum.z() / (MeV));
  //   // mgr->FillNtupleDColumn(2, 3, time / ns);
  //   // mgr->FillNtupleIColumn(1, 2, fV_hitPDG[i]); // Assuming column 5 is for PDG code
  //   // mgr->FillNtupleFColumn(1, 3, kinEnergy);
  //   // mgr->FillNtupleIColumn(1, 4, fV_hitParentID[i]); // Assuming column 6 is for Parent ID
  //   // mgr->FillNtupleIColumn(1, 5, fReactionCount); //i think it is filling this with 0 values... 
  //   mgr->AddNtupleRow(2);
  // }
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

// if (currentName != "Detector1" &&
//     currentName != "Detector2" &&
//     motherName != "Detector1" && currentName != "vacuumLayer"
//   && currentName != "stainlessSteel"){
//     return;
//     }

if (currentName != "Detector1"){
    return;
    }

    // if (currentName != "Detector1" &&
    // currentName != "Detector2" &&
    // motherName != "Detector1"){
    // return;
    // }


  // if (postStepPoint->GetPhysicalVolume()->GetName() != "Detector1" &&
  //     postStepPoint->GetPhysicalVolume()->GetName() != "Detector2")
  //   return;



  // // Store the energy deposited in the phantom
  // const G4float Edep = step->GetTotalEnergyDeposit();

  // if (Edep < 0.) { //changed from <= to < to include 0 energy deposits
  //   return;
  // }

  // fV_hitEdep.push_back(Edep);

  // // Store the hit position
  // const G4ThreeVector hitPos = postStepPoint->GetPosition();
  // fV_hitPos.push_back(hitPos);

  // const G4ThreeVector hitMomentum = postStepPoint->GetMomentum();
  // fV_hitMomentum.push_back(hitMomentum);

    // Store the hit time
  // const G4double hitTime = step->GetPostStepPoint()->GetGlobalTime();
  // fV_hitTime.push_back(hitTime);

  // const G4float kinEnergy = step->GetPostStepPoint()->GetKineticEnergy();
  // fV_KineticEnergy.push_back(kinEnergy);

    // Store PDG code
  const G4Track* track = step->GetTrack();
  if (track)
  {
    const G4ParticleDefinition* def = track->GetDefinition();
    if (def && def->GetAtomicNumber() == 8 && def->GetBaryonNumber() == 15)
    {
      fHasO15ThisEvent = true;
    }
  }
  // const G4ParticleDefinition* pd = track->GetDefinition();
  // // int pdgCode = track->GetDefinition()->GetPDGEncoding();
  // int particleType = 2; // default: other
  // int ParentID = track->GetParentID();

  // fV_hitPDG.push_back(pdgCode);

  // if (pd == G4Electron::ElectronDefinition()) {
  //   particleType = 0;
  // } else if (pd == G4Gamma::GammaDefinition()) {
  //   particleType = 1;
  // }
  // fV_hitPDG.push_back(particleType);
  // fV_hitParentID.push_back(ParentID);


  CheckPhotonuclearReaction(step);

}

void NucTechSteppingAction::CheckPhotonuclearReaction(const G4Step* step) {
  const G4VProcess* process = step->GetPostStepPoint()->GetProcessDefinedStep();
  if (!process) return;
  
  G4String processName = process->GetProcessName();
  if (processName.find("photonNuclear") == std::string::npos && 
      processName.find("PhotoNuclear") == std::string::npos) { 
        return;//if BOTH are NOT found, return.
  }

  ++fPhotonuclearStepsThisEvent;

  //std::string::npos means NOT found
    
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

  if (sPrintedPhotonuclearEvents.load(std::memory_order_relaxed) < kMaxPhotonuclearEventPrints)
  {
    const int printIndex = sPrintedPhotonuclearEvents.fetch_add(1, std::memory_order_relaxed);
    if (printIndex < kMaxPhotonuclearEventPrints)
    {
      G4int eventId = -1;
      const G4Event* evt = G4RunManager::GetRunManager()->GetCurrentEvent();
      if (evt) eventId = evt->GetEventID();

      G4cout << "Photonuclear event dump " << (printIndex + 1)
             << " | event=" << eventId
             << " | process=" << processName
             << " | nSecondaries=" << secondaries->size() << G4endl;

      for (const auto* secondary : *secondaries)
      {
        if (!secondary) continue;
        const G4ParticleDefinition* secDef = secondary->GetDefinition();
        const G4VProcess* creator = secondary->GetCreatorProcess();
        const G4String creatorName = creator ? creator->GetProcessName() : "none";
        G4cout << "  sec=" << secDef->GetParticleName()
               << " Z=" << secDef->GetAtomicNumber()
               << " A=" << secDef->GetBaryonNumber()
               << " creator=" << creatorName << G4endl;
      }
    }
  }

  bool hasNeutron = false;
  bool hasFluorine18 = false;
  bool hasNitrogen15 = false;
  bool hasOxygen18 = false;
  bool hasOxygen15 = false;
  bool hasCarbon12 = false;
  bool hasProton = false;
  int gammaCount = 0;
  int alphaCount = 0;

  for (const auto* secondary : *secondaries) {
    G4int Z = secondary->GetDefinition()->GetAtomicNumber();
    G4int A = secondary->GetDefinition()->GetBaryonNumber();
    
    // Check for neutron (Z=0, A=1)
    if (secondary->GetDefinition() == G4Neutron::NeutronDefinition()) {
        hasNeutron = true;
    }

    else if (secondary->GetDefinition()->GetParticleName() == "proton") {
      hasProton = true;
    }

    else if (secondary->GetDefinition()->GetParticleName() == "gamma") {
      ++gammaCount;
    }

    else if (secondary->GetDefinition()->GetParticleName() == "alpha") {
      ++alphaCount;
    }
    
    // Check for 18F nucleus (Z=9, A=18)
    if (Z == 9 && A == 18) {
        hasFluorine18 = true;
    }

    else if (Z == 7 && A == 15) {
      hasNitrogen15 = true;
    }

    else if (Z == 8 && A == 18) {
      hasOxygen18 = true;
    }

    else if (Z == 8 && A == 15) {
      hasOxygen15 = true;
    }

    else if (Z == 6 && A == 12) {
      hasCarbon12 = true;
    }
  }

    if (hasNeutron && hasOxygen15)
    {
      sChannelNO15.fetch_add(1, std::memory_order_relaxed);
      HitReactionCount++;
    }
    else if (hasCarbon12 && alphaCount >= 1)
    {
      sChannelC12Alpha.fetch_add(1, std::memory_order_relaxed);
    }
    else if (hasProton && hasNitrogen15)
    {
      sChannelPN15.fetch_add(1, std::memory_order_relaxed);
    }
    else if (alphaCount >= 3)
    {
      sChannelThreeAlpha.fetch_add(1, std::memory_order_relaxed);
  }
else {
  sChannelOther.fetch_add(1, std::memory_order_relaxed);
}

    // std::cout << "Photonuclear reaction particles A and Z:" << std::endl;
    // for (const auto* secondary : *secondaries) {
    //     G4int Z = secondary->GetDefinition()->GetAtomicNumber();
    //     G4int A = secondary->GetDefinition()->GetBaryonNumber();
    //     std::cout << "Particle: A=" << A << ", Z=" << Z << std::endl;
    // }

      // HitReactionCount++;
      // std::cout << "Count incremented to: " << fReactionCount << std::endl;
  }

