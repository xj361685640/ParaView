/*=========================================================================

  Plugin:   DigitalSignalProcessing
  Module:   vtkDSPDataModelTestingUtilities.h

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/**
 * @class   vtkDSPDataModelTestingUtilities
 * @brief   Defines helper methods for DSP data model testing
 */

#ifndef vtkDSPDataModelTestingUtilities_h
#define vtkDSPDataModelTestingUtilities_h

#include "vtkAOSDataArrayTemplate.h"
#include "vtkSmartPointer.h"

#include <iostream>
#include <vector>

namespace vtkDSPDataModelTestingUtilities
{
///@{
/**
 * Utility function to compare "got" value to "expected" one.
 */
template <typename ValueT>
bool testValue(ValueT got, ValueT expected, const char* funcName)
{
  if (got != expected)
  {
    std::cerr << "Error when checking " << funcName << "." << std::endl;
    std::cerr << "Expected " << expected << ", got " << got << "." << std::endl;
    return false;
  }
  return true;
};
template <typename ValueT1, typename ValueT2>
bool testValue(
  ValueT1 got, ValueT2 expected, int arrayIdx, int tupleIdx, int compIdx, const char* funcName)
{
  if (got != expected)
  {
    std::cerr << "Error when checking " << funcName << " for array = " << arrayIdx
              << ", tuple = " << tupleIdx << ", component = " << compIdx << "." << std::endl;
    std::cerr << "Expected " << expected << ", got " << got << "." << std::endl;
    return false;
  }
  return true;
};
///@}
}

#endif
