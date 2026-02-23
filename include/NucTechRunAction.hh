#ifndef NucTechRunAction_h
#define NucTechRunAction_h 1

#include "globals.hh"
#include "G4UserRunAction.hh"
#include "G4Timer.hh"
#include <vector>

class NucTechSteppingAction;

class NucTechRunAction : public G4UserRunAction
{
public:
  // Constructor to accept stepping action and output filename
  NucTechRunAction(const G4String& outFileName);
  virtual ~NucTechRunAction();

  virtual void BeginOfRunAction(const G4Run*);
  virtual void EndOfRunAction(const G4Run*);

private:
  G4Timer fTimer;

  // Member variable to store the output filename
  G4String fOutputFileName;
};

#endif