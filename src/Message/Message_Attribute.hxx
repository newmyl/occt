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

#ifndef _Message_Attribute_HeaderFile
#define _Message_Attribute_HeaderFile

#include <Standard_Transient.hxx>
#include <TCollection_AsciiString.hxx>

DEFINE_STANDARD_HANDLE(Message_Attribute, Standard_Transient)

//! Additional information of extended alert attribute
//! To provide other custom attribute container, it might be redefined.
class Message_Attribute : public Standard_Transient
{
public:
  //! Empty constructor
  Standard_EXPORT Message_Attribute (const TCollection_AsciiString& theName = TCollection_AsciiString(),
                                     const TCollection_AsciiString& theDescription = TCollection_AsciiString())
    : myName (theName), myDescription (theDescription) {}

  //! Return a C string to be used as a key for generating text user 
  //! messages describing this alert.
  //! The messages are generated with help of Message_Msg class, in
  //! Message_Report::Dump().
  //! Base implementation returns dynamic type name of the instance.
  virtual Standard_EXPORT Standard_CString GetMessageKey () const;

  //! Returns custom name of alert if it is set
  //! @return alert name
  const TCollection_AsciiString& GetName() const { return myName; }

  //! Sets the custom name of alert
  //! @param theName a name for the alert
  void SetName (const TCollection_AsciiString& theName) { myName = theName; }

  //! Sets desription of alert
  //! @param theName a name for the alert
  void SetDescription (const TCollection_AsciiString& theDescription) { myDescription = theDescription; }

  //! Returns description of alert if it is set
  //! @return alert description
  virtual const TCollection_AsciiString& GetDescription() const { return myDescription; }

  // OCCT RTTI
  DEFINE_STANDARD_RTTIEXT(Message_Attribute, Standard_Transient)

private:

  TCollection_AsciiString myName; //!< alert name, if defined is used in GetMessageKey
  TCollection_AsciiString myDescription; //!< alert description if defined

};

#endif // _Message_Attribute_HeaderFile
