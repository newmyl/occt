// Copyright (c) 1998-1999 Matra Datavision
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

#include <FSD_CmpFile.ixx>
#include <OSD.hxx>

#include <Storage_StreamModeError.hxx>
#include <Storage_StreamUnknownTypeError.hxx>
#include <Standard_PCharacter.hxx>

// Propagate the improvement (BUC60808) on all platforms
// #ifdef WNT
#define BUC60808
// BUC60808 : standard output in MS Visual C++ limites the quantity of figures (digits)
// in part of number after fixed point. The quantity of figures after fixed point 
// could not be more than 15 when we output double number using standard MS VS output operator '<<'.
// For example, the maximum double value DBL_MAX = 1.7976931348623157e+308 (see float.h) after 
// cout<<setprecision(17)<<DBL_MAX  becomes 1.79769313486232e+308  in stream of output 
// (it could be a file stream). When the number 1.79769313486232e+308 is read from the
// i/o streame (file, for example) it occurs more than allowed maximum value of double 
// type - 1.7976931348623157e+308.  Such value is considered as infinite. When it is written 
// to file (for example)  again it is written as 1.#INF. Such value can't be read from file next time.
//#endif

const Standard_CString MAGICNUMBER = "CMPFILE";

//=======================================================================
//function : FSD_CmpFile
//purpose  : 
//=======================================================================

FSD_CmpFile::FSD_CmpFile()
{

}

//=======================================================================
//function : IsGoodFileType
//purpose  : INFO SECTION
//           write
//=======================================================================

Storage_Error FSD_CmpFile::IsGoodFileType(const Handle(Storage_IODevice)& aDevice)
{
  FSD_CmpFile      f;
  Storage_Error s;

  s = f.Open(aDevice,Storage_VSRead);

  if (s == Storage_VSOk) {
    TCollection_AsciiString l;
    Standard_Size        len = strlen(FSD_CmpFile::MagicNumber());

    f.ReadChar(l,len);

    f.Close();

    if (strncmp(FSD_CmpFile::MagicNumber(),l.ToCString(),len) != 0) {
      s = Storage_VSFormatError;
    }
  }

  return s;
}

//=======================================================================
//function : Open
//purpose  : 
//=======================================================================

Storage_Error FSD_CmpFile::Open(const Handle(Storage_IODevice)& aDevice,const Storage_OpenMode aMode)
{
  SetDevice(aDevice);

  if ( Device().IsNull() )
    return Storage_VSOpenError;

  return Device()->Open(aMode);

  /*
  Storage_Error result = Storage_VSOk;

  SetName(aName);

  if (OpenMode() == Storage_VSNone) {

#if !defined(IRIX) && !defined(DECOSF1)
    if (aMode == Storage_VSRead) {
      myStream.open(aName.ToCString(),ios::in|ios::binary); // ios::nocreate is not portable
    }
    else if (aMode == Storage_VSWrite) {
      myStream.open(aName.ToCString(),ios::out|ios::binary);
    }
    else if (aMode == Storage_VSReadWrite) {
      myStream.open(aName.ToCString(),ios::in|ios::out|ios::binary);
    }
#else
    if (aMode == Storage_VSRead) {
      myStream.open(aName.ToCString(),ios::in); // ios::nocreate is not portable
    }
    else if (aMode == Storage_VSWrite) {
      myStream.open(aName.ToCString(),ios::out);
    }
    else if (aMode == Storage_VSReadWrite) {
      myStream.open(aName.ToCString(),ios::in|ios::out);
    }
#endif

    if (myStream.fail()) {
      result = Storage_VSOpenError;
    }
    else {
      myStream.precision(17);
      myStream.imbue (std::locale::classic()); // use always C locale
      SetOpenMode(aMode);
    }
  }
  else {
    result = Storage_VSAlreadyOpen;
  }
  return result;
  */
}

//=======================================================================
//function : IsEnd
//purpose  : 
//=======================================================================

Standard_Boolean FSD_CmpFile::IsEnd()
{
  return !Device().IsNull() ? Device()->IsEnd() : Standard_True;
}

//=======================================================================
//function : Close
//purpose  : 
//=======================================================================

Storage_Error FSD_CmpFile::Close()
{
  Storage_Error result = Storage_VSOk;

  if (OpenMode() != Storage_VSNone) {
    Device()->Close();
  }
  else {
    result = Storage_VSNotOpen;
  }

  return result;
}

