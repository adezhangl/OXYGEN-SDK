// Copyright DEWETRON GmbH 2019

#include "odkfw_software_channel_plugin.h"

#include "odkfw_version_check.h"

namespace odk
{
namespace framework
{
    static const char* REQUIRED_OXYGEN_VERSION = "5.6";

    SoftwareChannelPluginBase::SoftwareChannelPluginBase()
        : m_plugin_channels(std::make_shared<odk::framework::PluginChannels>())
        , m_custom_requests(std::make_shared<odk::framework::CustomRequestHandler>())
    {
        addMessageHandler(m_plugin_channels);
        addMessageHandler(m_custom_requests);
    }

    std::uint64_t SoftwareChannelPluginBase::init(std::string& error_message)
    {
        if (!checkOxygenCompatibility())
        {
            error_message = "Current version of Oxygen is not supported.";
            return odk::error_codes::UNSUPPORTED_VERSION;
        }

        registerResources();
        registerSoftwareChannel();
        return odk::error_codes::OK;
    }

    bool SoftwareChannelPluginBase::deinit()
    {
        getPluginChannels()->pauseTasks();
        unregisterSoftwareChannel();
        return true;
    }

    void SoftwareChannelPluginBase::registerSoftwareChannel()
    {
        const auto telegram = getSoftwareChannelInfo();
        auto xml_msg = getHost()->template createValue<odk::IfXMLValue>();
        xml_msg->set(telegram.generate().c_str());
        getHost()->messageSync(odk::host_msg::SOFTWARE_CHANNEL_REGISTER, 0, xml_msg.get(), nullptr);
    }

    void SoftwareChannelPluginBase::updateSoftwareChannel()
    {
        const auto telegram = getSoftwareChannelInfo();
        auto xml_msg = getHost()->template createValue<odk::IfXMLValue>();
        xml_msg->set(telegram.generate().c_str());
        getHost()->messageSync(odk::host_msg::SOFTWARE_CHANNEL_UPDATE, 0, xml_msg.get(), nullptr);
    }

    void SoftwareChannelPluginBase::unregisterSoftwareChannel()
    {
        auto channel_id = getHost()->template createValue<odk::IfStringValue>();
        const auto telegram = getSoftwareChannelInfo();
        channel_id->set(telegram.m_service_name.c_str());
        getHost()->messageSync(odk::host_msg::SOFTWARE_CHANNEL_UNREGISTER, 0, channel_id.get(), nullptr);
    }

    bool SoftwareChannelPluginBase::checkOxygenCompatibility()
    {
        auto name = getHost()->getValue<odk::IfStringValue>(odk::queries::PluginHost, odk::queries::PluginHost_Name);
        auto version = getHost()->getValue<odk::IfStringValue>(odk::queries::PluginHost, odk::queries::PluginHost_VersionString);
        if (name && version)
        {
            if (std::string("Oxygen") == name->getValue())
            {
                if (compareVersionStrings(REQUIRED_OXYGEN_VERSION, version->getValue()) <= 0)
                {
                    return true;
                }
            }
        }
        return false;
    }

    PluginChannelsPtr SoftwareChannelPluginBase::getPluginChannels()
    {
        return m_plugin_channels;
    }

    std::shared_ptr<CustomRequestHandler> SoftwareChannelPluginBase::getCustomRequestHandler()
    {
        return m_custom_requests;
    }

}
}

