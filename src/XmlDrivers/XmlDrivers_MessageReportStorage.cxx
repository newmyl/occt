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

#include <XmlDrivers_MessageReportStorage.hxx>

#include <Message.hxx>
#include <Message_AlertExtended.hxx>
#include <Message_AttributeVectorOfReal.hxx>
#include <Message_AttributeVectorOfRealVec3.hxx>
#include <Message_CompositeAlerts.hxx>
#include <Message_Report.hxx>

#include <TDataStd_AsciiString.hxx>
#include <TDataStd_Comment.hxx>
#include <TDataStd_Real.hxx>
#include <TDataStd_Name.hxx>
#include <TDataStd_RealArray.hxx>
#include <TDF_ChildIterator.hxx>
#include <TDocStd_Application.hxx>
#include <TDocStd_Document.hxx>
#include <XmlDrivers.hxx>

// =======================================================================
// function : GetApplication
// purpose :
// =======================================================================

const Handle(TDocStd_Application)& GetApplication()
{
  static Handle(TDocStd_Application) anApp;
  if (anApp.IsNull())
  {
    anApp = new TDocStd_Application;
    XmlDrivers::DefineFormat (anApp);
  }
  return anApp;
}

// =======================================================================
// function : ExportReport
// purpose :
// =======================================================================

void XmlDrivers_MessageReportStorage::ExportReport (const Handle(Message_Report)& theReport,
                                                    const TCollection_AsciiString& theFileName)
{
  if (theReport.IsNull())
    return;

  Handle(TDocStd_Document) aDocument;
  GetApplication()->NewDocument (TCollection_ExtendedString ("XmlOcaf"), aDocument);

  TDF_Label aMainLabel = aDocument->Main();

  for (int aGravityId = Message_Trace; aGravityId <= Message_Fail; aGravityId++)
  {
    Message_Gravity aGravity = (Message_Gravity)aGravityId;

    if (theReport->GetAlerts (aGravity).Size() == 0)
      continue;
    // Gravity Label
    TDF_Label aGravityLabel = aMainLabel.NewChild();
    // set gravity kind string
    TCollection_ExtendedString aName (Message::GravityToString ((Message_Gravity)aGravity));
    if (!aName.IsEmpty())
      TDataStd_Name::Set (aGravityLabel, aName);

    /// reserved label to store gravity information
    //TDF_Label aFirstAlertLabel = aGravityLabel.NewChild();

    // Alert Labels: labels are started from the second index
    const Message_ListOfAlert& anAlerts = theReport->GetAlerts (aGravity);
    for (Message_ListOfAlert::Iterator anAlertsIt (anAlerts); anAlertsIt.More(); anAlertsIt.Next())
    {
      exportAlert (anAlertsIt.Value(), aGravityLabel);
    }
  }
  GetApplication()->SaveAs (aDocument, theFileName);
}

// =======================================================================
// function : ImportReport
// purpose :
// =======================================================================

Handle(Message_Report) XmlDrivers_MessageReportStorage::ImportReport (const TCollection_AsciiString& theFileName)
{
  Handle(TDocStd_Application) anApplication = GetApplication();
  Standard_Integer aDocumentId = anApplication->IsInSession (theFileName);
  if (aDocumentId > 0)
  {
    Handle(TDocStd_Document) aDocument;
    anApplication->GetDocument (aDocumentId, aDocument);
    anApplication->Close (aDocument);
  }

  Handle(TDocStd_Document) aDocument;
  GetApplication()->Open (theFileName, aDocument);
  if (aDocument.IsNull())
    return Handle(Message_Report)();

  TDF_Label aMainLabel = aDocument->Main();
  if (aMainLabel.IsNull())
    return Handle(Message_Report)();

  TDF_Label aLabel;
  Handle(Message_Report) aReport = new Message_Report();
  for (TDF_ChildIterator aLabelsIt(aMainLabel); aLabelsIt.More(); aLabelsIt.Next())
  {
    TDF_Label aGravityLabel = aLabelsIt.Value();
    if (aGravityLabel.IsNull())
      continue;
    Handle(TDF_Attribute) anAttribute;
    if (!aGravityLabel.FindAttribute (TDataStd_Name::GetID(), anAttribute))
      continue;
    Handle(TDataStd_Name) aNameAttribute = Handle(TDataStd_Name)::DownCast (anAttribute);
    if (aNameAttribute.IsNull())
      continue;

    // get gravity type
    const TCollection_ExtendedString& aGravityName = aNameAttribute->Get();
    Message_Gravity aGravity = Message::GravityFromString (TCollection_AsciiString (aGravityName).ToCString());

    /// reserved label to store gravity information
    //TDF_Label aFirstAlertLabel = aGravityLabel.FindChild (1, Standard_False);

    // find alerts information, add corresponded alerts to the report
    for (TDF_ChildIterator anAlertLabelsIt (aGravityLabel); anAlertLabelsIt.More(); anAlertLabelsIt.Next())
    {
      TDF_Label anAlertLabel = anAlertLabelsIt.Value();
      if (anAlertLabel.IsNull())
        continue;

      importAlert (anAlertLabel, aGravity, aReport, Handle(Message_Alert)());
    }
  }
  return aReport;
}

