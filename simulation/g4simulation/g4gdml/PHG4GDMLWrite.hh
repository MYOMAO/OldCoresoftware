//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
//
// $Id: PHG4GDMLWrite.hh 69013 2013-04-15 09:41:13Z gcosmo $
//
//
// class PHG4GDMLWrite
//
// Class description:
//
// GDML writer.

// History:
// - Created.                                  Zoltan Torzsok, November 2007
// -------------------------------------------------------------------------

#ifndef _PHG4GDMLWRITE_INCLUDED_
#define _PHG4GDMLWRITE_INCLUDED_

#include <map>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#include <xercesc/dom/DOM.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
#pragma GCC diagnostic pop

#include <Geant4/G4Transform3D.hh>


#include "PHG4GDMLAuxStructType.hh"

class G4LogicalVolume;
class G4VPhysicalVolume;

class PHG4GDMLWrite
{
  typedef std::map<const G4LogicalVolume*,G4Transform3D> VolumeMapType;
  typedef std::map<const G4VPhysicalVolume*,G4String> PhysVolumeMapType;
  typedef std::map<G4int,G4int> DepthMapType;

  public:  // with description

    G4Transform3D Write(const G4String& filename,
                        const G4LogicalVolume* const topLog,
                        const G4String& schemaPath,
                        const G4int depth, G4bool storeReferences=true);
      //
      // Main method for writing GDML files.

    void AddModule(const G4VPhysicalVolume* const topVol);
    void AddModule(const G4int depth);
      //
      // Split geometry structure in modules, by volume subtree or level

    void AddAuxiliary(PHG4GDMLAuxStructType myaux);
      //
      // Import auxiliary structure

    static void SetAddPointerToName(G4bool);
      //
      // Specify if to add or not memory addresses to IDs.

    virtual void DefineWrite(xercesc::DOMElement*)=0;
    virtual void MaterialsWrite(xercesc::DOMElement*)=0;
    virtual void SolidsWrite(xercesc::DOMElement*)=0;
    virtual void StructureWrite(xercesc::DOMElement*)=0;
    virtual G4Transform3D TraverseVolumeTree(const G4LogicalVolume* const,
                                             const G4int)=0;
    virtual void SurfacesWrite()=0;
    virtual void SetupWrite(xercesc::DOMElement*,
                            const G4LogicalVolume* const)=0;
      //
      // Pure virtual methods implemented in concrete writer plugin's classes

    virtual void ExtensionWrite(xercesc::DOMElement*);
    virtual void UserinfoWrite(xercesc::DOMElement*);
    virtual void AddExtension(xercesc::DOMElement*,
                              const G4LogicalVolume* const);
      //
      // To be implemented in the client code for handling extensions
      // to the GDML schema, identified with the tag "extension".
      // The implementation should be placed inside a user-class
      // inheriting from PHG4GDMLWriteStructure and being registered
      // as argument to PHG4GDMLParser.

    G4String GenerateName(const G4String&,const void* const);

  protected:

    PHG4GDMLWrite();
    virtual ~PHG4GDMLWrite();

    VolumeMapType& VolumeMap();

    xercesc::DOMAttr* NewAttribute(const G4String&, const G4String&);
    xercesc::DOMAttr* NewAttribute(const G4String&, const G4double&);
    xercesc::DOMElement* NewElement(const G4String&);
    G4String Modularize(const G4VPhysicalVolume* const topvol,
                        const G4int depth);

    void AddAuxInfo(PHG4GDMLAuxListType* auxInfoList, xercesc::DOMElement* element);

    G4bool FileExists(const G4String&) const;
    PhysVolumeMapType& PvolumeMap();
    DepthMapType& DepthMap();

  protected:

    G4String SchemaLocation;
    static G4bool addPointerToName;
    xercesc::DOMDocument* doc;
    xercesc::DOMElement* extElement;
    xercesc::DOMElement* userinfoElement;
    XMLCh tempStr[10000];
    PHG4GDMLAuxListType auxList;
};

#endif

