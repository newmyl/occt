// Created on: 2002-02-22
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

#ifndef _Message_ProgressScope_HeaderFile
#define _Message_ProgressScope_HeaderFile

#include <Standard_TypeDef.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

class Message_ProgressIndicator;

//! Message_ProgressScope class provides convenient way to advance progress
//! indicator in context of complex program organized in hierarchical way,
//! where usually it is difficult (or even not possible) to consider process 
//! as linear with fixed step.
//!
//! The idea is that on each level (sub-operation) in hierarchy of operations
//! the local instance of the Message_ProgressScope class is created.
//! It reserves part of the upper-level scope and provides a way to 
//! consider this part as independent scale with locally defined range. 
//!
//! The position on the local scale may be advanced using one of
//! supported methods:
//! - Iteration-like advancement by specified steps
//! - Direct advance of the progress value
//! - Creation of a progress sub-scope on a specified range
//!
//! It is important that sub-scope must have life time less than 
//! the life time of its parent scope.
//!
//! The scope has a name that can be used in visualization of the progress.
//! It can be null. Note that the string is not copied, just pointer is stored.
//! So, the pointer must point to the string with life time
//! greater than the scope object.
//!
//! In multithreaded programs, for each task running concurrently it is recommended
//! creation of a separate progress scope. The same instance of the progress scope
//! must not be used concurrently from different threads.
//!
//! Note that all methods accepting pointers will safely accept null;
//! in this case no progress indication will be done.
//!
//! Example of usage in nested process:
//!
//! @code{.cpp}
//!   Handle(Draw_ProgressIndicator) aProgress = ...;
//!
//!   // Outer cycle
//!   Message_ProgressScope anOuter (aProgress, "Outer", 0, nbOuter, 1);
//!   for (Standard_Integer i = 0; i < nbOuter && anOuter.More(); i++, anOuter.Next())
//!   {
//!     // Inner cycle
//!     Message_ProgressScope anInner (anOuter, "Inner", 0, nbInner, 1);
//!     for (Standard_Integer j = 0; j < nbInner && anInner.More(); j++, anInner.Next())
//!     {
//!       // Cycle body
//!     }
//!   }
//! @endcode
class Message_ProgressScope 
{
public: //! @name Preparation methods

  //! Creates a top-level scope and selects parameters of the top-level scale.
  Standard_EXPORT Message_ProgressScope(const Handle(Message_ProgressIndicator)& theProgress,
                                        Standard_CString theName,
                                        Standard_Real theMin = 0, Standard_Real theMax = 100,
                                        Standard_Real theStep = 1, Standard_Boolean isInfinite = false);

  //! Creates a sub-scope covering current step of the parent scope, 
  //! and selects parameters of the current scale.
  Standard_EXPORT Message_ProgressScope(Message_ProgressScope* theParent,
                                        Standard_CString theName,
                                        Standard_Real theMin = 0, Standard_Real theMax = 100,
                                        Standard_Real theStep = 1, Standard_Boolean isInfinite = false);

  //! Set parameters for the scale of this scope.
  //! Note that this function must be called before advancing the progress in this scope.
  Standard_EXPORT void SetScale(double theMin, double theMax, double theStep, bool isInfinite = true);

  //! Sets the name of the scope.
  Standard_EXPORT void SetName(Standard_CString theName);

  //! Sets step size for the following steps
  void SetStep(Standard_Real theStep)
  {
    myStep = theStep;
  }

public: //! @name Advance by iterations

  //! Returns true if ProgressIndicator signals UserBreak
  Standard_EXPORT Standard_Boolean UserBreak() const;

  //! Returns false if ProgressIndicator signals UserBreak
  Standard_Boolean More() const
  {
    return !UserBreak();
  }

  //! Advances position by default step
  void Next()
  {
    Next(myStep);
  }

  //! Advances position by specified step
  Standard_EXPORT void Next(Standard_Real theStep);

public: //! @name Auxiliary methods to use in ProgressIndicator

  //! Returns the name of the scope (may be null)
  Standard_CString GetName() const
  {
    return myName;
  }

  //! Returns parent scope (null for top-level scope)
  const Message_ProgressScope* GetParent() const
  {
    return myParent;
  }

  //! Returns parent scope (null for top-level scope)
  Message_ProgressScope* GetParent()
  {
    return myParent;
  }

public: //! @name Destrucion, allocation

  //! Destructor - closes the scope and adds its scale to the total progress
  ~Message_ProgressScope()
  {
    Close();
  }

  //! Closes the scope and adds its scale to the total progress
  Standard_EXPORT void Close();

  DEFINE_STANDARD_ALLOC

protected: //! @name Internal methods

  //! Adds the child scope, reserving in the scale the space for it
  //! corresponding to the current step
  Standard_EXPORT void addScope(Message_ProgressScope* theChild);

  //! Remove the child scope
  void removeScope()
  {
    myNbChild--;
  }

  //! Convert value from this scale to global one 
  Standard_Real localToGlobal(const Standard_Real theVal, const Standard_Boolean isInf) const;

private:

  Handle(Message_ProgressIndicator) myProgress; //!< Handle of progress indicator instance
  Message_ProgressScope* myParent;  //!< Pointer to parent scope

  Standard_Real      myStart;       //!< Start position of this scope on total progress scale
  Standard_Real      myEnd;         //!< End position of this scope on total progress scale

  Standard_Real      myMin;         //!< Start of logical range of this scope
  Standard_Real      myMax;         //!< End of logical range of this scope
  Standard_Real      myStep;        //!< Default step 
  Standard_Real      myPosition;    //!< Current position (reserved by sub-scopes
                                    //!  or advanced directly)

  Standard_CString   myName;        //!< Name of the operation being done in this scope, or null
  Standard_Integer   myNbChild;     //!< Number of children - used to control code consistency
  Standard_Boolean   myIsInfinite;  //!< Option to advance by hyperbolic law

};

#endif // _Message_ProgressScope_HeaderFile
