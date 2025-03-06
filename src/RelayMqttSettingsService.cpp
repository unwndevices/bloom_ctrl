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

RelayMqttSettingsService::RelayMqttSettingsService(PsychicHttpServer *server,
                                                   ESP32SvelteKit *sveltekit) : _httpEndpoint(RelayMqttSettings::read,
                                                                                              RelayMqttSettings::update,
                                                                                              this,
                                                                                              server,
                                                                                              RELAY_BROKER_SETTINGS_PATH,
                                                                                              sveltekit->getSecurityManager(),
                                                                                              AuthenticationPredicates::IS_AUTHENTICATED),
                                                                                _fsPersistence(RelayMqttSettings::read,
                                                                                               RelayMqttSettings::update,
                                                                                               this,
                                                                                               sveltekit->getFS(),
                                                                                               RELAY_BROKER_SETTINGS_FILE)
{
}

void RelayMqttSettingsService::begin()
{
    _httpEndpoint.begin();
    _fsPersistence.readFromFS();
}
