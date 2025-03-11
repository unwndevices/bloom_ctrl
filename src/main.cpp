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

#include <ESP32SvelteKit.h>
#include <RelayMqttSettingsService.h>
#include <RelayStateService.h>
#include <TempMqttService.h>
#include <TempStateService.h>
#include <PsychicHttpServer.h>
#define SERIAL_BAUD_RATE 115200

PsychicHttpServer server;

ESP32SvelteKit esp32sveltekit(&server, 120);

RelayMqttSettingsService relayMqttSettingsService = RelayMqttSettingsService(&server,
                                                                             &esp32sveltekit);

RelayStateService relayStateService = RelayStateService(&server,
                                                        &esp32sveltekit,
                                                        &relayMqttSettingsService);

TempMqttService tempMqttService = TempMqttService(&server,
                                                  &esp32sveltekit);

TempStateService tempStateService = TempStateService(&server,
                                                     &esp32sveltekit);
void setup()
{
    // start serial and filesystem
    Serial.begin(SERIAL_BAUD_RATE);
    Serial.setDebugOutput(true);

    // start ESP32-SvelteKit
    esp32sveltekit.begin();

    // load the initial light settings
    relayStateService.begin();
    // start the light service
    relayMqttSettingsService.begin();

    tempStateService.begin();
    tempMqttService.begin();
}

void loop()
{
    tempStateService.loop();
    vTaskDelay(100);
}
