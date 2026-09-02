#ifndef NucTechPrimaryGeneratorAction_hh
#define NucTechPrimaryGeneratorAction_hh 1

#include "G4GeneralParticleSource.hh"
#include "G4VUserPrimaryGeneratorAction.hh"

class G4GeneralParticleSource;

class NucTechPrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
public:
  NucTechPrimaryGeneratorAction();
  virtual ~NucTechPrimaryGeneratorAction();

  virtual void GeneratePrimaries(G4Event*);
  static void ShowProgressBar(const int64_t evtID, const G4int totEvt);

private:
  std::unique_ptr<G4GeneralParticleSource> fGPS;
  
  static constexpr G4int ProgressBarLimit = 999;
  static constexpr G4int percentageFactor = 100;
  static constexpr G4int BAR_WIDTH = 50;
};

#endif
