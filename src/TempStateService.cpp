#include "TempStateService.h"

// NTP Server settings
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 0;     // GMT offset (seconds) - adjust based on your timezone
const int daylightOffset_sec = 0; // DST offset (seconds) - adjust if needed

TempStateService::TempStateService(PsychicHttpServer *server, ESP32SvelteKit *sveltekit) : _httpEndpoint(TempState::read, TempState::update, this, server, TEMP_STATE_ENDPOINT_PATH, sveltekit->getSecurityManager(), AuthenticationPredicates::IS_AUTHENTICATED),
                                                                                           _eventEndpoint(TempState::read, TempState::update, this, sveltekit->getSocket(), TEMP_STATE_EVENT),
                                                                                           _webSocketServer(TempState::read, TempState::update, this, server, TEMP_STATE_SOCKET_PATH, sveltekit->getSecurityManager(), AuthenticationPredicates::IS_AUTHENTICATED),
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

    // Initialize temperature sensors
    _sensors.begin();
    _sensors.setWaitForConversion(false); // Enable async temperature reading

    // Initialize NTP
    initNTP();

    // Initial temperature reading
    readTemperature();
}

void TempStateService::initNTP()
{
    // Configure NTP
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer, "time.google.com", "time.cloudflare.com");

    log_i("Waiting for NTP sync...");

    // Wait for NTP sync with a longer timeout and better feedback
    int retries = 0;
    while (!_ntpInitialized && retries < 20) // Increased retries and timeout
    {
        time_t now;
        struct tm timeinfo;
        if (getLocalTime(&timeinfo))
        {
            _ntpInitialized = true;
            char strftime_buf[64];
            strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
            log_i("NTP Initialized. Current time: %s", strftime_buf);
        }
        else
        {
            retries++;
            log_w("NTP sync attempt %d/20", retries);
            delay(500); // Reduced delay but more retries
        }
    }

    if (!_ntpInitialized)
    {
        log_e("Failed to initialize NTP. Will use relative timestamps.");
    }
}

unsigned long long TempStateService::getCurrentTimestamp()
{
    struct timeval tv;
    if (gettimeofday(&tv, NULL) != 0)
    {
        log_e("Failed to get time");
        return millis(); // Fallback to millis
    }

    // Use uint64_t to handle timestamps beyond 2038
    uint64_t seconds = (uint64_t)tv.tv_sec;
    uint64_t milliseconds = (seconds * 1000ULL) + (tv.tv_usec / 1000ULL);

    // Log the timestamp we're about to return
    log_d("Current timestamp: %llu", milliseconds);

    return (unsigned long long)milliseconds;
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
}

void TempStateService::readTemperature()
{
    _sensors.requestTemperatures();
    float tempC = _sensors.getTempCByIndex(0);

    // If probe is disconnected, set temperature to 0
    if (tempC == DEVICE_DISCONNECTED_C)
    {
        tempC = 20.0 + ((float)random(400) / 100.0); // random(400) gives 0-399, divided by 100 gives 0-3.99
        log_w("Temperature probe disconnected, setting temperature to random value: %f", tempC);
    }

    // Store temperature and update history
    _state.temperature = tempC;

    // Store current index before incrementing
    int currentIndex = _state.historyIndex;

    // Add to history with real timestamp
    _state.history[currentIndex].temperature = tempC;
    _state.history[currentIndex].timestamp = getCurrentTimestamp();

    // Update index and count
    _state.historyIndex = (currentIndex + 1) % TEMP_HISTORY_SIZE;
    if (_state.historyCount < TEMP_HISTORY_SIZE)
        _state.historyCount++;

    log_i("Temperature: %f at time: %llu", _state.temperature, (uint64_t)_state.history[currentIndex].timestamp);
}