//=======================================================================
//function : MagicNumber
//purpose  : ------------------ PROTECTED
//=======================================================================

const Standard_CString FSD_CmpFile::MagicNumber()
{
  return MAGICNUMBER;
}

//=======================================================================
//function : FlushEndOfLine
//purpose  : 
//=======================================================================

void FSD_CmpFile::FlushEndOfLine()
{
  TCollection_AsciiString aDummy;
  ReadLine (aDummy); // flush is nothing more than to read till the line-break
  /*
  static char Buffer[8192];
  char c;
  Standard_Boolean IsEnd = Standard_False;

  while (!IsEnd && !FSD_CmpFile::IsEnd()) {
    Buffer[0] = '\0';
    myStream.get(Buffer,8192,'\n');

    if (myStream.get(c) && c != '\r' && c != '\n') {
    }
    else {
      IsEnd = Standard_True;
    }
  }
  */
}

//=======================================================================
//function : WriteLine
//purpose  : write the line.
//=======================================================================

void FSD_CmpFile::WriteLine(const TCollection_AsciiString& aStr, const Standard_Boolean putNewLine)
{
  TCollection_AsciiString buffer = aStr;
  if ( putNewLine )
    buffer += TCollection_AsciiString("\n");

  if ( Device()->Write( (Standard_Address)buffer.ToCString(), buffer.Length() ) != buffer.Length() )
    Storage_StreamWriteError::Raise();
}


//=======================================================================
//function : ReadLine
//purpose  : read from the current position to the end of line.
//=======================================================================

void FSD_CmpFile::ReadLine(TCollection_AsciiString& buffer)
{
  Standard_Boolean IsEnd = Standard_False;
  
  buffer.Clear();

  while (!IsEnd && !FSD_CmpFile::IsEnd())
  {
    Standard_Character c;
    Device()->Read( &c, 1 );
    if ( c != '\n' && c != '\r')
    {
      buffer += c;
    }  
    else
    {
      buffer += '\0';
      IsEnd = Standard_True;
  }
}
}

//=======================================================================
//function : WriteExtendedLine
//purpose  : write from the current position to the end of line.
//=======================================================================

void FSD_CmpFile::WriteExtendedLine(const TCollection_ExtendedString& buffer)
{
  Standard_ExtString extBuffer;
  Standard_Integer   i;

  extBuffer = buffer.ToExtString();
  PutInteger(buffer.Length());
  for (i = 0; i < buffer.Length(); i++) {
    PutExtCharacter(extBuffer[i]);
  }

  Device()->Write("\n", 1);
}

//=======================================================================
//function : ReadExtendedLine
//purpose  : 
//=======================================================================

void FSD_CmpFile::ReadExtendedLine(TCollection_ExtendedString& buffer)
{
  Standard_ExtCharacter c;
  Standard_Integer i;

  GetInteger(i);

  for (i = 0; i < buffer.Length(); i++) {
    GetExtCharacter(c);
    buffer += c;
  }

  FlushEndOfLine();
}

//=======================================================================
//function : ReadChar
//purpose  : read <rsize> character from the current position.
//=======================================================================

void FSD_CmpFile::ReadChar(TCollection_AsciiString& buffer, const Standard_Size rsize)
{
  char             c;
  Standard_Size ccount = 0;

  buffer.Clear();

  while (!IsEnd() && (ccount < rsize)) {
    Device()->Read( &c, sizeof( char ) );
    buffer += c;
    ccount++;
  }
}

//=======================================================================
//function : ReadString
//purpose  : read from the first none space character position to the end of line.
//=======================================================================

void FSD_CmpFile::ReadString(TCollection_AsciiString& buffer)
{
  Standard_Boolean IsEnd = Standard_False,isFirstTime = Standard_True;
  
  buffer.Clear();
  
  while (!IsEnd && !FSD_CmpFile::IsEnd())
  {
    char c;
    Device()->Read(&c, sizeof(c));
    if (isFirstTime)
    {
      if (c == '\n' || c == ' ')
      {
        continue;
    }  
      else
      {
      isFirstTime = Standard_False;
    }
    }

    if (c != '\n')
    {
      buffer += c;
    }
    else
    {
      buffer += '\0';
      IsEnd = Standard_True;
  }
}
}

