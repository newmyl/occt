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

#include <Message_Report.hxx>
#include <Message_Msg.hxx>
#include <Message_Messenger.hxx>
#include <NCollection_Map.hxx>

IMPLEMENT_STANDARD_RTTIEXT(Message_Report,Standard_Transient)

//=======================================================================
//function : Message_Report
//purpose  :
//=======================================================================

Message_Report::Message_Report ()
: myIsStoreElapsedTime (Standard_False), myLimit (-1)
{
  SetActive (Standard_True);
}

//=======================================================================
//function : AddAlert
//purpose  :
//=======================================================================

void Message_Report::AddAlert (Message_Gravity theGravity, const Handle(Message_Alert)& theAlert,
                               const Handle(Message_Alert)& theParentAlert)
{
  if (!IsActive (theGravity))
    return;

  Standard_ASSERT_RETURN (! theAlert.IsNull(), "Attempt to add null alert",);
  Standard_ASSERT_RETURN (theGravity >= 0 && size_t(theGravity) < sizeof(myAlerts)/sizeof(myAlerts[0]), 
                          "Adding alert with gravity not in valid range",);

  Standard_Mutex::Sentry aSentry (myMutex);

  // iterate by already recorded alerts and try to merge new one with one of those
  Message_ListOfAlert& aList = theParentAlert.IsNull() ? myAlerts[theGravity] : theParentAlert->GetAlerts (theGravity);

  if (theAlert->SupportsMerge() && !aList.IsEmpty())
  {
    // merge is performed only for alerts of exactly same type and same name
    const Handle(Standard_Type)& aType = theAlert->DynamicType();
    for (Message_ListOfAlert::Iterator anIt(aList); anIt.More(); anIt.Next())
    {
      // if merged successfully, just return
      if (aType == anIt.Value()->DynamicType() && theAlert->GetName().IsEqual (anIt.Value()->GetName()) && theAlert->Merge (anIt.Value()))
        return;
    }
  }

  // if not merged, just add to the list
  aList.Append (theAlert);
  // remove alerts under the report only
  if (theParentAlert.IsNull() && myLimit > 0 && aList.Extent() >= myLimit)
    aList.RemoveFirst();

  if (IsStoreElapsedTime())
  {
    if (!myAlertOfTimer.IsNull())
      myAlertOfTimer->SetElapsedTime (myAlertTimer.IsStarted() ? myAlertTimer.ElapsedTime() : 0/*the first alert has time 0*/);

    myAlertOfTimer = theAlert;
    if (myAlertTimer.IsStarted())
#if OCC_VERSION_HEX < 0x070201
    {
      myAlertTimer.Stop();
      myAlertTimer.Start();
    }
#else
      myAlertTimer.Restart();
#endif
    else
      myAlertTimer.Start();
  }

  if (!myCallBack.IsNull())
    myCallBack->Update();
}

//=======================================================================
//function : GetAlerts
//purpose  :
//=======================================================================

const Message_ListOfAlert& Message_Report::GetAlerts (Message_Gravity theGravity) const
{
  static const Message_ListOfAlert anEmptyList;
  Standard_ASSERT_RETURN (theGravity >= 0 && size_t(theGravity) < sizeof(myAlerts)/sizeof(myAlerts[0]), 
                          "Requesting alerts for gravity not in valid range", anEmptyList);
  return myAlerts[theGravity];
}

//=======================================================================
//function : GetLastAlert
//purpose  :
//=======================================================================

Handle(Message_Alert) Message_Report::GetLastAlert (Message_Gravity theGravity, const bool isFindInAlertChildren)
{
  if (!IsActive (theGravity))
    return Handle(Message_Alert)();

  if (!isFindInAlertChildren)
    return GetAlerts (theGravity).Last();

  const Message_ListOfAlert& anAlerts = GetAlerts (theGravity);
  if (anAlerts.IsEmpty())
    return Handle(Message_Alert)();

  Handle(Message_Alert) aLastAlert = anAlerts.Last();
  while (!aLastAlert.IsNull() && !aLastAlert->GetAlerts (theGravity).IsEmpty())
    aLastAlert = aLastAlert->GetAlerts (theGravity).Last();

  return aLastAlert;
}

//=======================================================================
//function : HasAlert
//purpose  :
//=======================================================================

Standard_Boolean Message_Report::HasAlert (const Handle(Standard_Type)& theType)
{
  for (int iGravity = Message_Trace; iGravity <= Message_Fail; ++iGravity)
  {
    if (HasAlert (theType, (Message_Gravity)iGravity))
      return Standard_True;
  }
  return Standard_False;
}

//=======================================================================
//function : HasAlert
//purpose  :
//=======================================================================

Standard_Boolean Message_Report::HasAlert (const Handle(Standard_Type)& theType, Message_Gravity theGravity)
{
  Standard_ASSERT_RETURN (theGravity >= 0 && size_t(theGravity) < sizeof(myAlerts)/sizeof(myAlerts[0]), 
                          "Requesting alerts for gravity not in valid range", Standard_False);
  for (Message_ListOfAlert::Iterator anIt (myAlerts[theGravity]); anIt.More(); anIt.Next())
  {
    if (anIt.Value()->IsInstance(theType))
      return Standard_True;
  }
  return Standard_False;
}

//=======================================================================
//function : Clear
//purpose  :
//=======================================================================

