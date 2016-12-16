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

#ifndef _IGESSolid_GeneralModule_HeaderFile
#define _IGESSolid_GeneralModule_HeaderFile

#include <IGESData_GeneralModule.hxx>
/*

class IGESSolid_GeneralModule;
DEFINE_STANDARD_HANDLE(IGESSolid_GeneralModule, IGESData_GeneralModule)

//! Definition of General Services for IGESSolid (specific part)
//! This Services comprise : Shared & Implied Lists, Copy, Check
class IGESSolid_GeneralModule : public IGESData_GeneralModule
{
 public:

  //! Creates a GeneralModule from IGESSolid and puts it into GeneralLib
  IGESSolid_GeneralModule() {}
  
  //! Returns a DirChecker, specific for each type of Entity
  //! (identified by its Case Number) : this DirChecker defines
  //! constraints which must be respected by the DirectoryPart
  Standard_EXPORT IGESData_DirChecker DirChecker (const Standard_Integer CN, const Handle(IGESData_IGESEntity)& ent) const Standard_OVERRIDE;
  
  //! Returns a category number which characterizes an entity
  //! Shape for all
  Standard_EXPORT virtual Standard_Integer CategoryNumber (const Standard_Integer CN, const Handle(Standard_Transient)& ent, const Interface_ShareTool& shares) const Standard_OVERRIDE;

  DEFINE_STANDARD_RTTIEXT(IGESSolid_GeneralModule,IGESData_GeneralModule)
};
*/
#endif // _IGESSolid_GeneralModule_HeaderFile