//=======================================================================
//function : ReadWord
//purpose  : read from the current position to the next white space or end of line.
//=======================================================================

void FSD_CmpFile::ReadWord(TCollection_AsciiString& buffer)
{
  char c = '\0';
  char b[8193],*tmpb;
  Standard_Boolean IsEnd = Standard_False;
  Standard_Integer i;

  tmpb = b;
  memset(b,'\0',8193);
  buffer.Clear();

  while (!IsEnd && !FSD_CmpFile::IsEnd()) {
    Device()->Read( &c, sizeof(char) );
    if ((c != ' ') && (c != '\n'))
    {
      IsEnd = Standard_True;
  }
  }

  IsEnd = Standard_False;
  i = 0;

  while (!IsEnd && !FSD_CmpFile::IsEnd()) {
    if (i == 8192) {
      buffer += b;
      tmpb = b;
      memset(b,'\0',8193);
      i = 0;
    }
    *tmpb = c;
    tmpb++; i++;
    Device()->Read( &c, sizeof(char) );
    if ((c == '\n') || (c == ' '))
    {
      IsEnd = Standard_True;
  }
  }

  buffer += b;
}

//=======================================================================
//function : FindTag
//purpose  : 
//=======================================================================

Storage_Error FSD_CmpFile::FindTag(const Standard_CString aTag)
{
  TCollection_AsciiString l;
  
  ReadString(l);

  while ((strcmp(l.ToCString(),aTag) != 0) && !IsEnd()) {
    ReadString(l);
  }

  if (IsEnd()) {
    return Storage_VSSectionNotFound;
  }
  else {
    return Storage_VSOk;
  }
}

//=======================================================================
//function : SkipObject
//purpose  : 
//=======================================================================

void FSD_CmpFile::SkipObject()
{
  FlushEndOfLine();
}

//=======================================================================
//function : PutReference
//purpose  : ---------------------- PUBLIC : PUT
//=======================================================================

Storage_BaseDriver& FSD_CmpFile::PutReference(const Standard_Integer aValue)
{
  TCollection_AsciiString aStr = TCollection_AsciiString( aValue ) + " ";
  if ( Device()->Write( (Standard_Address)aStr.ToCString(), aStr.Length() ) != aStr.Length() )
    Storage_StreamWriteError::Raise();
  return *this;
}

//=======================================================================
//function : PutCharacter
//purpose  : 
//=======================================================================

Storage_BaseDriver& FSD_CmpFile::PutCharacter(const Standard_Character aValue)
{
  Standard_Integer i = aValue;
  TCollection_AsciiString aStr = TCollection_AsciiString( i ) + " ";
  if ( Device()->Write( (Standard_Address)aStr.ToCString(), aStr.Length() ) != aStr.Length() )
    Storage_StreamWriteError::Raise();

  return *this;
}

//=======================================================================
//function : PutExtCharacter
//purpose  : 
//=======================================================================

Storage_BaseDriver& FSD_CmpFile::PutExtCharacter(const Standard_ExtCharacter aValue)
{
  Standard_Integer i = aValue;
  TCollection_AsciiString aStr = TCollection_AsciiString( i ) + " ";
  if ( Device()->Write( (Standard_Address)aStr.ToCString(), aStr.Length() ) != aStr.Length() )
    Storage_StreamWriteError::Raise();

  return *this;
}

//=======================================================================
//function : PutInteger
//purpose  : 
//=======================================================================

Storage_BaseDriver& FSD_CmpFile::PutInteger(const Standard_Integer aValue)
{
  TCollection_AsciiString aStr = TCollection_AsciiString( aValue ) + " ";
  if ( Device()->Write( (Standard_Address)aStr.ToCString(), aStr.Length() ) != aStr.Length() )
    Storage_StreamWriteError::Raise();

  return *this;
}

//=======================================================================
//function : PutBoolean
//purpose  : 
//=======================================================================

Storage_BaseDriver& FSD_CmpFile::PutBoolean(const Standard_Boolean aValue)
{
  TCollection_AsciiString aStr = TCollection_AsciiString( (Standard_Integer)aValue ) + " ";
  if ( Device()->Write( (Standard_Address)aStr.ToCString(), aStr.Length() ) != aStr.Length() )
    Storage_StreamWriteError::Raise();

  return *this;
}