// =======================================================================
// function : exportAlert
// purpose :
// =======================================================================
 
void XmlDrivers_MessageReportStorage::exportAlert (const Handle(Message_Alert)& theAlert, const TDF_Label& theParentLabel)
{
   TDF_Label anAlertLabel = theParentLabel.NewChild();
  TDataStd_Name::Set (anAlertLabel, theAlert->DynamicType()->Name());

  /// reserved label to store parameters of the current label
  TDF_Label anAlertParmetersLabel = anAlertLabel.NewChild();
  exportAlertParameters (theAlert, anAlertParmetersLabel);

  for (int aGravityId = Message_Trace; aGravityId <= Message_Fail; aGravityId++)
  {
    // Gravity Label
    TDF_Label aGravityLabel = anAlertLabel.NewChild();
    // set gravity kind string
    TDataStd_Name::Set (aGravityLabel, Message::GravityToString ((Message_Gravity)aGravityId));

    Handle(Message_AlertExtended) anAlertExtended = Handle(Message_AlertExtended)::DownCast (theAlert);
    if (anAlertExtended.IsNull())
      continue;
    Handle(Message_CompositeAlerts) aComposite = anAlertExtended->GetCompositeAlerts();
    if (aComposite.IsNull())
      continue;

    const Message_ListOfAlert& anAlerts = aComposite->GetAlerts ((Message_Gravity)aGravityId);
    for (Message_ListOfAlert::Iterator anAlertsIt (anAlerts); anAlertsIt.More(); anAlertsIt.Next())
      exportAlert (anAlertsIt.Value(), aGravityLabel);
  }
}

// =======================================================================
// function : importAlert
// purpose :
// =======================================================================

void XmlDrivers_MessageReportStorage::importAlert (const TDF_Label& theAlertLabel,
                                                   const Message_Gravity theGravity,
                                                   Handle(Message_Report)& theReport,
                                                   const Handle(Message_Alert)& theParentAlert)
{
  TDF_Label aParametersLabel = theAlertLabel.FindChild (1, Standard_False);
  Handle(Message_Alert) anAlert = importAlertParameters (aParametersLabel);
  if (anAlert.IsNull())
    return;

  TDF_Label aLabel;
  TDF_ChildIterator aSubAlertsLabelsIt (theAlertLabel);
  aSubAlertsLabelsIt.Next(); // do not processing the first (parameters) label
  for (; aSubAlertsLabelsIt.More(); aSubAlertsLabelsIt.Next())
  {
    TDF_Label aGravityLabel = aSubAlertsLabelsIt.Value();
    if (aGravityLabel.IsNull())
      continue;
    Handle(TDF_Attribute) anAttribute;
    if (!aGravityLabel.FindAttribute (TDataStd_Name::GetID(), anAttribute))
      continue;
    Handle(TDataStd_Name) aNameAttribute = Handle(TDataStd_Name)::DownCast (anAttribute);
    if (aNameAttribute.IsNull())
      continue;

    // get gravity type
    Message_Gravity aGravity = Message::GravityFromString (TCollection_AsciiString (aNameAttribute->Get()).ToCString());
    // find alerts information, add corresponded alerts to the report
    for (TDF_ChildIterator anAlertLabelsIt (aGravityLabel); anAlertLabelsIt.More(); anAlertLabelsIt.Next())
    {
      TDF_Label anAlertLabel = anAlertLabelsIt.Value();
      if (anAlertLabel.IsNull())
        continue;

      importAlert (anAlertLabel, aGravity, theReport, anAlert);
    }
  }
  theReport->AddAlert (theGravity, anAlert, 0, theParentAlert);
}

