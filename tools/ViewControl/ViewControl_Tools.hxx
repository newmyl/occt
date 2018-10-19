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

#ifndef ViewControl_Tools_H
#define ViewControl_Tools_H

#include <gp_Pnt.hxx>
#include <gp_Trsf.hxx>
#include <Bnd_Box.hxx>
#include <Standard.hxx>
#include <Standard_Macro.hxx>
#include <TColgp_HArray1OfPnt.hxx>
#include <TCollection_AsciiString.hxx>

#include <Standard_WarningsDisable.hxx>
#include <QString>
#include <QVariant>
#include <Standard_WarningsRestore.hxx>

class QAction;
class QObject;
class QTableView;
class QWidget;

//! \class ViewControl_Tools
//! \brief The tool that gives auxiliary methods for qt elements manipulation
class ViewControl_Tools
{
public:

  //! Creates an action with the given text connected to the slot
  //! \param theText an action text value
  //! \param theSlot a listener of triggered signal of the new action
  //! \param theParent a parent object
  //! \param theContext listener of the action toggle
  //! \return a new action
  Standard_EXPORT static QAction* CreateAction (const QString& theText, const char* theSlot,
                                                QObject* theParent, QObject* theContext);

  //! Change palette of the widget to have white foreground
  //! \param theControl a widget to be modified
  Standard_EXPORT static void SetWhiteBackground (QWidget* theControl);

  //! Fills tree view by default sections parameters obtained in view's table model
  //! \param theTableView table view instance
  //! \param theOrientation header orientation
  Standard_EXPORT static void SetDefaultHeaderSections (QTableView* theTableView, const Qt::Orientation theOrientation);

  //! Convert handle pointer to string value
  //! \param thePointer a pointer
  //! \param isShortInfo if true, all '0' symbols in the beginning of the pointer are skipped
  //! \return the string value
  Standard_EXPORT static TCollection_AsciiString GetPointerInfo (const Handle(Standard_Transient)& thePointer,
                                                                 const bool isShortInfo = true);

  //! Convert pointer to string value
  //! \param thePointer a pointer
  //! \param isShortInfo if true, all '0' symbols in the beginning of the pointer are skipped
  //! \return the string value
  Standard_EXPORT static TCollection_AsciiString GetPointerInfo (const void* thePointer,
                                                                 const bool isShortInfo = true);

  //! Convert real value to string value
  //! \param theValue a short real value
  //! \return the string value
  Standard_EXPORT static QVariant ToVariant (const Standard_ShortReal theValue);

  //! Convert real value to string value
  //! \param theValue a real value
  //! \return the string value
  Standard_EXPORT static QVariant ToVariant (const Standard_Real theValue);

  //! Convert real value to real value
  //! \param theValue a string value
  //! \return the real value
  Standard_EXPORT static Standard_ShortReal ToShortRealValue (const QVariant& theValue);

  //! Convert real value to string value
  //! \param theValue a string value
  //! \return the real value
  Standard_EXPORT static Standard_Real ToRealValue (const QVariant& theValue);

  //! Returns text of orientation
  //! \param theLocation a location value
  //! \return text value
  Standard_EXPORT static TCollection_AsciiString ToString (const gp_Trsf& theValue);

  //! Returns text of point
  //! \param theValue a 3D point
  //! \return text value
  Standard_EXPORT static TCollection_AsciiString ToString (const gp_Pnt& theValue);

  //! Returns text of bounding box in form: (xmin, ymin, zmin), (xmax, ymax, zmax)
  //! \param theValue a bounding box
  //! \return text value
  Standard_EXPORT static TCollection_AsciiString ToString (const Bnd_Box& theValue);

  //! Returns text of array of points
  //! \param thePoints points
  //! \return text value
  Standard_EXPORT static TCollection_AsciiString ToString (const Handle(TColgp_HArray1OfPnt)& thePoints);

};

#endif
