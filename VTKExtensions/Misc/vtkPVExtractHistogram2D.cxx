/*=============================================================================
Copyright and License information
=============================================================================*/

#include "vtkPVExtractHistogram2D.h"

// VTK includes
#include "vtkDataArray.h"
#include "vtkDataArrayRange.h"
#include "vtkDataObject.h"
#include "vtkDoubleArray.h"
#include "vtkGradientFilter.h"
#include "vtkImageData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMath.h"
#include "vtkMultiProcessController.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkStreamingDemandDrivenPipeline.h"

vtkStandardNewMacro(vtkPVExtractHistogram2D);
vtkCxxSetObjectMacro(vtkPVExtractHistogram2D, Controller, vtkMultiProcessController);

//-------------------------------------------------------------------------------------------------
vtkPVExtractHistogram2D::vtkPVExtractHistogram2D()
{
  this->InitializeCache();
}

//-------------------------------------------------------------------------------------------------
vtkPVExtractHistogram2D::~vtkPVExtractHistogram2D()
{
  if (this->ComponentArrayCache[1])
  {
    if (!strcmp(this->ComponentArrayCache[1]->GetName(), "GradientMag"))
    {
      this->ComponentArrayCache[1]->UnRegister(this);
      this->ComponentArrayCache[1] = nullptr;
    }
  }
}

//------------------------------------------------------------------------------------------------
void vtkPVExtractHistogram2D::PrintSelf(ostream& os, vtkIndent indent)
{
  // os << indent << " = " << this-><< endl;
}

//------------------------------------------------------------------------------------------------
int vtkPVExtractHistogram2D::FillInputPortInformation(int port, vtkInformation* info)
{
  this->Superclass::FillInputPortInformation(port, info);

  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataObject");
  return 1;
}

//------------------------------------------------------------------------------------------------
int vtkPVExtractHistogram2D::RequestInformation(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  this->InitializeCache();
  this->GetInputArrays(inputVector);
  if (!this->ComponentArrayCache[0])
  {
    return 0;
  }
  this->ComputeComponentRange();

  int ext[6] = { 0, this->NumberOfBins[0] - 1, 0, this->NumberOfBins[1] - 1, 0, 0 };
  outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), ext, 6);
  double sp[3] = { 1, 1, 1 };
  sp[0] =
    (this->ComponentRangeCache[0][1] - this->ComponentRangeCache[0][0]) / this->NumberOfBins[0];
  sp[1] =
    (this->ComponentRangeCache[1][1] - this->ComponentRangeCache[1][0]) / this->NumberOfBins[1];
  outInfo->Set(vtkDataObject::SPACING(), sp, 3);
  double o[3] = { this->ComponentRangeCache[0][0], this->ComponentRangeCache[1][0], 0 };
  outInfo->Set(vtkDataObject::ORIGIN(), o, 3);

  vtkDataObject::SetPointDataActiveScalarInfo(outInfo, VTK_DOUBLE, 1);
  return 1;
}

//------------------------------------------------------------------------------------------------
int vtkPVExtractHistogram2D::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  vtkDataObject* input = inInfo->Get(vtkDataObject::DATA_OBJECT());
  if (!input || !this->ComponentArrayCache[0])
  {
    return 0;
  }

  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  vtkImageData* output = vtkImageData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));
  output->SetOrigin(this->ComponentRangeCache[0][0], this->ComponentRangeCache[1][0], 0.0);
  output->SetDimensions(this->NumberOfBins[0], this->NumberOfBins[1], 1);
  double sp[3] = { 1, 1, 1 };
  sp[0] =
    (this->ComponentRangeCache[0][1] - this->ComponentRangeCache[0][0]) / this->NumberOfBins[0];
  sp[1] =
    (this->ComponentRangeCache[1][1] - this->ComponentRangeCache[1][0]) / this->NumberOfBins[1];
  output->SetSpacing(sp);
  output->AllocateScalars(VTK_DOUBLE, 1);
  this->ComputeHistogram2D(output);

  return 1;
}

