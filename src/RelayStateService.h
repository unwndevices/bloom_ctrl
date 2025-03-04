#ifndef RelayStateService_h
#define RelayStateService_h

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

#include <RelayMqttSettingsService.h>

#include <EventSocket.h>
#include <HttpEndpoint.h>
#include <MqttEndpoint.h>
#include <EventEndpoint.h>
#include <WebSocketServer.h>
#include <ESP32SvelteKit.h>

#define DEFAULT_RELAY_STATE false
#define OFF_STATE "OFF"
#define ON_STATE "ON"

#define RELAY_SETTINGS_ENDPOINT_PATH "/rest/relayState"
#define RELAY_SETTINGS_SOCKET_PATH "/ws/relayState"
#define RELAY_SETTINGS_EVENT "relay"

class RelayState
{
public:
    struct RelayInfo
    {
        bool state;
        String name;
        uint8_t pin;
        String type;
    };

    std::vector<RelayInfo> relays;

    static void read(RelayState &settings, JsonObject &root)
    {
        JsonArray relayArray = root.createNestedArray("relays");
        for (const auto &relay : settings.relays)
        {
            JsonObject relayObj = relayArray.createNestedObject();
            relayObj["state"] = relay.state;
            relayObj["name"] = relay.name;
            relayObj["pin"] = relay.pin;
            relayObj["type"] = relay.type;
        }
    }

    static StateUpdateResult update(JsonObject &root, RelayState &relayState)
    {
        if (!root.containsKey("relays"))
        {
            return StateUpdateResult::ERROR;
        }

        bool changed = false;
        JsonArray relayArray = root["relays"];

        // Update existing relays
        for (JsonObject relayObj : relayArray)
        {
            uint8_t pin = relayObj["pin"];
            bool newState = relayObj["state"] | false;

            // Find and update the matching relay
            for (auto &relay : relayState.relays)
            {
                if (relay.pin == pin && relay.state != newState)
                {
                    relay.state = newState;
                    changed = true;
                }
            }
        }

        return changed ? StateUpdateResult::CHANGED : StateUpdateResult::UNCHANGED;
    }

    static void homeAssistRead(RelayState &settings, JsonObject &root)
    {
        JsonArray relayArray = root.createNestedArray("relays");
        for (const auto &relay : settings.relays)
        {
            JsonObject relayObj = relayArray.createNestedObject();
            relayObj["state"] = relay.state ? ON_STATE : OFF_STATE;
            relayObj["name"] = relay.name;
            relayObj["type"] = relay.type;
        }
    }

    static StateUpdateResult homeAssistUpdate(JsonObject &root, RelayState &relayState)
    {
        if (!root.containsKey("relays"))
        {
            return StateUpdateResult::ERROR;
        }

        bool changed = false;
        JsonArray relayArray = root["relays"];

        for (JsonObject relayObj : relayArray)
        {
            String name = relayObj["name"];
            String state = relayObj["state"];

            bool newState = state.equals(ON_STATE);
            if (!state.equals(ON_STATE) && !state.equals(OFF_STATE))
            {
                continue;
            }

            // Find and update the matching relay
            for (auto &relay : relayState.relays)
            {
                if (relay.name == name && relay.state != newState)
                {
                    relay.state = newState;
                    changed = true;
                }
            }
        }

        return changed ? StateUpdateResult::CHANGED : StateUpdateResult::UNCHANGED;
    }
};

class RelayStateService : public StatefulService<RelayState>
{
public:
    RelayStateService(PsychicHttpServer *server,
                      ESP32SvelteKit *sveltekit,
                      RelayMqttSettingsService *relayMqttSettingsService);

    void begin();

private:
    HttpEndpoint<RelayState> _httpEndpoint;
    EventEndpoint<RelayState> _eventEndpoint;
    MqttEndpoint<RelayState> _mqttEndpoint;
    WebSocketServer<RelayState> _webSocketServer;
    PsychicMqttClient *_mqttClient;
    RelayMqttSettingsService *_relayMqttSettingsService;

    void registerConfig();
    void onConfigUpdated();
};

#endif
