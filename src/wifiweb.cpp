#include "main.h"
#include <WebServer.h>

char *ssid_ap = "ESP32_AP";
char *password_ap = "";
int port = 2025;
WebServer server(port);

void handleRoot();
void handleConnect();
void wifiServerSetup();
void wifiServerHandle();

void wifiServerHandle()
{
    // Handle client requests
    server.handleClient();
}

void wifiServerSetup()
{
    // Set ESP32 to AP mode
    WiFi.softAP(ssid_ap, password_ap);
    // Configure server handling functions
    server.on("/", HTTP_GET, handleRoot);
    server.on("/connect", HTTP_POST, handleConnect);
    // Start server
    server.begin();
    Serial.println("WiFi hotspot started");
}
void handleRoot()
{
    // Return a simple HTML form for entering WiFi information
    server.send(200, "text/html", "<h1>ESP32 WiFi Provisioning</h1><p>Please enter your WiFi name and password</p><form action='/connect' method='post'><label for='ssid'>WiFi Name:</label><input type='text' id='ssid' name='ssid'><br><label for='password'>WiFi Password:</label><input type='password' id='password' name='password'><br><input type='submit' value='Connect'></form>");
}
void handleConnect()
{
    // Handle connection request, attempt to connect to the specified WiFi network
    String ssid = server.arg("ssid");
    String password = server.arg("password");
    if (ssid != "" && password != "")
    {
        WiFi.begin(ssid.c_str(), password.c_str());
        int count = 0;
        while (WiFi.status() != WL_CONNECTED)
        {
            delay(500);
            Serial.print(".");
            if (++count > 40)
            {
                break;
            }
        }
        if (WiFi.status() != WL_CONNECTED)
        {
            Serial.println("\nWiFi connection failed");
            server.send(400, "text/html", "<h1>Connection failed</h1><p>Please check your WiFi information and try again.</p>");
        }
        else
        {
            Serial.println("\nWiFi connected successfully");
            server.send(200, "text/html", "<h1>Connection successful</h1><p>Please disconnect from the network.</p>");
            WiFi.setAutoReconnect(true);
            // Store WiFi info to EEP
            preferences.putString("ssid", ssid);
            preferences.putString("password", password);
        }
    }
    else
    {
        Serial.println("Incomplete WiFi information");
        server.send(400, "text/html", "<h1>Error</h1><p>Please enter WiFi name and password.</p>");
    }
}
