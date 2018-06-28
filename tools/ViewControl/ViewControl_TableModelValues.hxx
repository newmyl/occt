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

#ifndef ViewControl_TableModelValues_H
#define ViewControl_TableModelValues_H

#include <Standard.hxx>

#include <inspector/TreeModel_HeaderSection.hxx>

#include <Standard_WarningsDisable.hxx>
#include <QAbstractTableModel>
#include <QList>
#include <QModelIndexList>
#include <QVariant>
#include <Standard_WarningsRestore.hxx>

//! \class ViewControl_TableModelValues
//! \brief This is an interace for ViewControl_TableModel to give real values of the model
//! It should be filled or redefined.
class ViewControl_TableModelValues
{
public:

  //! Constructor
  Standard_EXPORT ViewControl_TableModelValues (const Qt::Orientation& theOrientation = Qt::Vertical)
    : myUseTableProperties (false), myUseTablePropertiesXStep (false), myUseTablePropertiesXStepValue (-1)
  { SetOrientation (theOrientation); }

  //! Destructor
  virtual ~ViewControl_TableModelValues() {}

  //! Sets direction of the values applying, whether it should be placed by rows or by columns
  //! \param theOrientation if horizontal, the values are applyed by rows, otherwise by columns
  void SetOrientation (const Qt::Orientation& theOrientation) { myOrientation = theOrientation; }

  //! Fills the model values.
  //! \param theValues a container of table model values
  void SetValues (const QVector<QVariant>& theValues) { myValues = theValues; }

  //! Fills the model header values for orientation.
  //! \param theValues a container of header text values
  //! \param theOrientation an orientation of header
  void SetHeaderValues (const QList<TreeModel_HeaderSection>& theValues, const Qt::Orientation theOrientation)
  { myHeaderValues.insert (theOrientation, theValues); }

  //! Returns whether the column is hidden by default
  //! \param theColumnId a column index
  //! \return header section values container
  TreeModel_HeaderSection GetHeaderItem (const Qt::Orientation theOrientation, const int theColumnId) const
  { return myHeaderValues.contains(theOrientation) ? myHeaderValues[theOrientation][theColumnId] : TreeModel_HeaderSection(); }

  //! Stores information about table view header visibility
  //! \param theOrientation an orientation of header
  //! \param theVisibility if true, header is visible
  void SetHeaderVisible (const Qt::Orientation theOrientation, const bool theVisibility)
  { myVisibleHeader.insert (theOrientation, theVisibility); }

  //! Stores information about table view header visibility
  //! \param theOrientation an orientation of header
  //! \param theVisibility if true, header is visible
  bool IsHeaderVisible (const Qt::Orientation theOrientation) const
  { return myVisibleHeader.contains(theOrientation) ? myVisibleHeader[theOrientation] : true; }

  //! Set default section size if defined
  //! \param theOrientation an orientation of header
  //! \param theVisibility if true, header is visible
  void SetDefaultSectionSize (const Qt::Orientation theOrientation, const int& theSectionSize)
  { myDefaultSectionSize.insert(theOrientation, theSectionSize); }

  //! Get default section size if defined
  //! \param theOrientation an orientation of header
  //! \param theVisibility if true, header is visible
  bool GetDefaultSectionSize (const Qt::Orientation theOrientation, int& theSectionSize)
  {
    theSectionSize = myDefaultSectionSize.contains (theOrientation) ? myDefaultSectionSize[theOrientation] : -1;
    return myDefaultSectionSize.contains (theOrientation);
  }

  //! Stores whether the properties control of the table visible or not
  //! \param theUseProperties boolean state
  void SetUseTableProperties (const bool theUseProperties) { myUseTableProperties = theUseProperties; }

  //! Returns true if the properties control of the table visible or not
  //! \return boolean value
  bool UseTableProperties() const { return myUseTableProperties; }

  //! Stores whether the properties control of the table visible or not
  //! \param theUseProperties boolean state
  void SetUseTablePropertiesXStep (const bool theUseStep, const double theStep)
  { myUseTablePropertiesXStep = theUseStep; myUseTablePropertiesXStepValue = theStep; }

  //! Returns true if the properties control of the table visible or not
  //! \return boolean value
  double UseTablePropertiesXStep(bool& theUseStep) const
  { theUseStep = myUseTablePropertiesXStep; return myUseTablePropertiesXStepValue; }

  //! Returns number of columns, size of header values
  //! \param theParent an index of the parent item
  //! \return an integer value
  Standard_EXPORT virtual int ColumnCount (const QModelIndex& theParent = QModelIndex()) const;

  //! Returns number of rows, depending on orientation: myColumnCount or size of values container
  //! \param theParent an index of the parent item
  //! \return an integer value
  virtual int RowCount (const QModelIndex& theParent = QModelIndex()) const
  { return ColumnCount (theParent) > 0 ? myValues.size() / ColumnCount (theParent) : 0; }

  //! Returns content of the model index for the given role, it is obtained from internal container of values
  //! It returns value only for DisplayRole.
  //! \param theIndex a model index
  //! \param theRole a view role
  //! \return value intepreted depending on the given role
  Standard_EXPORT virtual QVariant Data (const int theRow, const int theColumn, int theRole = Qt::DisplayRole) const;

  //! Returns content of the model index for the given role, it is obtainer from internal container of header values
  //! It returns value only for DisplayRole.
  //! \param theSection an index of value in the container 
  //! \param theIndex a model index
  //! \param theRole a view role
  //! \return value intepreted depending on the given role
  Standard_EXPORT virtual QVariant HeaderData (int theSection, Qt::Orientation theOrientation, int theRole = Qt::DisplayRole) const;

  //! Returns flags for the item: ItemIsEnabled | Qt::ItemIsSelectable
  //! \param theIndex a model index
  //! \return flags
  virtual Qt::ItemFlags Flags (const QModelIndex& theIndex) const
  { return theIndex.isValid() ? Qt::ItemIsEnabled | Qt::ItemIsSelectable : Qt::NoItemFlags; }

  //! Returns minimum and maximum values of the table content
  //! \param theMinValue minimum
  //! \param theMaxValue maximum
  virtual void GetRangeValues (QString& theMinValue, QString& theMaxValue, const QModelIndexList& theSelected) const
  { (void)theMinValue; (void)theMaxValue; (void)theSelected; }

  //! Returns additional info
  virtual QString AdditionalInformation() const { return QString(); }

protected:
  //! Finds position in internal vector of values using the table column/row count
  //! \param theRow a row of a table cell
  //! \param theColumn a column of a table cell
  size_t getPosition (const int theRow, const int theColumn) const { return ColumnCount() * theRow + theColumn; }

  //! Returns true if the header item is italic of the parameter index
  //! \param theRow a model index row
  //! \param theColumn a model index column
  //! \param boolean value
  bool isItalicHeader (const int theRow, const int theColumn) const;

protected:

  Qt::Orientation myOrientation; //!< orientation how the values should fill the current table view
  QMap<Qt::Orientation, QList<TreeModel_HeaderSection> > myHeaderValues; //!< table header values
  QMap<Qt::Orientation, bool> myVisibleHeader; //! table header visibility
  QMap<Qt::Orientation, int> myDefaultSectionSize; //! table section default size
  QVector<QVariant> myValues; //! cached container of table values
  bool myUseTableProperties; //! state whether the table property control is visible
  bool myUseTablePropertiesXStep; //! true if XStep value is used
  double myUseTablePropertiesXStepValue; //! value to define OX step for 1D table, Z = 0
};

#endif
