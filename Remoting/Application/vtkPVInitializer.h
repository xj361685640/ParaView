// SPDX-FileCopyrightText: Copyright (c) Kitware Inc.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef vtkPVInitializer_h
#define vtkPVInitializer_h

#include "vtkPVPlugin.h"
#include "vtkPVServerManagerPluginInterface.h" // plugin interface mixin

#include "paraview_client_server.h"  // client server bindings
#include "paraview_server_manager.h" // server manager XML data

class vtkClientServerInterpreter;

class vtkPVInitializerPlugin
  : public vtkPVPlugin
  , public vtkPVServerManagerPluginInterface
{
  const char* GetPluginName() override { return "vtkPVInitializerPlugin"; }
  const char* GetPluginVersionString() override { return "0.0"; }
  bool GetRequiredOnServer() override { return false; }
  bool GetRequiredOnClient() override { return false; }
  const char* GetRequiredPlugins() override { return ""; }
  const char* GetDescription() override { return ""; }
  void GetBinaryResources(std::vector<std::string>&) override {}
  const char* GetEULA() override { return nullptr; }

  void GetXMLs(std::vector<std::string>& xmls) override
  {
    paraview_server_manager_initialize(xmls);
  }

  vtkClientServerInterpreterInitializer::InterpreterInitializationCallback
  GetInitializeInterpreterCallback() override
  {
    return paraview_client_server_initialize;
  }
};

inline void paraview_initialize()
{
  static vtkPVInitializerPlugin instance;
  vtkPVPlugin::ImportPlugin(&instance);
}

#endif

// VTK-HeaderTest-Exclude: vtkPVInitializer.h
