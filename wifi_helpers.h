// #include <Arduino.h>
// #include <WiFi.h>
// #include <default_configs.h>

// void connectToWifi()
// {
//     if (!WiFi.config(LOCAL_IP, GATEWAY, SUBNET, PRIMARY_DNS, SECONDARY_DNS))
//     {
//         Serial.println("STA Failed to configure");
//     }
//     Serial.print("Connecting to ");
//     Serial.println(SSID);
//     WiFi.begin(SSID, WIFI_PASSWORD);

//     while (WiFi.status() != WL_CONNECTED)
//     {
//         Serial.print(".");
//         delay(500);
//     }

//     Serial.print("Connected. IP: ");
//     Serial.println(WiFi.localIP());
// }

// void reconnectToWifi()
// {
//     while (WiFi.status() != WL_CONNECTED)
//     {
//         Serial.println("Reconnecting to WiFi...");
//         WiFi.disconnect();
//         WiFi.reconnect();
//         delay(RECONNECT_INTERVAL);
//     }
// }