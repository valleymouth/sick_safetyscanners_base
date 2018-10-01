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
 * \file Cola2Session.h
 *
 * \author  Lennart Puck <puck@fzi.de>
 * \date    2018-09-24
 */
//----------------------------------------------------------------------

#ifndef COLA2SESSION_H
#define COLA2SESSION_H

#include <sick_microscan3_ros_driver/datastructure/PacketBuffer.h>

#include <sick_microscan3_ros_driver/communication/AsyncTCPClient.h>

#include <sick_microscan3_ros_driver/cola2/CloseSession.h>
#include <sick_microscan3_ros_driver/cola2/Command.h>
#include <sick_microscan3_ros_driver/cola2/CreateSession.h>

#include <sick_microscan3_ros_driver/data_processing/ParseTCPPacket.h>
#include <sick_microscan3_ros_driver/data_processing/TCPPacketMerger.h>

#include <boost/bind.hpp>

namespace sick {
namespace cola2 {

class Command;
class CreateSession;

class Cola2Session
{
public:
  typedef std::shared_ptr<sick::cola2::Command> CommandPtr;

  Cola2Session(std::shared_ptr<communication::AsyncTCPClient> async_tcp_client);

  bool executeCommand(CommandPtr command);

  uint32_t getSessionID() const;
  void setSessionID(const uint32_t& session_id);

  uint16_t getNextRequestID();

  bool close();
  bool open();
  void waitForCompletion();

private:
  void processPacket(const sick::datastructure::PacketBuffer& packet);

  bool addCommand(uint16_t request_id, CommandPtr command);
  bool findCommand(uint16_t request_id, CommandPtr& command);
  bool removeCommand(uint16_t request_id);


  std::shared_ptr<sick::communication::AsyncTCPClient> m_async_tcp_client_ptr;
  std::shared_ptr<sick::data_processing::ParseTCPPacket> m_parser_ptr;
  std::shared_ptr<sick::data_processing::TCPPacketMerger> m_packet_merger_ptr;
  std::shared_ptr<sick::data_processing::ParseTCPPacket> m_tcp_parser_ptr;

  std::map<uint16_t, CommandPtr> m_pending_commands_map;

  boost::mutex m_execution_mutex;


  uint32_t m_session_id;
  uint16_t m_last_request_id;

  bool startProcessingAndRemovePendingCommandAfterwards(sick::datastructure::PacketBuffer& packet);
  bool addPacketToMerger(const sick::datastructure::PacketBuffer& packet);
  bool checkIfPacketIsCompleteAndOtherwiseListenForMorePackets();
  bool sendTelegramAndListenForAnswer(CommandPtr command);
};


} // namespace cola2
} // namespace sick

#endif
