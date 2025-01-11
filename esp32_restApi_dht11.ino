#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <HTTPClient.h>
#include <EEPROM.h>
#include <default_configs.h>
#include <wifi_helpers.h>
#include <http_helpers.h>
#include <json_helpers.h>

#define ADC_BATTERY_VOLTAGE_PIN 36

#define EEPROM_SIZE 1024

#define DHTPIN 27
#define DHTTYPE DHT11

unsigned long lastReadTime = 0;
float temperature;
float humidity;
float batteryVoltage;

DHT dht(DHTPIN, DHTTYPE);

WebServer server(80);

void setup_routing()
{
	server.on("/env", getEnv);
	server.on("/setup", HTTP_POST, handleSettingsSetup);
	server.begin();
}

void handleSettingsSetup()
{
	if (server.hasArg("plain") == false)
	{
		server.send(400, "application/json", "{\"error\":\"Invalid request\"}");
		return;
	}

	String body = server.arg("plain");
	DynamicJsonDocument doc(1024);
	DeserializationError error = deserializeJson(doc, body);

	if (error)
	{
		server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
		return;
	}

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

	saveSettingsToEEPROM();

	DynamicJsonDocument responseDoc(1024);
	responseDoc["voltageDividerRatio"] = VOLTAGE_DIVIDER_RATIO;
	responseDoc["voltageCorrection"] = VOLTAGE_CORRECTION;
	responseDoc["batteryMinimalVoltage"] = BATTERY_MINIMAL_VOLTAGE;
	responseDoc["sensorReadAndSendInterval"] = SENSOR_READ_AND_SEND_INTERVAL;
	responseDoc["requestTimeout"] = REQUEST_TIMEOUT;
	responseDoc["reconnectInterval"] = RECONNECT_INTERVAL;
	responseDoc["ssid"] = SSID;
	responseDoc["wifiPassword"] = WIFI_PASSWORD;
	responseDoc["googleAppsScriptUrl"] = GOOGLE_APPS_SCRIPT_URL;
	responseDoc["timeApiUrl"] = TIME_API_URL;
	responseDoc["webhookSite"] = WEBHOOK_SITE;
	responseDoc["localIp"] = LOCAL_IP.toString();
	responseDoc["gateway"] = GATEWAY.toString();
	responseDoc["subnet"] = SUBNET.toString();
	responseDoc["primaryDns"] = PRIMARY_DNS.toString();
	responseDoc["secondaryDns"] = SECONDARY_DNS.toString();

	String responseBody;
	serializeJson(responseDoc, responseBody);
	server.send(200, "application/json", responseBody);
}

void getEnv()
{
	Serial.println("Get env");
	gatherData();
	create_env_json(temperature, humidity, batteryVoltage);
	server.send(200, "application/json", buffer);
}

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

	if (jsonString.length() > 0)
	{
		DynamicJsonDocument doc(1024);
		DeserializationError error = deserializeJson(doc, jsonString);

		if (!error)
		{
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
	}
}

float readBatteryVoltage()
{
	int rawADC = analogRead(ADC_BATTERY_VOLTAGE_PIN);
	float voltage = rawADC * (3.3 / 4095.0) * VOLTAGE_DIVIDER_RATIO; // Convert ADC reading to voltage
	voltage *= VOLTAGE_CORRECTION;
	return voltage;
}

float readBatteryVoltagePrecise(int numReadings = 10, float outlierThreshold = 0.05)
{
	int adcValues[numReadings];
	float voltages[numReadings];

	// Step 1: Collect multiple ADC readings
	for (int i = 0; i < numReadings; i++)
	{
		adcValues[i] = analogRead(ADC_BATTERY_VOLTAGE_PIN);
		voltages[i] = adcValues[i] * (3.3 / 4095.0) * VOLTAGE_DIVIDER_RATIO;
		voltages[i] *= VOLTAGE_CORRECTION;
		delay(10); // Small delay to allow stable readings
	}

	// Step 2: Sort the voltage array (for median calculation)
	for (int i = 0; i < numReadings - 1; i++)
	{
		for (int j = i + 1; j < numReadings; j++)
		{
			if (voltages[i] > voltages[j])
			{
				float temp = voltages[i];
				voltages[i] = voltages[j];
				voltages[j] = temp;
			}
		}
	}

	// Step 3: Calculate the median
	float median;
	if (numReadings % 2 == 0)
	{
		median = (voltages[numReadings / 2 - 1] + voltages[numReadings / 2]) / 2.0;
	}
	else
	{
		median = voltages[numReadings / 2];
	}

	// Step 4: Remove outliers (values too far from the median)
	float sum = 0;
	int validCount = 0;
	for (int i = 0; i < numReadings; i++)
	{
		if (abs(voltages[i] - median) <= outlierThreshold * median)
		{
			sum += voltages[i];
			validCount++;
		}
	}

	// Step 5: Return the average of valid values
	return validCount > 0 ? sum / validCount : median;
}

