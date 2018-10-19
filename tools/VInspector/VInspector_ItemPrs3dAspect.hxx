// Created on: 2018-08-26
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

#ifndef VInspector_ItemPrs3dAspect_H
#define VInspector_ItemPrs3dAspect_H

#include <Standard.hxx>
#include <inspector/VInspector_ItemBase.hxx>

#include <Prs3d_BasicAspect.hxx>

class QItemSelectionModel;

class VInspector_ItemPrs3dAspect;
typedef QExplicitlySharedDataPointer<VInspector_ItemPrs3dAspect> VInspector_ItemPrs3dAspectPtr;

//! \class VInspector_ItemPrs3dAspect
//! Item presents information about AIS_InteractiveObject.
//! Parent is item context, children are item selections.
class VInspector_ItemPrs3dAspect : public VInspector_ItemBase
{

public:

  //! Creates an item wrapped by a shared pointer
  static VInspector_ItemPrs3dAspectPtr CreateItem (TreeModel_ItemBasePtr theParent, const int theRow, const int theColumn)
  { return VInspector_ItemPrs3dAspectPtr (new VInspector_ItemPrs3dAspect (theParent, theRow, theColumn)); }
  //! Destructor
  virtual ~VInspector_ItemPrs3dAspect() Standard_OVERRIDE {};

  //! Inits the item, fills internal containers
  Standard_EXPORT virtual void Init() Standard_OVERRIDE;

  //! Resets cached values
  Standard_EXPORT virtual void Reset() Standard_OVERRIDE;

  Standard_EXPORT Handle(Prs3d_BasicAspect) GetAspect() const;

  //! Returns sub aspect of the row if possible, e.g. Datum/Dimension aspect
  //! \param theRow child row index
  //! \param theName [out] drawer name
  Standard_EXPORT Handle(Prs3d_BasicAspect) GetPrs3dAspect (const int theRow,
                                                            TCollection_AsciiString& theName,
                                                            Standard_Boolean& theOwnAspect) const;

  //! Returns number of table rows
  //! \return an integer value
  virtual int GetTableRowCount() const Standard_OVERRIDE;

  //! Returns type of edit control for the model index. By default, it is an empty control
  //! \param theRow a model index row
  //! \param theColumn a model index column
  //! \return edit type
  virtual ViewControl_EditType GetTableEditType (const int theRow, const int theColumn) const Standard_OVERRIDE;

  //! Returns container of string values for enumeration in the model row
  //! \param theRow table model row index
  //! \param theColumn a model index column
  //! \return string values for the enumeration presented in the row or an empty container
  virtual QList<QVariant> GetTableEnumValues (const int theRow, const int theColumn) const Standard_OVERRIDE;

  //! Returns table value for the row in form: <function name> <function value>
  //! \param theRow a model index row
  //! \param theColumn a model index column
  virtual QVariant GetTableData (const int theRow, const int theColumn, const int theRole) const Standard_OVERRIDE;

  //! Sets the value into the table cell. Only 1st column value might be modified.
  //! \param theRow a model index row
  //! \param theColumn a model index column
  //! \param theValue a new cell value
  virtual bool SetTableData (const int theRow, const int theColumn, const QVariant& theValue) Standard_OVERRIDE;

protected:

  //! Initialize the current item. It is empty because Reset() is also empty.
  virtual void initItem() const Standard_OVERRIDE;

  //! Returns number of item selected
  //! \return rows count
  virtual int initRowCount() const Standard_OVERRIDE;

  //! Returns item information for the given role. Fills internal container if it was not filled yet
  //! \param theItemRole a value role
  //! \return the value
  virtual QVariant initValue (const int theItemRole) const Standard_OVERRIDE;

  //! Creates a child item in the given position.
  //! \param theRow the child row position
  //! \param theColumn the child column position
  //! \return the created item
  virtual TreeModel_ItemBasePtr createChild (int theRow, int theColumn) Standard_OVERRIDE;

private:

  //! Set interactive object into the current field
  //! \param theIO a presentation
  //! \param theName item display data value
  void setAspect (Handle(Prs3d_BasicAspect) theAspect, const TCollection_AsciiString& theName)
  { myAspect = theAspect; myName = theName; }

private:

  //! Returns number of table rows depending on the aspect kind
  //! \param theAspectKind kind or parent kind of aspect
  int getTableRowCount (const TCollection_AsciiString& theAspectKind) const;

  //! Returns number of table rows depending on the aspect kind
  //! \param theRow a model index row
  //! \param theAspectKind kind or parent kind of aspect
  //! \param theStartRow number of rows following before the parameter row, use sum of row and start row
  ViewControl_EditType getTableEditType (const int theRow,
                                         const TCollection_AsciiString& theAspectKind,
                                         const int theStartRow) const;

  //! Returns container of string values for enumeration in the model row depending on the aspect kind
  //! \param theRow table model row index
  //! \param theAspectKind kind or parent kind of aspect
  //! \param theStartRow number of rows following before the parameter row, use sum of row and start row
  //! \return string values for the enumeration presented in the row or an empty container
  QList<QVariant> getTableEnumValues (const int theRow,
                                      const TCollection_AsciiString& theAspectKind,
                                      const int theStartRow) const;

  //! Returns table value for the row in form: <function name> <function value> depending on the aspect kind
  //! \param theRow a model index row
  //! \param theColumn a model index column
  //! \param theAspectKind kind or parent kind of aspect
  //! \param theStartRow number of rows following before the parameter row, use sum of row and start row
  QVariant getTableData (const int theRow,
                         const int theColumn,
                         const int theRole,
                         const TCollection_AsciiString& theAspectKind,
                         const int theStartRow) const;

  //! Sets the value into the table cell depending on the aspect kind
  //! \param theRow a model index row
  //! \param theColumn a model index column
  //! \param theValue a new cell value
  bool setTableData (const int theRow,
                     const TCollection_AsciiString& theAspectKind,
                     const int theStartRow,
                     const QVariant& theValue) const;

  //! Returns color data depending on parameters
  //! \param theInfo information value if column is the first one
  //! \param theColor color value
  //! \param isFirstColumn true if the column id is zero, return info value
  //! \param theRole if background, returns color or text parameters of color
  QVariant getColorData (const TCollection_AsciiString& theInfo,
                         const Quantity_ColorRGBA& theColor,
                         const Standard_Boolean isFirstColumn,
                         const int theRole) const;

private:

  //! Constructor
  //! param theParent a parent item
  VInspector_ItemPrs3dAspect(TreeModel_ItemBasePtr theParent, const int theRow, const int theColumn)
  : VInspector_ItemBase(theParent, theRow, theColumn) {}

protected:

  Handle(Prs3d_BasicAspect) myAspect; //!< the current interactive context
  TCollection_AsciiString myName; //!< the item display data value
};

#endif
