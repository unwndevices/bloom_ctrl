#ifndef TempMqttService_h
#define TempMqttService_h

#include <HttpEndpoint.h>
#include <MqttEndpoint.h>
#include <ESP32SvelteKit.h>
#include "TempState.h"

// in this case the temp mqtt service is used to publish the temperature, no option to set it
#define TEMP_MQTT_SERVICE_PATH "/rest/tempMqttService"

class TempMqttService : public StatefulService<TempState>
{
public:
    TempMqttService(PsychicHttpServer *server, ESP32SvelteKit *sveltekit);
    void begin();

private:
    HttpEndpoint<TempState> _httpEndpoint;
};

#endif