//=======================================================================
//function : PutReal
//purpose  : 
//=======================================================================

Storage_BaseDriver& FSD_CmpFile::PutReal(const Standard_Real aValue)
{
  TCollection_AsciiString aStr = TCollection_AsciiString( aValue ) + " ";
  if ( Device()->Write( (Standard_Address)aStr.ToCString(), aStr.Length() ) != aStr.Length() )
    Storage_StreamWriteError::Raise();

  return *this;
}

//=======================================================================
//function : PutShortReal
//purpose  : 
//=======================================================================

Storage_BaseDriver& FSD_CmpFile::PutShortReal(const Standard_ShortReal aValue)
{
  TCollection_AsciiString aStr = TCollection_AsciiString( aValue ) + " ";
  if ( Device()->Write( (Standard_Address)aStr.ToCString(), aStr.Length() ) != aStr.Length() )
    Storage_StreamWriteError::Raise();

  return *this;
}

//=======================================================================
//function : GetReference
//purpose  : ----------------- PUBLIC : GET
//=======================================================================

Storage_BaseDriver& FSD_CmpFile::GetReference(Standard_Integer& aValue)
{
  TCollection_AsciiString aStr;
  ReadWord( aStr );
  if ( aStr.IsIntegerValue() )
    aValue = aStr.IntegerValue();
  else
    Storage_StreamTypeMismatchError::Raise();

  return *this;
}

//=======================================================================
//function : GetCharacter
//purpose  : 
//=======================================================================

Storage_BaseDriver& FSD_CmpFile::GetCharacter(Standard_Character& aValue)
{
  unsigned short i = 0;
  TCollection_AsciiString aStr;
  ReadWord( aStr );
  if ( aStr.IsIntegerValue() )
    i = (unsigned short)aStr.IntegerValue();
  else
    Storage_StreamTypeMismatchError::Raise();
    
  aValue = (char)i;

  return *this;
}

//=======================================================================
//function : GetExtCharacter
//purpose  : 
//=======================================================================

Storage_BaseDriver& FSD_CmpFile::GetExtCharacter(Standard_ExtCharacter& aValue)
{
  unsigned short i = 0;
  TCollection_AsciiString aStr;
  ReadWord( aStr );
  if ( aStr.IsIntegerValue() )
    i = (unsigned short)aStr.IntegerValue();
  else
    Storage_StreamTypeMismatchError::Raise();
  
  aValue = (Standard_ExtCharacter)i;
  
  return *this;
}

//=======================================================================
//function : GetInteger
//purpose  : 
//=======================================================================

Storage_BaseDriver& FSD_CmpFile::GetInteger(Standard_Integer& aValue)
{
  TCollection_AsciiString aStr;
  ReadWord( aStr );
  if ( aStr.IsIntegerValue() )
    aValue = aStr.IntegerValue();
  else
    Storage_StreamTypeMismatchError::Raise();

  return *this;
}

//=======================================================================
//function : GetBoolean
//purpose  : 
//=======================================================================

Storage_BaseDriver& FSD_CmpFile::GetBoolean(Standard_Boolean& aValue)
{
  TCollection_AsciiString aStr;
  ReadWord( aStr );
  if ( aStr.IsIntegerValue() )
    aValue = (Standard_Boolean)aStr.IntegerValue();
  else
    Storage_StreamTypeMismatchError::Raise();

  return *this;
}

//=======================================================================
//function : GetReal
//purpose  : 
//=======================================================================

Storage_BaseDriver& FSD_CmpFile::GetReal(Standard_Real& aValue)
{
  TCollection_AsciiString aStr;
  ReadWord( aStr );
  if ( aStr.IsRealValue() )
    aValue = aStr.RealValue();
  else
    Storage_StreamTypeMismatchError::Raise();

  return *this;
#else
  if (!(myStream >> aValue)) Storage_StreamTypeMismatchError::Raise();

  return *this;
#endif
}

//=======================================================================
//function : GetShortReal
//purpose  : 
//=======================================================================

