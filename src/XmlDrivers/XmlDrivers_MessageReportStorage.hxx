// Created on: 2018-06-10
// Created by: Natalia Ermolaeva
// Copyright (c) 2001-2014 OPEN CASCADE SAS
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

#ifndef _XmlDrivers_MessageReportStorage
#define _XmlDrivers_MessageReportStorage

#include <Standard_Handle.hxx>

#include <Message_Report.hxx>
#include <Message_Alert.hxx>
#include <TDF_Label.hxx>

//! Base class to store/restore Message_Report content in XML document
class XmlDrivers_MessageReportStorage
{
public:
  //! Create document for the report
  //! \param theReport the source report
  //! \param theFileName a file name
  Standard_EXPORT static void ExportReport (const Handle(Message_Report)& theReport, const TCollection_AsciiString& theFileName);

  //! Restores document file content into report instance
  //! \param theFileName a file name
  //! \return new report or NULL
  Standard_EXPORT static Handle(Message_Report) ImportReport (const TCollection_AsciiString& theFileName);

private:
  //! Create labels/attributes for the alert and place it under the parent label.
  //! It is recursive as alerts may contains other alerts
  //! \param theAlert a source alert
  //! \parm theParentLabel a label where alert label is placed
  static void exportAlert (const Handle(Message_Alert)& theAlert, const TDF_Label& theParentLabel);

  //! Creates a new alert by the label parameters
  //! \param theAlertLabel a source document label
  //! \param theGravity an alert gravity
  //! \param theReport a container of alerts
  //! \param theParentAlert a parent alert, if null, the parent is report
  static void importAlert (const TDF_Label& theAlertLabel,
                           const Message_Gravity theGravity,
                           Handle(Message_Report)& theReport,
                           const Handle(Message_Alert)& theParentAlert);

  //! Convert alert to a custom type and store parameters in child labels and attributes
  //! \param theAlert a source alert
  //! \parm theAlertLabel an alert label
  static void exportAlertParameters (const Handle(Message_Alert)& theAlert,
                                     const TDF_Label& theAlertLabel);

  //! Creates alert by label type filled by the label content
  //! \param theParametersLabel a label
  //! \return new alert or NULL
  static Handle(Message_Alert) importAlertParameters (const TDF_Label& aParametersLabel);
};

#endif // _XmlDrivers_MessageReportStorage
