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
#include "pinout.h"

#define DEFAULT_RELAY_STATE false
#define OFF_STATE "OFF"
#define ON_STATE "ON"

#define RELAY_SETTINGS_ENDPOINT_PATH "/rest/relayState"
#define RELAY_SETTINGS_SOCKET_PATH "/ws/relayState"
#define RELAY_SETTINGS_EVENT "relay"

class RelayState
{
public:
    bool lightRelay;
    bool pumpRelay;
    bool extraRelay;

    static void read(RelayState &settings, JsonObject &root)
    {
        root["light_relay"] = settings.lightRelay;
        root["pump_relay"] = settings.pumpRelay;
        root["extra_relay"] = settings.extraRelay;
    }

    static StateUpdateResult update(JsonObject &root, RelayState &relayState)
    {
        bool changed = false;

        if (root.containsKey("light_relay"))
        {
            bool newState = root["light_relay"] | DEFAULT_RELAY_STATE;
            if (relayState.lightRelay != newState)
            {
                relayState.lightRelay = newState;
                changed = true;
            }
        }

        if (root.containsKey("pump_relay"))
        {
            bool newState = root["pump_relay"] | DEFAULT_RELAY_STATE;
            if (relayState.pumpRelay != newState)
            {
                relayState.pumpRelay = newState;
                changed = true;
            }
        }

        if (root.containsKey("extra_relay"))
        {
            bool newState = root["extra_relay"] | DEFAULT_RELAY_STATE;
            if (relayState.extraRelay != newState)
            {
                relayState.extraRelay = newState;
                changed = true;
            }
        }

        return changed ? StateUpdateResult::CHANGED : StateUpdateResult::UNCHANGED;
    }

    static void homeAssistRead(RelayState &settings, JsonObject &root)
    {
        root["light_state"] = settings.lightRelay ? ON_STATE : OFF_STATE;
        root["pump_state"] = settings.pumpRelay ? ON_STATE : OFF_STATE;
        root["extra_state"] = settings.extraRelay ? ON_STATE : OFF_STATE;
    }

    static StateUpdateResult homeAssistUpdate(JsonObject &root, RelayState &relayState)
    {
        bool changed = false;

        if (root.containsKey("light_state"))
        {
            String state = root["light_state"].as<String>();
            bool newState = state.equals(ON_STATE);
            if (relayState.lightRelay != newState)
            {
                relayState.lightRelay = newState;
                changed = true;
            }
        }

        if (root.containsKey("pump_state"))
        {
            String state = root["pump_state"].as<String>();
            bool newState = state.equals(ON_STATE);
            if (relayState.pumpRelay != newState)
            {
                relayState.pumpRelay = newState;
                changed = true;
            }
        }

        if (root.containsKey("extra_state"))
        {
            String state = root["extra_state"].as<String>();
            bool newState = state.equals(ON_STATE);
            if (relayState.extraRelay != newState)
            {
                relayState.extraRelay = newState;
                changed = true;
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
