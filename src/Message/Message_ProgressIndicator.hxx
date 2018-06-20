// Created on: 2002-02-20
// Created by: Andrey BETENEV
// Copyright (c) 2002-2014 OPEN CASCADE SAS
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

#ifndef _Message_ProgressIndicator_HeaderFile
#define _Message_ProgressIndicator_HeaderFile

#include <Standard_TypeDef.hxx>
#include <Standard_Mutex.hxx>
#include <Standard_Handle.hxx>

class Message_ProgressScope;

DEFINE_STANDARD_HANDLE(Message_ProgressIndicator, Standard_Transient)

//! Defines abstract interface from program to the "user".
//! This includes progress indication and user break mechanisms.
//!
//! The progress indicator controls the progress scale with range from 0 to 1.
//! The caller has to divide its work on primary operations. Each iteration 
//! is assigned a sub-range. On the operation completion the caller calls the 
//! method Increment() to tell the indicator about the progress. The method
//! increment calls the method Show() to provide the user with update.
//! 
//! The recommended way of using progress indicator in the algorithm
//! is to use the class Message_ProgressSentry that provides iterator-like
//! interface for incrementing progress and opening nested scopes.
//!
//! The progress indicator supports concurrent processing and 
//! can be used in multithreaded applications.
//!
//! The user break is implemented as virtual function that should
//! return True in case if break signal from the user is received.
//!
//! The derived class should take care of visualisation of the
//! progress indicator (e.g. show total position at the graphical bar,
//! print scopes in text mode, or else), and for implementation
//! of user break mechanism (if necessary).

class Message_ProgressIndicator : public Standard_Transient
{

public:

  //! Resets the indicator to the initial state.
  //! The default implementation just nullifies position.
  virtual void Reset()
  {
    myPosition = 0.;
  }
  
  //! Should return True if user has send a break signal.
  //! Default implementation returns False.
  virtual Standard_Boolean UserBreak()
  {
    return Standard_False;
  }

  //! Update presentation of the progress indicator
  //! Called when progress position is changed.
  //! Flag isForce is intended for forcing update in case if it is
  //! optimized; all internal calls from ProgressIndicator are
  //! done with this flag equal to False.
  //! The parameter theName is (if it is not null) the title of the on-going operation.
  virtual Standard_Boolean Show (const Standard_Boolean isForce = Standard_True,
                                 const Message_ProgressScope* theScope = 0L) = 0;
  
  //! Returns total progress position ranged from 0. to 1.
  Standard_Real GetPosition() const
  {
    return myPosition;
  }

  DEFINE_STANDARD_RTTIEXT(Message_ProgressIndicator,Standard_Transient)

protected:
  
  //! Constructor
  Message_ProgressIndicator() : myPosition(0.) {}

  //! Increment the progress value by the specified step.
  //! The parameter theScope is the pointer to the calling 
  //! object; this feature can be used to track context of the process.
  Standard_EXPORT void Increment(const Standard_Real theStep,
                                 const Message_ProgressScope* theScope = 0L);

private:

  Standard_Real myPosition;
  Standard_Mutex myMutex;

  friend class Message_ProgressScope;
};

#endif // _Message_ProgressIndicator_HeaderFile
