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
#include <eeprom_helpers.h>
#include <batteries_helpers.h>
#include <adapters.h>

#define DHTTYPE DHT11

unsigned long lastReadTime = 0;
unsigned long lowBatteryTime = 0;
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

void gatherData()
{
	temperature = dht.readTemperature();
	humidity = dht.readHumidity();
	batteryVoltage = readBatteryVoltagePrecise();
}

void handleSettingsSetup()
{
	if (server.hasArg("plain") == false)
	{
		server.send(400, "application/json", "{\"error\":\"Invalid request\"}");
		return;
	}

	String body = server.arg("plain");
	DynamicJsonDocument jsonDocument(1024);

	DeserializationError error = deserializeJson(jsonDocument, body);

	if (error)
	{
		server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
		return;
	}

	setSettingsFromJson(jsonDocument);

	saveSettingsToEEPROM();

	String responseBody = createJsonStringFromSettings();
	server.send(200, "application/json", responseBody);
}

void getEnv()
{
	Serial.println("Get env");
	gatherData();
	createEnvJson(temperature, humidity, batteryVoltage);
	server.send(200, "application/json", buffer);
}

void sendData(bool lastMessage = false)
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
	String body;
	if (lastMessage)
	{
		body = "{\"temperature\":" + String(temperature) + ",\"humidity\":" + String(humidity) + ",\"time\":\"" + currentTime + "\",\"voltage\":\"DISCHARGED(" + String(batteryVoltage) + ")\"}";
	}
	else
	{
		body = "{\"temperature\":" + String(temperature) + ",\"humidity\":" + String(humidity) + ",\"time\":\"" + currentTime + "\",\"voltage\":" + String(batteryVoltage) + "}";
	}
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
	if (batteryVoltage < BATTERY_MINIMAL_VOLTAGE)
	{
		lowBatteryTime = lowBatteryTime == 0 ? millis() : lowBatteryTime;
		if (millis() - lowBatteryTime >= LOW_BATTERY_WORK_TIME)
		{
			// send goodbye message to google api
			sendData(true);
			delay(REQUEST_TIMEOUT);
			//  go deep sleep
			Serial.flush();
			esp_deep_sleep_start();
		}
	}
	else
	{
		lowBatteryTime = 0;
	}

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
