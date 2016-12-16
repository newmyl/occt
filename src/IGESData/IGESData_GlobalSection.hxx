// Created on: 1992-04-06
// Created by: Christian CAILLET
// Copyright (c) 1992-1999 Matra Datavision
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

#ifndef _IGESData_GlobalSection_HeaderFile
#define _IGESData_GlobalSection_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <Standard_Character.hxx>
#include <Standard_Integer.hxx>
#include <Standard_Real.hxx>
#include <Standard_Boolean.hxx>
class TCollection_HAsciiString;
class Interface_ParamSet;
class Interface_Check;
class gp_XYZ;


//! Description of a global section (corresponds to file header)
//! used as well in IGESModel, IGESReader and IGESWriter
//! Warning : From IGES-5.1, a parameter is added : LastChangeDate (concerns
//! transferred set of data, not the file itself)
//! Of course, it can be absent if read from earlier versions
//! (a default is then to be set to current date)
//! From 5.3, one more : ApplicationProtocol (optional)
class IGESData_GlobalSection 
{
 public:

  DEFINE_STANDARD_ALLOC

  //! Creates an empty GlobalSection, ready to be filled,
  //! Warning : No default value is provided
  IGESData_GlobalSection()
  : theSeparator(','),
    theEndMark(';'),
    theIntegerBits(32),
    theMaxPower10Single(38),
    theMaxDigitsSingle(6),
    theMaxPower10Double(308),
    theMaxDigitsDouble(15),
    theScale(1.),
    theUnitFlag(0),
    theLineWeightGrad(1),
    theMaxLineWeight(0.),
    theResolution(0.),
    theMaxCoord(0.),
    hasMaxCoord(Standard_False),
    theIGESVersion(11),
    theDraftingStandard(0)
  {}
  
  //! Fills GlobalSection from a ParamSet (i.e. taken from file)
  //! undefined parameters do not change default values when defined
  //! Fills Check about Corrections or Fails
  Standard_EXPORT void Init (const Handle(Interface_ParamSet)& params, Handle(Interface_Check)& ach);

  //! Copies data referenced by Handle (that is, Strings)
  //! usefull to "isolate" a GlobalSection after copy by "="
  //! (from a Model to another Model for instance)
  Standard_EXPORT void CopyRefs();
  
  //! Returns all contained data in the form of a ParamSet
  //! Remark : Strings are given under Hollerith form
  Standard_EXPORT Handle(Interface_ParamSet) Params() const;
  
  //! Returns the parameter delimiter character.
  Standard_Character Separator() const { return theSeparator; }

  //! Returns the record delimiter character.
  Standard_Character EndMark() const { return theEndMark; }

  //! Returns the name of the sending system.
  const Handle(TCollection_HAsciiString) & SendName() const { return theSendName; }

  //! Returns the name of the IGES file.
  const Handle(TCollection_HAsciiString) & FileName() const { return theFileName; }

  //! Returns the Native System ID of the system that created the IGES file.
  const Handle(TCollection_HAsciiString) & SystemId() const { return theSystemId; }

  //! Returns the name of the pre-processor used to write the IGES file.
  const Handle(TCollection_HAsciiString) & InterfaceVersion() const { return theInterfaceVersion; }

  //! Returns the number of binary bits for integer representations.
  Standard_Integer IntegerBits() const { return theIntegerBits; }

  //! Returns the maximum power of a decimal representation of a
  //! single-precision floating point number in the sending system.
  Standard_Integer MaxPower10Single() const { return theMaxPower10Single; }

  Standard_Integer MaxDigitsSingle() const { return theMaxDigitsSingle; }

  //! Returns the maximum power of a decimal representation of a
  //! double-precision floating point number in the sending system.
  Standard_Integer MaxPower10Double() const { return theMaxPower10Double; }

  Standard_Integer MaxDigitsDouble() const { return theMaxDigitsDouble; }

  //! Returns the name of the receiving system.
  const Handle(TCollection_HAsciiString) & ReceiveName() const { return theReceiveName; }

  //! Returns the scale used in the IGES file.
  Standard_Real Scale() const { return theScale; }

