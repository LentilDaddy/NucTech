#ifndef NucTechVSteppingAction_hh
#define NucTechVSteppingAction_hh

#include "G4UserSteppingAction.hh"
#include "globals.hh"

class NucTechVSteppingAction : public G4UserSteppingAction
{
public:
  virtual void BeginOfEventAction() {};
  virtual void EndOfEventAction() {};
};

#endif
