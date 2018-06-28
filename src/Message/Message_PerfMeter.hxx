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

#ifndef _Message_PerfMeter_HeaderFile
#define _Message_PerfMeter_HeaderFile

#include <Message_AlertExtended.hxx>
#include <Message_Gravity.hxx>

#include <NCollection_DataMap.hxx>
#include <Standard.hxx>

//! This class is performance meter for message alerts. The usage is create the simple instance of it (not a pointer)
//! in a method. Finalizing of the method will call destructor of the meter and store time into alert.
class Message_PerfMeter
{

public:
  //! Constructs and starts (if autoStart is true) the named meter
  //! One string key is used for all alert meters. The perf meter is not started automatically, it will be done in
  //! AddAlert method
  Message_PerfMeter() : myGravity (Message_Info) {}

  //! Assures stopping upon destruction
  Standard_EXPORT ~Message_PerfMeter();

  //! Returns message alert on the level, by default the upper alert is returned
  //! \param theLevelId a level of child position inside the upper alert of performance meter
  //! \return alert instance or NULL
  Handle(Message_AlertExtended) GetAlert (const Standard_Integer theLevelId = 0) const;

  //! Processes the parameter alert. There are some variants:
  //! - current alert is NULL, the alert becomes the current one and perf meter is started
  //! - last alert of the current alert is stopped (perf meter becomes NULL, time is calculated),
  //! the parameter alert is started (perf meter becomes the current one)
  Standard_EXPORT void AddAlert (const Handle(Message_Alert)& theAlert);

protected:

  //! Sets message alert on the level
  //! \param theLevelId a level of child position inside the upper alert of performance meter
  //! \param theAlert an alert to be stored
  //! \return true if succeeded, false if there is stored alert on the given level
  Standard_Boolean setAlert (const Standard_Integer theLevelId, const Handle(Message_Alert)& theAlert);

  //! Stop alert, store time into the alert and last child alert
  //! \param theLevelId a level of child position inside the upper alert of performance meter
  Standard_Boolean releaseAlert (const Standard_Integer theLevelId);

  //! Sets NULL perf meter and parameter time into the alert
  //! \param theLevelId a level of child position inside the upper alert of performance meter
  Standard_Boolean stopAlert (const Standard_Integer theLevelId);

  //! Returns level of given alert if it participates in the performance meter
  //! \param theAlert modified alert
  //! \return level value or -1
  Standard_Integer getLevel (const Handle(Message_Alert)& theAlert);

  //! Returns cumulative time of the performance meter
  //! \param theMeterId a performance meter index
  //! \return time value
  static Standard_Real getCurrentTime();

protected:
  //! Struct to store start time for the alert
  struct AlertInfo
  {
    AlertInfo (const Handle(Message_AlertExtended)& theAlert, Standard_Real theStartTime)
      : myAlert (theAlert), myStartTime (theStartTime) {}

    Handle(Message_AlertExtended) myAlert; //!< an alert
    Standard_Real myStartTime; //!< start time of the alert
  };

  NCollection_DataMap<Standard_Integer, AlertInfo> myActiveAlerts; //!< container of current alerts
  Message_Gravity myGravity; //!< perf meter alert gravity
};


#endif // _Message_PerfMeter_HeaderFile