  //! Returns the unit flag that was used to write the IGES file.
  Standard_Integer UnitFlag() const { return theUnitFlag; }

  //! Returns the name of the unit the IGES file was written in.
  const Handle(TCollection_HAsciiString) & UnitName() const { return theUnitName; }

  //! Returns the maximum number of line weight gradations.
  Standard_Integer LineWeightGrad() const { return theLineWeightGrad; }

  //! Returns the of maximum line weight width in IGES file units.
  Standard_Real MaxLineWeight() const { return theMaxLineWeight; }

  //! Returns the IGES file creation date.
  const Handle(TCollection_HAsciiString) & Date() const { return theDate; }

  //! Returns the resolution used in the IGES file.
  Standard_Real Resolution() const { return theResolution; }

  //! Returns the approximate maximum coordinate value found in the model.
  Standard_Real MaxCoord() const { return theMaxCoord; }

  //! Returns True if the approximate maximum coordinate value found in
  //! the model is greater than 0.
  Standard_Boolean HasMaxCoord() const { return hasMaxCoord; }

  //! Returns the name of the IGES file author.
  const Handle(TCollection_HAsciiString) & AuthorName() const { return theAuthorName; }

  //! Returns the name of the company where the IGES file was written.
  const Handle(TCollection_HAsciiString) & CompanyName() const { return theCompanyName; }

  //! Returns the IGES version that the IGES file was written in.
  Standard_Integer IGESVersion() const { return theIGESVersion; }

  Standard_Integer DraftingStandard() const { return theDraftingStandard; }

  //! Returns the date and time when the model was created or last
  //! modified (for IGES 5.1 and later).
  const Handle(TCollection_HAsciiString) & LastChangeDate() const { return theLastChangeDate; }

  //! Returns True if the date and time when the model was created or
  //! last modified are specified, i.e. not defaulted to NULL.
  Standard_Boolean HasLastChangeDate() const { return (!theLastChangeDate.IsNull()); }

  Standard_EXPORT void SetLastChangeDate();
  
  const Handle(TCollection_HAsciiString) & ApplicationProtocol() const { return theAppliProtocol; }

  Standard_Boolean HasApplicationProtocol() const { return !theAppliProtocol.IsNull(); }

  //! Returns a string built from year,
  //! month, day, hour, minute and second values. The form of the
  //! resulting string is defined as follows:
  //! -      -1: YYMMDD.HHNNSS,
  //! -       0: YYYYMMDD.HHNNSS,
  //! -       1: YYYY-MM-DD:HH-NN-SS, where:
  //! - YYYY or YY is 4 or 2 digit year,
  //! - HH is hour (00-23),
  //! - MM is month (01-12),
  //! - NN is minute (00-59)
  //! - DD is day (01-31),
  //! - SS is second (00-59).
  Standard_EXPORT static Handle(TCollection_HAsciiString) NewDateString (const Standard_Integer year, const Standard_Integer month, const Standard_Integer day, const Standard_Integer hour, const Standard_Integer minut, const Standard_Integer second, const Standard_Integer mode = -1);
  
  //! Converts the string given in the
  //! form YYMMDD.HHNNSS or YYYYMMDD.HHNNSS to either
  //! YYMMDD.HHNNSS, YYYYMMDD.HHNNSS or YYYY-MM-DD:HH-NN-SS.
  Standard_EXPORT static Handle(TCollection_HAsciiString) NewDateString (const Handle(TCollection_HAsciiString)& date, const Standard_Integer mode = 1);
  
  //! Returns the unit value (in
  //! meters) that the IGES file was written in.
  Standard_EXPORT Standard_Real UnitValue() const;
  
  void SetSeparator (const Standard_Character val) { theSeparator = val; }

  void SetEndMark (const Standard_Character val) { theEndMark = val; }

  Standard_EXPORT void SetSendName (const Handle(TCollection_HAsciiString)& val);
  
  Standard_EXPORT void SetFileName (const Handle(TCollection_HAsciiString)& val);
  
  Standard_EXPORT void SetSystemId (const Handle(TCollection_HAsciiString)& val);
  
