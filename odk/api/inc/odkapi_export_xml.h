// Copyright DEWETRON GmbH 2020
#pragma once

#include "odkapi_property_list_xml.h"
#include "odkapi_types.h"
#include "odkuni_defines.h"
#include "odkuni_xpugixml.h"

#include <boost/utility/string_view.hpp>
#include <map>
#include <string>
#include <vector>

namespace odk
{
    class RegisterExport
    {
    public:
        enum StartExportAction
        {
            SELECT_FILE,
            SELECT_DIRECTORY,
            NONE
        };

        RegisterExport();

        bool parse(const boost::string_view& xml_string);

        ODK_NODISCARD std::string generate() const;

        std::string m_format_name;
        std::string m_format_id;
        std::string m_file_extension;

        std::string m_ui_item_small;
        std::string m_ui_item_full;

        StartExportAction m_start_export_action;
    };

    class ExportProperties
    {
    public:
        ExportProperties();

        bool parse(const boost::string_view& xml_string);
        bool parse(pugi::xml_node export_properties_node);

        ODK_NODISCARD std::string generate() const;

        /*
        * creates XML without header to reuse in another xml document
        */
        ODK_NODISCARD std::string generateNodeXML() const;

        std::vector<std::uint64_t> m_channels;
        std::vector<odk::Interval<double>> m_export_intervals;
        std::string m_filename;
        std::string m_format_id;
        odk::PropertyList m_custom_properties;
    };

    class StartExport
    {
    public:
        StartExport();

        bool parse(const boost::string_view& xml_string);

        ODK_NODISCARD std::string generate() const;

        /* transaction id must be > 0!!*/
        std::uint64_t m_transaction_id;
        ExportProperties m_properties;
    };

    class ValidateExport
    {
    public:
        ValidateExport();

        bool parse(const boost::string_view& xml_string);

        ODK_NODISCARD std::string generate() const;

        ExportProperties m_properties;
    };

    struct ChannelError
    {
        ChannelError(std::uint64_t channel_id, std::uint64_t error_code);
        ChannelError(std::uint64_t channel_id, std::uint64_t error_code, const std::string& error_message);

        std::uint64_t channel_id;
        std::uint64_t error_code;
        std::string error_message;
    };

    class ValidateExportResponse
    {
    public:
        ValidateExportResponse();

        bool parse(const boost::string_view& xml_string);

        ODK_NODISCARD std::string generate() const;

        bool m_success;
        std::vector<ChannelError> m_channel_errors;
        std::vector<ChannelError> m_channel_warnings;
    };
}