Storage_BaseDriver& FSD_CmpFile::GetShortReal(Standard_ShortReal& aValue)
{
  TCollection_AsciiString aStr;
  ReadWord( aStr );
  if ( aStr.IsRealValue() )
    aValue = aStr.RealValue();
  else
    Storage_StreamTypeMismatchError::Raise();

  return *this;
#else
  if (!(myStream >> aValue)) Storage_StreamTypeMismatchError::Raise();
 return *this;
#endif
}

//=======================================================================
//function : Destroy
//purpose  : 
//=======================================================================

void FSD_CmpFile::Destroy()
{
  if (OpenMode() != Storage_VSNone) {
    Close();
  }
}

//=======================================================================
//function : BeginWriteInfoSection
//purpose  : -------------------------- INFO : WRITE
//=======================================================================

Storage_Error FSD_CmpFile::BeginWriteInfoSection() 
{
  WriteLine( FSD_CmpFile::MagicNumber() );
  WriteLine( "BEGIN_INFO_SECTION" );

  return Storage_VSOk;
}

//=======================================================================
//function : WriteInfo
//purpose  : 
//=======================================================================

void FSD_CmpFile::WriteInfo(const Standard_Integer nbObj,
			 const TCollection_AsciiString& dbVersion,
			 const TCollection_AsciiString& date,
			 const TCollection_AsciiString& schemaName,
			 const TCollection_AsciiString& schemaVersion,
			 const TCollection_ExtendedString& appName,
			 const TCollection_AsciiString& appVersion,
			 const TCollection_ExtendedString& dataType,
			 const TColStd_SequenceOfAsciiString& userInfo) 
{
  Standard_Integer i;

  PutInteger( nbObj );
  WriteLine( "" );
  WriteLine( dbVersion );
  WriteLine( date );
  WriteLine( schemaName );
  WriteLine( schemaVersion );
  WriteExtendedLine(appName);
  WriteLine( appVersion );
  WriteExtendedLine(dataType);
  PutInteger( userInfo.Length() );
  WriteLine( "" );

  for (i = 1; i <= userInfo.Length(); i++)
    WriteLine( userInfo.Value(i) );
  }

//=======================================================================
//function : EndWriteInfoSection
//purpose  : read
//=======================================================================

Storage_Error FSD_CmpFile::EndWriteInfoSection() 
{
  WriteLine( "END_INFO_SECTION" );
  return Storage_VSOk;
}

//=======================================================================
//function : BeginReadInfoSection
//purpose  : 
//=======================================================================

Storage_Error FSD_CmpFile::BeginReadInfoSection() 
{
  Storage_Error s;
  TCollection_AsciiString l;
  Standard_Size        len = strlen(FSD_CmpFile::MagicNumber());

  ReadChar(l,len);
  
  if (strncmp(FSD_CmpFile::MagicNumber(),l.ToCString(),len) != 0) {
    s = Storage_VSFormatError;
  }
  else {
    s = FindTag("BEGIN_INFO_SECTION");
  }

  return s;
}

//=======================================================================
//function : ReadInfo
//purpose  : ------------------- INFO : READ
//=======================================================================

void FSD_CmpFile::ReadInfo(Standard_Integer& nbObj,
			TCollection_AsciiString& dbVersion,
			TCollection_AsciiString& date,
			TCollection_AsciiString& schemaName,
			TCollection_AsciiString& schemaVersion,
			TCollection_ExtendedString& appName,
			TCollection_AsciiString& appVersion,
			TCollection_ExtendedString& dataType,
			TColStd_SequenceOfAsciiString& userInfo) 
{
  GetInteger( nbObj );

  FlushEndOfLine();

  ReadLine(dbVersion);
  ReadLine(date);
  ReadLine(schemaName);
  ReadLine(schemaVersion);
  ReadExtendedLine(appName);
  ReadLine(appVersion);
  ReadExtendedLine(dataType);

  Standard_Integer i,len = 0;

  GetInteger( len );

  FlushEndOfLine();

  TCollection_AsciiString line;

  for (i = 1; i <= len && !IsEnd(); i++) {
    ReadLine(line);
    userInfo.Append(line);
    line.Clear();
  }
}

//=======================================================================
//function : EndReadInfoSection
//purpose  : COMMENTS SECTION
//           write
//=======================================================================

Storage_Error FSD_CmpFile::EndReadInfoSection() 
{
  return FindTag("END_INFO_SECTION");
}

//=======================================================================
//function : BeginWriteCommentSection
//purpose  : ---------------- COMMENTS : WRITE
//=======================================================================

