// Copyright DEWETRON GmbH 2019

#include "odkapi_config_item_keys.h"
#include "odkfw_custom_request_handler.h"
#include "odkfw_properties.h"
#include "odkfw_software_channel_plugin.h"
#include "odkbase_message_return_value_holder.h"
#include "odkapi_utils.h"

#include "qml.rcc.h"
#include "sdk_csv_utils.h"

#include <fstream>
#include <functional>
#include <string.h>

// Manifest constains necessary metadata for oxygen plugins
//   OxygenPlugin.name: unique plugin identifier; please use your (company) name to avoid name conflicts. This name is also used as a prefix in all custom config item keys.
//   OxygenPlugin.uuid: unique number (generated by a GUID/UUID generator tool) that stored in configuration files to match channels etc. to the correct plugin
static const char* PLUGIN_MANIFEST =
R"XML(<?xml version="1.0"?>
<OxygenPlugin name="ODK_REPLAY_SYNC_SCALAR" version="1.0" uuid="E70860DB-1A21-4C1E-8329-55A1871ACC7A">
  <Info name="Example Plugin: Simple file replay">
    <Vendor name="DEWETRON GmbH"/>
    <Description>SDK Example plugin implementing file replay into a synchronous scalar channel.</Description>
  </Info>
  <Host minimum_version="5.3"/>
  <UsesUIExtensions/>
</OxygenPlugin>
)XML";

// A minimal translation file that maps the internal ConfigItem key to a nicer text for the user
static const char* TRANSLATION_EN =
R"XML(<?xml version="1.0"?>
<TS version="2.1" language="en" sourcelanguage="en">
    <context><name>ConfigKeys</name>
        <message><source>ODK_REPLAY_SYNC_SCALAR/InputFile</source><translation>Input File</translation></message>
    </context>

    <!-- Translations for custom UI item -->
    <context><name>ODK_REPLAY_SYNC_SCALAR/AddChannel</name>
        <message><source>Not a valid CSV file</source><translation>No valid CSV file selected</translation></message>
    </context>
</TS>
)XML";


// Keys for ConfigItems that are used to store channel settings

// Custom key (prefixed by plugin name) to store path to the input file
static const char* KEY_INPUT_FILE = "ODK_REPLAY_SYNC_SCALAR/InputFile";

using namespace odk::framework;

class ReplayChannel : public SoftwareChannelInstance
{
public:

    ReplayChannel()
        : m_input_file(new EditableStringProperty(""))
        , m_acquisition_rate(new EditableScalarProperty(1000, "Hz", 0.01, 10000000)) //SampleRate can be configured freely in a wide range (0.01 to 10000000 Hz)
        , m_next_tick(std::numeric_limits<uint64_t>::max())
    {
        // add some proposed sample rates
        m_acquisition_rate->addOption(1);
        m_acquisition_rate->addOption(100);
        m_acquisition_rate->addOption(1000);
    }

    // Describe how the software channel should be shown in the "Add Channel" dialog
    static odk::RegisterSoftwareChannel getSoftwareChannelInfo()
    {
        odk::RegisterSoftwareChannel telegram;
        telegram.m_display_name = "Example Plugin: Simple file replay";
        telegram.m_service_name = "CreateChannel";
        telegram.m_display_group = "Data Sources";
        telegram.m_description = "Adds a synchronous channel that delivers samples read from a CSV file.";
        telegram.m_ui_item_add = "AddChannel";
        return telegram;
    }

    bool setup(const std::vector<odk::Property>& properties) override
    {
        odk::PropertyList props(properties);

        auto csv_file = props.getString("ODK_REPLAY_SYNC_SCALAR/CSVFile");
        if (!csv_file.empty())
        {
            m_input_file->setValue(csv_file);
            update();
        }

        return true;
    }

    void updatePropertyTypes(const PluginChannelPtr& output_channel) override
    {
        ODK_UNUSED(output_channel);
    }

    void updateStaticPropertyConstraints(const PluginChannelPtr& channel) override
    {
        ODK_UNUSED(channel);
    }

