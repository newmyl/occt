// Created on: 2018-08-16
// Created by: Natalia ERMOLAEVA
// Copyright (c) 2018 OPEN CASCADE SAS
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

#ifndef VInspector_ItemOpenGlElement_H
#define VInspector_ItemOpenGlElement_H

#include <Standard.hxx>
#include <inspector/VInspector_ItemBase.hxx>

class Graphic3d_Group;
class OpenGl_Element;

class VInspector_ItemOpenGlElement;
typedef QExplicitlySharedDataPointer<VInspector_ItemOpenGlElement> VInspector_ItemOpenGlElementPtr;

//! \class VInspector_ItemOpenGlElement
//! Parent item, that corresponds to AIS_InteractiveContext
//! Children of the item are:
//! - "Property" item to show context attributes such as selection filters and drawer properties
//! - presentation items to show all interactive elements displayed/erased in the context
class VInspector_ItemOpenGlElement : public VInspector_ItemBase
{
public:

  //! Creates an item wrapped by a shared pointer
  static VInspector_ItemOpenGlElementPtr CreateItem (TreeModel_ItemBasePtr theParent, const int theRow, const int theColumn)
  { return VInspector_ItemOpenGlElementPtr (new VInspector_ItemOpenGlElement (theParent, theRow, theColumn)); }

  //! Destructor
  virtual ~VInspector_ItemOpenGlElement() Standard_OVERRIDE {};

  //! Returns the current graphic3d group, init item if it was not initialized yet
  //! \return graphic group
  Standard_EXPORT OpenGl_Element* GetElement() const;

  //! Inits the item, fills internal containers
  Standard_EXPORT virtual void Init() Standard_OVERRIDE;

  //! Resets cached values
  Standard_EXPORT virtual void Reset() Standard_OVERRIDE;

  //! Returns number of table rows
  //! \return an integer value
  virtual int GetTableRowCount() const Standard_OVERRIDE;

  //! Returns table value for the row in form: <function name> <function value>
  //! \param theRow a model index row
  //! \param theColumn a model index column
  virtual QVariant GetTableData (const int theRow, const int theColumn, const int theRole) const Standard_OVERRIDE;

protected:

  //! Initialize the current item. It creates a backup of the specific item information
  //! Do nothing as context has been already set into item
  virtual void initItem() const Standard_OVERRIDE;

  //! Returns number of displayed presentations
  //! \return rows count
  Standard_EXPORT virtual int initRowCount() const Standard_OVERRIDE;

  //! Returns item information for the given role. Fills internal container if it was not filled yet
  //! \param theItemRole a value role
  //! \return the value
  Standard_EXPORT virtual QVariant initValue (const int theItemRole) const Standard_OVERRIDE;

protected:

  //! Creates a child item in the given position.
  //! \param theRow the child row position
  //! \param theColumn the child column position
  //! \return the created item
  virtual TreeModel_ItemBasePtr createChild (int theRow, int theColumn) Standard_OVERRIDE;

private:
  enum VInspector_ElementKind
  {
    VInspector_ElementKind_PrimitiveArray, //!< OpenGl_PrimitiveArray
    VInspector_ElementKind_Text, //!< OpenGl_Text
    VInspector_ElementKind_Undefined //!< another OpenGl_Element kind
  };

  //! Returns kind of the current element
  //! \return kind
  VInspector_ElementKind GetElementKind() const;

  //! Constructor
  //! param theParent a parent item
  //! \param theRow the item row positition in the parent item
  //! \param theColumn the item column positition in the parent item
  VInspector_ItemOpenGlElement(TreeModel_ItemBasePtr theParent, const int theRow, const int theColumn)
    : VInspector_ItemBase(theParent, theRow, theColumn), myElement (0) {}

private:
  OpenGl_Element* myElement; //! current element
};

#endif
