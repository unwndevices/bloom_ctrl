#include "TempStateService.h"

TempStateService::TempStateService(PsychicHttpServer *server, ESP32SvelteKit *sveltekit) : _httpEndpoint(TempState::read, TempState::update, this, server, TEMP_STATE_ENDPOINT_PATH, sveltekit->getSecurityManager(), AuthenticationPredicates::IS_AUTHENTICATED),
                                                                                           _eventEndpoint(TempState::read, TempState::update, this, sveltekit->getSocket(), TEMP_STATE_EVENT),
                                                                                           _webSocketServer(TempState::read, TempState::update, this, server, TEMP_STATE_SOCKET_PATH, sveltekit->getSecurityManager(), AuthenticationPredicates::IS_AUTHENTICATED),
                                                                                           _fsPersistence(TempState::read, TempState::update, this, sveltekit->getFS(), TEMP_HISTORY_FILE),
                                                                                           _oneWire(SENSOR_TEMP),
                                                                                           _sensors(&_oneWire)
{
}

void TempStateService::begin()
{
    // Initialize endpoints
    _httpEndpoint.begin();
    _eventEndpoint.begin();
    _webSocketServer.begin();

    // Load saved state
    _fsPersistence.readFromFS();

    // Initialize temperature sensors
    _sensors.begin();
    _sensors.setWaitForConversion(false); // Enable async temperature reading

    // Initial temperature reading
    readTemperature();
}

void TempStateService::loop()
{
    unsigned long currentMillis = millis();

    // Read temperature at regular intervals
    if (currentMillis - _lastRead >= TEMP_READ_INTERVAL)
    {
        readTemperature();
        _lastRead = currentMillis;
    }

    // Save to filesystem periodically
    if (currentMillis - _lastSave >= SAVE_INTERVAL)
    {
        _fsPersistence.writeToFS();
        _lastSave = currentMillis;
    }
}

void TempStateService::readTemperature()
{
    _sensors.requestTemperatures();
    float tempC = _sensors.getTempCByIndex(0);

    if (tempC != DEVICE_DISCONNECTED_C)
    {
        _state.temperature = tempC;

        // Add to history using correct member name
        _state.history[_state.historyIndex].temperature = tempC;
        _state.history[_state.historyIndex].timestamp = millis();

        _state.historyIndex = (_state.historyIndex + 1) % TEMP_HISTORY_SIZE;
        if (_state.historyCount < TEMP_HISTORY_SIZE)
            _state.historyCount++;

        log_i("Temperature: %f", _state.temperature);
    }
    else
    {
        log_e("Error reading temperature");
    }
}