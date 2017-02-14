// Created on: 2017-02-13
// Created by: Sergey NIKONOV
// Copyright (c) 2000-2017 OPEN CASCADE SAS
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

#ifndef _XCAFDoc_NoteBinData_HeaderFile
#define _XCAFDoc_NoteBinData_HeaderFile

#include <XCAFDoc_Note.hxx>

class OSD_File;
class TCollection_AsciiString;
class TCollection_ExtendedString;

class XCAFDoc_NoteBinData;
DEFINE_STANDARD_HANDLE(XCAFDoc_NoteBinData, XCAFDoc_Note)

class XCAFDoc_NoteBinData : public XCAFDoc_Note
{
public:

  DEFINE_STANDARD_RTTIEXT(XCAFDoc_NoteBinData, XCAFDoc_Note)

  Standard_EXPORT static const Standard_GUID& GetID();

  Standard_EXPORT static Standard_Boolean IsMine(const TDF_Label& theLabel);

  Standard_EXPORT static Handle(XCAFDoc_NoteBinData) Set(const TDF_Label&                  theLabel,
                                                         const TCollection_ExtendedString& theUserName,
                                                         const TCollection_ExtendedString& theTimeStamp,
                                                         const TCollection_ExtendedString& theTitle,
                                                         OSD_File&                         theFile,
                                                         const TCollection_AsciiString&    theMIMEtype);

  Standard_EXPORT XCAFDoc_NoteBinData();

  Standard_EXPORT void Set(const TCollection_ExtendedString& theTitle, 
                           OSD_File&                         theFile,
                           const TCollection_AsciiString&    theMIMEtype);

  Standard_EXPORT void Set(const TCollection_ExtendedString& theTitle, 
                           const TCollection_AsciiString&    theData,
                           const TCollection_AsciiString&    theMIMEtype);

  Standard_EXPORT const TCollection_ExtendedString& Title() const;
  Standard_EXPORT const TCollection_AsciiString& Data() const;
  Standard_EXPORT const TCollection_AsciiString& MIMEtype() const;

public:

  Standard_EXPORT const Standard_GUID& ID() const Standard_OVERRIDE;

  Standard_EXPORT Handle(TDF_Attribute) NewEmpty() const Standard_OVERRIDE;

  Standard_EXPORT void Restore(const Handle(TDF_Attribute)& theAttrFrom) Standard_OVERRIDE;

  Standard_EXPORT void Paste(const Handle(TDF_Attribute)&       theAttrInto,
                             const Handle(TDF_RelocationTable)& theRT) const Standard_OVERRIDE;

  Standard_EXPORT Standard_OStream& Dump(Standard_OStream& theOS) const Standard_OVERRIDE;

protected:

  TCollection_ExtendedString myTitle;
  TCollection_AsciiString myData;
  TCollection_AsciiString myMIMEtype;
};

#endif // _XCAFDoc_NoteBinData_HeaderFile
