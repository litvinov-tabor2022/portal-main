#include "WebServer.h"

WebServer::WebServer(PortalFramework *framework) {
    webServer = new AsyncWebServer(HTTP_PORT);
    this->framework = framework;

    webServer->on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
        Debug.println("Handling GET /status");
        request->send(200, "text/plain", R"({"status":"ok"})");
    });

    webServer->on("/data", HTTP_GET, [this](AsyncWebServerRequest *request) {
        Debug.println("Handling GET /data");
        serveCommitLogFile(request);
    });

    webServer->on("/data", HTTP_DELETE, [this](AsyncWebServerRequest *request) {
        Debug.println("Handling DELETE /data");
        if (this->framework->storage.deleteCommitLogFile()) {
            request->send(200, "text/plain", R"({"status":"deleted"})");
        } else {
            request->send(500, "text/plain", R"({"status":"Can't delete the commit log"})");
        }
    });

    webServer->on("/time", HTTP_PUT, [this](AsyncWebServerRequest *request) {
        Debug.println("Handling PUT /time");
        this->framework->clocks.setCurrentTime(request->arg("secs").toInt());
        request->send(200, "text/plain", R"({"status":"updated"})");
    });
}

void WebServer::serveCommitLogFile(AsyncWebServerRequest *request) {
    this->framework->storage.withCommitLogFile([request](File file) mutable {
        Debug.printf("Serving commit log file, size %d bytes\n", file.size());

        AsyncWebServerResponse *response = request->beginResponse(
                CONTENT_TYPE_JSONL,
                file.size(),
                [file](uint8_t *buffer, size_t maxLen, size_t total) mutable -> size_t {
                    int bytes = file.read(buffer, maxLen);

                    // close file at the end
                    if (bytes + total == file.size()) file.close();

                    return max(0, bytes); // return 0 even when no bytes were loaded
                }
        );

        response->addHeader(F("Access-Control-Allow-Origin"), F("*"));
        request->send(response);
    });
}

void WebServer::start() {
    webServer->begin();
}

void WebServer::stop() {
    webServer->end();
}
