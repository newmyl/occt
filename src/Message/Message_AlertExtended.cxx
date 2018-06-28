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

#include <Message_AlertExtended.hxx>
#include <Message_Attribute.hxx>
#include <Message_CompositeAlerts.hxx>
#include <Message_Report.hxx>

#include <Standard_Assert.hxx>

IMPLEMENT_STANDARD_RTTIEXT(Message_AlertExtended,Message_Alert)

//=======================================================================
//function : GetMessageKey
//purpose  :
//=======================================================================

Standard_CString Message_AlertExtended::GetMessageKey () const
{
  if (myAttribute.IsNull())
    return Message_Alert::GetMessageKey();

  return myAttribute->GetMessageKey();
}

//=======================================================================
//function : GetCompositeAlerts
//purpose  : 
//=======================================================================

Handle (Message_CompositeAlerts) Message_AlertExtended::GetCompositeAlerts (const Standard_Boolean isCreate)
{
  if (myCompositAlerts.IsNull() && isCreate)
    myCompositAlerts = new Message_CompositeAlerts();

  return myCompositAlerts;
}

//=======================================================================
//function : AddAlert
//purpose  : 
//=======================================================================

Handle(Message_Alert) Message_AlertExtended::AddAlert (const Handle(Message_Report)& theReport,
                                                       const Handle(Message_Attribute)& theAttribute,
                                                       Message_PerfMeter* thePerfMeter,
                                                       const Handle(Message_Alert)& theParentAlert)
{
  if (!theReport->IsActive (Message_Info))
    return Handle(Message_Alert)();

  Handle(Message_AlertExtended) anAlert = new Message_AlertExtended();
  anAlert->SetAttribute (theAttribute);
  theReport->AddAlert (Message_Info, anAlert, thePerfMeter, theParentAlert);

  return anAlert;
}