    bool update() override
    {
        // channel is only valid if we can properly parse the specified csv file
        // in production code the parsing should not be done on every config change, but only if the filename was updated
        CSVNumberReader csv;
        std::ifstream input_stream(m_input_file->getValue());

        m_next_tick = std::numeric_limits<uint64_t>::max();
        m_values.clear();

        double range_min = std::numeric_limits<double>::max();
        double range_max = std::numeric_limits<double>::lowest();

        if (input_stream && csv.parse(input_stream) && !csv.m_values.empty())
        {
            m_values.reserve(csv.m_values.size());
            size_t column_index = 0;
            for (const auto& row_values : csv.m_values)
            {
                if (row_values.size() > column_index)
                {
                    range_min = std::min(range_min, row_values[column_index]);
                    range_max = std::max(range_max, row_values[column_index]);
                    m_values.push_back(row_values[column_index]);
                }
                else
                {
                    m_values.push_back(std::numeric_limits<double>::quiet_NaN());
                }
            }
        }

        bool is_valid = range_min <= range_max;

        getRootChannel()->setRange({range_min, range_max, "", ""});
        getRootChannel()->setValid(is_valid);
        getRootChannel()->getRangeProperty()->setLive(is_valid);
        getRootChannel()->setSimpleTimebase(m_acquisition_rate->getValue().m_val);

        return is_valid;
    }

    void create(odk::IfHost* host) override
    {
        ODK_UNUSED(host);

        getRootChannel()->setDefaultName("Replay channel")
            .setSampleFormat(
                odk::ChannelDataformat::SampleOccurrence::SYNC,
                odk::ChannelDataformat::SampleFormat::DOUBLE,
                1)
            .setSimpleTimebase(m_acquisition_rate->getValue().m_val)
            .setDeletable(true)
            .addProperty(KEY_INPUT_FILE, m_input_file)
            .addProperty(ODK_CI_KEY_ACQUISITION_RATE, m_acquisition_rate);
    }

    bool configure(
        const odk::UpdateChannelsTelegram& request,
        std::map<std::uint32_t, std::uint32_t>& channel_id_map) override
    {
        configureFromTelegram(request, channel_id_map);
        return true;
    }

    void prepareProcessing(odk::IfHost* host) override
    {
        const auto ts = getMasterTimestamp(host);
        const auto rate_factor = m_acquisition_rate->getValue().m_val / ts.m_frequency;
        m_next_tick = static_cast<std::uint64_t>(ts.m_ticks* rate_factor);
    }

    void process(ProcessingContext& context, odk::IfHost *host) override
    {
        ODK_UNUSED(context);

        std::uint32_t channel_id = getRootChannel()->getLocalId();
        auto ts = getMasterTimestamp(host);

        auto tick = m_next_tick;
        const auto rate_factor = m_acquisition_rate->getValue().m_val / ts.m_frequency;
        std::uint64_t target_tick = static_cast<std::uint64_t>(ts.m_ticks * rate_factor);
        while (tick < target_tick)
        {
            auto idx = tick % m_values.size();
            auto sz = std::min(m_values.size() - idx, target_tick - tick);
            addSamples(host, channel_id, tick, &m_values[idx], sizeof(double) * sz);
            tick += sz;
        }
        m_next_tick = tick;
    }

private:
    std::shared_ptr<EditableStringProperty> m_input_file;
    std::shared_ptr<EditableScalarProperty> m_acquisition_rate;
    std::vector<double> m_values;

    std::uint64_t m_next_tick; // timestamp of the next sample that will be generated in doProcess()
};

class ReplaySyncScalarPlugin : public SoftwareChannelPlugin<ReplayChannel>
{
public:
    ReplaySyncScalarPlugin()
        : m_custom_requests(std::make_shared<odk::framework::CustomRequestHandler>())
    {
        addMessageHandler(m_custom_requests);

        namespace arg = std::placeholders;
        m_custom_requests->registerFunction(1, "checkCSVFile", std::bind(&ReplaySyncScalarPlugin::checkCSVFile, this, arg::_1, arg::_2));
    }

    void registerResources() final
    {
        addTranslation(TRANSLATION_EN);
        addQtResources(plugin_resources::QML_RCC_data, plugin_resources::QML_RCC_size);
    }

    std::uint64_t checkCSVFile(const odk::PropertyList& params, odk::PropertyList& returns)
    {
        const auto filename = params.getString("filename");

        CSVNumberReader csv;
        std::ifstream input_stream(filename);
        const bool valid = input_stream && csv.parse(input_stream) && !csv.m_values.empty();

        returns.setBool("valid", valid);
        return odk::error_codes::OK;
    }

private:
    std::shared_ptr<odk::framework::CustomRequestHandler> m_custom_requests;
};

OXY_REGISTER_PLUGIN1("ODK_REPLAY_SYNC_SCALAR", PLUGIN_MANIFEST, ReplaySyncScalarPlugin);

