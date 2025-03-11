#ifndef TempStateService_h
#define TempStateService_h

#include <EventSocket.h>
#include <HttpEndpoint.h>
#include <MqttEndpoint.h>
#include <EventEndpoint.h>
#include <WebSocketServer.h>
#include <ESP32SvelteKit.h>

#include <OneWire.h>
#include <DallasTemperature.h>
#include <FSPersistence.h>

#include "TempState.h"
#include "pinout.h"

#define TEMP_HISTORY_FILE "/config/tempHistory.json"
#define TEMP_STATE_ENDPOINT_PATH "/rest/tempState"
#define TEMP_STATE_SOCKET_PATH "/ws/tempState"
#define TEMP_STATE_EVENT "temp"
#define TEMP_READ_INTERVAL 60000 // Read temperature every 60 seconds

class TempStateService : public StatefulService<TempState>
{
public:
    TempStateService(PsychicHttpServer *server,
                     ESP32SvelteKit *sveltekit);

    void begin();
    void loop();

private:
    HttpEndpoint<TempState> _httpEndpoint;
    EventEndpoint<TempState> _eventEndpoint;
    WebSocketServer<TempState> _webSocketServer;
    FSPersistence<TempState> _fsPersistence;

    OneWire _oneWire;
    DallasTemperature _sensors;
    unsigned long _lastRead = 0;
    unsigned long _lastSave = 0;

    void readTemperature();
    static const unsigned long SAVE_INTERVAL = 300000; // Save every 5 minutes
};

#endif
