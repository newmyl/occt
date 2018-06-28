// Created on: 2018-06-10
// Created by: Natalia Ermolaeva
// Copyright (c) 2017 OPEN CASCADE SAS
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

#include <Message_CompositeAlerts.hxx>
#include <Message_AlertExtended.hxx>
#include <Standard_Assert.hxx>

IMPLEMENT_STANDARD_RTTIEXT(Message_CompositeAlerts, Standard_Transient)

//=======================================================================
//function : Merge
//purpose  : 
//=======================================================================

Standard_Boolean Message_CompositeAlerts::Merge (const Handle(Message_Alert)& theTarget)
{
  Handle(Message_AlertExtended) anExtAlert = Handle(Message_AlertExtended)::DownCast (theTarget);
  if (!anExtAlert.IsNull() && !anExtAlert->GetCompositeAlerts().IsNull())
  {
    Handle(Message_CompositeAlerts) anExtCompositeAlert = anExtAlert->GetCompositeAlerts();
    // hierarchical alerts can not be merged
    for (int iGravity = Message_Trace; iGravity <= Message_Fail; ++iGravity)
    {
      if (!GetAlerts ((Message_Gravity)iGravity).IsEmpty() ||
          !anExtCompositeAlert->GetAlerts ((Message_Gravity)iGravity).IsEmpty())
        return Standard_False;
    }
  }
  // by default, merge trivially
  return Standard_True;
}

//=======================================================================
//function : GetAlerts
//purpose  :
//=======================================================================

Message_ListOfAlert& Message_CompositeAlerts::GetAlerts (const Message_Gravity theGravity)
{
  return myChildAlerts[theGravity];
}

//=======================================================================
//function : HasAlerts
//purpose  :
//=======================================================================

Standard_Boolean Message_CompositeAlerts::HasAlert (const Handle(Message_Alert)& theAlert)
{
  for (int iGravity = Message_Trace; iGravity <= Message_Fail; ++iGravity)
  {
    Message_ListOfAlert& anAlerts = GetAlerts ((Message_Gravity)iGravity);
    if (anAlerts.Contains (theAlert))
      return Standard_True;
  }
  return Standard_False;
}
