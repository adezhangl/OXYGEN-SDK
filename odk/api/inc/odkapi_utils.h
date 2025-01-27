// Copyright DEWETRON GmbH 2019
#pragma once

#define ODK_EXTENSION_FUNCTIONS //enable C++ integration

#include "odkbase_if_host_fwd.h"
#include "odkbase_basic_values.h"

#include "odkapi_timestamp_xml.h"

#include "odkuni_defines.h"

#include <cmath>
#include <cstring>
#include <limits>
#include <vector>

namespace odk
{
    ODK_NODISCARD bool isAnalysisModeActive(odk::IfHost* host);

    ODK_NODISCARD odk::Timestamp getMasterTimestamp(odk::IfHost* host);

    ODK_NODISCARD odk::AbsoluteTime getAcquisitionStartTime(odk::IfHost* host);

    ODK_NODISCARD odk::AbsoluteTime getMeasurementStartTime(odk::IfHost* host);

    /**
     * Sends a odk::host_msg::ADD_CONTIGUOUS_SAMPLES message to the host
     */
    void addSamples(odk::IfHost* host, std::uint32_t local_channel_id, std::uint64_t timestamp, const void* data, size_t data_size);

    /**
     * Sends a odk::host_msg::ADD_SAMPLE message to the host
     */
    void addSample(odk::IfHost* host, std::uint32_t local_channel_id, std::uint64_t timestamp, const void* data, size_t data_size);

    template <class T>
    inline void addSample(odk::IfHost* host, std::uint32_t local_channel_id, std::uint64_t timestamp, const T& data)
    {
        addSample(host, local_channel_id, timestamp, &data, sizeof(T));
    }

    void updateChannelState(odk::IfHost* host, std::uint32_t local_channel_id, std::uint64_t timestamp);

    template <class T>
    bool parseXMLValue(const odk::IfValue* param, T& parser)
    {
        if (param && param->getType() == odk::IfValue::Type::TYPE_XML)
        {
            auto xml_value = static_cast<const odk::IfXMLValue*>(param);
            return parser.parse(xml_value->getValue());
        }
        return false;
    }

    std::uint64_t sendSyncXMLMessage(odk::IfHost* host, odk::MessageId msg_id, std::uint64_t key, const char* param_data, size_t param_size, const odk::IfValue** ret);

    /**
     * Convert from seconds to ticks
     *
     * @param time      sample time in seconds
     * @param frequency sample rate in Hz
     * @return          tick value
     */
    ODK_NODISCARD inline std::uint64_t convertTimeToTickAtOrAfter(double time, double frequency)
    {
        if (time == 0.0)
        {
            return 0;
        }
        return static_cast<std::uint64_t>(std::nextafter(std::nextafter(time, 0.0) * frequency, std::numeric_limits<double>::lowest())) + 1;
    }

    /**
     * Convert from tick values to time in seconds
     * 
     * @param tick      sample position in ticks
     * @param frequency sample rate in Hz
     * @return          tick value converted to seconds
     */
    ODK_NODISCARD inline double convertTickToTime(std::uint64_t tick, double frequency)
    {
        return std::nextafter(tick / frequency, std::numeric_limits<double>::max());
    }
}
