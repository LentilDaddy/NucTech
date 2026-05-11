#ifndef AnnihilationPhotonsSteppingAction_hh
#define AnnihilationPhotonsSteppingAction_hh

#include "NucTechVSteppingAction.hh"

#include "G4ThreeVector.hh"

#include <atomic>
#include <numeric>
#include <set>
#include <vector>

class NucTechSteppingAction: public NucTechVSteppingAction
{
public:
  NucTechSteppingAction();
  // NucTechSteppingAction(G4double foilThickness);
  virtual void BeginOfEventAction();
  virtual void UserSteppingAction(const G4Step*);
  virtual void EndOfEventAction();

  void CheckPhotonuclearReaction(const G4Step* step);

  static void ResetDiagnostics();
  static long long GetTotalPhotonuclearSteps();
  static long long GetTotalTaggedReactions();
  static long long GetEventsWithPhotonuclear();
  static long long GetEventsWithTaggedReaction();
  static long long GetEventsWithAnyF18();
  static long long GetEventsWithPhotonuclearAndAnyF18();
  static long long GetChannelNF18();
  static long long GetChannelN15Alpha();
  static long long GetChannelO18Proton();
  static long long GetChannelThreeAlpha();
  static long long GetChannelF19GammaLike();
  static long long GetChannelOther();

private:
  // These are used to remember quantities from call to call of
  // UserSteppingAction
  // std::vector<G4float> fV_hitEdep;
  // std::vector<G4ThreeVector> fV_hitPos;
  // std::vector<G4ThreeVector> fV_hitMomentum;
  // std::vector<G4double> fV_hitTime;
  // std::vector<G4float> fV_KineticEnergy;
  // G4double fEdepDetector2;
  // G4double fFoilThickness;
  // std::vector<int> fV_hitPDG;
  // G4int fPrimariesDetector1;
  // std::set<G4int> fPrimariesDetector1;
  // std::vector<int> fV_hitParentID;
  int HitReactionCount;
  int fPhotonuclearStepsThisEvent;
  bool fHasF18ThisEvent;

  static std::atomic<long long> sTotalPhotonuclearSteps;
  static std::atomic<long long> sTotalTaggedReactions;
  static std::atomic<long long> sEventsWithPhotonuclear;
  static std::atomic<long long> sEventsWithTaggedReaction;
  static std::atomic<long long> sEventsWithAnyF18;
  static std::atomic<long long> sEventsWithPhotonuclearAndAnyF18;
  static std::atomic<long long> sChannelNF18;
  static std::atomic<long long> sChannelN15Alpha;
  static std::atomic<long long> sChannelO18Proton;
  static std::atomic<long long> sChannelThreeAlpha;
  static std::atomic<long long> sChannelF19GammaLike;
  static std::atomic<long long> sChannelOther;
  static std::atomic<int> sPrintedPhotonuclearEvents;

};

#endif
