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
    // configure relays as outputs
    pinMode(RELAY_LIGHT, OUTPUT);
    pinMode(RELAY_PUMP, OUTPUT);
    pinMode(RELAY_EXTRA, OUTPUT);

    // configure MQTT callback
    _mqttClient->onConnect(std::bind(&RelayStateService::registerConfig, this));

    // configure update handler for when the settings change
    _relayMqttSettingsService->addUpdateHandler([&](const String &originId)
                                                { registerConfig(); },
                                                false);

    // configure settings service update handler to update relay states
    addUpdateHandler([&](const String &originId)
                     { onConfigUpdated(); },
                     false);
}

void RelayStateService::begin()
{
    _httpEndpoint.begin();
    _eventEndpoint.begin();
    _webSocketServer.begin();

    // Initialize default states
    _state.lightRelay = DEFAULT_RELAY_STATE;
    _state.pumpRelay = DEFAULT_RELAY_STATE;
    _state.extraRelay = DEFAULT_RELAY_STATE;

    onConfigUpdated();
}

void RelayStateService::onConfigUpdated()
{
    // Note: Most relays are active LOW, so we invert the logic
    digitalWrite(RELAY_LIGHT, _state.lightRelay);
    digitalWrite(RELAY_PUMP, _state.pumpRelay);
    digitalWrite(RELAY_EXTRA, _state.extraRelay);
}

void RelayStateService::registerConfig()
{
    if (!_mqttClient->connected())
    {
        return;
    }

    // Register each relay as a separate entity in Home Assistant
    _relayMqttSettingsService->read([&](RelayMqttSettings &settings)
                                    {
        // Light Relay
        {
            String configTopic = settings.mqttPath + "/light/config";
            String subTopic = settings.mqttPath + "/light/set";
            String pubTopic = settings.mqttPath + "/light/state";
            
            JsonDocument doc;
            doc["~"] = settings.mqttPath + "/light";
            doc["name"] = settings.name + " Light";
            doc["unique_id"] = settings.uniqueId + "_light";
            doc["cmd_t"] = "~/set";
            doc["stat_t"] = "~/state";
            doc["schema"] = "json";
            
            String payload;
            serializeJson(doc, payload);
            _mqttClient->publish(configTopic.c_str(), 0, false, payload.c_str());
        }

        // Pump Relay
        {
            String configTopic = settings.mqttPath + "/pump/config";
            String subTopic = settings.mqttPath + "/pump/set";
            String pubTopic = settings.mqttPath + "/pump/state";
            
            JsonDocument doc;
            doc["~"] = settings.mqttPath + "/pump";
            doc["name"] = settings.name + " Pump";
            doc["unique_id"] = settings.uniqueId + "_pump";
            doc["cmd_t"] = "~/set";
            doc["stat_t"] = "~/state";
            doc["schema"] = "json";
            
            String payload;
            serializeJson(doc, payload);
            _mqttClient->publish(configTopic.c_str(), 0, false, payload.c_str());
        }

        // Extra Relay
        {
            String configTopic = settings.mqttPath + "/extra/config";
            String subTopic = settings.mqttPath + "/extra/set";
            String pubTopic = settings.mqttPath + "/extra/state";
            
            JsonDocument doc;
            doc["~"] = settings.mqttPath + "/extra";
            doc["name"] = settings.name + " Extra";
            doc["unique_id"] = settings.uniqueId + "_extra";
            doc["cmd_t"] = "~/set";
            doc["stat_t"] = "~/state";
            doc["schema"] = "json";
            
            String payload;
            serializeJson(doc, payload);
            _mqttClient->publish(configTopic.c_str(), 0, false, payload.c_str());
        } });
}
