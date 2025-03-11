# Developing with the Framework

The back end is a set of REST endpoints hosted by a PsychicHttp instance. The `lib/framework` directory contains the majority of the back end code. The framework contains a number of useful utility classes which you can use when extending it. The project also comes with a demo project to give you some help getting started.

The framework's source is split up by feature, for example `WiFiScanner.h` implements the end points for scanning for available networks where as `WiFiSettingsService.h` handles configuring the WiFi settings and managing the WiFi connection.

## Initializing the Framework

The `src/main.cpp` file constructs the web server and initializes the framework. You can add endpoints to the server here to support your IoT project. The main loop is also accessible so you can run your own code easily.

The following code creates the web server and esp32sveltekit framework:

```cpp
PsychicHttpServer server;
ESP32SvelteKit esp32sveltekit(&server, 120);
```

ESP32SvelteKit is instantiated with a reference to the server and a number of HTTP endpoints. The underlying ESP-IDF HTTP Server statically allocates memory for each endpoint and needs to know how many there are. Best is to inspect your `WWWData.h` file for the number of Endpoints from SvelteKit (currently 60), the framework itself has 37 endpoints, and Lighstate Demo has 7 endpoints. Each `_server.on()` counts as an endpoint. Don't forget to add a couple of spare, just in case. Each HttpEndpoint adds 2 endpoints, if CORS is enabled it adds another endpoint for the CORS preflight request.

Now in the setup() function the initialization is performed:

```cpp
void setup() {
  // start serial and filesystem
  Serial.begin(SERIAL_BAUD_RATE);

  // start the framework and demo project
  esp32sveltekit.begin();
}
```

`server.begin()` is called by ESP32-SvelteKit, as the start-up sequence is crucial.

## Stateful Service

The framework promotes a modular design and exposes features you may re-use to speed up the development of your project. Where possible it is recommended that you use the features the frameworks supplies.

The `StatefulService.h` class is responsible for managing state. It has an API which allows other code to update or respond to updates in the state it manages. You can define a data class to hold state, then build a StatefulService class to manage it. After that you may attach HTTP endpoints, WebSockets or MQTT topics to the StatefulService instance to provide commonly required features.

Here is a simple example of a state class and a StatefulService to manage it:

```cpp
class LightState {
 public:
  bool on = false;
  uint8_t brightness = 255;
};

class LightStateService : public StatefulService<LightState> {
};
```

### Update Handler

You may listen for changes to state by registering an update handler callback:

```cpp
// register an update handler
update_handler_id_t myUpdateHandler = lightStateService.addUpdateHandler(
  [&](const String& originId) {
    Serial.print("The light's state has been updated by: ");
    Serial.println(originId);
  }
);

// remove the update handler
lightStateService.removeUpdateHandler(myUpdateHandler);
```

### Hook Handler

Sometimes if can be desired to hook into every update of a state, even if the StateUpdateResult is `StateUpdateResult::UNCHANGED`:

```cpp
// register a hook handler
hook_handler_id_t myHookHandler = lightStateService.addHookHandler(
  [&](const String& originId, StateUpdateResult &result) {
    Serial.printf("The light's state has been updated by: %s with result %d\n", originId, result);
  }
);

// remove the hook handler
lightStateService.removeHookHandler(myHookHandler);
```

### Read & Update State

StatefulService exposes a read function for safe state access:

```cpp
lightStateService.read([&](LightState& state) {
  digitalWrite(LED_PIN, state.on ? HIGH : LOW); // apply the state update to the LED_PIN
});
```

And an update function for state modifications:

```cpp
lightStateService.update([&](LightState& state) {
   if (state.on) {
    return StateUpdateResult::UNCHANGED; // lights were already on
  }
  state.on = true;  // turn on the lights
  return StateUpdateResult::CHANGED; // notify StatefulService
}, "timer");
```

### JSON Serialization

For external interfaces (HTTP, WebSockets, MQTT), state must be JSON-serializable:

```cpp
class LightState {
 public:
  bool on = false;
  uint8_t brightness = 255;

  static void read(LightState& state, JsonObject& root) {
    root["on"] = state.on;
    root["brightness"] = state.brightness;
  }

  static StateUpdateResult update(JsonObject& root, LightState& state) {
    state.on = root["on"] | false;
    state.brightness = root["brightness"] | 255;
    return StateUpdateResult::CHANGED;
  }
};
```

Usage examples:

```cpp
// Read state to JSON
JsonObject jsonObject = jsonDocument.to<JsonObject>();
lightStateService->read(jsonObject, LightState::read);

// Update state from JSON
JsonObject jsonObject = jsonDocument.as<JsonObject>();
lightStateService->update(jsonObject, LightState::update, "timer");
```

## Communication Interfaces

### HTTP RESTful Endpoint

```cpp
class LightStateService : public StatefulService<LightState> {
 public:
  LightStateService(PsychicHttpServer* server, ESP32SvelteKit *sveltekit) :
      _httpEndpoint(LightState::read,
                   LightState::update,
                   this,
                   server,
                   "/rest/lightState",
                   sveltekit->getSecurityManager(),
                   AuthenticationPredicates::IS_AUTHENTICATED) {
  }

  void begin() {
    _httpEndpoint.begin();
  }

 private:
  HttpEndpoint<LightState> _httpEndpoint;
};
```

### File System Persistence

