#include "NucTechRunAction.hh"
#include "NucTechSteppingAction.hh"

#include "G4AnalysisManager.hh"
#include "G4Run.hh"

#include <cmath>

NucTechRunAction::NucTechRunAction(const G4String& outFileName) 
: fOutputFileName(outFileName)
{}

NucTechRunAction::~NucTechRunAction() = default;

void NucTechRunAction::BeginOfRunAction(const G4Run* /*run*/)
{
  NucTechSteppingAction::ResetDiagnostics();

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
  mgr->CreateNtuple("EnergySpectrum", "NucTech");
  // // mgr->CreateNtupleFColumn("EventEdep"); //total energy deposited PER EVENT
  // // // mgr->CreateNtupleDColumn("EventEdepDetector2"); // Add this branch
  // // // mgr->CreateNtupleDColumn("FoilThickness");      // Add this branch
  mgr->CreateNtupleIColumn("ReactionCount"); //total reaction count PER EVENT
  mgr->FinishNtuple();
  
  // mgr->CreateNtuple("IndividualHits", "NucTech");
  // // mgr->CreateNtupleDColumn("HitEdep");

  // mgr->CreateNtupleFColumn("HitZ");
  // // mgr->CreateNtupleFColumn("HitX"); //positions of each hit
  // // mgr->CreateNtupleFColumn("HitY");
  // // mgr->CreateNtupleFColumn("HitR");
  // // mgr->CreateNtupleDColumn("HitPx");
  // // mgr->CreateNtupleDColumn("HitPy");
  // // mgr->CreateNtupleDColumn("HitPz");
  // // mgr->CreateNtupleDColumn("HitTime"); // <-- Add this line
  // // mgr->CreateNtupleIColumn("HitPDG"); // <-- Add this line for PDG code
  // // mgr->CreateNtupleFColumn("HitKineticEnergy");
  // // mgr->CreateNtupleIColumn("HitParentID"); // <-- Add this line for Parent ID
  // // mgr->CreateNtupleIColumn("ReactionCount");
  
  
  // mgr->FinishNtuple();

  if (IsMaster())
  {
    fTimer.Start();
  } 
}

void NucTechRunAction::EndOfRunAction(const G4Run* run)
{
  if (IsMaster())
  {
    fTimer.Stop();
    G4cout << "\nReal simulation time = "
           << fTimer.GetRealElapsed() << "s \n" << G4endl;

    const long long nEvents = static_cast<long long>(run->GetNumberOfEvent());
    const long long photonuclearSteps = NucTechSteppingAction::GetTotalPhotonuclearSteps();
    const long long taggedReactions = NucTechSteppingAction::GetTotalTaggedReactions();
    const long long eventsWithPhotonuclear = NucTechSteppingAction::GetEventsWithPhotonuclear();
    const long long eventsWithTaggedReaction = NucTechSteppingAction::GetEventsWithTaggedReaction();
    const long long eventsWithAnyF18 = NucTechSteppingAction::GetEventsWithAnyF18();
    const long long eventsWithPhotonuclearAndAnyF18 = NucTechSteppingAction::GetEventsWithPhotonuclearAndAnyF18();
    const long long channelNF18 = NucTechSteppingAction::GetChannelNF18();
    const long long channelN15Alpha = NucTechSteppingAction::GetChannelN15Alpha();
    const long long channelO18Proton = NucTechSteppingAction::GetChannelO18Proton();
    const long long channelThreeAlpha = NucTechSteppingAction::GetChannelThreeAlpha();
    const long long channelF19GammaLike = NucTechSteppingAction::GetChannelF19GammaLike();
    const long long channelOther = NucTechSteppingAction::GetChannelOther();

    G4double pEventPhotonuclear = 0.;
    G4double pEventTagged = 0.;
    G4double pEventAnyF18 = 0.;
    G4double pEventPhotonuclearAndAnyF18 = 0.;
    G4double meanTaggedPerPrimary = 0.;
    if (nEvents > 0)
    {
      const G4double nEventsAsDouble = static_cast<G4double>(nEvents);
      pEventPhotonuclear = static_cast<G4double>(eventsWithPhotonuclear) / nEventsAsDouble;
      pEventTagged = static_cast<G4double>(eventsWithTaggedReaction) / nEventsAsDouble;
      pEventAnyF18 = static_cast<G4double>(eventsWithAnyF18) / nEventsAsDouble;
      pEventPhotonuclearAndAnyF18 = static_cast<G4double>(eventsWithPhotonuclearAndAnyF18) / nEventsAsDouble;
      meanTaggedPerPrimary = static_cast<G4double>(taggedReactions) / nEventsAsDouble;
    }

    const auto printChannel = [&](const char* label, const long long count) {
      G4double frac = 0.;
      G4double err = 0.;
      if (photonuclearSteps > 0)
      {
        const G4double n = static_cast<G4double>(photonuclearSteps);
        frac = static_cast<G4double>(count) / n;
        err = std::sqrt(frac * (1. - frac) / n);
      }
      G4cout << "    " << label << ": " << count
             << "  frac=" << frac
             << " +/- " << err << G4endl;
    };

    G4cout << "Photonuclear diagnostics:" << G4endl
           << "  Primaries (events): " << nEvents << G4endl
           << "  Photonuclear process steps: " << photonuclearSteps << G4endl
           << "  Events with >=1 photonuclear step: " << eventsWithPhotonuclear << G4endl
           << "  Strict tagged reactions (n + 18F): " << taggedReactions << G4endl
           << "  Events with >=1 strict tag: " << eventsWithTaggedReaction << G4endl
           << "  Events with >=1 F18 track (any origin): " << eventsWithAnyF18 << G4endl
           << "  Events with photonuclear step AND any F18 track: " << eventsWithPhotonuclearAndAnyF18 << G4endl
           << "  P(event has photonuclear step): " << pEventPhotonuclear << G4endl
           << "  P(event has strict tag): " << pEventTagged << G4endl
           << "  P(event has any F18): " << pEventAnyF18 << G4endl
           << "  P(event has photonuclear AND any F18): " << pEventPhotonuclearAndAnyF18 << G4endl
           << "  Mean strict tags per primary: " << meanTaggedPerPrimary << G4endl
              << "  Photonuclear channel summary (per process step):" << G4endl;

            printChannel("n + F18", channelNF18);
            printChannel("N15 + alpha", channelN15Alpha);
            printChannel("O18 + proton", channelO18Proton);
            printChannel("3 alpha", channelThreeAlpha);
            printChannel("F19 + gammas (elastic-like)", channelF19GammaLike);
            printChannel("other", channelOther);

            G4cout
           << G4endl;
  }

  G4AnalysisManager* mgr = G4AnalysisManager::Instance();
  mgr->Write();
  mgr->CloseFile();
  mgr->Clear();
}
