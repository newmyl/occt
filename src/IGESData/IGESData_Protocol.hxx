// Created on: 1993-05-05
// Created by: Christian CAILLET
// Copyright (c) 1993-1999 Matra Datavision
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

#ifndef _IGESData_Protocol_HeaderFile
#define _IGESData_Protocol_HeaderFile

#include <Interface_Protocol.hxx>
class IGESData_IGESEntity;
class IGESData_IGESType;

class IGESData_Protocol;
DEFINE_STANDARD_HANDLE(IGESData_Protocol, Interface_Protocol)

//! Description of basic Protocol for IGES
//! This comprises treatement of IGESModel and Recognition of
//! Undefined-FreeFormat-Entity
class IGESData_Protocol : public Interface_Protocol
{
 public:

  IGESData_Protocol() {}
  
  //! Returns a Case Number, specific of each recognized Type
  //! Here, Undefined and Free Format Entities have the Number 1.
  Standard_EXPORT Standard_Integer TypeNumber (const Handle(Standard_Type)& atype) const Standard_OVERRIDE;
  
  //! Creates an empty Model for IGES Norm
  Standard_EXPORT Handle(Interface_InterfaceModel) NewModel() const Standard_OVERRIDE;
  
  //! Creates a new Unknown Entity for IGES (UndefinedEntity)
  Standard_EXPORT Handle(Standard_Transient) UnknownEntity() const Standard_OVERRIDE;
  
  //! Returns True if <ent> is an Unknown Entity for the Norm, i.e.
  //! Type UndefinedEntity, status Unknown
  Standard_EXPORT Standard_Boolean IsUnknownEntity (const Handle(Standard_Transient)& ent) const Standard_OVERRIDE;

  //! Creates a new entity of the given type
  Standard_EXPORT Handle(IGESData_IGESEntity) NewEntity(const IGESData_IGESType &theType) const;

  DEFINE_STANDARD_RTTIEXT(IGESData_Protocol,Interface_Protocol)
};

#endif // _IGESData_Protocol_HeaderFile
