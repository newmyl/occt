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

#include <IGESGraph_HighLight.hxx>
#include <IGESFile_Reader.hxx>
#include <IGESData_IGESWriter.hxx>
#include <IGESData_DirChecker.hxx>
#include <Message_Messenger.hxx>
#include <IGESData_IGESDumper.hxx>
#include <IGESData_Dump.hxx>

IMPLEMENT_STANDARD_RTTIEXT(IGESGraph_HighLight,IGESData_IGESEntity)

void IGESGraph_HighLight::OwnRead (IGESFile_Reader &PR)
{ 
  Standard_Integer aNbPropertyValues = 0;
  PR.ReadInteger(aNbPropertyValues,"No. of property values");
  if (aNbPropertyValues != 1)
    PR.AddFail("No. of Property values : Value is not 1");

  myHighLight = 0; // Default Value
  PR.ReadInteger(myHighLight,"Highlight flag");
}

void IGESGraph_HighLight::OwnWrite (IGESData_IGESWriter& IW) const
{ 
  IW.Send(1);
  IW.Send(myHighLight);
}

IGESData_DirChecker IGESGraph_HighLight::DirChecker () const
{ 
  IGESData_DirChecker DC (406, 20);
  DC.Structure(IGESData_DefVoid);
  DC.LineFont(IGESData_DefVoid);
  DC.LineWeight(IGESData_DefVoid);
  DC.Color(IGESData_DefVoid);
  DC.BlankStatusIgnored();
  DC.UseFlagIgnored();
  DC.HierarchyStatusIgnored();
  return DC;
}

void IGESGraph_HighLight::OwnDump (const IGESData_IGESDumper &, const Handle(Message_Messenger) &S, const Standard_Integer) const
{
  S << "IGESGraph_HighLight" << endl;
  S << "No. of property values : 1" << endl;
  S << "Highlight Status : " << myHighLight << endl;
  S << endl;
}
