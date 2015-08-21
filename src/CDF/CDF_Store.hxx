// Created on: 1995-11-13
// Created by: Jean-Louis Frenkel
// Copyright (c) 1995-1999 Matra Datavision
// Copyright (c) 1999-2014 OPEN CASCADE SAS
//
// This file is part of Open CASCADE Technology software library.
//
// This library is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License version 2.1 as published
// by the Free Software Foundation, with special exception defined in the file
// OCCT_LGPL_EXCEPTION.txt. Consult the file LICENSE_LGPL_21.txt included in OCCT
// distribution for complete text of the license and disclaimer of any warranty.
//
// Alternatively, this file may be used under the terms of Open CASCADE
// commercial license or contractual agreement.

#ifndef _CDF_Store_HeaderFile
#define _CDF_Store_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <Standard_Boolean.hxx>
#include <TCollection_ExtendedString.hxx>
#include <PCDM_StoreStatus.hxx>
#include <CDF_TryStoreStatus.hxx>
#include <Standard_ExtString.hxx>
#include <CDF_StoreSetNameStatus.hxx>
#include <CDF_SubComponentStatus.hxx>
class CDF_StoreList;
class CDM_Document;
class TCollection_ExtendedString;



class CDF_Store 
{
public:

  DEFINE_STANDARD_ALLOC

  
  //! creates a store list  from the document of the current selection.
  Standard_EXPORT CDF_Store(const Handle(CDM_Document)& aDocument);
  
  //! Checks will make the following control. Check must be used before
  //! using Create method. Check will not be done twice by Create.
  //!
  //! 1) controls whether there is one document in the current selection.
  //! 2) controls whether there is a storage driver for the document.
  //! 3) controls whether the applicationdatatype associated with the driver
  //! exists as metadata in the metadata manager (Design Manager for example).
  //! 4) controls whether there is a storage driver for each subcomponents
  //! of the document if there are.  If the control fails for a subcomponent,
  //! the control stops and name and type of this subcomponent can be get
  //! with LastName method
  //! 5) controls whether the applicationdatatype associated with the driver of
  //! each subcomponent exists as metadata in the metadata manager
  //! (Design Manager for example). If the control fails for a subcomponent,
  //! the control stops and name and type of this subcomponent can be get
  //! with LastName method
  Standard_EXPORT CDF_TryStoreStatus Check();
  
  //! in the case of a subcomponent for which no storage driver exists,
  //! returns the name of the subcomponent if there is one.
  Standard_EXPORT Standard_ExtString LastName() const;

  Standard_EXPORT Handle(Storage_IODevice) Device() const;
  
    //! returns the name under which the current document will be stored
  Standard_EXPORT Standard_ExtString Name() const;

  Standard_EXPORT Standard_Boolean SetDevice(const Handle(Storage_IODevice)& aDevice);

  Standard_EXPORT Handle(Storage_IODevice) CDF_Store::MetaDataDevice() const;
  
  //! returns true if the current document is already stored
  Standard_EXPORT Standard_Boolean IsStored() const;
  
  Standard_EXPORT Standard_Boolean IsModified() const;
  
  Standard_EXPORT Standard_Boolean CurrentIsConsistent() const;
  
  Standard_EXPORT Standard_Boolean IsConsistent() const;
  
  Standard_EXPORT Standard_Boolean HasAPreviousVersion() const;
  
  Standard_EXPORT Standard_ExtString PreviousVersion() const;
  
  //! returns true if the currentdocument is the main one, ie the document
  //! of the current selection.
  Standard_EXPORT Standard_Boolean IsMainDocument() const;
  
  //! defines the   folder in which  the document  should be
  //! stored.  returns  Standard_True if  the Folder exists,
  //! Standard_False otherwise.
  Standard_EXPORT Standard_Boolean SetFolder (const TCollection_ExtendedString& aFolder);
  
  //! defines the name under which the document should be stored.
  Standard_EXPORT CDF_StoreSetNameStatus SetName (const Standard_ExtString aName);
  
  Standard_EXPORT void SetComment (const Standard_ExtString aComment);
  
  Standard_EXPORT Standard_ExtString Comment() const;
  
  //! defines the name under which the document should be stored.
  //! uses for example after modification of the folder.
  Standard_EXPORT CDF_StoreSetNameStatus RecheckName();
  
  Standard_EXPORT Standard_Boolean SetPreviousVersion (const Standard_ExtString aPreviousVersion);
  
  Standard_EXPORT void Realize();
  
  //! returns the complete path of the created meta-data.
  Standard_EXPORT Standard_ExtString Path() const;
  
  //! returns the path of  the previous store is the object
  //! is already stored, otherwise an empty string;
  Standard_EXPORT Standard_ExtString MetaDataPath() const;
  
  //! returns the description of the format of the main object.
  Standard_EXPORT Standard_ExtString Description() const;
  
  //! Allows to Start a new Iteration from beginning
  Standard_EXPORT void InitComponent();
  
  //! Returns True if there are more entries to return
  Standard_EXPORT Standard_Boolean MoreComponent() const;
  
  //! Go to the next entry
  //! (if there is not, Value will raise an exception)
  Standard_EXPORT void NextComponent();
  
  Standard_EXPORT void SetCurrent();
  
  //! Returns item value of current entry
  Standard_EXPORT Standard_ExtString Component() const;
  
  Standard_EXPORT Standard_Boolean HasSubComponents() const;
  
  Standard_EXPORT CDF_SubComponentStatus SubComponentStatus (const Standard_ExtString aPresentation) const;
  
  Standard_EXPORT void SetCurrent (const Standard_ExtString aPresentation);
  
  //! the two following methods can
  //! be used just after Realize  or Import --  method to know if
  //! thes methods worked correctly, and if not why.
  Standard_EXPORT void SetMain();
  
  Standard_EXPORT PCDM_StoreStatus StoreStatus() const;
  
  Standard_EXPORT Standard_ExtString AssociatedStatusText() const;
  
  //! defines the name under which the document should be stored.
  Standard_EXPORT CDF_StoreSetNameStatus SetName (const TCollection_ExtendedString& aName);
  
  //! defines the   folder in which  the document  should be
  //! stored.  returns  Standard_True if  the Folder exists,
  //! Standard_False otherwise.
  Standard_EXPORT Standard_Boolean SetFolder (const Standard_ExtString aFolder);




protected:





private:

  
  Standard_EXPORT CDF_Store();
  
  Standard_EXPORT void FindDefault();
  
  Standard_EXPORT void Init();


  Handle(CDF_StoreList) myList;
  Handle(CDM_Document) myMainDocument;
  Handle(CDM_Document) myCurrentDocument;
  Standard_Boolean myHasSubComponents;
  Standard_Boolean myIsMainDocument;
  TCollection_ExtendedString myLastName;
  TCollection_ExtendedString myPath;
  TCollection_ExtendedString myText;
  PCDM_StoreStatus myStatus;


};







#endif // _CDF_Store_HeaderFile
