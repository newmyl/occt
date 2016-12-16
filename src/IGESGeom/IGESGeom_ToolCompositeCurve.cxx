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

#include <IGESData_DirChecker.hxx>
#include <IGESData_Dump.hxx>
#include <IGESData_HArray1OfIGESEntity.hxx>
#include <IGESData_IGESDumper.hxx>
#include <IGESData_IGESEntity.hxx>
#include <IGESData_IGESReaderData.hxx>
#include <IGESData_IGESWriter.hxx>
#include <IGESData_ParamCursor.hxx>
#include <IGESData_ParamReader.hxx>
#include <IGESGeom_CompositeCurve.hxx>
#include <IGESGeom_ToolCompositeCurve.hxx>
#include <Interface_Check.hxx>
#include <Interface_EntityIterator.hxx>
#include <Interface_Macros.hxx>
#include <Interface_ShareTool.hxx>
#include <Message_Messenger.hxx>
#include <Message_Msg.hxx>
#include <Standard_DomainError.hxx>


void  IGESGeom_ToolCompositeCurve::ReadOwnParams
  (const Handle(IGESGeom_CompositeCurve)& ent,
   const Handle(IGESData_IGESReaderData)& IR, IGESData_ParamReader& PR) const
{
  Handle(IGESData_HArray1OfIGESEntity) tempEntities;

  Standard_Integer num;
  Standard_Boolean st = PR.ReadInteger(num);
  if (st && (num > 0)){
    Message_Msg Msg80("XSTEP_80");
    PR.ReadEnts (IR,PR.CurrentList(num),Msg80,tempEntities);
  }
  else{
    Message_Msg Msg79("XSTEP_79");
    PR.SendFail(Msg79);
  }

  DirChecker(ent).CheckTypeAndForm(PR.CCheck(),ent);
  ent->Init(tempEntities);
}

void  IGESGeom_ToolCompositeCurve::WriteOwnParams
  (const Handle(IGESGeom_CompositeCurve)& ent, IGESData_IGESWriter& IW)  const
{
  Standard_Integer num = ent->NbCurves();  Standard_Integer i;
  IW.Send(num);
  for ( num = ent->NbCurves(), i = 1; i <= num; i++ )
    IW.Send(ent->Curve(i));
}

void  IGESGeom_ToolCompositeCurve::OwnShared
  (const Handle(IGESGeom_CompositeCurve)& ent, Interface_EntityIterator& iter) const
{
  Standard_Integer num = ent->NbCurves();
  for ( Standard_Integer i = 1; i <= num; i++ )
    iter.GetOneItem(ent->Curve(i));
}

IGESData_DirChecker  IGESGeom_ToolCompositeCurve::DirChecker
  (const Handle(IGESGeom_CompositeCurve)& /* ent */ )  const
{
  IGESData_DirChecker DC(102, 0);
  DC.Structure(IGESData_DefVoid);
  DC.GraphicsIgnored();
  DC.LineFont(IGESData_DefAny);
//  DC.LineWeight(IGESData_DefValue);
  DC.Color(IGESData_DefAny);
  return DC;
}

void  IGESGeom_ToolCompositeCurve::OwnDump
  (const Handle(IGESGeom_CompositeCurve)& ent, const IGESData_IGESDumper& dumper,
   const Handle(Message_Messenger)& S, const Standard_Integer level)  const
{
  S << "IGESGeom_CompositeCurve" << endl;
  S << "Curve Entities : " << endl;
  IGESData_DumpEntities(S,dumper ,level,1, ent->NbCurves(),ent->Curve);
  S << endl;
}
