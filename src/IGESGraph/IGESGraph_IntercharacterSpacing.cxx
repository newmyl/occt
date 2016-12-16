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

#include <IGESGraph_IntercharacterSpacing.hxx>
#include <IGESFile_Reader.hxx>
#include <IGESData_IGESWriter.hxx>
#include <IGESData_DirChecker.hxx>
#include <Message_Messenger.hxx>
#include <IGESData_IGESDumper.hxx>
#include <IGESData_Dump.hxx>

IMPLEMENT_STANDARD_RTTIEXT(IGESGraph_IntercharacterSpacing,IGESData_IGESEntity)

void IGESGraph_IntercharacterSpacing::OwnRead (IGESFile_Reader &PR)
{ 
  Standard_Integer aNbPropertyValues = 0;
  PR.ReadInteger(aNbPropertyValues,"No. of property values");
  if (aNbPropertyValues != 1)
    PR.AddFail("No. of Property values : Value is not 1");

  PR.ReadReal(myISpace,"Intercharacter space in % of text height");
}

void IGESGraph_IntercharacterSpacing::OwnWrite (IGESData_IGESWriter& IW) const
{ 
  IW.Send(1);
  IW.Send(myISpace);
}

IGESData_DirChecker IGESGraph_IntercharacterSpacing::DirChecker () const
{ 
  IGESData_DirChecker DC (406, 18);
  DC.Structure(IGESData_DefVoid);
  DC.LineFont(IGESData_DefVoid);
  DC.LineWeight(IGESData_DefVoid);
  DC.Color(IGESData_DefVoid);
  DC.BlankStatusIgnored();
  DC.UseFlagIgnored();
  DC.HierarchyStatusIgnored();
  return DC;
}

void IGESGraph_IntercharacterSpacing::OwnCheck (const Interface_ShareTool &, Handle(Interface_Check) &ach) const
{
  if ((myISpace < 0.0) || (myISpace > 100.0))
    ach->AddFail("Intercharacter Space : Value not in the range [0-100]");
}

void IGESGraph_IntercharacterSpacing::OwnDump (const IGESData_IGESDumper &, const Handle(Message_Messenger) &S, const Standard_Integer) const
{
  S << "IGESGraph_IntercharacterSpacing" << endl;
  S << "No. of property values : 1" << endl;
  S << "Intercharacter space in % of text height : " << myISpace << endl;
  S << endl;
}
