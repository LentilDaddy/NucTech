#ifndef AnnihilationPhotonsSteppingAction_hh
#define AnnihilationPhotonsSteppingAction_hh

#include "NucTechVSteppingAction.hh"

#include "G4ThreeVector.hh"

#include <numeric>
#include <vector>
#include <set>

class NucTechSteppingAction: public NucTechVSteppingAction
{
public:
  NucTechSteppingAction();
  // NucTechSteppingAction(G4double foilThickness);
  virtual void BeginOfEventAction();
  virtual void UserSteppingAction(const G4Step*);
  virtual void EndOfEventAction();

private:
  // These are used to remember quantities from call to call of
  // UserSteppingAction
  std::vector<G4float> fV_hitEdep;
  std::vector<G4ThreeVector> fV_hitPos;
  // std::vector<G4ThreeVector> fV_hitMomentum;
  // std::vector<G4double> fV_hitTime;
  std::vector<G4float> fV_KineticEnergy;
  // G4double fEdepDetector2;
  // G4double fFoilThickness;
  std::vector<int> fV_hitPDG;
  // G4int fPrimariesDetector1;
  // std::set<G4int> fPrimariesDetector1;
  std::vector<int> fV_hitParentID;

};

#endif
