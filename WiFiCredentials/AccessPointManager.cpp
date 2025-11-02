#include "AccessPointManager.h"

AccessPointManager::AccessPointManager(const char* ssid, const char* password, uint32_t timeout)
    : ssid(ssid)
    , password(password)
    , timeout(timeout)
      , server(80)
{
}

bool AccessPointManager::start() {
    WiFi.mode(WIFI_AP);
    if (!WiFi.softAP(ssid, password)) {
        return false;
    }

    ip = WiFi.softAPIP();

    setupWebServer();
    startTime = millis();
    started = true;
    return true;
}

void AccessPointManager::stop() {
    if (started) {
        server.end();
        WiFi.softAPdisconnect(true);
        started = false;
    }
}

void AccessPointManager::setupWebServer() {
    server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
            this->handleRoot(request);
            });

    server.on("/save", HTTP_POST, [this](AsyncWebServerRequest *request) {
            this->handleSave(request);
            });

    server.begin();
}

void AccessPointManager::handleRoot(AsyncWebServerRequest *request) {
    String html = "<!DOCTYPE html><html><head><title>ESP8266 Config</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>body{font-family:Arial;margin:40px;background:#f0f0f0}";
    html += ".container{background:white;padding:30px;border-radius:10px;max-width:400px;margin:0 auto;box-shadow:0 2px 10px rgba(0,0,0,0.1)}";
    html += "h1{color:#333;margin-top:0}input{width:100%;padding:10px;margin:10px 0;box-sizing:border-box;border:1px solid #ddd;border-radius:5px}";
    html += "button{background:#4CAF50;color:white;padding:12px;border:none;border-radius:5px;cursor:pointer;width:100%;font-size:16px}";
    html += "button:hover{background:#45a049}</style></head><body>";
    html += "<div class='container'><h1>WiFi Configuration</h1>";
    html += "<form action='/save' method='POST'>";
    html += "<label>WiFi SSID:</label><input type='text' name='ssid' required><br>";
    html += "<label>Password:</label><input type='password' name='password' required><br>";
    html += "<button type='submit'>Save & Restart</button>";
    html += "</form></div></body></html>";

    request->send(200, "text/html", html);
}

void AccessPointManager::handleSave(AsyncWebServerRequest *request) {
    if (!request->hasParam("ssid", true) || !request->hasParam("password", true)) {
        request->send(400, "text/plain", "Missing parameters");
        return;
    }

    String ssid = request->getParam("ssid", true)->value();
    String password = request->getParam("password", true)->value();
    WiFiCredentials creds(ssid.c_str(), password.c_str());
    onCredentialsSavedCallback(creds);

    request->send(200, "text/html",
        "<html><body><h2>Credentials received!</h2><p>Device will try to connect...</p></body></html>");
}

