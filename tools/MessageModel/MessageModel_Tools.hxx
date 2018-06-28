// Created on: 2017-06-16
// Created by: Natalia ERMOLAEVA
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

#ifndef MessageModel_Tools_H
#define MessageModel_Tools_H

#include <Standard.hxx>

#include <TCollection_AsciiString.hxx>
#include <Standard_Transient.hxx>
#include <TopoDS_Shape.hxx>

#include <QList>
//#include <QColor>

class Message_Alert;
class ViewControl_TableModelValues;

class ViewControl_Table;

//! \class MessageModel_Tools
//! It gives auxiliary methods for Message classes manipulation
class MessageModel_Tools
{
public:

  //! Convert pointer to string value
  //! \param thePointer a pointer
  //! \param isShortInfo if true, all '0' symbols in the beginning of the pointer are skipped
  //! \return the string value 
  Standard_EXPORT static TCollection_AsciiString GetPointerInfo (const Handle(Standard_Transient)& thePointer,
                                                                 const bool isShortInfo = true);

  //! Returns pointer alert information depening on type of alert (Transient_Object or TopoDS_TShape)
  //! \param theAlert a message alert
  //! \return text presentation
  Standard_EXPORT static TCollection_AsciiString GetPointerAlertInfo (const Handle(Message_Alert)& theAlert);

  //! Returns TopoDS_Shape type alert information or empty string
  //! \param theAlert a message alert
  //! \return text presentation
  Standard_EXPORT static TCollection_AsciiString GetShapeTypeAlertInfo (const Handle(Message_Alert)& theAlert);

  //! Returns dynamic type of an alert object/shape
  //! \param theAlert a message alert
  //! \return text presentation
  Standard_EXPORT static TCollection_AsciiString GetStandardTypeAlertInfo (const Handle(Message_Alert)& theAlert);

  //! Fills container of table values
  //! \param theAlert a message alert
  //! \param theTableValue container of values
  Standard_EXPORT static void GetPropertyTableValues (const Handle(Message_Alert)& theAlert,
                                                      QList<ViewControl_TableModelValues*>& theTableValues);

  //! Creates TopoDS_Compound with TopoDS_Edge/Vertex on alert data
  //! \param theAlert a message alert
  //! \parm theSelectedIndices a container of indices
  //! \param theTableView source view
  Standard_EXPORT TopoDS_Shape static BuildShape (const Handle(Message_Alert)& theAlert, QList<int> aSelectedIndices,
                                                  ViewControl_Table* theTableView);
};

#endif
