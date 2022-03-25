#ifndef PORTAL_WEBSERVER_H
#define PORTAL_WEBSERVER_H

#include <vector>
#include "Constants.h" // this is universal over ESP32/ESP8266
#include "ESPAsyncWebServer.h" // this is universal over ESP32/ESP8266
#include "PortalFramework.h"

class WebServer {
public:
    explicit WebServer(PortalFramework *framework);

    void start();
    void stop();

private:
    AsyncWebServer *webServer;
    PortalFramework *framework;

    void serveCommitLogFile(AsyncWebServerRequest *request);
};

#endif //PORTAL_WEBSERVER_H
