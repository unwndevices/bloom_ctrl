/**
 *   ESP32 SvelteKit
 *
 *   A simple, secure and extensible framework for IoT projects for ESP32 platforms
 *   with responsive Sveltekit front-end built with TailwindCSS and DaisyUI.
 *   https://github.com/theelims/ESP32-sveltekit
 *
 *   Copyright (C) 2018 - 2023 rjwats
 *   Copyright (C) 2023 - 2024 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the LGPL v3 license. See the LICENSE file for details.
 **/

#include <RelayStateService.h>
#include <pinout.h>

RelayStateService::RelayStateService(PsychicHttpServer *server,
                                     ESP32SvelteKit *sveltekit,
                                     RelayMqttSettingsService *relayMqttSettingsService) : _httpEndpoint(RelayState::read,
                                                                                                         RelayState::update,
                                                                                                         this,
                                                                                                         server,
                                                                                                         RELAY_SETTINGS_ENDPOINT_PATH,
                                                                                                         sveltekit->getSecurityManager(),
                                                                                                         AuthenticationPredicates::IS_AUTHENTICATED),
                                                                                           _eventEndpoint(RelayState::read,
                                                                                                          RelayState::update,
                                                                                                          this,
                                                                                                          sveltekit->getSocket(),
                                                                                                          RELAY_SETTINGS_EVENT),
                                                                                           _mqttEndpoint(RelayState::homeAssistRead,
                                                                                                         RelayState::homeAssistUpdate,
                                                                                                         this,
                                                                                                         sveltekit->getMqttClient()),
                                                                                           _webSocketServer(RelayState::read,
                                                                                                            RelayState::update,
                                                                                                            this,
                                                                                                            server,
                                                                                                            RELAY_SETTINGS_SOCKET_PATH,
                                                                                                            sveltekit->getSecurityManager(),
                                                                                                            AuthenticationPredicates::IS_AUTHENTICATED),
                                                                                           _mqttClient(sveltekit->getMqttClient()),
                                                                                           _relayMqttSettingsService(relayMqttSettingsService)
{
    // Initialize the three relays
    _state.relays = {
        {false, "Light", RELAY_LIGHT, "light"},
        {false, "Pump", RELAY_PUMP, "pump"},
        {false, "Extra", RELAY_EXTRA, "extra"}};

    // Configure pins for all relays
    for (const auto &relay : _state.relays)
    {
        pinMode(relay.pin, OUTPUT);
        digitalWrite(relay.pin, LOW); // Initialize all relays to OFF state
    }

    // Configure MQTT callback
    _mqttClient->onConnect(std::bind(&RelayStateService::registerConfig, this));

    // Configure update handler for when the relay settings change
    _relayMqttSettingsService->addUpdateHandler([&](const String &originId)
                                                { registerConfig(); },
                                                false);

    // Configure settings service update handler to update relay states
    addUpdateHandler([&](const String &originId)
                     { onConfigUpdated(); },
                     false);
}

void RelayStateService::begin()
{
    _httpEndpoint.begin();
    _eventEndpoint.begin();
    onConfigUpdated();
}

void RelayStateService::onConfigUpdated()
{
    log_d("RelayStateService::onConfigUpdated");

    // Update physical relay states
    for (const auto &relay : _state.relays)
    {
        digitalWrite(relay.pin, relay.state ? HIGH : LOW);
    }
}

void RelayStateService::registerConfig()
{
    if (!_mqttClient->connected())
    {
        return;
    }

    _relayMqttSettingsService->read([&](RelayMqttSettings &settings)
                                    {
        // Register each relay with Home Assistant
        for (const auto& relay : _state.relays) {
            String basePath = settings.mqttPath + "/" + relay.type;
            String configTopic = basePath + "/config";
            String subTopic = basePath + "/set";
            String pubTopic = basePath + "/state";

            JsonDocument doc;
            doc["~"] = basePath;
            doc["name"] = relay.name;
            doc["unique_id"] = settings.uniqueId + "_" + relay.type;
            doc["cmd_t"] = "~/set";
            doc["stat_t"] = "~/state";
            doc["schema"] = "json";

            String payload;
            serializeJson(doc, payload);
            _mqttClient->publish(configTopic.c_str(), 0, false, payload.c_str());

            // Configure MQTT topics for this relay
            _mqttEndpoint.configureTopics(pubTopic, subTopic);
        } });
}
