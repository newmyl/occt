// Created on: 2018-06-20
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

#ifndef _MessageModel_ReportCallBack_HeaderFile
#define _MessageModel_ReportCallBack_HeaderFile

#include <Message_ReportCallBack.hxx>

#include "inspector/TInspectorAPI_Communicator.hxx"

class TInspectorAPI_Communicator;
DEFINE_STANDARD_HANDLE(MessageModel_ReportCallBack, Message_ReportCallBack)

//! Class to update tree view model by message report events.
class MessageModel_ReportCallBack : public Message_ReportCallBack
{
public:

  //! Constructor
  MessageModel_ReportCallBack (TInspectorAPI_Communicator* theWindow) : Message_ReportCallBack(), myCommunicator (theWindow) {}

  //! Destructor
  ~MessageModel_ReportCallBack() {}

  // Interface to be implemented in the child to process the message report event
  // \theAlert updated alert
  virtual void Update(const Handle(Message_Alert)& theAlert) { (void)theAlert; myCommunicator->UpdateContent(); }

  // OCCT RTTI
  DEFINE_STANDARD_RTTIEXT(MessageModel_ReportCallBack, Message_ReportCallBack)

private:
  TInspectorAPI_Communicator* myCommunicator; //< communicator
};

#endif // _MessageModel_ReportCallBack_HeaderFile