// =======================================================================
// function : exportAlertParameters
// purpose :
// =======================================================================
void XmlDrivers_MessageReportStorage::exportAlertParameters (const Handle(Message_Alert)& theAlert, const TDF_Label& theAlertLabel)
{
  Handle(Message_AlertExtended) anAlertExtended = Handle(Message_AlertExtended)::DownCast (theAlert);
  if (anAlertExtended.IsNull()) // name attribute is empty
    return;

  // store attribute time
  Handle(Message_Attribute) anAttribute = anAlertExtended->Attribute();

  TDataStd_Name::Set (theAlertLabel, anAttribute->DynamicType()->Name());
  TDataStd_Real::Set (theAlertLabel, anAlertExtended->GetCumulativeTime());

  TDataStd_AsciiString::Set (theAlertLabel, anAttribute->GetName());
  TCollection_AsciiString aDescription = anAttribute->GetDescription();
  if (!aDescription.IsEmpty())
    TDataStd_Comment::Set (theAlertLabel, anAttribute->GetDescription());

  Standard_CString aDynamicTypeName = anAttribute->DynamicType()->Name();
  if (aDynamicTypeName == STANDARD_TYPE (Message_AttributeVectorOfReal)->Name())
  {
    Handle(Message_AttributeVectorOfReal) aRealArrayAlert = Handle(Message_AttributeVectorOfReal)::DownCast (anAttribute);
    // store values
    const NCollection_Vector<double>& anArrayValues = aRealArrayAlert->GetValues();
    // create real list attribute only if there are values in the attribute
    if (anArrayValues.IsEmpty())
      return;
    int anArraySize = anArrayValues.Length();
    Handle(TDataStd_RealArray) aRealListAttribute = TDataStd_RealArray::Set (theAlertLabel, 0, anArraySize - 1);
    for (int aValueId = 0; aValueId < anArraySize; aValueId++)
      aRealListAttribute->SetValue (aValueId, anArrayValues.Value (aValueId));
  }
  else if (aDynamicTypeName == STANDARD_TYPE (Message_AttributeVectorOfRealVec3)->Name())
  {
    Handle(Message_AttributeVectorOfRealVec3) aRealArrayAlert = Handle(Message_AttributeVectorOfRealVec3)::DownCast (anAttribute);
    // store values
    const NCollection_Vector<NCollection_Vec3<double>>& anArrayValues = aRealArrayAlert->GetValues();
    // create real list attribute only if there are values in the attribute
    if (anArrayValues.IsEmpty())
      return;
    int anArraySize = anArrayValues.Length();
    Handle(TDataStd_RealArray) aRealListAttribute = TDataStd_RealArray::Set (theAlertLabel, 0, 3 * anArraySize - 1);
    for (int aValueId = 0; aValueId < anArraySize; aValueId++)
    {
      NCollection_Vec3<double> aValue = anArrayValues.Value (aValueId);
      aRealListAttribute->SetValue (3 * aValueId, aValue.x());
      aRealListAttribute->SetValue (3 * aValueId + 1, aValue.y());
      aRealListAttribute->SetValue (3 * aValueId + 2, aValue.z());
    }
  }
}

