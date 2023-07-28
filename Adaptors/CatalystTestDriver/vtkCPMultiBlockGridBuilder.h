// SPDX-FileCopyrightText: Copyright (c) Kitware Inc.
// SPDX-License-Identifier: BSD-3-Clause
/**
 * @class   vtkCPMultiBlockGridBuilder
 * @brief   Class for creating multiblock grids.
 *
 * Class for creating vtkMultiBlockDataSet grids for a test driver.
 */

#ifndef vtkCPMultiBlockGridBuilder_h
#define vtkCPMultiBlockGridBuilder_h

#include "vtkCPBaseGridBuilder.h"
#include "vtkPVCatalystTestDriverModule.h" // needed for export macros

class vtkCPGridBuilder;
class vtkDataObject;
class vtkMultiBlockDataSet;
struct vtkCPMultiBlockGridBuilderInternals;

class VTKPVCATALYSTTESTDRIVER_EXPORT vtkCPMultiBlockGridBuilder : public vtkCPBaseGridBuilder
{
public:
  static vtkCPMultiBlockGridBuilder* New();
  vtkTypeMacro(vtkCPMultiBlockGridBuilder, vtkCPBaseGridBuilder);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /**
   * Return a grid.  BuiltNewGrid is 0 if the grid is the same
   * as the last time step.
   */
  vtkDataObject* GetGrid(unsigned long timeStep, double time, int& builtNewGrid) override;

  /**
   * Get the Grid.
   */
  vtkMultiBlockDataSet* GetGrid();

  /**
   * Add a vtkCPGridBuilder.
   */
  void AddGridBuilder(vtkCPGridBuilder* gridBuilder);

  /**
   * Remove a vtkCPGridBuilder.
   */
  void RemoveGridBuilder(vtkCPGridBuilder* gridBuilder);

  /**
   * Clear out all of the current vtkCPGridBuilders.
   */
  void RemoveAllGridBuilders();

  /**
   * Get the number of vtkCPGridBuilders.
   */
  unsigned int GetNumberOfGridBuilders();

  /**
   * Get a specific vtkCPGridBuilder.
   */
  vtkCPGridBuilder* GetGridBuilder(unsigned int which);

protected:
  vtkCPMultiBlockGridBuilder();
  ~vtkCPMultiBlockGridBuilder() override;

  /**
   * Set the Grid.
   */
  void SetGrid(vtkMultiBlockDataSet* multiBlock);

private:
  vtkCPMultiBlockGridBuilder(const vtkCPMultiBlockGridBuilder&) = delete;
  void operator=(const vtkCPMultiBlockGridBuilder&) = delete;

  /**
   * The grid that is returned.
   */
  vtkMultiBlockDataSet* Grid;

  ///@{
  /**
   * Internals used for storing the vtkCPGridBuilders.
   */
  vtkCPMultiBlockGridBuilderInternals* Internal;
};
//@}

#endif
