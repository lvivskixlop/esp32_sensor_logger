#define EEPROM_SIZE 1024

void saveSettingsToEEPROM()
{
    DynamicJsonDocument doc(1024);
    doc["voltageDividerRatio"] = VOLTAGE_DIVIDER_RATIO;
    doc["voltageCorrection"] = VOLTAGE_CORRECTION;
    doc["batteryMinimalVoltage"] = BATTERY_MINIMAL_VOLTAGE;
    doc["sensorReadAndSendInterval"] = SENSOR_READ_AND_SEND_INTERVAL;
    doc["requestTimeout"] = REQUEST_TIMEOUT;
    doc["reconnectInterval"] = RECONNECT_INTERVAL;
    doc["ssid"] = SSID;
    doc["wifiPassword"] = WIFI_PASSWORD;
    doc["googleAppsScriptUrl"] = GOOGLE_APPS_SCRIPT_URL;
    doc["timeApiUrl"] = TIME_API_URL;
    doc["webhookSite"] = WEBHOOK_SITE;
    doc["localIp"] = LOCAL_IP.toString();
    doc["gateway"] = GATEWAY.toString();
    doc["subnet"] = SUBNET.toString();
    doc["primaryDns"] = PRIMARY_DNS.toString();
    doc["secondaryDns"] = SECONDARY_DNS.toString();

    String jsonString;
    serializeJson(doc, jsonString);

    for (int i = 0; i < jsonString.length(); i++)
    {
        EEPROM.write(i, jsonString[i]);
    }
    EEPROM.write(jsonString.length(), '\0'); // Null-terminate the string
    EEPROM.commit();
}

void loadSettingsFromEEPROM()
{
    String jsonString;
    for (int i = 0; i < EEPROM_SIZE; i++)
    {
        char c = EEPROM.read(i);
        if (c == '\0')
            break;
        jsonString += c;
    }

    if (jsonString.length() <= 0)
        return;

    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, jsonString);

    if (error)
        return;

    if (doc.containsKey("voltageDividerRatio"))
        VOLTAGE_DIVIDER_RATIO = doc["voltageDividerRatio"].as<float>();
    if (doc.containsKey("voltageCorrection"))
        VOLTAGE_CORRECTION = doc["voltageCorrection"].as<float>();
    if (doc.containsKey("batteryMinimalVoltage"))
        BATTERY_MINIMAL_VOLTAGE = doc["batteryMinimalVoltage"].as<float>();
    if (doc.containsKey("sensorReadAndSendInterval"))
        SENSOR_READ_AND_SEND_INTERVAL = doc["sensorReadAndSendInterval"].as<int>();
    if (doc.containsKey("requestTimeout"))
        REQUEST_TIMEOUT = doc["requestTimeout"].as<int>();
    if (doc.containsKey("reconnectInterval"))
        RECONNECT_INTERVAL = doc["reconnectInterval"].as<int>();
    if (doc.containsKey("ssid"))
        SSID = strdup(doc["ssid"].as<const char *>());
    if (doc.containsKey("wifiPassword"))
        WIFI_PASSWORD = strdup(doc["wifiPassword"].as<const char *>());
    if (doc.containsKey("googleAppsScriptUrl"))
        GOOGLE_APPS_SCRIPT_URL = strdup(doc["googleAppsScriptUrl"].as<const char *>());
    if (doc.containsKey("timeApiUrl"))
        TIME_API_URL = strdup(doc["timeApiUrl"].as<const char *>());
    if (doc.containsKey("webhookSite"))
        WEBHOOK_SITE = strdup(doc["webhookSite"].as<const char *>());
    if (doc.containsKey("localIp"))
        LOCAL_IP.fromString(doc["localIp"].as<const char *>());
    if (doc.containsKey("gateway"))
        GATEWAY.fromString(doc["gateway"].as<const char *>());
    if (doc.containsKey("subnet"))
        SUBNET.fromString(doc["subnet"].as<const char *>());
    if (doc.containsKey("primaryDns"))
        PRIMARY_DNS.fromString(doc["primaryDns"].as<const char *>());
    if (doc.containsKey("secondaryDns"))
        SECONDARY_DNS.fromString(doc["secondaryDns"].as<const char *>());
}