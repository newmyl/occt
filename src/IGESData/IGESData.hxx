// Created on: 1992-04-06
// Created by: Christian CAILLET
// Copyright (c) 1992-1999 Matra Datavision
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

#ifndef _IGESData_HeaderFile
#define _IGESData_HeaderFile

#include <Standard.hxx>
#include <Standard_Handle.hxx>
class IGESData_Protocol;
class IGESData_IGESModel;

//! basic description of an IGES Interface
class IGESData 
{
 public:

  //! Prepares General dynamic data used for IGESData specifically :
  //! Protocol and Modules, which treat UndefinedEntity
  Standard_EXPORT static void Init();
  
  //! Returns a Protocol from IGESData (avoids to create it)
  Standard_EXPORT static const Handle(IGESData_Protocol) & Protocol();
  
  //! Returns a Protocol from IGESData (avoids to create it)
  Standard_EXPORT static Handle(IGESData_IGESModel) NewModel ();
  
  //! From the name of unit, computes flag number, 0 if incorrect
  //! (in this case, user defined entity remains possible)
  Standard_EXPORT static Standard_Integer UnitNameFlag (const Standard_CString name);
  
  //! From the flag of unit, determines value in MM, 0 if incorrect
  Standard_EXPORT static Standard_Real UnitFlagValue (const Standard_Integer flag);
  
  //! From the flag of unit, determines its name, "" if incorrect
  Standard_EXPORT static Standard_CString UnitFlagName (const Standard_Integer flag);
  
  //! From the flag of IGES version, returns name, "" if incorrect
  Standard_EXPORT static Standard_CString IGESVersionName (const Standard_Integer flag);
  
  //! Returns the maximum allowed value for IGESVersion Flag
  static Standard_Integer IGESVersionMax () { return 11; }
  
  //! From the flag of drafting standard, returns name, "" if incorrect
  Standard_EXPORT static Standard_CString DraftingName (const Standard_Integer flag);
  
  //! Returns the maximum allowed value for Drafting Flag
  static Standard_Integer DraftingMax () { return 7; }
};

#endif // _IGESData_HeaderFile
