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
#include <IGESData_IGESDumper.hxx>
#include <IGESData_IGESReaderData.hxx>
#include <IGESData_IGESWriter.hxx>
#include <IGESData_ParamCursor.hxx>
#include <IGESData_ParamReader.hxx>
#include <IGESGeom_Direction.hxx>
#include <IGESGeom_Point.hxx>
#include <IGESSolid_SphericalSurface.hxx>
#include <IGESSolid_ToolSphericalSurface.hxx>
#include <Interface_Check.hxx>
#include <Interface_EntityIterator.hxx>
#include <Interface_Macros.hxx>
#include <Interface_ShareTool.hxx>
#include <Message_Messenger.hxx>
#include <Standard_DomainError.hxx>


void  IGESSolid_ToolSphericalSurface::ReadOwnParams
  (const Handle(IGESSolid_SphericalSurface)& ent,
   const Handle(IGESData_IGESReaderData)& IR, IGESData_ParamReader& PR) const
{
  Handle(IGESGeom_Point) tempCenter;
  Standard_Real tempRadius;
  Handle(IGESGeom_Direction) tempAxis;        // default Unparameterised
  Handle(IGESGeom_Direction) tempRefdir;      // default Unparameterised

  PR.ReadEntity(IR, "Center point", STANDARD_TYPE(IGESGeom_Point), tempCenter);
  PR.ReadReal(tempRadius,"Radius");

  if (ent->FormNumber() == 1)      // Parametrised surface
    {
      PR.ReadEntity(IR, "Axis direction", STANDARD_TYPE(IGESGeom_Direction), tempAxis);
      PR.ReadEntity(IR, "Reference direction", STANDARD_TYPE(IGESGeom_Direction), tempRefdir);
    }

  DirChecker(ent).CheckTypeAndForm(PR.CCheck(),ent);
  ent->Init (tempCenter, tempRadius, tempAxis, tempRefdir);
}

void  IGESSolid_ToolSphericalSurface::WriteOwnParams
  (const Handle(IGESSolid_SphericalSurface)& ent, IGESData_IGESWriter& IW) const
{
  IW.Send(ent->Center());
  IW.Send(ent->Radius());
  if (ent->IsParametrised())
    {
      IW.Send(ent->Axis());
      IW.Send(ent->ReferenceDir());
    }
}

IGESData_DirChecker  IGESSolid_ToolSphericalSurface::DirChecker
  (const Handle(IGESSolid_SphericalSurface)& /*ent*/) const
{
  IGESData_DirChecker DC(196, 0, 1);

  DC.Structure  (IGESData_DefVoid);
  DC.LineFont   (IGESData_DefAny);
  DC.Color      (IGESData_DefAny);

  DC.BlankStatusIgnored ();
  DC.SubordinateStatusRequired (1);
  DC.HierarchyStatusIgnored ();
  return DC;
}

void  IGESSolid_ToolSphericalSurface::OwnDump
  (const Handle(IGESSolid_SphericalSurface)& ent, const IGESData_IGESDumper& dumper,
   const Handle(Message_Messenger)& S, const Standard_Integer level) const
{
  S << "IGESSolid_SphericalSurface" << endl;
  Standard_Integer sublevel = (level <= 4) ? 0 : 1;

  S << "Center : ";
  dumper.Dump(ent->Center(),S, sublevel);
  S << endl;
  S << "Radius : " << ent->Radius() << endl;
  if (ent->IsParametrised())
    {
      S << "Surface is Parametrised" << endl;
      S << "Axis direction      : ";
      dumper.Dump(ent->Axis(),S, sublevel);
      S << endl;
      S << "Reference direction : ";
      dumper.Dump(ent->ReferenceDir(),S, sublevel);
      S << endl;
    }
  else S << "Surface is UnParametrised" << endl;
}
