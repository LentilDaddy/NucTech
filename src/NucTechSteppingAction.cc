#include "NucTechSteppingAction.hh"

#include <algorithm>

#include "G4AnalysisManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4VProcess.hh"
#include "G4Track.hh"
#include "G4StepPoint.hh"

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
std::atomic<long long> NucTechSteppingAction::sChannelNF18{0};
std::atomic<long long> NucTechSteppingAction::sChannelN15Alpha{0};
std::atomic<long long> NucTechSteppingAction::sChannelO18Proton{0};
std::atomic<long long> NucTechSteppingAction::sChannelThreeAlpha{0};
std::atomic<long long> NucTechSteppingAction::sChannelF19GammaLike{0};
std::atomic<long long> NucTechSteppingAction::sChannelOther{0};
std::atomic<int> NucTechSteppingAction::sPrintedPhotonuclearEvents{0};

namespace {
constexpr int kMaxPhotonuclearEventPrints = 15;
}

NucTechSteppingAction::NucTechSteppingAction()
    : HitReactionCount(0),
      fPhotonuclearStepsThisEvent(0),
      fHasF18ThisEvent(false),
      fPrimaryEnteredTarget(false),
      fPrimaryEntryTrackLength(0.)
{}

void NucTechSteppingAction::ResetDiagnostics()
{
  sTotalPhotonuclearSteps.store(0, std::memory_order_relaxed);
  sTotalTaggedReactions.store(0, std::memory_order_relaxed);
  sEventsWithPhotonuclear.store(0, std::memory_order_relaxed);
  sEventsWithTaggedReaction.store(0, std::memory_order_relaxed);
  sEventsWithAnyF18.store(0, std::memory_order_relaxed);
  sEventsWithPhotonuclearAndAnyF18.store(0, std::memory_order_relaxed);
  sChannelNF18.store(0, std::memory_order_relaxed);
  sChannelN15Alpha.store(0, std::memory_order_relaxed);
  sChannelO18Proton.store(0, std::memory_order_relaxed);
  sChannelThreeAlpha.store(0, std::memory_order_relaxed);
  sChannelF19GammaLike.store(0, std::memory_order_relaxed);
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

long long NucTechSteppingAction::GetChannelNF18()
{
  return sChannelNF18.load(std::memory_order_relaxed);
}

long long NucTechSteppingAction::GetChannelN15Alpha()
{
  return sChannelN15Alpha.load(std::memory_order_relaxed);
}

long long NucTechSteppingAction::GetChannelO18Proton()
{
  return sChannelO18Proton.load(std::memory_order_relaxed);
}

long long NucTechSteppingAction::GetChannelThreeAlpha()
{
  return sChannelThreeAlpha.load(std::memory_order_relaxed);
}

long long NucTechSteppingAction::GetChannelF19GammaLike()
{
  return sChannelF19GammaLike.load(std::memory_order_relaxed);
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
  fHasF18ThisEvent = false;
  fPrimaryEnteredTarget = false;
  fPrimaryEntryTrackLength = 0.;
  fV_F18KineticEnergy.clear();
  fFirstF18KineticEnergy = -1.0;
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

  if (fHasF18ThisEvent)
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

}

void NucTechSteppingAction::UserSteppingAction(const G4Step *step) {
  G4Track *track = step->GetTrack();
  
  // 1. Primary Proton Filter
  if (track->GetTrackID() != 1 || track->GetDefinition()->GetPDGEncoding() != 2212)
    return;

  G4StepPoint *preStepPoint = step->GetPreStepPoint();
  G4StepPoint *postStepPoint = step->GetPostStepPoint();

  if (!preStepPoint || !preStepPoint->GetPhysicalVolume())
    return;

  if (preStepPoint->GetPhysicalVolume()->GetName() != "Detector1")
    return;

  const G4double stepLength = step->GetStepLength();
  if (stepLength <= 0.)
    return;

  const G4double deltaEnergy = (preStepPoint->GetKineticEnergy() - postStepPoint->GetKineticEnergy()) / MeV;
  if (deltaEnergy <= 0.) return;

  // 2. Compute path length along the local target Z-axis 
  // (Assuming Detector1 is placed along Z, using local position avoids tracking errors)
  // G4ThreePosition localPos = preStepPoint->GetTouchableHandle()->GetHistory()
  //                               ->GetTopTransform().TransformPoint(preStepPoint->GetPosition());
  
  // Alternative: If particle enters target at z_entry, pathLength = (preStepPoint->GetPosition().z() - z_entry) / cm
  G4double pathLength = (track->GetTrackLength() - stepLength / 2.0) / cm; 

  // 3. Fill H1 with raw energy deposit deltaEnergy (NOT stopping power)
  G4AnalysisManager *mgr = G4AnalysisManager::Instance();
  mgr->FillH1(0, pathLength, deltaEnergy); 
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
  bool hasProton = false;
  bool hasFluorine19 = false;
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
        const G4double ke = secondary->GetKineticEnergy();
        fV_F18KineticEnergy.push_back(ke);
        if (fFirstF18KineticEnergy < 0.0) {
          fFirstF18KineticEnergy = ke;
        }
        G4AnalysisManager::Instance()->FillH1(0, ke / MeV);
    }

    else if (Z == 7 && A == 15) {
      hasNitrogen15 = true;
    }

    else if (Z == 8 && A == 18) {
      hasOxygen18 = true;
    }

    else if (Z == 9 && A == 19) {
      hasFluorine19 = true;
    }
  }

  if (hasNeutron && hasFluorine18)
  {
    sChannelNF18.fetch_add(1, std::memory_order_relaxed);
  }
  else if (alphaCount >= 3)
  {
    sChannelThreeAlpha.fetch_add(1, std::memory_order_relaxed);
  }
  else if (hasNitrogen15 && alphaCount >= 1)
  {
    sChannelN15Alpha.fetch_add(1, std::memory_order_relaxed);
  }
  else if (hasOxygen18 && hasProton)
  {
    sChannelO18Proton.fetch_add(1, std::memory_order_relaxed);
  }
  else if (hasFluorine19 && gammaCount >= 1 && !hasProton && !hasNeutron && alphaCount == 0)
  {
    sChannelF19GammaLike.fetch_add(1, std::memory_order_relaxed);
  }
  else
  {
    sChannelOther.fetch_add(1, std::memory_order_relaxed);
  }

  // If we have both products, the reaction occurred (implies target was 19F)
  if (hasNeutron && hasFluorine18) {
    // //print all particle types produced in the reaction
    // std::cout << "Photonuclear reaction particles A and Z:" << std::endl;
    // for (const auto* secondary : *secondaries) {
    //     G4int Z = secondary->GetDefinition()->GetAtomicNumber();
    //     G4int A = secondary->GetDefinition()->GetBaryonNumber();
    //     std::cout << "Particle: A=" << A << ", Z=" << Z << std::endl;
    // }

      HitReactionCount++;
      // std::cout << "Count incremented to: " << fReactionCount << std::endl;
  }
  else {
    // std::cout << "Photonuclear reaction did not produce both neutron and 18F." << std::endl;
    // std::cout << "Count incremented to: " << fReactionCount << std::endl;
    return;
  }
}