void gatherData()
{
	temperature = dht.readTemperature();
	humidity = dht.readHumidity();
	batteryVoltage = readBatteryVoltagePrecise();
}

void callApi(String url, String method, String body, String header, String &response)
{
	if (WiFi.status() != WL_CONNECTED)
	{
		Serial.println("WiFi not connected");
		return;
	}

	HTTPClient http;
	http.setTimeout(REQUEST_TIMEOUT);
	http.begin(url);

	int httpResponseCode;
	if (method == "POST")
	{
		String host = getHostHeaderFromUrl(url);
		http.addHeader("Host", host);
		http.addHeader("Content-Type", header);
		String contentLengthString = String(body.length());
		http.addHeader("Content-Length", contentLengthString);
		Serial.println("Sending request to: " + url + "\nbody: " + body + "\nContent-Type: " + header + "\nContent-Length: " + contentLengthString + "\nHost: " + host);
		httpResponseCode = http.POST(body);
	}
	else if (method == "GET")
	{
		httpResponseCode = http.GET();
	}
	else
	{
		Serial.println("Unsupported HTTP method");
		http.end();
		return;
	}

	if (httpResponseCode > 0)
	{
		response = http.getString();
		Serial.println(httpResponseCode);
		Serial.println(response);
	}
	else
	{
		String error = http.errorToString(httpResponseCode);
		Serial.print("Error on sending request: ");
		Serial.println(error);

		// http.setTimeout(REQUEST_TIMEOUT);
		// http.begin(WEBHOOK_SITE); // call webhook for debugging
		// http.POST(error);
	}

	http.end();
}

void sendData()
{
	gatherData();
	String currentTime = getTimeFromAPI();

	if (isnan(temperature) || isnan(humidity) || currentTime == "")
	{
		Serial.println("Failed to gather complete data. Skipping...");
	}

	if (WiFi.status() != WL_CONNECTED)
	{
		return;
	}
	String body = "{\"temperature\":" + String(temperature) + ",\"humidity\":" + String(humidity) + ",\"time\":\"" + currentTime + "\",\"voltage\":" + String(batteryVoltage) + "}";
	String response;
	callApi(GOOGLE_APPS_SCRIPT_URL, "POST", body, "application/json", response);
}

String getTimeFromAPI()
{
	String response;
	callApi(TIME_API_URL, "GET", "", "application/json", response);

	if (response.length() > 0)
	{
		DynamicJsonDocument doc(1024);
		deserializeJson(doc, response);
		return doc["dateTime"].as<String>();
	}
	else
	{
		return "";
	}
}

void setup()
{
	Serial.begin(9600);
	dht.begin();
	connectToWifi();
	setup_routing();
	pinMode(ADC_BATTERY_VOLTAGE_PIN, INPUT); // Configure ADC pin
	EEPROM.begin(EEPROM_SIZE);
	loadSettingsFromEEPROM();
}

void loop()
{
	// if (batteryVoltage < BATTERY_MINIMAL_VOLTAGE)
	// {
	// 	// go deep sleep
	// 	//		Serial.flush();
	// 	//		esp_deep_sleep_start();
	// }

	unsigned long currentMillis = millis();

	if (currentMillis - lastReadTime >= SENSOR_READ_AND_SEND_INTERVAL)
	{
		sendData();
		lastReadTime = currentMillis;
	}

	if (WiFi.status() != WL_CONNECTED)
	{
		reconnectToWifi();
	}
	server.handleClient();
}