//------------------------------------------------------------------------------------------------
void vtkPVExtractHistogram2D::InitializeCache()
{
  this->ComponentIndexCache[0] = 0;
  this->ComponentIndexCache[1] = 0;
  this->ComponentArrayCache[0] = nullptr;
  this->ComponentArrayCache[1] = nullptr;
  this->ComponentRangeCache[0][0] = 0.0;
  this->ComponentRangeCache[0][1] = 1.0;
  this->ComponentRangeCache[1][0] = 0.0;
  this->ComponentRangeCache[1][1] = 1.0;
}

//------------------------------------------------------------------------------------------------
void vtkPVExtractHistogram2D::GetInputArrays(vtkInformationVector** inputVector)
{
  if (!inputVector)
  {
    return;
  }

  // TODO: Handle a composite dataset

  this->ComponentArrayCache[0] = this->GetInputArrayToProcess(0, inputVector);

  // Figure out if we are using the gradient magnitude for the Y axis
  if (this->UseGradientForYAxis)
  {
    vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
    vtkDataObject* input = inInfo->Get(vtkDataObject::DATA_OBJECT());
    this->ComputeGradient(input);
  }
  else
  {
    if (this->ComponentArrayCache[1])
    {
      if (!strcmp(this->ComponentArrayCache[1]->GetName(), "GradientMag"))
      {
        this->ComponentArrayCache[1]->UnRegister(this);
        this->ComponentArrayCache[1] = nullptr;
      }
    }
    vtkInformationVector* inArrayVec = this->Information->Get(INPUT_ARRAYS_TO_PROCESS());
    if (inArrayVec->GetNumberOfInformationObjects() > 1)
    {
      this->ComponentArrayCache[1] = this->GetInputArrayToProcess(1, inputVector);
    }
    else
    {
      this->ComponentArrayCache[1] = this->ComponentArrayCache[0];
    }
  }
}

//------------------------------------------------------------------------------------------------
void vtkPVExtractHistogram2D::ComputeComponentRange()
{
  if (!this->ComponentArrayCache[0] || !this->ComponentArrayCache[1])
  {
    return;
  }

  this->ComponentIndexCache[0] = this->Component0;
  if (this->ComponentIndexCache[0] < 0 ||
    this->ComponentIndexCache[0] > this->ComponentArrayCache[0]->GetNumberOfComponents())
  {
    this->ComponentIndexCache[0] = 0;
  }
  this->ComponentIndexCache[1] = this->Component1;
  if (this->ComponentIndexCache[1] < 0 ||
    this->ComponentIndexCache[1] > this->ComponentArrayCache[1]->GetNumberOfComponents())
  {
    this->ComponentIndexCache[1] = 0;
  }

  if (!this->UseCustomBinRanges0)
  {
    this->ComponentArrayCache[0]->GetFiniteRange(
      this->ComponentRangeCache[0], this->ComponentIndexCache[0]);
  }
  else
  {
    this->ComponentRangeCache[0][0] = this->CustomBinRanges0[0];
    this->ComponentRangeCache[0][1] = this->CustomBinRanges0[1];
  }
  if (this->ComponentRangeCache[0][1] < this->ComponentRangeCache[0][0])
  {
    double tmp = this->ComponentRangeCache[0][1];
    this->ComponentRangeCache[0][1] = this->ComponentRangeCache[0][0];
    this->ComponentRangeCache[0][0] = tmp;
  }

  if (this->UseGradientForYAxis)
  {
    this->ComponentArrayCache[1]->GetFiniteRange(
      this->ComponentRangeCache[1], this->ComponentIndexCache[1]);
  }
  else
  {
    if (this->UseCustomBinRanges1)
    {
      this->ComponentRangeCache[1][0] = this->CustomBinRanges1[0];
      this->ComponentRangeCache[1][1] = this->CustomBinRanges1[1];
    }
    else
    {
      this->ComponentArrayCache[1]->GetFiniteRange(
        this->ComponentRangeCache[1], this->ComponentIndexCache[1]);
    }
  }

  if (this->ComponentRangeCache[1][1] < this->ComponentRangeCache[1][0])
  {
    double tmp = this->ComponentRangeCache[1][1];
    this->ComponentRangeCache[1][1] = this->ComponentRangeCache[1][0];
    this->ComponentRangeCache[1][0] = tmp;
  }
}

