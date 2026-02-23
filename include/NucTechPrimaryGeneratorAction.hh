#ifndef NucTechPrimaryGeneratorAction_hh
#define NucTechPrimaryGeneratorAction_hh 1

#include "G4GeneralParticleSource.hh"
#include "G4VUserPrimaryGeneratorAction.hh"
#include "globals.hh"
#include <memory>

// Forward declaration of the ROOT class so Geant4 knows it exists
class TH1D;
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

  // This is the pointer that will hold your energy spectrum
  TH1D* fEnergyHist;
  
  static constexpr G4int ProgressBarLimit = 999;
  static constexpr G4int percentageFactor = 100;
  static constexpr G4int BAR_WIDTH = 50;
};

#endif