Storage_Error FSD_CmpFile::BeginWriteCommentSection() 
{
  WriteLine( "BEGIN_COMMENT_SECTION" );

  return Storage_VSOk;
}

//=======================================================================
//function : WriteComment
//purpose  : 
//=======================================================================

void FSD_CmpFile::WriteComment(const TColStd_SequenceOfExtendedString& aCom)
{
 Standard_Integer i,aSize;

 aSize = aCom.Length();
 WriteLine (aSize);

 for (i = 1; i <= aSize; i++) {
   WriteExtendedLine(aCom.Value(i));
 }
}

//=======================================================================
//function : EndWriteCommentSection
//purpose  : read
//=======================================================================

Storage_Error FSD_CmpFile::EndWriteCommentSection() 
{
  WriteLine ("END_COMMENT_SECTION");

  return Storage_VSOk;
}

//=======================================================================
//function : BeginReadCommentSection
//purpose  : ---------------- COMMENTS : READ
//=======================================================================

Storage_Error FSD_CmpFile::BeginReadCommentSection() 
{
  return FindTag("BEGIN_COMMENT_SECTION");
}

//=======================================================================
//function : ReadComment
//purpose  : 
//=======================================================================

void FSD_CmpFile::ReadComment(TColStd_SequenceOfExtendedString& aCom)
{
  TCollection_ExtendedString line;
  Standard_Integer           len,i;

  GetInteger (len);
  
  FlushEndOfLine();  

  for (i = 1; i <= len && !IsEnd(); i++) {
    ReadExtendedLine(line);
    aCom.Append(line);
    line.Clear();
  }
}

//=======================================================================
//function : EndReadCommentSection
//purpose  : 
//=======================================================================

Storage_Error FSD_CmpFile::EndReadCommentSection() 
{
  return FindTag("END_COMMENT_SECTION");
}

//=======================================================================
//function : BeginWriteTypeSection
//purpose  : --------------- TYPE : WRITE
//=======================================================================

Storage_Error FSD_CmpFile::BeginWriteTypeSection() 
{
  WriteLine ("BEGIN_TYPE_SECTION");

  return Storage_VSOk;
}

//=======================================================================
//function : SetTypeSectionSize
//purpose  : 
//=======================================================================

void FSD_CmpFile::SetTypeSectionSize(const Standard_Integer aSize) 
{
  WriteLine (aSize);
}

//=======================================================================
//function : WriteTypeInformations
//purpose  : 
//=======================================================================

void FSD_CmpFile::WriteTypeInformations(const Standard_Integer typeNum,
				      const TCollection_AsciiString& typeName) 
{
  TCollection_AsciiString aLine;
  WriteLine (aLine + typeNum + " " + typeName.ToCString());
}

//=======================================================================
//function : EndWriteTypeSection
//purpose  : read
//=======================================================================

Storage_Error FSD_CmpFile::EndWriteTypeSection() 
{
  WriteLine ("END_TYPE_SECTION");

  return Storage_VSOk;
}

//=======================================================================
//function : BeginReadTypeSection
//purpose  : ------------------- TYPE : READ
//=======================================================================

Storage_Error FSD_CmpFile::BeginReadTypeSection() 
{
  return FindTag("BEGIN_TYPE_SECTION");
}

//=======================================================================
//function : TypeSectionSize
//purpose  : 
//=======================================================================

Standard_Integer FSD_CmpFile::TypeSectionSize() 
{
  Standard_Integer i;

  GetInteger (i);

  FlushEndOfLine();

  return i;
}

//=======================================================================
//function : ReadTypeInformations
//purpose  : 
//=======================================================================

void FSD_CmpFile::ReadTypeInformations(Standard_Integer& typeNum,
				    TCollection_AsciiString& typeName) 
{
  GetInteger (typeNum);
  ReadLine(typeName);
  FlushEndOfLine();
}

//=======================================================================
//function : EndReadTypeSection
//purpose  : ROOT SECTION
//           write
//=======================================================================

Storage_Error FSD_CmpFile::EndReadTypeSection() 
{
  return FindTag("END_TYPE_SECTION");
}

//=======================================================================
//function : BeginWriteRootSection
//purpose  : -------------------- ROOT : WRITE
//=======================================================================

