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
//! Example of preparation of progress indicator:
//!
//! @code{.cpp}
//!   Handle(Message_ProgressIndicator) aProgress = new MyProgressIndicator;
//!   Message_ProgressScope* aRootPS = aProgress->GetRootScope();
//!   aRootPS->SetName("myAlgo");
//!   aRootPS->SetScale(0, 1, 1);
//!   myAlgo.Perform(aRootPS);
//! @endcode
//!
//! Example of usage in sequential process:
//!
//! @code{.cpp}
//!   Message_ProgressScope aWholePS(aRootPS, "Whole process", 0, 100, 1);
//!   aWholePS.SetStep(20);
//!   // ... do one step taking 20%
//!   aWholePS.Next();
//!   if (aWholePS.UserBreak())
//!     return;
//!
//!   aWholePS.SetStep(50);
//!   // ... do next step taking 50%
//!   aWholePS.Next();
//!   if (aWholePS.UserBreak())
//!     return;
//!
//!   aWholePS.SetStep(30);
//!   // ... do the last step taking 30%
//!   aWholePS.Next();
//!   if (aWholePS.UserBreak())
//!     return;
//! @endcode
//!
//! Example of usage in nested process:
//!
//! @code{.cpp}
//!   // Outer cycle
//!   Message_ProgressScope anOuter (aRoot, "Outer", 0, nbOuter, 1);
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
//!
//! Example of usage in parallel process:
//!
//! @code{.cpp}
//! struct Task
//! {
//!   Data* Data;
//!   Message_ProgressScope Progr;
//!
//!   Task(const Data& theData, Message_ProgressScope* theProgr)
//!     : Data(theData), Progr(theProgr, NULL, 0, 1, 1) {}
//!
//!   void operator()(Task& theTask) const
//!   {
//!     if (theTask.Progr.More())
//!     {
//!       // ... process data
//!       theTask.Progr.Next();
//!     }
//!   }
//! };
//! {
//!   std::vector<Data*> aData; // somehow got data
//!   std::vector<Task> aTasks;
//!
//!   Message_ProgressScope aPS(aRoot, "Data processing", 0, aData.size(), 1);
//!   for (Standard_Integer i = 0; i < aData.size(); ++i)
//!     aTasks.push_back(Task(aData[i], &aPS));
//!   
//!   OSD_Parallel::ForEach(aTasks.begin(), aTasks.end(), Task());
//! }
//! @endcode
class Message_ProgressScope
{
public: //! @name Preparation methods

  //! Creates a sub-scope covering current step of the parent scope, 
  //! and selects parameters of the current scale. The top most scope 
  //! must be got using the method GetRoopScope() of Message_ProgressIndicator.
  //! If theParent is null then an empty scope is created that does nothing.
  Standard_EXPORT Message_ProgressScope(Message_ProgressScope* theParent,
                                        Standard_CString theName,
                                        Standard_Real theMin = 0, Standard_Real theMax = 100,
                                        Standard_Real theStep = 1, Standard_Boolean isInfinite = false);

  //! Copy constructor. It is saved for compatibility with old compilers where
  //! move semantic is not supported. Then it makes possible to insert an empty scope 
  //! to a container and then move the real scope to the instance in the container.
  Standard_EXPORT Message_ProgressScope(const Message_ProgressScope& theOther);

  //! Copy assignment. Avoid using this method if theOther is connected
  //! to a real indicator. Use Move() instead.
  Standard_EXPORT Message_ProgressScope& operator=(const Message_ProgressScope& theOther);

#ifndef OCCT_NO_RVALUE_REFERENCE

  //! Move constructor. It makes possible to move a prepared scope to a container.
  Message_ProgressScope(Message_ProgressScope&& theOther)
  {
    Move(theOther);
  }

  //! Move assignment. It makes possible to move a prepared scope to a container.
  Message_ProgressScope& operator=(Message_ProgressScope&& theOther)
  {
    return Move(theOther);
  }
#endif

  //! Move assignment.
  //! This scope will take responsibility of updating progress indicator from theOther.
  //! The moved object will forget pointer to the parent and the indicator.
  //! This is useful when putting the progress scope into a container.
  //! If you just make a copy then the indicator will get increments 
  //! from the destructors twice. Therefore it is needed to Move temporary object to
  //! the container.
  Standard_EXPORT Message_ProgressScope& Move(Message_ProgressScope& theOther);

  //! Set parameters for the scale of this scope.
  //! Note that this function must be called before advancing the progress in this scope.
  Standard_EXPORT void SetScale(Standard_Real theMin, Standard_Real theMax,
                                Standard_Real theStep, Standard_Boolean isInfinite = false);

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

  //! Returns the maximal value of progress in this scope
  Standard_Real GetMax() const
  {
    return myMax;
  }

  //! Returns the current value of progress in this scope
  Standard_Real GetValue() const
  {
    return Max(myPosition, myPosBySS);
  }

  //! Returns the infinite flag
  Standard_Boolean IsInfinite() const
  {
    return myIsInfinite;
  }

  //! Get start position of this scope on total progress scale
  Standard_Real GetStart() const
  {
    return myStart;
  }

  //! Get end position of this scope on total progress scale
  Standard_Real GetEnd() const
  {
    return myEnd;
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
  
  //! Creates a top-level scope and selects parameters of the top-level scale.
  Standard_EXPORT Message_ProgressScope(Message_ProgressIndicator* theProgress,
                                        Standard_CString theName,
                                        Standard_Real theMin = 0, Standard_Real theMax = 100,
                                        Standard_Real theStep = 1, Standard_Boolean isInfinite = false);

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

  Message_ProgressIndicator* myProgress; //!< Handle of progress indicator instance
  Message_ProgressScope* myParent;  //!< Pointer to parent scope

  Standard_Real      myStart;       //!< Start position of this scope on total progress scale
  Standard_Real      myEnd;         //!< End position of this scope on total progress scale

  Standard_Real      myMax;         //!< Maximal value of progress in this scope
  Standard_Real      myStep;        //!< Default step 
  Standard_Real      myPosition;    //!< Current position advanced directly by Next
  Standard_Real      myPosBySS;     //!< Current position reserved by sub-scopes

  Standard_CString   myName;        //!< Name of the operation being done in this scope, or null
  Standard_Integer   myNbChild;     //!< Number of children - used to control code consistency
  Standard_Boolean   myIsInfinite;  //!< Option to advance by hyperbolic law

  friend class Message_ProgressIndicator;
};

#endif // _Message_ProgressScope_HeaderFile
