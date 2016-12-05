// Created on: 1993-05-10
// Created by: Christian CAILLET
// Copyright (c) 1993-1999 Matra Datavision
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

#ifndef _IGESDraw_GeneralModule_HeaderFile
#define _IGESDraw_GeneralModule_HeaderFile

#include <IGESData_GeneralModule.hxx>


class IGESDraw_GeneralModule;
DEFINE_STANDARD_HANDLE(IGESDraw_GeneralModule, IGESData_GeneralModule)

//! Definition of General Services for IGESDraw (specific part)
//! This Services comprise : Shared & Implied Lists, Copy, Check
class IGESDraw_GeneralModule : public IGESData_GeneralModule
{
 public:

  //! Creates a GeneralModule from IGESDraw and puts it into GeneralLib
  IGESDraw_GeneralModule() {}
  
  //! Lists the Entities shared by a given IGESEntity <ent>, from
  //! its specific parameters : specific for each type
  Standard_EXPORT void OwnSharedCase (const Standard_Integer CN, const Handle(IGESData_IGESEntity)& ent, Interface_EntityIterator& iter) const Standard_OVERRIDE;
  
  //! Specific list of Entities implied by an IGESEntity <ent> (in
  //! addition to Associativities). Redefined for ViewsVisible ...
  Standard_EXPORT virtual void OwnImpliedCase (const Standard_Integer CN, const Handle(IGESData_IGESEntity)& ent, Interface_EntityIterator& iter) const Standard_OVERRIDE;
  
  //! Returns a DirChecker, specific for each type of Entity
  //! (identified by its Case Number) : this DirChecker defines
  //! constraints which must be respected by the DirectoryPart
  Standard_EXPORT IGESData_DirChecker DirChecker (const Standard_Integer CN, const Handle(IGESData_IGESEntity)& ent) const Standard_OVERRIDE;
  
  //! Performs Specific Semantic Check for each type of Entity
  Standard_EXPORT void OwnCheckCase (const Standard_Integer CN, const Handle(IGESData_IGESEntity)& ent, const Interface_ShareTool& shares, Handle(Interface_Check)& ach) const Standard_OVERRIDE;
  
  //! Specific creation of a new void entity
  Standard_EXPORT Standard_Boolean NewVoid (const Standard_Integer CN, Handle(Standard_Transient)& entto) const Standard_OVERRIDE;
  
  //! Returns a category number which characterizes an entity
  //! Planar : Auxiliary
  //! Subfigures and ConnectPoint : Structure
  //! others : Drawing
  Standard_EXPORT virtual Standard_Integer CategoryNumber (const Standard_Integer CN, const Handle(Standard_Transient)& ent, const Interface_ShareTool& shares) const Standard_OVERRIDE;

  DEFINE_STANDARD_RTTIEXT(IGESDraw_GeneralModule,IGESData_GeneralModule)
};

#endif // _IGESDraw_GeneralModule_HeaderFile
