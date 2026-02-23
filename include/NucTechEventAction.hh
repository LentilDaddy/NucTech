#ifndef NucTechEventAction_hh
#define NucTechEventAction_hh

#include "G4UserEventAction.hh"

// class NucTechRunAction;
class NucTechVSteppingAction;

class NucTechEventAction : public G4UserEventAction
{
public:
  NucTechEventAction(NucTechVSteppingAction*);
  virtual ~NucTechEventAction();

  virtual void BeginOfEventAction(const G4Event*);
  virtual void EndOfEventAction(const G4Event*);

private:
  NucTechVSteppingAction* fpNucTechVSteppingAction;
};

#endif
