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

#ifndef MessageModel_TableModelRealVec3Values_H
#define MessageModel_TableModelRealVec3Values_H

#ifdef _MSC_VER
  #pragma warning(disable : 4127 4718) // conditional expression is constant, recursive call has no side effects
#endif

#include <inspector/ViewControl_TableModelValues.hxx>

#include <Message_AttributeVectorOfRealVec3.hxx>

//! \class MessageModel_TableModelRealVec3Values
//! \brief This is an implementation for ViewControl_TableModel to present Message_AttributeVectorOfRealVec3 object
class MessageModel_TableModelRealVec3Values : public ViewControl_TableModelValues
{
public:

  //! Constructor
  Standard_EXPORT MessageModel_TableModelRealVec3Values (const Handle(Message_Attribute)& theAttribute, const int theSectionWidth);

  //! Destructor
  virtual ~MessageModel_TableModelRealVec3Values() Standard_OVERRIDE {}

  //! Returns number of columns, size of header values
  //! \param theParent an index of the parent item
  //! \return an integer value
  Standard_EXPORT virtual int ColumnCount (const QModelIndex& theParent = QModelIndex()) const;

  //! Returns number of rows, depending on orientation: myColumnCount or size of values container
  //! \param theParent an index of the parent item
  //! \return an integer value
  virtual int RowCount (const QModelIndex& theParent = QModelIndex()) const { (void)theParent; return 1; }

  //! Returns content of the model index for the given role, it is obtained from internal container of values
  //! It returns value only for DisplayRole.
  //! \param theIndex a model index
  //! \param theRole a view role
  //! \return value intepreted depending on the given role
  Standard_EXPORT virtual QVariant Data (const int theRow, const int theColumn, int theRole = Qt::DisplayRole) const;

  //! Returns minimum and maximum values of selected items or the table content (if selected list is empty)
  //! It finds firstly Xmax, then Ymax, after that Zmax. The same for min values.
  //! \param theMinValue minimum
  //! \param theMaxValue maximum
  //! \param theSelected selected cells
  virtual void GetRangeValues (QString& theMinValue, QString& theMaxValue, const QModelIndexList& theSelected) const;

private:
  Handle(Message_AttributeVectorOfRealVec3) myAttribute; //!< alert attribute, container of table values
};

#endif
