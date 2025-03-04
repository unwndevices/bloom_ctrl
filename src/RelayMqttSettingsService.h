#ifndef RelayMqttSettingsService_h
#define RelayMqttSettingsService_h

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

#include <HttpEndpoint.h>
#include <FSPersistence.h>
#include <SettingValue.h>
#include <ESP32SvelteKit.h>

#define RELAY_BROKER_SETTINGS_FILE "/config/brokerSettings.json"
#define RELAY_BROKER_SETTINGS_PATH "/rest/brokerSettings"

class RelayMqttSettings
{
public:
    String mqttPath;
    String name;
    String uniqueId;

    static void read(RelayMqttSettings &settings, JsonObject &root)
    {
        root["mqtt_path"] = settings.mqttPath;
        root["name"] = settings.name;
        root["unique_id"] = settings.uniqueId;
    }

    static StateUpdateResult update(JsonObject &root, RelayMqttSettings &settings)
    {
        settings.mqttPath = root["mqtt_path"] | SettingValue::format("bloom/relay/#{unique_id}");
        settings.name = root["name"] | SettingValue::format("relay-#{unique_id}");
        settings.uniqueId = root["unique_id"] | SettingValue::format("relay-#{unique_id}");
        return StateUpdateResult::CHANGED;
    }
};

class RelayMqttSettingsService : public StatefulService<RelayMqttSettings>
{
public:
    RelayMqttSettingsService(PsychicHttpServer *server, ESP32SvelteKit *sveltekit);
    void begin();

private:
    HttpEndpoint<RelayMqttSettings> _httpEndpoint;
    FSPersistence<RelayMqttSettings> _fsPersistence;
};

#endif // end RelayMqttSettingsService_h
