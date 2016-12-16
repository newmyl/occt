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

#include <gp_Vec.hxx>
#include <gp_XYZ.hxx>
#include <IGESData_DirChecker.hxx>
#include <IGESData_Dump.hxx>
#include <IGESData_IGESDumper.hxx>
#include <IGESData_IGESEntity.hxx>
#include <IGESData_IGESReaderData.hxx>
#include <IGESData_IGESWriter.hxx>
#include <IGESData_ParamCursor.hxx>
#include <IGESData_ParamReader.hxx>
#include <IGESData_Status.hxx>
#include <IGESGeom_OffsetSurface.hxx>
#include <IGESGeom_ToolOffsetSurface.hxx>
#include <Interface_Check.hxx>
#include <Interface_EntityIterator.hxx>
#include <Interface_Macros.hxx>
#include <Interface_ShareTool.hxx>
#include <Message_Messenger.hxx>
#include <Message_Msg.hxx>
#include <Standard_DomainError.hxx>


void IGESGeom_ToolOffsetSurface::ReadOwnParams
  (const Handle(IGESGeom_OffsetSurface)& ent,
   const Handle(IGESData_IGESReaderData)& IR, IGESData_ParamReader& PR) const
{
  gp_XYZ anIndicator;
  Standard_Real aDistance;
  Handle(IGESData_IGESEntity) aSurface;
  IGESData_Status aStatus;

  // Reading the offset indicator
  PR.ReadXYZ(anIndicator);
  // Reading the offset distance
  if (!PR.ReadReal(aDistance)){
    Message_Msg Msg163("XSTEP_163");
    PR.SendFail(Msg163);
  }
  // Reading the surface entity to be offset
  if (!PR.ReadEntity(IR, aStatus, aSurface)) {
    Message_Msg Msg164("XSTEP_164");
    switch(aStatus) {
    case IGESData_ReferenceError: {  
      Message_Msg Msg216 ("IGES_216");
      Msg164.Arg(Msg216.Value());
      PR.SendFail(Msg164);
      break; }
    case IGESData_EntityError: {
      Message_Msg Msg217 ("IGES_217");
      Msg164.Arg(Msg217.Value());
      PR.SendFail(Msg164);
      break; }
    default:{
    }
    }
  }

  DirChecker(ent).CheckTypeAndForm(PR.CCheck(),ent);
  ent->Init(anIndicator, aDistance, aSurface);
}

void IGESGeom_ToolOffsetSurface::WriteOwnParams
  (const Handle(IGESGeom_OffsetSurface)& ent, IGESData_IGESWriter& IW)  const
{
  IW.Send( ent->OffsetIndicator().X() );
  IW.Send( ent->OffsetIndicator().Y() );
  IW.Send( ent->OffsetIndicator().Z() );
  IW.Send( ent->Distance() );
  IW.Send( ent->Surface() );
}

void  IGESGeom_ToolOffsetSurface::OwnShared
  (const Handle(IGESGeom_OffsetSurface)& ent, Interface_EntityIterator& iter) const
{
  iter.GetOneItem( ent->Surface() );
}


IGESData_DirChecker IGESGeom_ToolOffsetSurface::DirChecker
  (const Handle(IGESGeom_OffsetSurface)& /* ent */ )   const
{
  IGESData_DirChecker DC(140, 0);
  DC.Structure(IGESData_DefVoid);
  DC.LineFont(IGESData_DefAny);
//  DC.LineWeight(IGESData_DefValue);
  DC.Color(IGESData_DefAny);
  DC.HierarchyStatusIgnored();
  return DC;
}

void IGESGeom_ToolOffsetSurface::OwnDump
  (const Handle(IGESGeom_OffsetSurface)& ent, const IGESData_IGESDumper& dumper,
   const Handle(Message_Messenger)& S, const Standard_Integer level)  const
{
  Standard_Integer sublevel = (level <= 4) ? 0 : 1;

  S << "IGESGeom_OffsetSurface" << endl;

  S << "Offset Indicator     : ";
  IGESData_DumpXYZL(S,level, ent->OffsetIndicator(), ent->VectorLocation());
  S << endl;
  S << "Offset Distance      : " << ent->Distance() << "  ";
  S << "Surface to be offset : ";
  dumper.Dump(ent->Surface(),S, sublevel);
  S << endl;
}
