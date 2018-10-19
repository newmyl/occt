// Created on: 1997-08-08
// Created by: Jean-Louis Frenkel
// Copyright (c) 1997-1999 Matra Datavision
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


#include <CDF_Application.hxx>
#include <CDF_Directory.hxx>
#include <CDF_FWOSDriver.hxx>
#include <CDF_MetaDataDriver.hxx>
#include <CDF_MetaDataDriverFactory.hxx>
#include <CDF_Session.hxx>
#include <PCDM.hxx>
#include <PCDM_Reader.hxx>
#include <Plugin.hxx>
#include <Standard_ErrorHandler.hxx>
#include <Standard_Failure.hxx>
#include <Standard_GUID.hxx>
#include <Standard_MultiplyDefined.hxx>
#include <Standard_NoSuchObject.hxx>
#include <Standard_Type.hxx>
#include <TCollection_ExtendedString.hxx>
#include <Standard_Mutex.hxx>

IMPLEMENT_STANDARD_RTTIEXT(CDF_Session,Standard_Transient)

static Handle(CDF_Session) CS;
static Standard_Mutex gMutex;
//=======================================================================
//function : 
//purpose  : 
//=======================================================================
CDF_Session::CDF_Session ()
{  
  Standard_MultiplyDefined_Raise_if(!CS.IsNull()," a session already exists");
  CS = this;
}

//=======================================================================
//function : Exists
//purpose  : 
//=======================================================================
Standard_Boolean CDF_Session::Exists() {
  return !CS.IsNull();
}

//=======================================================================
//function : CurrentSession
//purpose  : 
//=======================================================================
Handle(CDF_Session) CDF_Session::CurrentSession() {
  Standard_NoSuchObject_Raise_if(CS.IsNull(), "no session has been created");
  return CS;
}

//=======================================================================
//function : AddApplication
//purpose  : adds the application to the session with unique name
//=======================================================================
Standard_Boolean CDF_Session::AddApplication(const Handle(CDF_Application)& theApp,
  const Standard_ThreadId theID)
{
  Standard_Boolean aRetValue(Standard_False);
  if (!theApp.IsNull())
  {
    gMutex.Lock();
    if (!myAppDirectory.IsBound(theID))
    {
      aRetValue = myAppDirectory.Bind(theID, theApp);        
    }
    gMutex.Unlock();
  }
  return aRetValue;
}

//=======================================================================
//function : FindApplication
//purpose  : 
//=======================================================================
Standard_Boolean CDF_Session::FindApplication(const Standard_ThreadId theID, Handle(CDF_Application)& theApp) const
{
  if (myAppDirectory.IsBound(theID))
  {
    return myAppDirectory.Find(theID, theApp);
  }
  return Standard_False;
}
//=======================================================================
//function : RemoveApplication
//purpose  : removes the application with name=<theName> from the session
//=======================================================================
Standard_Boolean CDF_Session::RemoveApplication(const Standard_ThreadId theID)
{
  Standard_Boolean aRetValue(Standard_False);
  gMutex.Lock();
  if (myAppDirectory.IsBound(theID))
  {
    aRetValue = myAppDirectory.UnBind(theID);
  }
  gMutex.Unlock();
  return aRetValue;
}