void Message_Report::Clear ()
{
  for (unsigned int i = 0; i < sizeof(myAlerts)/sizeof(myAlerts[0]); ++i)
  {
    myAlerts[i].Clear();
  }
}

//=======================================================================
//function : Clear
//purpose  :
//=======================================================================

void Message_Report::Clear (Message_Gravity theGravity)
{
  Standard_ASSERT_RETURN (theGravity >= 0 && size_t(theGravity) < sizeof(myAlerts)/sizeof(myAlerts[0]), 
                          "Requesting alerts for gravity not in valid range", );
  myAlerts[theGravity].Clear();
}

//=======================================================================
//function : Clear
//purpose  :
//=======================================================================

void Message_Report::Clear (const Handle(Standard_Type)& theType)
{
  for (unsigned int i = 0; i < sizeof(myAlerts)/sizeof(myAlerts[0]); ++i)
  {
    for (Message_ListOfAlert::Iterator anIt (myAlerts[i]); anIt.More(); )
    {
      if (anIt.Value().IsNull() || anIt.Value()->IsInstance (theType))
      {
        myAlerts[i].Remove (anIt);
      }
      else
      {
        anIt.More();
      }
    }
  }
}

//=======================================================================
//function : Dump
//purpose  :
//=======================================================================

void Message_Report::Dump (Standard_OStream& theOS)
{
  for (int iGravity = Message_Trace; iGravity <= Message_Fail; ++iGravity)
  {
    Dump (theOS, (Message_Gravity)iGravity);
  }
}

//=======================================================================
//function : Dump
//purpose  :
//=======================================================================

void Message_Report::Dump (Standard_OStream& theOS, Message_Gravity theGravity)
{
  Standard_ASSERT_RETURN (theGravity >= 0 && size_t(theGravity) < sizeof(myAlerts)/sizeof(myAlerts[0]), 
                          "Requesting alerts for gravity not in valid range", );

  // report each type of warning only once
  NCollection_Map<Handle(Standard_Type)> aPassedAlerts;
  for (Message_ListOfAlert::Iterator anIt (myAlerts[theGravity]); anIt.More(); anIt.Next())
  {
    if (aPassedAlerts.Add (anIt.Value()->DynamicType()))
    {
      Message_Msg aMsg (anIt.Value()->GetMessageKey());
      theOS << aMsg.Original() << std::endl;
    }
  }
}

//=======================================================================
//function : Dump
//purpose  :
//=======================================================================

void Message_Report::SendMessages (const Handle(Message_Messenger)& theMessenger)
{
  for (int iGravity = Message_Trace; iGravity <= Message_Fail; ++iGravity)
  {
    SendMessages (theMessenger, (Message_Gravity)iGravity);
  }
}

//=======================================================================
//function : Dump
//purpose  :
//=======================================================================

void Message_Report::SendMessages (const Handle(Message_Messenger)& theMessenger, Message_Gravity theGravity)
{
  Standard_ASSERT_RETURN (theGravity >= 0 && size_t(theGravity) < sizeof(myAlerts)/sizeof(myAlerts[0]), 
                          "Requesting alerts for gravity not in valid range", );

  // report each type of warning only once
  NCollection_Map<Handle(Standard_Type)> aPassedAlerts;
  for (Message_ListOfAlert::Iterator anIt (myAlerts[theGravity]); anIt.More(); anIt.Next())
  {
    if (aPassedAlerts.Add (anIt.Value()->DynamicType()))
    {
      Message_Msg aMsg (anIt.Value()->GetMessageKey());
      theMessenger->Send (aMsg, theGravity);
    }
  }
}

//=======================================================================
//function : Merge
//purpose  :
//=======================================================================

void Message_Report::Merge (const Handle(Message_Report)& theOther,
                            const Handle(Message_Alert)& theParentAlert)
{
  for (int iGravity = Message_Trace; iGravity <= Message_Fail; ++iGravity)
  {
    Merge (theOther, (Message_Gravity)iGravity, theParentAlert);
  }
}

//=======================================================================
//function : Merge
//purpose  :
//=======================================================================

void Message_Report::Merge (const Handle(Message_Report)& theOther, Message_Gravity theGravity,
  const Handle(Message_Alert)& theParentAlert)
{
  for (Message_ListOfAlert::Iterator anIt (theOther->GetAlerts(theGravity)); anIt.More(); anIt.Next())
  {
    AddAlert (theGravity, anIt.Value(), theParentAlert);
  }
}

//=======================================================================
//function : SetActive
//purpose  :
//=======================================================================

void Message_Report::SetActive (const Standard_Boolean& theActive, int theGravity)
{
  if (theGravity < 0)
  {
    for (int iGravity = Message_Trace; iGravity <= Message_Fail; ++iGravity)
      SetActive (theActive, iGravity);
    return;
  }

  Standard_ASSERT_RETURN (theGravity >= 0 && size_t (theGravity) < sizeof (myAlerts) / sizeof (myAlerts[0]), 
                          "Set active report with gravity not in valid range", );
  myIsActive[theGravity] = theActive;
}

//=======================================================================
//function : ApplyLastElapsedTime
//purpose  :
//=======================================================================

void Message_Report::ApplyLastElapsedTime()
{
  if (myAlertOfTimer.IsNull())
    return;

  myAlertOfTimer->SetElapsedTime (myAlertTimer.IsStarted() ? myAlertTimer.ElapsedTime() : 0/*the first alert has time 0*/);
  myAlertOfTimer = NULL;
}

