/*=========================================================================

  Program:   ParaView
  Module:    vtkPVThumbWheel.cxx

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkPVThumbWheel.h"

#include "vtkClientServerID.h"
#include "vtkKWEntry.h"
#include "vtkKWLabel.h"
#include "vtkKWMenu.h"
#include "vtkKWPushButton.h"
#include "vtkKWThumbWheel.h"
#include "vtkObjectFactory.h"
#include "vtkPVApplication.h"
#include "vtkPVSource.h"
#include "vtkPVXMLElement.h"
#include "vtkPVXMLPackageParser.h"
#include "vtkSMDoubleRangeDomain.h"
#include "vtkSMDoubleVectorProperty.h"
#include "vtkSMIntRangeDomain.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkPVTraceHelper.h"

//-----------------------------------------------------------------------------
vtkStandardNewMacro(vtkPVThumbWheel);
vtkCxxRevisionMacro(vtkPVThumbWheel, "1.18");

//-----------------------------------------------------------------------------
vtkPVThumbWheel::vtkPVThumbWheel()
{
  this->Label = vtkKWLabel::New();
  this->Label->SetParent(this);
  this->ThumbWheel = vtkKWThumbWheel::New();
  this->ThumbWheel->SetParent(this);
}

//-----------------------------------------------------------------------------
vtkPVThumbWheel::~vtkPVThumbWheel()
{
  this->Label->Delete();
  this->ThumbWheel->Delete();
}

//-----------------------------------------------------------------------------
void vtkPVThumbWheel::Create(vtkKWApplication *app)
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }

  // Call the superclass to create the whole widget

  this->Superclass::Create(app);
  
  // Now a label
  this->Label->Create(app);
  this->Label->SetJustificationToRight();
  if (strlen(this->Label->GetText()) > 0)
    {
    this->Label->SetWidth(18);
    }
  this->Script("pack %s -side left", this->Label->GetWidgetName());
  
  // Now the thumb wheel
  this->ThumbWheel->PopupModeOn();
  this->ThumbWheel->Create(app);
  this->ThumbWheel->DisplayEntryOn();
  this->ThumbWheel->DisplayEntryAndLabelOnTopOff();
  this->ThumbWheel->ExpandEntryOn();
  this->ThumbWheel->ClampMinimumValueOn();
  this->ThumbWheel->SetInteractionModeToNonLinear(0);
  this->ThumbWheel->SetNonLinearMaximumMultiplier(10);
  this->ThumbWheel->SetEndCommand(this, "ModifiedCallback");
  this->ThumbWheel->GetEntry()->SetBind(this, "<KeyRelease>",
                                        "ModifiedCallback");
  
  this->Script("pack %s -side left -fill x -expand 1", this->ThumbWheel->GetWidgetName());
}

//-----------------------------------------------------------------------------
void vtkPVThumbWheel::SetValue(float val)
{
  float oldVal = static_cast<float>(this->ThumbWheel->GetValue());
  this->ThumbWheel->SetValue(val);
  if (oldVal != val)
    {
    this->ModifiedCallback();
    }
}

//-----------------------------------------------------------------------------
void vtkPVThumbWheel::SetResolution(float res)
{
  this->ThumbWheel->SetResolution(res);
}

//-----------------------------------------------------------------------------
void vtkPVThumbWheel::SetMinimumValue(float min)
{
  this->ThumbWheel->SetMinimumValue(min);
}

//-----------------------------------------------------------------------------
float vtkPVThumbWheel::GetValue()
{
  return this->ThumbWheel->GetValue();
}

//-----------------------------------------------------------------------------
void vtkPVThumbWheel::SetLabel(const char *str)
{
  this->Label->SetText(str);
  if (str && str[0] &&
      (this->GetTraceHelper()->GetObjectNameState() == 
       vtkPVTraceHelper::ObjectNameStateUninitialized ||
       this->GetTraceHelper()->GetObjectNameState() == 
       vtkPVTraceHelper::ObjectNameStateDefault) )
    {
    this->GetTraceHelper()->SetObjectName(str);
    this->GetTraceHelper()->SetObjectNameState(
      vtkPVTraceHelper::ObjectNameStateSelfInitialized);
    }
}

//-----------------------------------------------------------------------------
void vtkPVThumbWheel::SetBalloonHelpString(const char *str)
{
  this->Superclass::SetBalloonHelpString(str);

  if (this->Label)
    {
    this->Label->SetBalloonHelpString(str);
    }

  if (this->ThumbWheel)
    {
    this->ThumbWheel->SetBalloonHelpString(str);
    }
}

//-----------------------------------------------------------------------------
vtkPVThumbWheel* vtkPVThumbWheel::ClonePrototype(
  vtkPVSource *pvSource, vtkArrayMap<vtkPVWidget*, vtkPVWidget*>*map)
{
  vtkPVWidget *clone = this->ClonePrototypeInternal(pvSource, map);
  return vtkPVThumbWheel::SafeDownCast(clone);
}

//-----------------------------------------------------------------------------
void vtkPVThumbWheel::SaveInBatchScript(ofstream *file)
{
  vtkClientServerID sourceID = this->PVSource->GetVTKSourceID(0);
  
  if (sourceID.ID == 0 || !this->SMPropertyName)
    {
    vtkErrorMacro("Sanity check failed. " << this->GetClassName());
    return;
    }
  
  *file << "  if { [[$pvTemp" << sourceID << " GetProperty "
        << this->SMPropertyName
        << "] GetClassName] == \"vtkSMIntVectorProperty\"} {" << endl;
  *file << "    set value [expr round(" << this->GetValue() << ")]" << endl;
  *file << "  } else {" << endl;
  *file << "    set value " << this->GetValue() << endl;
  *file << "  }" << endl;
  
  *file << "  [$pvTemp" << sourceID << " GetProperty "
        << this->SMPropertyName << "] SetElement 0 $value" << endl;
}

//-----------------------------------------------------------------------------
void vtkPVThumbWheel::Accept()
{
  float scalar = this->ThumbWheel->GetValue();
  float entryValue = this->ThumbWheel->GetEntry()->GetValueAsFloat();
  if (entryValue != scalar)
    {
    scalar = entryValue;
    this->ThumbWheel->SetValue(entryValue);
    }
  
  vtkSMDoubleVectorProperty *dvp = vtkSMDoubleVectorProperty::SafeDownCast(
    this->GetSMProperty());
  vtkSMIntVectorProperty *ivp = vtkSMIntVectorProperty::SafeDownCast(
    this->GetSMProperty());

  if (dvp)
    {
    dvp->SetElement(0, this->GetValue());
    }
  else if (ivp)
    {
    ivp->SetElement(0, static_cast<int>(this->GetValue()));
    }

  this->Superclass::Accept();
}

//-----------------------------------------------------------------------------
void vtkPVThumbWheel::Initialize()
{
  vtkSMDoubleVectorProperty *dvp = vtkSMDoubleVectorProperty::SafeDownCast(
    this->GetSMProperty());
  vtkSMIntVectorProperty *ivp = vtkSMIntVectorProperty::SafeDownCast(
    this->GetSMProperty());

  if (dvp)
    {
    this->SetValue(dvp->GetElement(0));
    }
  else if (ivp)
    {
    this->SetValue(ivp->GetElement(0));
    }
}

//-----------------------------------------------------------------------------
void vtkPVThumbWheel::ResetInternal()
{
  this->Initialize();
  this->ModifiedFlag = 0;
}

//-----------------------------------------------------------------------------
void vtkPVThumbWheel::Trace(ofstream *file)
{
  if (!this->GetTraceHelper()->Initialize(file))
    {
    return;
    }
  
  *file << "$kw(" << this->GetTclName() << ") SetValue "
        << this->GetValue() << endl;
}

//-----------------------------------------------------------------------------
void vtkPVThumbWheel::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();
  
  this->PropagateEnableState(this->Label);
  this->PropagateEnableState(this->ThumbWheel);
}

//-----------------------------------------------------------------------------
void vtkPVThumbWheel::CopyProperties(vtkPVWidget *clone, vtkPVSource *source,
                                  vtkArrayMap<vtkPVWidget*, vtkPVWidget*>* map)
{
  this->Superclass::CopyProperties(clone, source, map);
  vtkPVThumbWheel *pvtw = vtkPVThumbWheel::SafeDownCast(clone);
  if (pvtw)
    {
    pvtw->SetMinimumValue(this->ThumbWheel->GetMinimumValue());
    pvtw->SetResolution(this->ThumbWheel->GetResolution());
    pvtw->SetLabel(this->Label->GetText());
    }
  else
    {
    vtkErrorMacro("Internal error. Could not downcast clone to PVThumbWheel.");
    }
}

//-----------------------------------------------------------------------------
int vtkPVThumbWheel::ReadXMLAttributes(vtkPVXMLElement *element,
                                       vtkPVXMLPackageParser *parser)
{
  if(!this->Superclass::ReadXMLAttributes(element, parser)) { return 0; }

  // Setup the label
  const char *label = element->GetAttribute("label");
  if (!label)
    {
    label = element->GetAttribute("trace_name");
    
    if (!label)
      {
      vtkErrorMacro("No label attribute.");
      return 0;
      }
    }
  this->SetLabel(label);
  
  // Setup the resolution
  float resolution;
  if (!element->GetScalarAttribute("resolution", &resolution))
    {
    resolution = 1;
    }
  this->SetResolution(resolution);
 
  // Setup the minimum value
  float min;
  if (!element->GetScalarAttribute("minimum_value", &min))
    {
    min = 0;
    }
  this->SetMinimumValue(min);
  
  return 1;
}

//-----------------------------------------------------------------------------
void vtkPVThumbWheel::PrintSelf(ostream &os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
