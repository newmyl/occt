// Created on: 2017-06-26
// Created by: Andrey Betenev
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

#ifndef _Message_Report_HeaderFile
#define _Message_Report_HeaderFile

#include <Message_Gravity.hxx>
#include <Message_ListOfAlert.hxx>
#include <Standard_Mutex.hxx>

class Message_Messenger;
class Message_PerfMeter;
class Message_Report;
class Message_ReportCallBack;

DEFINE_STANDARD_HANDLE(Message_Report, Standard_Transient)

//! Container for alert messages, sorted according to their gravity.
//! 
//! For each gravity level, alerts are stored in simple list.
//! If alert being added can be merged with another alert of the same
//! type already in the list, it is merged and not added to the list.
//! 
//! This class is intended to be used as follows:
//!
//! - In the process of execution, algorithm fills report by alert objects
//!   using methods AddAlert()
//!
//! - The result can be queried for presence of particular alert using 
//!   methods HasAlert()
//!
//! - The reports produced by nested or sequentially executed algorithms
//!   can be collected in one using method Merge()
//!
//! - The report can be shown to the user either as plain text with method 
//!   Dump() or in more advanced way, by iterating over lists returned by GetAlerts()
//!
//! - Report can be cleared by methods Clear() (usually after reporting)

class Message_Report : public Standard_Transient
{
public:

  //! Empty constructor
  Standard_EXPORT Message_Report ();
  
  //! Add alert with specified gravity.
  //! This method is thread-safe, i.e. alerts can be added from parallel threads safely.
  Standard_EXPORT void AddAlert (Message_Gravity theGravity, const Handle(Message_Alert)& theAlert);

  //! Add alert with specified gravity. The alert is a sub-alert of report or another alert
  //! The parent alert is the parameter alert or the last alert if found else report.
  //! \param theGravity a message gravity
  //! \param theAlert a new alert to be added
  //! \param thePerfMeter performance meter calculates the alert spent time and participate in searching the last alert if needed
  //! \param theParentAlert if not NULL specifies parent alert where the alert should be placed, if the parent is an extednded alert
  Standard_EXPORT void AddAlert (const Message_Gravity theGravity, const Handle(Message_Alert)& theAlert,
                                 Message_PerfMeter* thePerfMeter, const Handle(Message_Alert)& theParentAlert = Handle(Message_Alert)());

  //! Returns list of collected alerts with specified gravity
  Standard_EXPORT const Message_ListOfAlert& GetAlerts (Message_Gravity theGravity) const;

  //! Returns true if specific type of alert is recorded
  Standard_EXPORT Standard_Boolean HasAlert (const Handle(Standard_Type)& theType);

  //! Returns true if specific type of alert is recorded with specified gravity
  Standard_EXPORT Standard_Boolean HasAlert (const Handle(Standard_Type)& theType, Message_Gravity theGravity);

  //! Clears all collected alerts
  Standard_EXPORT void Clear ();

  //! Clears collected alerts with specified gravity
  Standard_EXPORT void Clear (Message_Gravity theGravity);

  //! Clears collected alerts with specified type
  Standard_EXPORT void Clear (const Handle(Standard_Type)& theType);

  //! Dumps all collected alerts to stream
  Standard_EXPORT void Dump (Standard_OStream& theOS);

  //! Dumps collected alerts with specified gravity to stream
  Standard_EXPORT void Dump (Standard_OStream& theOS, Message_Gravity theGravity);

  //! Sends all collected alerts to messenger
  Standard_EXPORT void SendMessages (const Handle(Message_Messenger)& theMessenger);

  //! Dumps collected alerts with specified gravity to messenger
  Standard_EXPORT void SendMessages (const Handle(Message_Messenger)& theMessenger, Message_Gravity theGravity);

  //! Merges data from theOther report into this
  Standard_EXPORT void Merge (const Handle(Message_Report)& theOther);

  //! Merges alerts with specified gravity from theOther report into this
  Standard_EXPORT void Merge (const Handle(Message_Report)& theOther, Message_Gravity theGravity);

  //! Returns a state whether the report is active
  //! \return the state
  Standard_Boolean IsActive (const Message_Gravity theGravity) const { return myIsActive[theGravity]; }

  //! Starts a timer to compute time between messages. If a timer has already been started, it will be stopped
  //! \param theActive new state of report active
  //! \param theGravity gravity type, if '-1', apply value for all gravity kinds
  Standard_EXPORT void SetActive (const Standard_Boolean theActive, const Standard_Integer theGravity = -1);

  //! Sets a state whether the time of each alert should be stored
  //! \param theValue boolean state
  //! \return the previous state
  Standard_EXPORT void UsePerfMeter (const Standard_Boolean theValue)
  { myIsUsePerfMeter = theValue; }

  //! Returns a state whether the time of each alert is stored
  //! \return the state
  Standard_EXPORT Standard_Boolean IsUsePerfMeter () const
  { return myIsUsePerfMeter; }

  //! Returns maximum number of collecting alerts. If the limit is achieved,
  //! adding of a new alert after removing the first cached alert.
  //! \return the limit value
  Standard_EXPORT Standard_Integer GetLimit() const { return myLimit; }

  //! Sets maximum number of collecting alerts.
  //! \return theLimit limit value
  Standard_EXPORT void SetLimit(const Standard_Integer theLimit) { myLimit = theLimit; }

  //! Sets a listener for the report events
  void SetCallBack(const Handle(Message_ReportCallBack)& theCallBack) { myCallBack = theCallBack; }

  //! Returns listener of the reports events
  const Handle(Message_ReportCallBack)& GetCallBack() const { return myCallBack; }

  // OCCT RTTI
  DEFINE_STANDARD_RTTIEXT(Message_Report,Standard_Transient)

protected:
  //! Returns last alert in list of report alerts or last alert in hierarchical tree of alerts provided by
  //! the last alert
  //! \parm theGravity a message gravity
  Standard_EXPORT Handle(Message_Alert) getLastAlert (const Message_Gravity theGravity);

protected:
  Standard_Mutex myMutex;

  // store messages in a lists sorted by gravity;
  // here we rely on knowledge that Message_Fail is the last element of the enum
  Message_ListOfAlert myAlerts[Message_Fail + 1];

  Standard_Boolean myIsActive[Message_Fail + 1];
  Standard_Boolean myIsUsePerfMeter; //! If true, each alert will store time
  Standard_Integer myLimit; //! Maximum number of collected alerts

  Handle(Message_ReportCallBack) myCallBack; //! signal about performed modifications
};

#endif // _Message_Report_HeaderFile
