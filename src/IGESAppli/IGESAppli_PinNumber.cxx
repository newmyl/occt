// Created by: CKY / Contract Toubro-Larsen
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

//--------------------------------------------------------------------
//--------------------------------------------------------------------

#include <IGESAppli_PinNumber.hxx>
#include <Standard_Type.hxx>
#include <TCollection_HAsciiString.hxx>

IMPLEMENT_STANDARD_RTTIEXT(IGESAppli_PinNumber,IGESData_IGESEntity)

void IGESAppli_PinNumber::Init
  (const Standard_Integer nbPropVal,
   const Handle(TCollection_HAsciiString)& aValue)
{
  thePinNumber        = aValue;
  theNbPropertyValues = nbPropVal;
  InitTypeAndForm(406,8);
}

void IGESAppli_PinNumber::OwnCheck (const Interface_ShareTool &, const Handle(Interface_Check) &theCheck) const
{
  if (SubordinateStatus() != 0)
    if (DefLevel() != IGESData_DefOne &&
	    DefLevel() != IGESData_DefSeveral)
      theCheck->AddFail("Level type: Incorrect");
  if (NbPropertyValues() != 1)
    theCheck->AddFail("Number of Property Values != 1");
  //UNFINISHED
  //Level to be ignored if the property is subordinate -- queried
}
