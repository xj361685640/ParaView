/*=========================================================================

  Program:   ParaView
  Module:    $RCSfile$

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkPMWriterProxy.h"

#include "vtkClientServerInterpreter.h"
#include "vtkClientServerStream.h"
#include "vtkCompleteArrays.h"
#include "vtkMultiProcessController.h"
#include "vtkObjectFactory.h"
#include "vtkPMInputProperty.h"
#include "vtkProcessModule.h"
#include "vtkPVXMLElement.h"

vtkStandardNewMacro(vtkPMWriterProxy);
//----------------------------------------------------------------------------
vtkPMWriterProxy::vtkPMWriterProxy()
{
  this->FileNameMethod = NULL;
}

//----------------------------------------------------------------------------
vtkPMWriterProxy::~vtkPMWriterProxy()
{
}

//----------------------------------------------------------------------------
bool vtkPMWriterProxy::ReadXMLAttributes(vtkPVXMLElement* element)
{
  if (!this->Superclass::ReadXMLAttributes(element))
    {
    return false;
    }

  const char* setFileNameMethod = element->GetAttribute("file_name_method");
  if (setFileNameMethod)
    {
    this->SetFileNameMethod(setFileNameMethod);
    }

  return true;
}

//----------------------------------------------------------------------------
bool vtkPMWriterProxy::CreateVTKObjects(vtkSMMessage* message)
{
  if (this->ObjectsCreated)
    {
    return true;
    }

  if (!this->Superclass::CreateVTKObjects(message))
    {
    return false;
    }

  vtkObjectBase* object = this->GetVTKObject();
  vtkPMProxy* writerProxy = this->GetSubProxyHelper("Writer");
  if (writerProxy)
    {
    vtkClientServerStream stream;
    stream << vtkClientServerStream::Invoke
      << object
      << "SetWriter"
      << writerProxy->GetVTKObject()
      << vtkClientServerStream::End;
    if (this->FileNameMethod)
      {
      stream << vtkClientServerStream::Invoke
        << object
        << "SetFileNameMethod"
        << this->FileNameMethod
        << vtkClientServerStream::End;
      }
    this->Interpreter->ProcessStream(stream);
    }

  vtkPMProxy* helper = this->GetSubProxyHelper("PreGatherHelper");
  if (helper)
    {
    vtkClientServerStream stream;
    stream << vtkClientServerStream::Invoke
      << object
      << "SetPreGatherHelper" << helper->GetVTKObject()
      << vtkClientServerStream::End;
    this->Interpreter->ProcessStream(stream);
    }

  helper = this->GetSubProxyHelper("PostGatherHelper");
  if (helper)
    {
    vtkClientServerStream stream;
    stream << vtkClientServerStream::Invoke
      << object
      << "SetPostGatherHelper" << helper->GetVTKObject()
      << vtkClientServerStream::End;
    this->Interpreter->ProcessStream(stream);
    }

  // Pass piece/process information to the writer if it needs it.
  vtkProcessModule::GetProcessModule()->ReportInterpreterErrorsOff();
  vtkMultiProcessController* controller =
    vtkMultiProcessController::GetGlobalController();
  int numProcs = controller->GetNumberOfProcesses();
  int procId = controller->GetLocalProcessId();

  vtkClientServerStream stream;
  stream << vtkClientServerStream::Invoke
    << object
    << "SetNumberOfPieces"
    << numProcs
    << vtkClientServerStream::End;
  this->Interpreter->ProcessStream(stream);
  stream.Reset();

  // ALTERNATIVE: 1
  stream << vtkClientServerStream::Invoke
    << object
    << "SetStartPiece"
    << procId
    << vtkClientServerStream::End;
  stream << vtkClientServerStream::Invoke
    << object
    << "SetEndPiece"
    << procId
    << vtkClientServerStream::End;
  this->Interpreter->ProcessStream(stream);
  stream.Reset();

  // ALTERNATIVE: 2
  stream << vtkClientServerStream::Invoke
    << object
    << "SetPiece"
    << procId
    << vtkClientServerStream::End;
  this->Interpreter->ProcessStream(stream);
  vtkProcessModule::GetProcessModule()->ReportInterpreterErrorsOn();
  stream.Reset();

  return true;
}

//----------------------------------------------------------------------------
void vtkPMWriterProxy::AddInput(
  int input_port, vtkAlgorithmOutput* connection, const char* method)
{
  vtkPMProxy* completeArraysPM = this->GetSubProxyHelper("CompleteArrays");
  vtkCompleteArrays* completeArrays =
    completeArraysPM? vtkCompleteArrays::SafeDownCast(
      completeArraysPM->GetVTKObject()) : NULL;
  if (completeArrays)
    {
    completeArrays->SetInputConnection(connection);
    this->Superclass::AddInput(input_port, completeArrays->GetOutputPort(),
      method);
    }
  else
    {
    this->Superclass::AddInput(input_port, connection, method);
    }
}

//----------------------------------------------------------------------------
void vtkPMWriterProxy::CleanInputs(const char* method)
{
  vtkPMProxy* completeArraysPM = this->GetSubProxyHelper("CompleteArrays");
  vtkCompleteArrays* completeArrays =
    completeArraysPM? vtkCompleteArrays::SafeDownCast(
      completeArraysPM->GetVTKObject()) : NULL;
  if (completeArrays)
    {
    completeArrays->SetInputConnection(NULL);
    }
  this->Superclass::CleanInputs(method);
}

//----------------------------------------------------------------------------
void vtkPMWriterProxy::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "FileNameMethod: " << this->FileNameMethod << endl;
}
