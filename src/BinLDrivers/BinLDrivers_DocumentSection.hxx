// Created by: Alexander GRIGORIEV
// Copyright (c) 2007-2014 OPEN CASCADE SAS
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

#ifndef _BinLDrivers_DocumentSection_HeaderFile
#define _BinLDrivers_DocumentSection_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <TCollection_AsciiString.hxx>
#include <Standard_Size.hxx>
#include <Standard_Boolean.hxx>
#include <Standard_OStream.hxx>
#include <Standard_IStream.hxx>
#include <Storage_IODevice.hxx>
class TCollection_AsciiString;


//! More or less independent part of the saved/restored document
//! that is distinct from OCAF data themselves but may be referred
//! by them.
class BinLDrivers_DocumentSection 
{
public:

  DEFINE_STANDARD_ALLOC

  
  //! Empty constructor
  Standard_EXPORT BinLDrivers_DocumentSection();
  
  //! Constructor
  Standard_EXPORT BinLDrivers_DocumentSection(const TCollection_AsciiString& theName, const Standard_Boolean isPostRead);
  
  //! Query the name of the section.
  Standard_EXPORT const TCollection_AsciiString& Name() const;
  
  //! Query the status: if the Section should be read after OCAF;
  //! False means that the Section is read before starting to
  //! read OCAF data.
  Standard_EXPORT Standard_Boolean IsPostRead() const;
  
  //! Query the offset of the section in the persistent file
  Standard_EXPORT Standard_Size Offset() const;
  
  //! Set the offset of the section in the persistent file
  Standard_EXPORT void SetOffset (const Standard_Size theOffset);
  
  //! Query the length of the section in the persistent file
  Standard_EXPORT Standard_Size Length() const;
  
  //! Set the length of the section in the persistent file
  Standard_EXPORT void SetLength (const Standard_Size theLength);
  
  //! Create a Section entry in the Document TOC (list of sections)
  Standard_EXPORT void WriteTOC (const Handle(Storage_IODevice)& theDevice);
  
  //! Save Offset and Length data into the Section entry
  //! in the Document TOC (list of sections)
  Standard_EXPORT void Write (const Handle(Storage_IODevice)& theDevice, const Standard_Size theOffset);
  
  //! Fill a DocumentSection instance from the data that are read
  //! from TOC.
  Standard_EXPORT static void ReadTOC (BinLDrivers_DocumentSection& theSection, const Handle(Storage_IODevice)& theDevice);




protected:





private:



  TCollection_AsciiString myName;
  Standard_Size myValue[2];
  Standard_Boolean myIsPostRead;


};







#endif // _BinLDrivers_DocumentSection_HeaderFile
