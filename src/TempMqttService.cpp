#include <TempMqttService.h>

TempMqttService::TempMqttService(PsychicHttpServer *server, ESP32SvelteKit *sveltekit) : _httpEndpoint(TempState::read,
                                                                                                       TempState::update,
                                                                                                       this,
                                                                                                       server,
                                                                                                       TEMP_MQTT_SERVICE_PATH,
                                                                                                       sveltekit->getSecurityManager(),
                                                                                                       AuthenticationPredicates::IS_AUTHENTICATED)
{
}

void TempMqttService::begin()
{
    _httpEndpoint.begin();
}
