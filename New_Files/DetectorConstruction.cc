// code taken from B1 example


#include "DetectorConstruction.hh"

#include "G4RunManager.hh"
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4Cons.hh"
#include "G4Orb.hh"
#include "G4Sphere.hh"
#include "G4Trd.hh"
#include "G4Tubs.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "SensitiveDetector.hh"
#include "G4SDManager.hh"
#include "G4GDMLParser.hh"

namespace B1
{
G4VPhysicalVolume* DetectorConstruction::Construct()
{
  fReadFile = "/home/henry/gdml_files/epic2.gdml"; //  "SiCylinder.gdml";
  fParser.Read(fReadFile);
  G4VPhysicalVolume* fWorldPhysVol = fParser.GetWorldVolume();
  G4SDManager* SDman = G4SDManager::GetSDMpointer();
  G4String SDname = "Tracker";
  SensitiveDetector* aSD = new SensitiveDetector(SDname);
  SDman->AddNewDetector( aSD );
  const G4GDMLAuxMapType* auxmap = fParser.GetAuxMap();
  for(G4GDMLAuxMapType::const_iterator iter=auxmap->begin();      iter!=auxmap->end(); iter++) {
    for (G4GDMLAuxListType::const_iterator vit=(*iter).second.begin();        vit!=(*iter).second.end();vit++)    {
      if ((*vit).type=="SensDet")      {
        G4cout << "Attaching sensitive detector " << (*vit).value  << " to volume " << ((*iter).first)->GetName()     <<  G4endl << G4endl;
        G4VSensitiveDetector* mydet = SDman->FindSensitiveDetector((*vit).value);
        if(mydet)         {
          G4LogicalVolume* myvol = (*iter).first;
          myvol->SetSensitiveDetector(mydet);
            fScoringVolume = myvol; // not in G04
        }        else       {
          G4cout << (*vit).value << " detector not found" << G4endl;
        }
      }
    }
}

  return fWorldPhysVol;


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

}
}
