/*
// Copyright (c) 2021-2025 Hewlett-Packard Enterprise Development, LP
//
// Hewlett-Packard and the Hewlett-Packard logo are trademarks of
// Hewlett-Packard Development Company, L.P. in the U.S. and/or other countries.
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
*/

#include <phosphor-logging/lg2.hpp>
#include <phosphor-logging/elog.hpp>
#include <sdbusplus/bus.hpp>

extern "C" {
#include "DataExtract.h"
}

static const std::map<int, std::string> log_map {
    {EVT_TYPE_IML, "IML"},
    {EVT_TYPE_IEL, "IEL"},
    {EVT_TYPE_SL, "SL"}
};

static const std::map<int, std::string> phosphor_loglevel_map {
    {EVT_CRITICAL, "xyz.openbmc_project.Logging.Entry.Level.Error"},
    {EVT_CAUTION, "xyz.openbmc_project.Logging.Entry.Level.Warning"},
    {EVT_INFORMATIONAL, "xyz.openbmc_project.Logging.Entry.Level.Informational"},
    {EVT_REPAIRED, "xyz.openbmc_project.Logging.Entry.Level.Informational"}};

#define PHOSPHOR_LOGGING_BUS_NAME        "xyz.openbmc_project.Logging"
#define PHOSPHOR_LOGGING_OBJECT_PATH     "/xyz/openbmc_project/logging"
#define PHOSPHOR_LOGGING_INTERFACE_NAME  "xyz.openbmc_project.Logging.Create"

sdbusplus::bus_t bus(sdbusplus::bus::new_default());

void dbus_send(std::string message,
                     uint32_t evtClass,
                     uint32_t  evtCode,
                     int logType,
                     std::string action,
                     int severity)
{
    try
    {
        std::map<std::string, std::string> additionalData;
        additionalData["_PID"] = std::to_string(getpid());
        additionalData.emplace("CLASS", std::to_string(evtClass));
        additionalData.emplace("CODE", std::to_string(evtCode));
        additionalData.emplace("LOG_TYPE", log_map.find(logType)->second);
        additionalData.emplace("EVENT_ACTION", action);
        auto method = bus.new_method_call(PHOSPHOR_LOGGING_BUS_NAME, PHOSPHOR_LOGGING_OBJECT_PATH,
                                          PHOSPHOR_LOGGING_INTERFACE_NAME, "Create");

        auto level = phosphor_loglevel_map.find(severity)->second;
        // ssa{ss} "This is a Test" "xyz.openbmc_project.Logging.Entry.Level.Error" 2 "KEY1" "VALUE1" "KEY2" "VALUE2"
        method.append(message, level, additionalData);

        auto reply = bus.call(method);
    }
    catch (const std::exception& e)
    {
        lg2::error("Failed creating event log for {ERROR} due to {EXCEPTION}",
                   "ERROR", message, "EXCEPTION", e.what());
    }

    return;
}