//------------------------------------------------------------------------------------------------
void vtkPVExtractHistogram2D::ComputeHistogram2D(vtkImageData* histogram)
{
  if (!this->ComponentArrayCache[0] || !histogram)
  {
    return;
  }

  auto histArray = histogram->GetPointData()->GetScalars();
  histArray->FillComponent(0, 0);

  const auto arr1Range = vtk::DataArrayTupleRange(this->ComponentArrayCache[0]);
  const auto arr2Range = vtk::DataArrayTupleRange(this->ComponentArrayCache[1]);

  const vtk::TupleIdType numTuples = arr1Range.size();
  if (arr2Range.size() != numTuples)
  {
    vtkErrorMacro("<< Both arrays should be the same size");
    return;
  }

  for (vtk::TupleIdType tupleId = 0; tupleId < numTuples; ++tupleId)
  {
    const auto a1 = arr1Range[tupleId][this->ComponentIndexCache[0]];
    vtkIdType bin1 =
      static_cast<vtkIdType>((a1 - this->ComponentRangeCache[0][0]) * (this->NumberOfBins[0] - 1) /
        (this->ComponentRangeCache[0][1] - this->ComponentRangeCache[0][0]));
    const auto a2 = arr2Range[tupleId][this->ComponentIndexCache[1]];
    vtkIdType bin2 =
      static_cast<vtkIdType>((a2 - this->ComponentRangeCache[1][0]) * (this->NumberOfBins[1] - 1) /
        (this->ComponentRangeCache[1][1] - this->ComponentRangeCache[1][0]));
    vtkIdType histIndex = bin2 * this->NumberOfBins[0] + bin1;
    histArray->SetTuple1(histIndex, histArray->GetTuple1(histIndex) + 1);
  }
  return;
}

//------------------------------------------------------------------------------------------------
void vtkPVExtractHistogram2D::ComputeGradient(vtkDataObject* input)
{
  if (!input)
  {
    return;
  }

  vtkNew<vtkGradientFilter> gf;
  gf->SetInputData(input);
  gf->SetComputeGradient(true);
  gf->SetComputeVorticity(false);
  gf->SetComputeDivergence(false);
  gf->SetComputeQCriterion(false);
  gf->SetResultArrayName("Gradient");
  gf->SetInputArrayToProcess(
    0, 0, 0, this->GetInputArrayAssociation(0, input), this->ComponentArrayCache[0]->GetName());
  gf->Update();

  const auto gradientArray = gf->GetOutput()->GetPointData()->GetArray("Gradient");
  const auto gradientArrRange = vtk::DataArrayTupleRange(gradientArray);
  const vtk::TupleIdType numTuples = gradientArrRange.size();

  if (this->ComponentArrayCache[1])
  {
    if (!strcmp(this->ComponentArrayCache[1]->GetName(), "GradientMag"))
    {
      this->ComponentArrayCache[1]->UnRegister(this);
      this->ComponentArrayCache[1] = nullptr;
    }
  }

  this->ComponentArrayCache[1] = vtkDoubleArray::New();
  this->ComponentArrayCache[1]->SetName("GradientMag");
  this->ComponentArrayCache[1]->SetNumberOfComponents(1);
  this->ComponentArrayCache[1]->SetNumberOfTuples(numTuples);

  auto gradMagRange = vtk::DataArrayTupleRange(this->ComponentArrayCache[1]);

  for (vtk::TupleIdType tupleId = 0; tupleId < numTuples; ++tupleId)
  {
    double grad[3];
    gradientArrRange[tupleId].GetTuple(grad);
    gradMagRange[tupleId][0] = vtkMath::Norm(grad);
  }
}