Storage_Error FSD_CmpFile::BeginWriteRootSection() 
{
  WriteLine ("BEGIN_ROOT_SECTION");

  return Storage_VSOk;
}

//=======================================================================
//function : SetRootSectionSize
//purpose  : 
//=======================================================================

void FSD_CmpFile::SetRootSectionSize(const Standard_Integer aSize) 
{
  WriteLine (aSize);
}

//=======================================================================
//function : WriteRoot
//purpose  : 
//=======================================================================

void FSD_CmpFile::WriteRoot(const TCollection_AsciiString& rootName, const Standard_Integer aRef, const TCollection_AsciiString& rootType) 
{
  TCollection_AsciiString aLine;
  WriteLine (aLine + aRef + " " + rootName.ToCString() + " " + rootType.ToCString());
}

//=======================================================================
//function : EndWriteRootSection
//purpose  : read
//=======================================================================

Storage_Error FSD_CmpFile::EndWriteRootSection() 
{
  WriteLine ("END_ROOT_SECTION");

  return Storage_VSOk;
}

//=======================================================================
//function : BeginReadRootSection
//purpose  : ----------------------- ROOT : READ
//=======================================================================

Storage_Error FSD_CmpFile::BeginReadRootSection() 
{
  return FindTag("BEGIN_ROOT_SECTION");
}

//=======================================================================
//function : RootSectionSize
//purpose  : 
//=======================================================================

Standard_Integer FSD_CmpFile::RootSectionSize() 
{
  Standard_Integer i;
  GetInteger (i);
  FlushEndOfLine();
  
  return i;
}

//=======================================================================
//function : ReadRoot
//purpose  : 
//=======================================================================

void FSD_CmpFile::ReadRoot(TCollection_AsciiString& rootName, Standard_Integer& aRef,TCollection_AsciiString& rootType) 
{
  GetInteger (aRef);
  ReadWord(rootName);
  ReadWord(rootType);
}

//=======================================================================
//function : EndReadRootSection
//purpose  : REF SECTION
//           write
//=======================================================================

Storage_Error FSD_CmpFile::EndReadRootSection() 
{
  return FindTag("END_ROOT_SECTION");
}

//=======================================================================
//function : BeginWriteRefSection
//purpose  : -------------------------- REF : WRITE
//=======================================================================

Storage_Error FSD_CmpFile::BeginWriteRefSection() 
{
  WriteLine ("BEGIN_REF_SECTION");
  return Storage_VSOk;
}

//=======================================================================
//function : SetRefSectionSize
//purpose  : 
//=======================================================================

void FSD_CmpFile::SetRefSectionSize(const Standard_Integer aSize) 
{
  WriteLine (aSize);
}

//=======================================================================
//function : WriteReferenceType
//purpose  : 
//=======================================================================

void FSD_CmpFile::WriteReferenceType(const Standard_Integer reference,
				  const Standard_Integer typeNum) 
{
  TCollection_AsciiString aLine;
  WriteLine (aLine + reference + " " + typeNum);
}

//=======================================================================
//function : EndWriteRefSection
//purpose  : read
//=======================================================================

Storage_Error FSD_CmpFile::EndWriteRefSection() 
{
  WriteLine ("END_REF_SECTION");

  return Storage_VSOk;
}

//=======================================================================
//function : BeginReadRefSection
//purpose  : ----------------------- REF : READ
//=======================================================================

Storage_Error FSD_CmpFile::BeginReadRefSection() 
{
  return FindTag("BEGIN_REF_SECTION");
}

//=======================================================================
//function : RefSectionSize
//purpose  : 
//=======================================================================

Standard_Integer FSD_CmpFile::RefSectionSize() 
{
  Standard_Integer i;

  GetInteger (i);
  FlushEndOfLine();

  return i;
}

//=======================================================================
//function : ReadReferenceType
//purpose  : 
//=======================================================================

void FSD_CmpFile::ReadReferenceType(Standard_Integer& reference,
				 Standard_Integer& typeNum) 
{
  GetInteger (reference);
  GetInteger (typeNum);
  FlushEndOfLine();
}

//=======================================================================
//function : EndReadRefSection
//purpose  : DATA SECTION
//           write
//=======================================================================

Storage_Error FSD_CmpFile::EndReadRefSection() 
{
  return FindTag("END_REF_SECTION");
}