```cpp
class LightStateService : public StatefulService<LightState> {
 public:
  LightStateService(ESP32SvelteKit *sveltekit) :
      _fsPersistence(LightState::read,
                    LightState::update,
                    this,
                    sveltekit->getFS(),
                    "/config/lightState.json") {
  }

 private:
  FSPersistence<LightState> _fsPersistence;
};
```

### Event Socket Endpoint

```cpp
class LightStateService : public StatefulService<LightState> {
 public:
  LightStateService(ESP32SvelteKit *sveltekit) :
      _eventEndpoint(LightState::read,
                    LightState::update,
                    this,
                    sveltekit->getSocket(),
                    "led") {}

  void begin() {
    _eventEndpoint.begin();
  }

 private:
  EventEndpoint<LightState> _eventEndpoint;
};
```

### WebSocket Server

```cpp
class LightStateService : public StatefulService<LightState> {
 public:
  LightStateService(PsychicHttpServer* server, ESP32SvelteKit *sveltekit) :
      _webSocket(LightState::read,
                LightState::update,
                this,
                server,
                "/ws/lightState",
                sveltekit->getSecurityManager(),
                AuthenticationPredicates::IS_AUTHENTICATED) {
  }

  void begin() {
    _webSocketServer.begin();
  }

 private:
  WebSocketServer<LightState> _webSocketServer;
};
```

### MQTT Client

```cpp
class LightStateService : public StatefulService<LightState> {
 public:
  LightStateService(ESP32SvelteKit *sveltekit) :
      _mqttEndpoint(LightState::read,
                  LightState::update,
                  this,
                  sveltekit->getMqttClient(),
                  "homeassistant/light/my_light/set",
                  "homeassistant/light/my_light/state") {
  }

 private:
  MqttEndpoint<LightState> _mqttEndpoint;
};
```

Configure MQTT topics at runtime:

```cpp
_mqttEndpoint.configureBroker("homeassistant/light/desk_lamp/set",
                             "homeassistant/light/desk_lamp/state");
```

## Event Socket System

The Event Socket System provides a WebSocket-based communication path between client and ESP32. Clients subscribe to topics they're interested in.

### Message Format

```json
{
  "event": "led",
  "data": {
    "led_on": true
  }
}
```

Subscribe message format:

```json
{
  "event": "subscribe",
  "data": "analytics"
}
```

### Event Management

Register and emit events:

```cpp
// Register event
_socket.registerEvent("CustomEvent");

// Emit event
void emitEvent(String event, JsonObject &jsonObject,
               const char *originId = "", bool onlyToSameOrigin = false);

// Receive events
_socket.onEvent("CustomEvent",[&](JsonObject &root, int originId) {
  bool ledState = root["led_on"];
});

// Subscribe notifications
_socket.onSubscribe("CustomEvent",[&](const String &originId) {
  Serial.println("New Client subscribed: " + originId);
});
```

### Push Notifications

```cpp
esp32sveltekit.getNotificationService()->pushNotification("Message", PUSHINFO);
```

## Security Features

The framework uses JWT tokens for authentication. Available predicates:

| Predicate          | Description                           |
| ------------------ | ------------------------------------- |
| `NONE_REQUIRED`    | No authentication required            |
| `IS_AUTHENTICATED` | Any authenticated principal permitted |
| `IS_ADMIN`         | Admin authentication required         |

Example usage:

```cpp
server->on("/rest/someService", HTTP_GET,
  _securityManager->wrapRequest(
    std::bind(&SomeService::someService, this, std::placeholders::_1),
    AuthenticationPredicates::IS_AUTHENTICATED
  )
);
```

## Additional Features

### MDNS Service

```cpp
esp32sveltekit.setMDNSAppName("ESP32 SvelteKit Demo App");
```

### Loop Function

```cpp
esp32sveltekit.addLoopFunction(callback);
```

### Factory Reset & Recovery

```cpp
// Factory reset
esp32sveltekit.factoryReset();

// Recovery mode
esp32sveltekit.recoveryMode();
```

### Deep Sleep

Configure in `factory_settings.ini`:

```ini
; Deep Sleep Configuration
-D WAKEUP_PIN_NUMBER=38 ; pin number to wake up the ESP
-D WAKEUP_SIGNAL=0 ; 1 for wakeup on HIGH, 0 for wakeup on LOW
```

Runtime configuration:

```cpp
esp32sveltekit.getSleepService()->setWakeUpPin(int pin, bool level,
    pinTermination termination = pinTermination::FLOATING);

esp32sveltekit.getSleepService()->attachOnSleepCallback();
esp32sveltekit.getSleepService()->sleepNow();
```

### Battery Service

```cpp
// Update battery status
esp32sveltekit.getBatteryService()->updateSOC(float stateOfCharge);
esp32sveltekit.getBatteryService()->setCharging(boolean isCharging);
```

### Custom Features

```cpp
esp32sveltekit.getFeatureService()->addFeature("custom_feature", true);
```

## OTA Updates

Two update methods available:

1. Browser Upload:

   - Enable with `FT_UPLOAD_FIRMWARE=1` in features.ini
   - Uses browser file upload interface

2. Update Server Download:
   - Enable with `FT_DOWNLOAD_FIRMWARE=1` in features.ini
   - Requires SSL and FT_NTP=1
   - Supports GitHub Releases or custom HTTPS server

Configure version in platformio.ini:

```ini
-D BUILD_TARGET="$PIOENV"
-D APP_NAME=\"ESP32-Sveltekit\"
-D APP_VERSION=\"0.3.0\"
```

> **Note**: OTA updates might be unstable on single-core ESP32 variants.
