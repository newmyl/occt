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

#ifndef _Message_CompositeAlerts_HeaderFile
#define _Message_CompositeAlerts_HeaderFile

#include <Message_Alert.hxx>
#include <Message_Gravity.hxx>
#include <Message_ListOfAlert.hxx>
#include <Standard_Transient.hxx>

class Message_CompositeAlerts;
DEFINE_STANDARD_HANDLE(Message_CompositeAlerts, Standard_Transient)

//! Base class of the hierarchy of classes describing various situations
//! occurring during execution of some algorithm or procedure.
//!
//! Alert should provide unique text identifier that can be used to distinguish 
//! particular type of alerts, e.g. to get text message string describing it. 
//! See method GetMessageKey(); by default, dynamic type name is used.
//!
//! Alert can contain some data. To avoid duplication of data, new alert
//! can be merged with another one of the same type. Method SupportsMerge() 
//! should return true if merge is supported; method Merge() should do the
//! merge if possible and return true in that case and false otherwise.
//! 
class Message_CompositeAlerts : public Standard_Transient
{
public:
  //! Empty constructor
  Standard_EXPORT Message_CompositeAlerts () {}

  //! If possible, merge data contained in this alert to theTarget.
  //! @return True if merged.
  //! Base implementation always returns true.
  virtual Standard_EXPORT Standard_Boolean Merge (const Handle(Message_Alert)& theTarget);
  
  //! Returns list of collected alerts with specified gravity
  Standard_EXPORT Message_ListOfAlert& GetAlerts (const Message_Gravity theGravity);

  //! Returns true if the alert belong the list of the child alerts.
  //! \param theAlert an alert to be checked as a child alert
  //! \return true if the alert is found in a container of children
  Standard_EXPORT Standard_Boolean HasAlert (const Handle(Message_Alert)& theAlert);

  // OCCT RTTI
  DEFINE_STANDARD_RTTIEXT(Message_CompositeAlerts,Standard_Transient)

protected:
  // store messages in a lists sorted by gravity;
  // here we rely on knowledge that Message_Fail is the last element of the enum
  Message_ListOfAlert myChildAlerts[Message_Fail + 1];
};

#endif // _Message_CompositeAlerts_HeaderFile
