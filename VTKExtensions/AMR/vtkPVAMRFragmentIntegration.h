// SPDX-FileCopyrightText: Copyright (c) Kitware Inc.
// SPDX-FileCopyrightText: Copyright 2013 Sandia Corporation
// SPDX-License-Identifier: LicenseRef-BSD-3-Clause-Sandia-USGov
/**
 * @class   vtkPVAMRFragmentIntegration
 * @brief   Generates fragment analysis from an
 * amr volume and a previously run contour on that volume
 *
 *
 *
 * .SEE vtkAMRFragmentIntegration
 *
 */

#ifndef vtkPVAMRFragmentIntegration_h
#define vtkPVAMRFragmentIntegration_h

#include "vtkAMRFragmentIntegration.h"
#include "vtkPVVTKExtensionsAMRModule.h" //needed for exports

// Forware declaration.
class vtkPVAMRFragmentIntegrationInternal;

class VTKPVVTKEXTENSIONSAMR_EXPORT vtkPVAMRFragmentIntegration : public vtkAMRFragmentIntegration
{
public:
  static vtkPVAMRFragmentIntegration* New();
  vtkTypeMacro(vtkPVAMRFragmentIntegration, vtkAMRFragmentIntegration);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkPVAMRFragmentIntegration();
  ~vtkPVAMRFragmentIntegration() override;

  ///@{
  /**
   * Add to list of volume arrays which are used for generating contours.
   */
  void AddInputVolumeArrayToProcess(const char* name);
  void ClearInputVolumeArrayToProcess();
  ///@}

  ///@{
  /**
   * Add to list of mass arrays
   */
  void AddInputMassArrayToProcess(const char* name);
  void ClearInputMassArrayToProcess();
  ///@}

  ///@{
  /**
   * Add to list of volume weighted arrays
   */
  void AddInputVolumeWeightedArrayToProcess(const char* name);
  void ClearInputVolumeWeightedArrayToProcess();
  ///@}

  ///@{
  /**
   * Add to list of mass weighted arrays
   */
  void AddInputMassWeightedArrayToProcess(const char* name);
  void ClearInputMassWeightedArrayToProcess();
  ///@}

  void SetContourConnection(vtkAlgorithmOutput*);

  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

private:
  vtkPVAMRFragmentIntegration(const vtkPVAMRFragmentIntegration&) = delete;
  void operator=(const vtkPVAMRFragmentIntegration&) = delete;

protected:
  double VolumeFractionSurfaceValue;
  vtkPVAMRFragmentIntegrationInternal* Implementation;
};

#endif // vtkPVAMRFragmentIntegration_h
