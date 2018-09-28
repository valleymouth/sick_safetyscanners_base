// this is for emacs file handling -*- mode: c++; indent-tabs-mode: nil -*-

// -- BEGIN LICENSE BLOCK ----------------------------------------------

/*!
*  Copyright (C) 2018, SICK AG, Waldkirch
*  Copyright (C) 2018, FZI Forschungszentrum Informatik, Karlsruhe, Germany
*
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.

*/

// -- END LICENSE BLOCK ------------------------------------------------

//----------------------------------------------------------------------
/*!
 * \file ParseApplicationData.cpp
 *
 * \author  Lennart Puck <puck@fzi.de>
 * \date    2018-09-24
 */
//----------------------------------------------------------------------

#include <sick_microscan3_ros_driver/data_processing/ParseApplicationData.h>

namespace sick {
namespace data_processing {

ParseApplicationData::ParseApplicationData()
{
  m_reader_ptr = boost::make_shared<sick::data_processing::ReadWriteHelper>();
}

datastructure::ApplicationData
ParseApplicationData::parseUDPSequence(datastructure::PacketBuffer buffer,
                                       datastructure::Data& data)
{
  datastructure::ApplicationData application_data;

  if (!checkIfPreconditionsAreMet(data))
  {
    application_data.setIsEmpty(true);
    return application_data;
  }
  const BYTE* data_ptr(buffer.getBuffer().data() +
                       data.getDataHeaderPtr()->getApplicationDataBlockOffset());

  setDataInApplicationData(data_ptr, application_data);
  return application_data;
}

bool ParseApplicationData::checkIfPreconditionsAreMet(datastructure::Data& data)
{
  if (!checkIfApplicationDataIsPublished(data))
  {
    return false;
  }
  if (!checkIfDataContainsNeededParsedBlocks(data))
  {
    return false;
  }
  return true;
}

bool ParseApplicationData::checkIfApplicationDataIsPublished(datastructure::Data& data)
{
  if (data.getDataHeaderPtr()->getApplicationDataBlockOffset() == 0 &&
      data.getDataHeaderPtr()->getApplicationDataBlockSize() == 0)
  {
    return false;
  }
  return true;
}

bool ParseApplicationData::checkIfDataContainsNeededParsedBlocks(datastructure::Data& data)
{
  if (data.getDataHeaderPtr()->isEmpty())
  {
    return false;
  }
  return true;
}

void ParseApplicationData::setDataInApplicationData(
  const BYTE* data_ptr, datastructure::ApplicationData& application_data)
{
  setApplicationInputsInApplicationData(data_ptr, application_data);
  setApplicationOutputsInApplicationData(data_ptr, application_data);
}

void ParseApplicationData::setApplicationInputsInApplicationData(
  const BYTE* data_ptr, datastructure::ApplicationData& application_data)
{
  datastructure::ApplicationInputs inputs;
  setDataInApplicationInputs(data_ptr, inputs);
  application_data.setInputs(inputs);
}

void ParseApplicationData::setApplicationOutputsInApplicationData(
  const BYTE* data_ptr, datastructure::ApplicationData& application_data)
{
  datastructure::ApplicationOutputs outputs;
  setDataInApplicationOutputs(data_ptr, outputs);
  application_data.setOutputs(outputs);
}

void ParseApplicationData::setDataInApplicationInputs(const BYTE* data_ptr,
                                                      datastructure::ApplicationInputs& inputs)
{
  setUnsafeInputsInApplicationInputs(data_ptr, inputs);
  setMonitoringCaseInputsInApplicationInputs(data_ptr, inputs);
  setLinearVelocityInputsInApplicationInputs(data_ptr, inputs);
  setSleepModeInputInApplicationInputs(data_ptr, inputs);
}

void ParseApplicationData::setDataInApplicationOutputs(const BYTE* data_ptr,
                                                       datastructure::ApplicationOutputs& outputs)
{
  setEvalutaionPathsOutputsInApplicationOutputs(data_ptr, outputs);
  setMonitoringCaseOutputsInApplicationOutputs(data_ptr, outputs);
  setSleepModeOutputInApplicationOutputs(data_ptr, outputs);
  setErrorFlagsInApplicationOutputs(data_ptr, outputs);
  setLinearVelocityOutoutsInApplicationOutputs(data_ptr, outputs);
  setResultingVelocityOutputsInApplicationOutputs(data_ptr, outputs);
  setOutputFlagsinApplicationOutput(data_ptr, outputs);
}

void ParseApplicationData::setUnsafeInputsInApplicationInputs(
  const BYTE* data_ptr, datastructure::ApplicationInputs& inputs)
{
  setUnsafeInputsSourcesInApplicationInputs(data_ptr, inputs);
  setUnsafeInputsFlagsInApplicationInputs(data_ptr, inputs);
}

void ParseApplicationData::setUnsafeInputsSourcesInApplicationInputs(
  const BYTE* data_ptr, datastructure::ApplicationInputs& inputs)
{
  UINT32 word32 = m_reader_ptr->readUINT32LittleEndian(data_ptr, 0);
  std::vector<bool> input_sources;
  for (int i = 0; i < 32; i++)
  {
    input_sources.push_back(static_cast<bool>(word32 & (0x01 << i)));
  }
  inputs.setUnsafeInputsInputSourcesVector(input_sources);
}

void ParseApplicationData::setUnsafeInputsFlagsInApplicationInputs(
  const BYTE* data_ptr, datastructure::ApplicationInputs& inputs)
{
  UINT32 word32 = m_reader_ptr->readUINT32LittleEndian(data_ptr, 4);
  std::vector<bool> input_flags;
  for (int i = 0; i < 32; i++)
  {
    input_flags.push_back(static_cast<bool>(word32 & (0x01 << i)));
  }
  inputs.setUnsafeInputsFlagsVector(input_flags);
}

void ParseApplicationData::setMonitoringCaseInputsInApplicationInputs(
  const BYTE* data_ptr, datastructure::ApplicationInputs& inputs)
{
  setMonitoringCaseNumbersInApplicationInputs(data_ptr, inputs);
  setMonitoringCaseFlagsInApplicationInputs(data_ptr, inputs);
}

void ParseApplicationData::setMonitoringCaseNumbersInApplicationInputs(
  const BYTE* data_ptr, datastructure::ApplicationInputs& inputs)
{
  std::vector<UINT16> monitoring_cases;
  for (int i = 0; i < 20; i++)
  {
    monitoring_cases.push_back(m_reader_ptr->readUINT16LittleEndian(data_ptr, 12 + i * 2));
  }
  inputs.setMonitoringCaseVector(monitoring_cases);
}


void ParseApplicationData::setMonitoringCaseFlagsInApplicationInputs(
  const BYTE* data_ptr, datastructure::ApplicationInputs& inputs)
{
  UINT32 word32 = m_reader_ptr->readUINT32LittleEndian(data_ptr, 52);
  std::vector<bool> monitoring_flags;
  // 20 for each case one
  for (int i = 0; i < 20; i++)
  {
    monitoring_flags.push_back(static_cast<bool>(word32 & (0x01 << i)));
  }
  inputs.setMonitoringCaseFlagsVector(monitoring_flags);
}

void ParseApplicationData::setLinearVelocityInputsInApplicationInputs(
  const BYTE* data_ptr, datastructure::ApplicationInputs& inputs)
{
  setLinearVelocity0InApplicationInputs(data_ptr, inputs);
  setLinearVelocity1InApplicationInputs(data_ptr, inputs);
  setLinearVelocityFlagsInApplicationInputs(data_ptr, inputs);
}

void ParseApplicationData::setLinearVelocity0InApplicationInputs(
  const BYTE* data_ptr, datastructure::ApplicationInputs& inputs)
{
  inputs.setVelocity0(m_reader_ptr->readUINT16LittleEndian(data_ptr, 56));
}

void ParseApplicationData::setLinearVelocity1InApplicationInputs(
  const BYTE* data_ptr, datastructure::ApplicationInputs& inputs)
{
  inputs.setVelocity1(m_reader_ptr->readUINT16LittleEndian(data_ptr, 58));
}

void ParseApplicationData::setLinearVelocityFlagsInApplicationInputs(
  const BYTE* data_ptr, datastructure::ApplicationInputs& inputs)
{
  UINT8 word8 = m_reader_ptr->readUINT8LittleEndian(data_ptr, 60);

  inputs.setVelocity0Valid(static_cast<bool>(word8 & (0x01 << 0)));
  inputs.setVelocity1Valid(static_cast<bool>(word8 & (0x01 << 1)));
  // reserved bits 2,3
  inputs.setVelocity0TransmittedSafely(static_cast<bool>(word8 & (0x01 << 4)));
  inputs.setVelocity1TransmittedSafely(static_cast<bool>(word8 & (0x01 << 5)));
}

void ParseApplicationData::setSleepModeInputInApplicationInputs(
  const BYTE* data_ptr, datastructure::ApplicationInputs& inputs)
{
  inputs.setSleepModeInput(m_reader_ptr->readUINT8LittleEndian(data_ptr, 74));
}


void ParseApplicationData::setEvalutaionPathsOutputsInApplicationOutputs(
  const BYTE* data_ptr, datastructure::ApplicationOutputs& outputs)
{
  setEvaluationPathsOutputsEvalOutInApplicationOutputs(data_ptr, outputs);
  setEvaluationPathsOutputsIsSafeInApplicationOutputs(data_ptr, outputs);
  setEvaluationPathsOutputsValidFlagsInApplicationOutputs(data_ptr, outputs);
}

void ParseApplicationData::setEvaluationPathsOutputsEvalOutInApplicationOutputs(
  const BYTE* data_ptr, datastructure::ApplicationOutputs& outputs)
{
  UINT32 word32 = m_reader_ptr->readUINT32LittleEndian(data_ptr, 140);

  std::vector<bool> eval_out;
  for (int i = 0; i < 20; i++)
  {
    eval_out.push_back(word32 & (0x01 << i));
  }
  outputs.setEvalOutVector(eval_out);
}

void ParseApplicationData::setEvaluationPathsOutputsIsSafeInApplicationOutputs(
  const BYTE* data_ptr, datastructure::ApplicationOutputs& outputs)
{
  UINT32 word32 = m_reader_ptr->readUINT32LittleEndian(data_ptr, 144);

  std::vector<bool> eval_out_is_safe;
  for (int i = 0; i < 20; i++)
  {
    eval_out_is_safe.push_back(word32 & (0x01 << i));
  }
  outputs.setEvalOutIsSafeVector(eval_out_is_safe);
}


void ParseApplicationData::setEvaluationPathsOutputsValidFlagsInApplicationOutputs(
  const BYTE* data_ptr, datastructure::ApplicationOutputs& outputs)
{
  UINT32 word32 = m_reader_ptr->readUINT32LittleEndian(data_ptr, 148);

  std::vector<bool> eval_out_is_valid;
  for (int i = 0; i < 20; i++)
  {
    eval_out_is_valid.push_back(word32 & (0x01 << i));
  }
  outputs.setEvalOutIsValidVector(eval_out_is_valid);
}

void ParseApplicationData::setMonitoringCaseOutputsInApplicationOutputs(
  const BYTE* data_ptr, datastructure::ApplicationOutputs& outputs)
{
  setMonitoringCaseNumbersInApplicationOutputs(data_ptr, outputs);
  setMonitoringCaseFlagsInApplicationOutputs(data_ptr, outputs);
}

void ParseApplicationData::setMonitoringCaseNumbersInApplicationOutputs(
  const BYTE* data_ptr, datastructure::ApplicationOutputs& outputs)
{
  std::vector<UINT16> output_monitoring_cases;

  for (int i = 0; i < 20; i++)
  {
    output_monitoring_cases.push_back(m_reader_ptr->readUINT16LittleEndian(data_ptr, 152 + i * 2));
  }
  outputs.setMonitoringCaseVector(output_monitoring_cases);
}


void ParseApplicationData::setMonitoringCaseFlagsInApplicationOutputs(
  const BYTE* data_ptr, datastructure::ApplicationOutputs& outputs)
{
  UINT32 word32 = m_reader_ptr->readUINT32LittleEndian(data_ptr, 192);

  std::vector<bool> output_monitoring_flags;
  // 20 for each case one
  for (int i = 0; i < 20; i++)
  {
    output_monitoring_flags.push_back(static_cast<bool>(word32 & (0x01 << i)));
  }
  outputs.setMonitoringCaseFlagsVector(output_monitoring_flags);
}

void ParseApplicationData::setSleepModeOutputInApplicationOutputs(
  const BYTE* data_ptr, datastructure::ApplicationOutputs& outputs)
{
  outputs.setSleepModeOutput(m_reader_ptr->readUINT8LittleEndian(data_ptr, 193));
}

void ParseApplicationData::setErrorFlagsInApplicationOutputs(
  const BYTE* data_ptr, datastructure::ApplicationOutputs& outputs)
{
  UINT8 word8 = m_reader_ptr->readUINT8LittleEndian(data_ptr, 194);

  outputs.setHostErrorFlagContaminationWarning(static_cast<bool>(word8 & (0x01 << 0)));
  outputs.setHostErrorFlagContaminationError(static_cast<bool>(word8 & (0x01 << 1)));
  outputs.setHostErrorFlagManipulationError(static_cast<bool>(word8 & (0x01 << 2)));
  outputs.setHostErrorFlagGlare(static_cast<bool>(word8 & (0x01 << 3)));
  outputs.setHostErrorFlagReferenceContourIntruded(static_cast<bool>(word8 & (0x01 << 4)));
  outputs.setHostErrorFlagCriticalError(static_cast<bool>(word8 & (0x01 << 5)));
}

void ParseApplicationData::setLinearVelocityOutoutsInApplicationOutputs(
  const BYTE* data_ptr, datastructure::ApplicationOutputs& outputs)
{
  setLinearVelocity0InApplicationOutputs(data_ptr, outputs);
  setLinearVelocity1InApplicationOutputs(data_ptr, outputs);
  setLinearVelocityFlagsInApplicationOutputs(data_ptr, outputs);
}

void ParseApplicationData::setLinearVelocity0InApplicationOutputs(
  const BYTE* data_ptr, datastructure::ApplicationOutputs& outputs)
{
  outputs.setVelocity0(m_reader_ptr->readUINT16LittleEndian(data_ptr, 200));
}

void ParseApplicationData::setLinearVelocity1InApplicationOutputs(
  const BYTE* data_ptr, datastructure::ApplicationOutputs& outputs)
{
  outputs.setVelocity1(m_reader_ptr->readUINT16LittleEndian(data_ptr, 202));
}

void ParseApplicationData::setLinearVelocityFlagsInApplicationOutputs(
  const BYTE* data_ptr, datastructure::ApplicationOutputs& outputs)
{
  UINT8 word8 = m_reader_ptr->readUINT8LittleEndian(data_ptr, 204);

  outputs.setVelocity0Valid(static_cast<bool>(word8 & (0x01 << 0)));
  outputs.setVelocity1Valid(static_cast<bool>(word8 & (0x01 << 1)));
  // reserved bits 2,3
  outputs.setVelocity0TransmittedSafely(static_cast<bool>(word8 & (0x01 << 4)));
  outputs.setVelocity1TransmittedSafely(static_cast<bool>(word8 & (0x01 << 5)));
  // reserved bits 6,7
}

void ParseApplicationData::setResultingVelocityOutputsInApplicationOutputs(
  const BYTE* data_ptr, datastructure::ApplicationOutputs& outputs)
{
  setResultingVelocityInApplicationOutputs(data_ptr, outputs);
  setResultingVelocityFlagsInApplicationOutputs(data_ptr, outputs);
}

void ParseApplicationData::setResultingVelocityInApplicationOutputs(
  const BYTE* data_ptr, datastructure::ApplicationOutputs& outputs)
{
  std::vector<INT16> resulting_velocities;
  for (int i = 0; i < 20; i++)
  {
    resulting_velocities.push_back(m_reader_ptr->readINT16LittleEndian(data_ptr, 208 + i * 2));
  }
  outputs.setResultingVelocityVector(resulting_velocities);
}

void ParseApplicationData::setResultingVelocityFlagsInApplicationOutputs(
  const BYTE* data_ptr, datastructure::ApplicationOutputs& outputs)
{
  UINT32 word32 = m_reader_ptr->readUINT32LittleEndian(data_ptr, 248);

  std::vector<bool> resulting_velocities_flags;
  // 20 for each case one
  for (int i = 0; i < 20; i++)
  {
    resulting_velocities_flags.push_back(static_cast<bool>(word32 & (0x01 << i)));
  }
  outputs.setResultingVelocityIsValidVector(resulting_velocities_flags);
}

void ParseApplicationData::setOutputFlagsinApplicationOutput(
  const BYTE* data_ptr, datastructure::ApplicationOutputs& outputs)
{
  UINT8 word8 = m_reader_ptr->readUINT8LittleEndian(data_ptr, 259);

  outputs.setFlagsSleepModeOutputIsValid(static_cast<bool>(word8 & (0x01 << 0)));
  outputs.setFlagsHostErrorFlagsAreValid(static_cast<bool>(word8 & (0x01 << 1)));
}


} // namespace data_processing
} // namespace sick
