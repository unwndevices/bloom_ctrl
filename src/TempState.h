#ifndef TempState_h
#define TempState_h

#include <ArduinoJson.h>

#define TEMP_HISTORY_SIZE 100 // Store last 100 readings

struct TempReading
{
    float temperature;
    unsigned long timestamp;
};

class TempState
{
public:
    float temperature;
    TempReading history[TEMP_HISTORY_SIZE];
    int historyIndex = 0;
    int historyCount = 0;

    static void read(TempState &settings, JsonObject &root)
    {
        root["temperature"] = settings.temperature;
        root["history_index"] = settings.historyIndex;
        root["history_count"] = settings.historyCount;

        // Create history array using new ArduinoJson syntax
        JsonArray historyArray = root["history"].to<JsonArray>();
        for (int i = 0; i < settings.historyCount; i++)
        {
            JsonObject reading = historyArray.add<JsonObject>();
            reading["temperature"] = settings.history[i].temperature;
            reading["timestamp"] = settings.history[i].timestamp;
        }
    }

    static StateUpdateResult update(JsonObject &root, TempState &tempState)
    {
        bool changed = false;

        if (root["history_index"].is<int>())
        {
            tempState.historyIndex = root["history_index"].as<int>();
            changed = true;
        }
        if (root["history_count"].is<int>())
        {
            tempState.historyCount = root["history_count"].as<int>();
            changed = true;
        }
        if (root["history"].is<JsonArray>())
        {
            JsonArray historyArray = root["history"];
            int i = 0;
            for (JsonObject reading : historyArray)
            {
                if (i >= TEMP_HISTORY_SIZE)
                    break;
                tempState.history[i].temperature = reading["temperature"].as<float>();
                tempState.history[i].timestamp = reading["timestamp"].as<unsigned long>();
                i++;
            }
            changed = true;
        }
        return changed ? StateUpdateResult::CHANGED : StateUpdateResult::UNCHANGED;
    }

    static void homeAssistRead(TempState &settings, JsonObject &root)
    {
        root["temperature"] = settings.temperature;
    }
};

#endif