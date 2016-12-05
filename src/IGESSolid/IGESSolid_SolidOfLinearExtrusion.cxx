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

#include <gp_Dir.hxx>
#include <gp_GTrsf.hxx>
#include <IGESSolid_SolidOfLinearExtrusion.hxx>
#include <Interface_EntityIterator.hxx>

IMPLEMENT_STANDARD_RTTIEXT(IGESSolid_SolidOfLinearExtrusion,IGESData_IGESEntity)

void IGESSolid_SolidOfLinearExtrusion::Init
  (const Handle(IGESData_IGESEntity)& aCurve,
   const Standard_Real Length, const gp_XYZ& Direction)
{
  theCurve     = aCurve;
  theLength    = Length;
  theDirection = Direction;           // default (0,0,1)
  InitTypeAndForm(164,0);
}

gp_Dir IGESSolid_SolidOfLinearExtrusion::ExtrusionDirection () const
{
  return gp_Dir(theDirection);
}

gp_Dir IGESSolid_SolidOfLinearExtrusion::TransformedExtrusionDirection () const
{
  if (!HasTransf()) return gp_Dir(theDirection);
  else
    {
      gp_XYZ tmp = theDirection;
      gp_GTrsf loc = Location();
      loc.SetTranslationPart(gp_XYZ(0.,0.,0.));
      loc.Transforms(tmp);
      return gp_Dir(tmp);
    }
}

void IGESSolid_SolidOfLinearExtrusion::OwnShared(Interface_EntityIterator &theIter) const
{
  theIter.GetOneItem(Curve());
}

void IGESSolid_SolidOfLinearExtrusion::OwnCheck (const Interface_ShareTool &, const Handle(Interface_Check) &theCheck) const
{
  if (ExtrusionLength() <= 0.0)
    theCheck->AddFail("Length of extrusion : Not Positive");
}
