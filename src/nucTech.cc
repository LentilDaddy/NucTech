#include "NucTechActionInitialization.hh"
#include "NucTechDetectorConstruction.hh"

#include "G4EmLivermorePhysics.hh"
#include "G4HadronicParameters.hh"
#define G4MULTITHREADED
#ifdef G4MULTITHREADED
#include "G4MTRunManager.hh"
#else
#include "G4RunManager.hh"
#endif
#include "G4PhysListFactory.hh"
#include "G4RadioactiveDecayPhysics.hh"
#include "G4StepLimiterPhysics.hh"
#include "G4UIExecutive.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"
#include <time.h>

int main(int argc, char** argv)
{
  // Do this first to capture all output
  G4UIExecutive* ui = nullptr;
  if (argc == 1) 
  {
    ui = new G4UIExecutive(argc, argv);
  }

  // Handle output filename from arguments
  G4String outFilename = "out.root"; // Default filename
  if (argc > 2)
  {
    outFilename = argv[2]; // Use argument as output filename
  }

  // Choose the Random engine
  G4Random::setTheEngine(new CLHEP::RanecuEngine);

  // seed |=1;  // Make sure it's odd
  // constexpr G4long seed = 1502758933;  // Activate this line with a fixed seed if desired
  G4long seed = static_cast<G4long>(std::time(nullptr));
  if (seed % 2 ==0) seed+=1;
  CLHEP::HepRandom::setTheSeed(seed);

#ifdef G4MULTITHREADED
  G4MTRunManager* runManager = new G4MTRunManager;
#else
  G4RunManager* runManager = new G4RunManager;
#endif

  // Initialize DetectorConstruction
  runManager->SetUserInitialization(new NucTechDetectorConstruction);

  // Initialize Physics List
  G4PhysListFactory* physListFactory = new G4PhysListFactory(0);
  G4VModularPhysicsList* physics =
    physListFactory->GetReferencePhysList("FTFP_BERT");

  physics->ReplacePhysics(new G4EmLivermorePhysics);
  physics->ReplacePhysics(new G4RadioactiveDecayPhysics);
  physics->SetVerboseLevel(0);

  // Register additional physics
  physics->RegisterPhysics(new G4StepLimiterPhysics());

  // Set Physics List in RunManager
  runManager->SetUserInitialization(physics);

  //G4HadronicParameters::Instance()->SetTimeThresholdForRadioactiveDecay(
  //1.0e+60 * CLHEP::year);

  // Initialize ActionInitialization
  runManager->SetUserInitialization(new NucTechActionInitialization(outFilename));

  G4UImanager* UImanager = G4UImanager::GetUIpointer();

  if (ui == nullptr)
  {
    // Batch mode
    G4String command = "/control/execute ";
    G4String fileName = argv[1];
    UImanager->ApplyCommand(command + fileName);
  }
  else
  {
    // Interactive mode
    G4VisManager* visManager = new G4VisExecutive("Quiet");
    visManager->Initialize();
    UImanager->ApplyCommand("/control/execute vis.mac");
    ui->SessionStart();
    delete ui;
    delete visManager;
  }

  // Cleanup
  delete runManager;

  return 0;
}
