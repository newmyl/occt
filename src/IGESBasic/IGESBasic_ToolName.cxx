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

#include <IGESBasic_Name.hxx>
#include <IGESBasic_ToolName.hxx>
#include <IGESData_DirChecker.hxx>
#include <IGESData_Dump.hxx>
#include <IGESData_IGESDumper.hxx>
#include <IGESData_IGESReaderData.hxx>
#include <IGESData_IGESWriter.hxx>
#include <IGESData_ParamCursor.hxx>
#include <IGESData_ParamReader.hxx>
#include <Interface_Check.hxx>
#include <Interface_EntityIterator.hxx>
#include <Interface_Macros.hxx>
#include <Interface_ShareTool.hxx>
#include <Message_Messenger.hxx>
#include <Standard_DomainError.hxx>
#include <TCollection_HAsciiString.hxx>


void  IGESBasic_ToolName::ReadOwnParams
  (const Handle(IGESBasic_Name)& ent,
   const Handle(IGESData_IGESReaderData)& /* IR */, IGESData_ParamReader& PR) const
{
  Standard_Integer tempNbPropertyValues;
  Handle(TCollection_HAsciiString) tempName;

  PR.ReadInteger(tempNbPropertyValues,"Number of property values");
  PR.ReadText("Name",tempName);

  DirChecker(ent).CheckTypeAndForm(PR.CCheck(),ent);
  ent->Init(tempNbPropertyValues,tempName);
}

void  IGESBasic_ToolName::WriteOwnParams
  (const Handle(IGESBasic_Name)& ent, IGESData_IGESWriter& IW) const
{
  IW.Send(ent->NbPropertyValues());
  IW.Send(ent->Value());
}

Standard_Boolean  IGESBasic_ToolName::OwnCorrect
  (const Handle(IGESBasic_Name)& ent) const
{
  Standard_Boolean res = (ent->NbPropertyValues() != 1);
  if (res) ent->Init(1,ent->Value());
  return res;    // nbpropertyvalues = 1
}

IGESData_DirChecker  IGESBasic_ToolName::DirChecker
  (const Handle(IGESBasic_Name)& /*ent*/ ) const
{
  IGESData_DirChecker DC(406,15);  //Form no = 15 & Type = 406
  DC.Structure(IGESData_DefVoid);
  DC.GraphicsIgnored();
  DC.BlankStatusIgnored();
  DC.UseFlagIgnored();
  DC.HierarchyStatusIgnored();
  return DC;
}

void  IGESBasic_ToolName::OwnCheck
  (const Handle(IGESBasic_Name)& ent,
   const Interface_ShareTool& , Handle(Interface_Check)& ach) const
{
  if (ent->NbPropertyValues() != 1)
    ach->AddFail("Number of Property Values != 1");
}

void  IGESBasic_ToolName::OwnDump
  (const Handle(IGESBasic_Name)& ent, const IGESData_IGESDumper& /* dumper */,
   const Handle(Message_Messenger)& S, const Standard_Integer /* level */) const
{
  S << "IGESBasic_Name" << endl;

  S << "Number of property values : " << ent->NbPropertyValues() << endl;
  S << "Name : ";
  IGESData_DumpString(S,ent->Value());
  S << endl;
}