  Standard_EXPORT void SetInterfaceVersion (const Handle(TCollection_HAsciiString)& val);
  
  void SetIntegerBits (const Standard_Integer val) { theIntegerBits = val; }

  void SetMaxPower10Single (const Standard_Integer val) { theMaxPower10Single = val; }

  void SetMaxDigitsSingle (const Standard_Integer val) { theMaxDigitsSingle = val; }

  void SetMaxPower10Double (const Standard_Integer val) { theMaxPower10Double = val; }

  void SetMaxDigitsDouble (const Standard_Integer val) { theMaxDigitsDouble = val; }

  Standard_EXPORT void SetReceiveName (const Handle(TCollection_HAsciiString)& val);
  
  void SetScale (const Standard_Real val) { theScale = val; }

  void SetUnitFlag (const Standard_Integer val) { theUnitFlag = val; }

  Standard_EXPORT void SetUnitName (const Handle(TCollection_HAsciiString)& val);
  
  void SetLineWeightGrad (const Standard_Integer val) { theLineWeightGrad = val; }

  void SetMaxLineWeight (const Standard_Real val) { theMaxLineWeight = val; }

  Standard_EXPORT void SetDate (const Handle(TCollection_HAsciiString)& val);
  
  void SetResolution (const Standard_Real val) { theResolution = val; }
  
  Standard_EXPORT void SetMaxCoord (const Standard_Real val = 0.0);
  
  Standard_EXPORT void MaxMaxCoord (const Standard_Real val = 0.0);
  
  Standard_EXPORT void MaxMaxCoords (const gp_XYZ& xyz);
  
  Standard_EXPORT void SetAuthorName (const Handle(TCollection_HAsciiString)& val);
  
  Standard_EXPORT void SetCompanyName (const Handle(TCollection_HAsciiString)& val);
  
  void SetIGESVersion (const Standard_Integer val) { theIGESVersion = val; }

  void SetDraftingStandard (const Standard_Integer val) { theDraftingStandard = val; }

  Standard_EXPORT void SetLastChangeDate (const Handle(TCollection_HAsciiString)& val);
  
  Standard_EXPORT void SetApplicationProtocol (const Handle(TCollection_HAsciiString)& val);

 private:
  
  //! Returns a string withpout its Hollerith marks (nnnH ahead).
  //! Remark : all strings stored in GlobalSection are expurged
  //! from Hollerith informations (without nnnH)
  //! If <astr> is not Hollerith form, it is simply copied
  Standard_EXPORT Handle(TCollection_HAsciiString) TranslatedFromHollerith (const Handle(TCollection_HAsciiString)& astr) const;

  Standard_Character theSeparator;
  Standard_Character theEndMark;
  Handle(TCollection_HAsciiString) theSendName;
  Handle(TCollection_HAsciiString) theFileName;
  Handle(TCollection_HAsciiString) theSystemId;
  Handle(TCollection_HAsciiString) theInterfaceVersion;
  Standard_Integer theIntegerBits;
  Standard_Integer theMaxPower10Single;
  Standard_Integer theMaxDigitsSingle;
  Standard_Integer theMaxPower10Double;
  Standard_Integer theMaxDigitsDouble;
  Handle(TCollection_HAsciiString) theReceiveName;
  Standard_Real theScale;
  Standard_Integer theUnitFlag;
  Handle(TCollection_HAsciiString) theUnitName;
  Standard_Integer theLineWeightGrad;
  Standard_Real theMaxLineWeight;
  Handle(TCollection_HAsciiString) theDate;
  Standard_Real theResolution;
  Standard_Real theMaxCoord;
  Standard_Boolean hasMaxCoord;
  Handle(TCollection_HAsciiString) theAuthorName;
  Handle(TCollection_HAsciiString) theCompanyName;
  Standard_Integer theIGESVersion;
  Standard_Integer theDraftingStandard;
  Handle(TCollection_HAsciiString) theLastChangeDate;
  Handle(TCollection_HAsciiString) theAppliProtocol;
};

#endif // _IGESData_GlobalSection_HeaderFile
