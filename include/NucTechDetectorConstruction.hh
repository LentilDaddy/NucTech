#ifndef NUCTECH_DETECTOR_CONSTRUCTION_HH
#define NUCTECH_DETECTOR_CONSTRUCTION_HH

#include "G4GlobalMagFieldMessenger.hh"
#include "G4VUserDetectorConstruction.hh"

class G4VPhysicalVolume;
class G4UserLimits;

class NucTechDetectorConstruction : public G4VUserDetectorConstruction {
public:
  NucTechDetectorConstruction();
  virtual ~NucTechDetectorConstruction();

  virtual G4VPhysicalVolume *Construct() override;

  // G4double GetFoilThickness() const { return fFoilThickness; } // <-- Add this
  
private:
  void DefineMaterials();
  G4VPhysicalVolume *DefineVolumes();

  G4bool fCheckOverlaps;
  G4UserLimits *fStepLimit;
  // G4double fFoilThickness; // <-- Add this
};

#endif // NUCTECH_DETECTOR_CONSTRUCTION_HH