// =======================================================================
// function : importAlertParameters
// purpose :
// =======================================================================
Handle(Message_Alert) XmlDrivers_MessageReportStorage::importAlertParameters (const TDF_Label& aParametersLabel)
{
  Handle(TDF_Attribute) anAttribute;
  if (!aParametersLabel.FindAttribute (TDataStd_Name::GetID(), anAttribute))
    return Handle(Message_Alert)();

  Handle(TDataStd_Name) aDynamicTypeAttribute = Handle(TDataStd_Name)::DownCast (anAttribute);
  if (aDynamicTypeAttribute.IsNull())
    return Handle(Message_Alert)();
  const TCollection_ExtendedString& aDynamicTypeName = aDynamicTypeAttribute->Get();

  TCollection_ExtendedString aDescription;
  if (aParametersLabel.FindAttribute (TDataStd_Comment::GetID(), anAttribute))
  {
    Handle(TDataStd_Comment) aDescriptionAttribute = Handle(TDataStd_Comment)::DownCast (anAttribute);
    if (!aDescriptionAttribute.IsNull())
      aDescription = aDescriptionAttribute->Get();
  }

  Handle(Message_AlertExtended) anAlert = new Message_AlertExtended();
  Handle(Message_Attribute) aMessageAttribute;
  if (aDynamicTypeName == STANDARD_TYPE (Message_Attribute)->Name())
    aMessageAttribute = new Message_Attribute();
  else if (aDynamicTypeName == STANDARD_TYPE (Message_AttributeVectorOfReal)->Name())
  {
    // values
    NCollection_Vector<double> anArrayValues;
    if (!aParametersLabel.FindAttribute (TDataStd_RealArray::GetID(), anAttribute))
      return Handle(Message_Alert)();

    Handle(TDataStd_RealArray) aValuesAttribute = Handle(TDataStd_RealArray)::DownCast (anAttribute);
    if (aValuesAttribute.IsNull())
      return Handle(Message_Alert)();

    for (int aValueId = aValuesAttribute->Lower(); aValueId <= aValuesAttribute->Upper(); aValueId++)
      anArrayValues.Append (aValuesAttribute->Value (aValueId));

    aMessageAttribute = new Message_AttributeVectorOfReal (anArrayValues);
  }
  else if (aDynamicTypeName == STANDARD_TYPE (Message_AttributeVectorOfRealVec3)->Name())
  {
    // values
    NCollection_Vector<NCollection_Vec3<double>> anArrayValues;
    if (!aParametersLabel.FindAttribute(TDataStd_RealArray::GetID(), anAttribute))
      return Handle(Message_Alert)();

    Handle(TDataStd_RealArray) aValuesAttribute = Handle(TDataStd_RealArray)::DownCast (anAttribute);
    if (aValuesAttribute.IsNull())
      return Handle(Message_Alert)();

    for (int aValueId = aValuesAttribute->Lower(); aValueId <= aValuesAttribute->Upper();
         aValueId = aValueId + 3)
    {
      NCollection_Vec3<double> aValue (aValuesAttribute->Value (aValueId),
                                       aValuesAttribute->Value (aValueId + 1),
                                       aValuesAttribute->Value (aValueId + 2));
      anArrayValues.Append (aValue);
    }
    aMessageAttribute = new Message_AttributeVectorOfRealVec3 (anArrayValues);
  }

  if (!aMessageAttribute.IsNull())
  {
    // name
    if (!aParametersLabel.FindAttribute (TDataStd_AsciiString::GetID(), anAttribute))
      return Handle(Message_Alert)();
    Handle(TDataStd_AsciiString) aNameAttribute = Handle(TDataStd_AsciiString)::DownCast (anAttribute);
    if (aNameAttribute.IsNull())
      return Handle(Message_Alert)();

    aMessageAttribute->SetName (aNameAttribute->Get());
    aMessageAttribute->SetDescription (aDescription);

    anAlert->SetAttribute (aMessageAttribute);
  }

  // time
  Standard_Real aTime = -1;
  Handle(TDataStd_Real) aTimeAttribute;
  if (aParametersLabel.FindAttribute (TDataStd_Real::GetID(), aTimeAttribute))
    aTime = aTimeAttribute->Get();

  anAlert->SetCumulativeTime (aTime);
  return anAlert;
}
