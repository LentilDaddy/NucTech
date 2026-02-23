#ifndef NucTechActionInitialization_h
#define NucTechActionInitialization_h 1

#include "G4VUserActionInitialization.hh"
#include "G4String.hh"

class NucTechActionInitialization : public G4VUserActionInitialization
{
public:
  NucTechActionInitialization(const G4String& outFileName);
  virtual ~NucTechActionInitialization();

  virtual void BuildForMaster() const;
  virtual void Build() const;

private:
  G4String fOutFileName;
};

#endif