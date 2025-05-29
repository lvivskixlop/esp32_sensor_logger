StaticJsonDocument<4096> jsonDocument;
char buffer[4096];

void addJsonObject(char *tag, float value, char *unit)
{
    JsonObject obj = jsonDocument.createNestedObject();
    obj["type"] = tag;
    obj["value"] = value;
    obj["unit"] = unit;
}

void createEnvJson(float temperature, float humidity, float batteryVoltage, float solarVoltage)
{
    jsonDocument.clear();
    addJsonObject("temperature", temperature, "°C");
    addJsonObject("humidity", humidity, "%");
    addJsonObject("batteryVoltage", batteryVoltage, "V");
    addJsonObject("solarVoltage", solarVoltage, "V");
    serializeJson(jsonDocument, buffer);
}

void setSettingsFromJson(DynamicJsonDocument &jsonDocument)
{
    if (jsonDocument.containsKey("batteryVoltageDividerRatio"))
        BATTERY_VOLTAGE_DIVIDER_RATIO = jsonDocument["batteryVoltageDividerRatio"].as<float>();
    if (jsonDocument.containsKey("batteryVoltageCorrection"))
        BATTERY_VOLTAGE_CORRECTION = jsonDocument["batteryVoltageCorrection"].as<float>();
    if (jsonDocument.containsKey("solarVoltageDividerRatio"))
        SOLAR_VOLTAGE_DIVIDER_RATIO = jsonDocument["solarVoltageDividerRatio"].as<float>();
    if (jsonDocument.containsKey("solarVoltageCorrection"))
        SOLAR_VOLTAGE_CORRECTION = jsonDocument["solarVoltageCorrection"].as<float>();
    if (jsonDocument.containsKey("batteryMinimalVoltage"))
        BATTERY_MINIMAL_VOLTAGE = jsonDocument["batteryMinimalVoltage"].as<float>();
    if (jsonDocument.containsKey("sensorReadAndSendInterval"))
        SENSOR_READ_AND_SEND_INTERVAL = jsonDocument["sensorReadAndSendInterval"].as<int>();
    if (jsonDocument.containsKey("requestTimeout"))
        REQUEST_TIMEOUT = jsonDocument["requestTimeout"].as<int>();
    if (jsonDocument.containsKey("reconnectInterval"))
        RECONNECT_INTERVAL = jsonDocument["reconnectInterval"].as<int>();
    if (jsonDocument.containsKey("lowBatteryWorkTime"))
        LOW_BATTERY_WORK_TIME = jsonDocument["lowBatteryWorkTime"].as<int>();
    if (jsonDocument.containsKey("ssid"))
        SSID = strdup(jsonDocument["ssid"].as<const char *>());
    if (jsonDocument.containsKey("wifiPassword"))
        WIFI_PASSWORD = strdup(jsonDocument["wifiPassword"].as<const char *>());
    if (jsonDocument.containsKey("googleAppsScriptUrl"))
        GOOGLE_APPS_SCRIPT_URL = strdup(jsonDocument["googleAppsScriptUrl"].as<const char *>());
    if (jsonDocument.containsKey("timeApiUrl"))
        TIME_API_URL = strdup(jsonDocument["timeApiUrl"].as<const char *>());
    if (jsonDocument.containsKey("webhookSite"))
        WEBHOOK_SITE = strdup(jsonDocument["webhookSite"].as<const char *>());
    if (jsonDocument.containsKey("localIp"))
        LOCAL_IP.fromString(jsonDocument["localIp"].as<const char *>());
    if (jsonDocument.containsKey("gateway"))
        GATEWAY.fromString(jsonDocument["gateway"].as<const char *>());
    if (jsonDocument.containsKey("subnet"))
        SUBNET.fromString(jsonDocument["subnet"].as<const char *>());
    if (jsonDocument.containsKey("primaryDns"))
        PRIMARY_DNS.fromString(jsonDocument["primaryDns"].as<const char *>());
    if (jsonDocument.containsKey("secondaryDns"))
        SECONDARY_DNS.fromString(jsonDocument["secondaryDns"].as<const char *>());
}

String createJsonStringFromSettings()
{
    DynamicJsonDocument jsonDocument(1024);

    jsonDocument["batteryVoltageDividerRatio"] = BATTERY_VOLTAGE_DIVIDER_RATIO;
    jsonDocument["batteryVoltageCorrection"] = BATTERY_VOLTAGE_CORRECTION;
    jsonDocument["solarVoltageDividerRatio"] = SOLAR_VOLTAGE_DIVIDER_RATIO;
    jsonDocument["solarVoltageCorrection"] = SOLAR_VOLTAGE_CORRECTION;
    jsonDocument["batteryMinimalVoltage"] = BATTERY_MINIMAL_VOLTAGE;
    jsonDocument["sensorReadAndSendInterval"] = SENSOR_READ_AND_SEND_INTERVAL;
    jsonDocument["requestTimeout"] = REQUEST_TIMEOUT;
    jsonDocument["reconnectInterval"] = RECONNECT_INTERVAL;
    jsonDocument["lowBatteryWorkTime"] = LOW_BATTERY_WORK_TIME;
    jsonDocument["ssid"] = SSID;
    jsonDocument["wifiPassword"] = WIFI_PASSWORD;
    jsonDocument["googleAppsScriptUrl"] = GOOGLE_APPS_SCRIPT_URL;
    jsonDocument["timeApiUrl"] = TIME_API_URL;
    jsonDocument["webhookSite"] = WEBHOOK_SITE;
    jsonDocument["localIp"] = LOCAL_IP.toString();
    jsonDocument["gateway"] = GATEWAY.toString();
    jsonDocument["subnet"] = SUBNET.toString();
    jsonDocument["primaryDns"] = PRIMARY_DNS.toString();
    jsonDocument["secondaryDns"] = SECONDARY_DNS.toString();

    String jsonSrting;
    serializeJson(jsonDocument, jsonSrting);
    return jsonSrting;
}