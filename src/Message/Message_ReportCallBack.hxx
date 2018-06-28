// Created on: 2017-12-01
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

#ifndef _Message_ReportCallBack_HeaderFile
#define _Message_ReportCallBack_HeaderFile

#include <Standard_Handle.hxx>
#include <Standard_Transient.hxx>
#include <Standard_Type.hxx>

class Message_Alert;
class Message_ReportCallBack;
DEFINE_STANDARD_HANDLE(Message_ReportCallBack, Standard_Transient)

//! Interface to have possibility of processing message report events.
class Message_ReportCallBack : public Standard_Transient
{
public:

  //! Empty constructor
  Standard_EXPORT Message_ReportCallBack ();
  ~Message_ReportCallBack () {}

  //! Interface to be implemented in the child to process the message report event
  //! \param theAlert created or modified message alert
  virtual void Update(const Handle(Message_Alert)& theAlert) = 0;

  //! OCCT RTTI
  DEFINE_STANDARD_RTTIEXT(Message_ReportCallBack, Standard_Transient)
};

#endif // _Message_ReportCallBack_HeaderFile