//=======================================================================
//function : BeginWriteDataSection
//purpose  : -------------------- DATA : WRITE
//=======================================================================

Storage_Error FSD_CmpFile::BeginWriteDataSection() 
{
  WriteLine ("BEGIN_DATA_SECTION", Standard_False);
  return Storage_VSOk;
}

//=======================================================================
//function : WritePersistentObjectHeader
//purpose  : 
//=======================================================================

void FSD_CmpFile::WritePersistentObjectHeader(const Standard_Integer aRef,
					   const Standard_Integer aType) 
{
  TCollection_AsciiString aLine;
  WriteLine (aLine + "\n#" + aRef + "%" + aType + " ", Standard_False);
}

//=======================================================================
//function : BeginWritePersistentObjectData
//purpose  : 
//=======================================================================

void FSD_CmpFile::BeginWritePersistentObjectData() 
{
  
}

//=======================================================================
//function : BeginWriteObjectData
//purpose  : 
//=======================================================================

void FSD_CmpFile::BeginWriteObjectData() 
{
  
}

//=======================================================================
//function : EndWriteObjectData
//purpose  : 
//=======================================================================

void FSD_CmpFile::EndWriteObjectData() 
{
  
}

//=======================================================================
//function : EndWritePersistentObjectData
//purpose  : 
//=======================================================================

void FSD_CmpFile::EndWritePersistentObjectData() 
{
  
}

//=======================================================================
//function : EndWriteDataSection
//purpose  : read
//=======================================================================

Storage_Error FSD_CmpFile::EndWriteDataSection() 
{
  WriteLine ("\nEND_DATA_SECTION");

  return Storage_VSOk;
}

//=======================================================================
//function : BeginReadDataSection
//purpose  : ---------------------- DATA : READ
//=======================================================================

Storage_Error FSD_CmpFile::BeginReadDataSection() 
{
  return FindTag("BEGIN_DATA_SECTION");
}

//=======================================================================
//function : ReadPersistentObjectHeader
//purpose  : 
//=======================================================================

void FSD_CmpFile::ReadPersistentObjectHeader(Standard_Integer& aRef,
					  Standard_Integer& aType) 
{
  char c;

  Device()->Read( &c, sizeof( char ) );

  while (c != '#') {
    if (IsEnd() || (c != ' ') || (c == '\r')|| (c == '\n')) {
      Storage_StreamFormatError::Raise();
    }
    Device()->Read( &c, sizeof( char ) );
  }

  GetInteger (aRef);

  Device()->Read( &c, sizeof( char ) );

  while (c != '%') {
    if (IsEnd() || (c != ' ') || (c == '\r')|| (c == '\n')) {
      Storage_StreamFormatError::Raise();
    }
    Device()->Read( &c, sizeof( char ) );
  }

  GetInteger (aType);
}

//=======================================================================
//function : BeginReadPersistentObjectData
//purpose  : 
//=======================================================================

void FSD_CmpFile::BeginReadPersistentObjectData() 
{
}

//=======================================================================
//function : BeginReadObjectData
//purpose  : 
//=======================================================================

void FSD_CmpFile::BeginReadObjectData() 
{
}

//=======================================================================
//function : EndReadObjectData
//purpose  : 
//=======================================================================

void FSD_CmpFile::EndReadObjectData() 
{
}

//=======================================================================
//function : EndReadPersistentObjectData
//purpose  : 
//=======================================================================

void FSD_CmpFile::EndReadPersistentObjectData() 
{
  char c;

  Device()->Read( &c, sizeof( char ) );
  while (c != '\n' && (c != '\r')) {
    if (IsEnd() || (c != ' ')) {
      Storage_StreamFormatError::Raise();
    }
    Device()->Read( &c, sizeof( char ) );
  }
 if (c == '\r') {
   Device()->Read( &c, sizeof( char ) );
 }
}

//=======================================================================
//function : EndReadDataSection
//purpose  : 
//=======================================================================

Storage_Error FSD_CmpFile::EndReadDataSection() 
{
  return FindTag("END_DATA_SECTION");
}

//=======================================================================
//function : Tell
//purpose  : return position in the file. Return -1 upon error.
//=======================================================================

Storage_Position FSD_CmpFile::Tell()
{
    return Device()->Tell();
  }
