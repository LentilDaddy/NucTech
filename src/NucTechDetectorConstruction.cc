#include "NucTechDetectorConstruction.hh"
#include "G4Tubs.hh"
#include "G4Box.hh"
#include "G4Element.hh"
#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4UserLimits.hh"
#include "G4VisAttributes.hh"
#include "G4PVReplica.hh"
//#include "G4FieldBuilder.hh"
#include "G4FieldManager.hh"
#include "G4UniformMagField.hh"
#include "G4TransportationManager.hh"
// #include "NucTechMagneticField.hh"





NucTechDetectorConstruction::NucTechDetectorConstruction()
    : fStepLimit(nullptr) {}

NucTechDetectorConstruction::~NucTechDetectorConstruction() {
  delete fStepLimit;

}


G4VPhysicalVolume *NucTechDetectorConstruction::Construct() {
  G4bool checkOverlaps = true;

  // Materials
  G4NistManager *nist = G4NistManager::Instance();
  G4Material *world_mat = nist->FindOrBuildMaterial("G4_Galactic");
  G4Material *vacuum = nist->FindOrBuildMaterial("G4_Galactic");

  
G4Element* elS = nist->FindOrBuildElement("S");  // Sulfur
G4Element* elF = nist->FindOrBuildElement("F");  // Fluorine
G4Element* elP = nist->FindOrBuildElement("P");  // Phosphorus
G4Element* elC = nist->FindOrBuildElement("C");  // Carbon
G4Element* elU = nist->FindOrBuildElement("U");  // Uranium

G4int ncomponents, natoms;
G4String name;
// define a material from elements.   case 1: chemical molecule
G4double medium_density = 1.339*g/cm3; //keep the same because we will be changing pressure anyway.
G4Material* SF6 = new G4Material(name="SF6", medium_density, ncomponents=2);
SF6->AddElement(elS, natoms=1);
SF6->AddElement(elF, natoms=6);

G4Material* PF5 = new G4Material(name="PF5", medium_density, ncomponents=2);
PF5->AddElement(elP, natoms=1);
PF5->AddElement(elF, natoms=5);

G4Material* UF6 = new G4Material(name="UF6", medium_density, ncomponents=2);
UF6->AddElement(elU, natoms=1);
UF6->AddElement(elF, natoms=6);


G4Material* CF4 = new G4Material(name="CF4", medium_density, ncomponents=2);
CF4->AddElement(elC, natoms=1);
CF4->AddElement(elF, natoms=4);

G4Material* C3F8 = new G4Material(name="C3F8", medium_density, ncomponents=2);
C3F8->AddElement(elC, natoms=3);
C3F8->AddElement(elF, natoms=8);

  //G4Material *foil = nist->FindOrBuildMaterial("G4_Au");
  G4Material *foil = nist->FindOrBuildMaterial("G4_Cu"); //swap target to tungsten
  G4Material *medium = SF6;
  G4Material *Fe_Steel = nist->FindOrBuildMaterial("G4_STAINLESS-STEEL");

  /***** Experimental hall *****/

  G4double worldHalfLength = 0.5 * m; //doubled this to ensure no overlap with detector

  G4VSolid *world =
      new G4Box("World", worldHalfLength, worldHalfLength, worldHalfLength);

  G4LogicalVolume *world_logical =
      new G4LogicalVolume(world, world_mat, "World");

  G4VPhysicalVolume *world_physical =
      new G4PVPlacement(G4Transform3D(), world_logical, "World", 0, false, 0);

  /***** Detector *****/

  G4double det_radius = 9. * cm;
  // G4double dzFoilPart = 5. * mm; // foil thickness. 4mm of initial layer!
  G4double dzVacuum = 10. * cm;
  G4double dzFoil = 6*mm;
  G4double dzSteel = 0.5*mm;
  G4double det_halfDepth = 10. * cm;
  // G4int nSlices = 200;


  G4VSolid *det_solid =
    new G4Tubs("Detector1", 0.*cm, det_radius, det_halfDepth, 0.*deg, 360.*deg); //i think this is just the dimensions


  G4LogicalVolume *det_logical =
      new G4LogicalVolume(det_solid, medium, "Detector1");

   G4double &det_PosZ = det_halfDepth; // place it so no overlap with foil

  new G4PVPlacement(nullptr,                         // No rotation
		    G4ThreeVector(0., 0., det_PosZ + dzFoil + dzVacuum + dzSteel), // Translation (so no overlap with foil)
                    det_logical,                     // Logical volume
                    "Detector1",                     // Name
                    world_logical,                   // Mother volume
                    false,          // Not a parameterized volume
                    0,              // Copy number
                    checkOverlaps); // Overlap checking



  G4VSolid *midLayerSolid =
    new G4Tubs("Detector2", 0.*cm, det_radius, dzFoil / 2, 0.*deg, 360.*deg);  

  // Logical volume using the foil material
  G4LogicalVolume *midLayer_log =
      new G4LogicalVolume(midLayerSolid, foil, "Detector2");



  /***** Slices in detector *****/

// G4double sliceHalfThickness = 0.5 * mm;
// G4VSolid* sliceSolid = new G4Tubs("SliceSolid", 0.*cm, det_radius, sliceHalfThickness, 0.*deg, 360.*deg);

// G4LogicalVolume* sliceLogical =
//     new G4LogicalVolume(sliceSolid, medium, "SliceLogical");


// new G4PVReplica("SlicePhysical",   // name
//                 sliceLogical,      // logical volume of slice
//                 det_logical,       // mother volume (Detector1)
//                 kZAxis,            // replicate along Z (the cylinder axis). kZAxis is predefined in G4PhysicalConstants.hh
//                 nSlices,              // number of slices (Detector1 half-depth × 2 / 1 mm)
//                 1.0 * mm);         // slice thickness

      /*Place the foil*/
  new G4PVPlacement(nullptr,                   // no rotation
		    G4ThreeVector(0., 0., dzFoil/2 ), // at detector start
                    midLayer_log,              // its logical volume
                    "Detector2",               // name
                    world_logical,               // mother is your world volume
                    false,                     // not parameterized
                    0,                         // copy number
                    checkOverlaps              // overlap checking
  );



  G4VSolid *vacuumLayer =
    new G4Tubs("vacuumLayer", 0.*cm, det_radius, dzVacuum / 2, 0.*deg, 360.*deg);  

  G4LogicalVolume *vacuumLayer_log =
      new G4LogicalVolume(vacuumLayer, vacuum, "vacuumLayer");


      /*Place the vacuum layer*/
  new G4PVPlacement(nullptr,                   
		    G4ThreeVector(0., 0., dzFoil + dzVacuum/2 ), //there is a thin layer of foil at end - eventually need to change this to stainless steel
                    vacuumLayer_log,              // its logical volume
                    "vacuumLayer",               // name
                    world_logical,               // mother is Detector 2 volume
                    false,                     // not parameterized
                    0,                         // copy number
                    !checkOverlaps              // overlap checking
  );




  G4VSolid *stainlessSteel =
    new G4Tubs("stainlessSteel", 0.*cm, det_radius, dzSteel / 2, 0.*deg, 360.*deg);  

  G4LogicalVolume *stainlessSteel_log =
      new G4LogicalVolume(stainlessSteel, Fe_Steel, "stainlessSteel");


      /*Place the vacuum layer*/
  new G4PVPlacement(nullptr,                   
		    G4ThreeVector(0., 0., dzFoil + dzVacuum + dzSteel/2 ), //there is a thin layer of foil at end - eventually need to change this to stainless steel
                    stainlessSteel_log,              // its logical volume
                    "stainlessSteel",               // name
                    world_logical,               // mother is Detector 2 volume
                    false,                     // not parameterized
                    0,                         // copy number
                    !checkOverlaps              // overlap checking
  );








  // --- local magnetic field attached to vacuumLayer_log ---
  // Option A: uniform B-field (simple, recommended)
  G4ThreeVector bVec(0., 1.0*tesla, 0.); // change vector/magnitude as needed
  G4UniformMagField* uniformField = new G4UniformMagField(bVec);
  G4FieldManager* vacFieldMgr = new G4FieldManager(uniformField);
  vacFieldMgr->CreateChordFinder(uniformField);
  vacuumLayer_log->SetFieldManager(vacFieldMgr, true);




  /***** Step limit *****/

  G4double maxStep = .01 * mm; //changed from 0.05
  fStepLimit = new G4UserLimits(maxStep);
  det_logical->SetUserLimits(fStepLimit); //assigned to detector 1
  midLayer_log->SetUserLimits(fStepLimit); //assigned to detector 2 (the foil?)
  // sliceLogical->SetUserLimits(fStepLimit); //should this be applied to the slices?

  /***** Visualisation *****/

  world_logical->SetVisAttributes(G4VisAttributes::GetInvisible());

  G4VisAttributes *det_vis =
      new G4VisAttributes(G4Colour(173. / 255., 216. / 255., 230. / 255., .8));
  det_vis->SetDaughtersInvisible(false);
  det_vis->SetForceSolid(true);
  det_logical->SetVisAttributes(det_vis);

  G4VisAttributes *mixVis = new G4VisAttributes(G4Colour(1.0, 0.84, 0.0, 0.8));
  mixVis->SetForceSolid(true);
  midLayer_log->SetVisAttributes(mixVis);

  return world_physical;
}

