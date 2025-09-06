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
#include <adapters.h>
#include <voltage_helpers.h>
#include <time_helpers.h>
// We’ll use LEDC channel 0, timer 0, 25 kHz, 7-bit resolution → 0…127 steps
#define PWM_CHANNEL 0
#define PWM_FREQUENCY 25000
#define PWM_RES_BITS 7

#define DHTTYPE DHT11

unsigned long lastReadTime = 0;
unsigned long lowBatteryTime = 0;
float temperature;
float humidity;
float batteryVoltage;
int soilMoisture;
bool relay1State = false;
bool relay2State = false;

DHT dht(DHTPIN, DHTTYPE);
WebServer server(80);

void setup_routing()
{
	server.on("/env", getEnv);
	server.on("/setup", HTTP_POST, handleSettingsSetup);
	server.on("/relay1", HTTP_POST, handleRelay1);
	server.on("/relay2", HTTP_POST, handleRelay2);
	server.on("/pwm", HTTP_POST, handlePwm);
	server.begin();
}

void handlePwm()
{
	if (server.hasArg("pwm"))
	{
		String pwmString = server.arg("pwm");
		int pwm = pwmString.toInt();
		ledcWrite(PWM_PIN, pwm);
		server.send(200, "application/json", "{\"success\":true,\"pwm\":" + pwmString + "}");
	}
	else
	{
		server.send(400, "application/json", "{\"error\":\"Missing pwm parameter\"}");
	}
}

void handleRelay1()
{
	if (server.hasArg("state"))
	{
		String state = server.arg("state");
		relay1State = (state == "1" || state == "true" || state == "on");
		digitalWrite(RELAY_1_PIN, relay1State ? LOW : HIGH);
		server.send(200, "application/json", "{\"success\":true,\"relay1\":" + String(relay1State ? "true" : "false") + "}");
	}
	else
	{
		server.send(400, "application/json", "{\"error\":\"Missing state parameter\"}");
	}
}

void handleRelay2()
{
	if (server.hasArg("state"))
	{
		String state = server.arg("state");
		relay2State = (state == "1" || state == "true" || state == "on");
		digitalWrite(RELAY_2_PIN, relay2State ? LOW : HIGH);
		server.send(200, "application/json", "{\"success\":true,\"relay2\":" + String(relay2State ? "true" : "false") + "}");
	}
	else
	{
		server.send(400, "application/json", "{\"error\":\"Missing state parameter\"}");
	}
}

void gatherData()
{
	batteryVoltage = readVoltagePrecise(ADC_BATTERY_VOLTAGE_PIN, BATTERY_VOLTAGE_DIVIDER_RATIO, BATTERY_VOLTAGE_CORRECTION);
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
	gatherData();
	String currentTime = time_get_iso8601();
	createEnvJson(batteryVoltage, relay1State, relay2State, currentTime);
	server.send(200, "application/json", buffer);
}

void updateRelaysByTime()
{
	// Extract hour from time string (format: "YYYY-MM-DDTHH:mm:ss.sssZ")
	int currentHour = time_get_hour();

	// Update Relay 1
	if (currentHour >= RELAY1_ON_HOUR && currentHour < RELAY1_OFF_HOUR)
	{
		relay1State = true;
		digitalWrite(RELAY_1_PIN, relay1State ? LOW : HIGH); // Remember: LOW turns relay ON due to active-LOW
	}
	else
	{

		relay1State = false;
		digitalWrite(RELAY_1_PIN, relay1State ? LOW : HIGH);
	}

	// Update Relay 2
	if (currentHour >= RELAY2_ON_HOUR && currentHour < RELAY2_OFF_HOUR)
	{

		relay2State = true;
		digitalWrite(RELAY_2_PIN, relay2State ? LOW : HIGH);
	}
	else
	{

		relay2State = false;
		digitalWrite(RELAY_2_PIN, relay2State ? LOW : HIGH);
	}
}

void sendData(bool lastMessage = false)
{
	gatherData();
	String currentTime = time_get_iso8601();
	if (currentTime.length() == 0) {
		currentTime = getTimeFromAPI(); // keep your old API fallback
	}

	if (currentTime.length() > 0)
	{
		updateRelaysByTime(); // Update relay states based on time
	}

	if (isnan(temperature) || isnan(humidity) || currentTime == "")
	{
		return;
	}

	if (WiFi.status() != WL_CONNECTED)
	{
		return;
	}

	String body;
	if (lastMessage)
	{
		body = "{\"temperature\":" + String(temperature) + ",\"humidity\":" + String(humidity) + ",\"time\":\"" + currentTime + "\",\"voltage\":\"DISCHARGED(" + String(batteryVoltage) + ")\",\"soilMoisture\":" + String(soilMoisture) + ",\"relay1\":" + String(relay1State ? "true" : "false") + ",\"relay2\":" + String(relay2State ? "true" : "false") + "}";
	}
	else
	{
		body = "{\"temperature\":" + String(temperature) + ",\"humidity\":" + String(humidity) + ",\"time\":\"" + currentTime + "\",\"batteryVoltage\":" + String(batteryVoltage) + ",\"soilMoisture\":" + String(soilMoisture) + ",\"relay1\":" + String(relay1State ? "true" : "false") + ",\"relay2\":" + String(relay2State ? "true" : "false") + "}";
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
		DeserializationError error = deserializeJson(doc, response);
		if (error)
		{
			return "";
		}
		return doc["dateTime"].as<String>();
	}
	else
	{
		return "";
	}
}

void setup()
{
	delay(1000);
	Serial.begin(9600);
	delay(1000);

	connectToWifi();
	time_init();
	setup_routing();

	// Setup pins
	pinMode(ADC_BATTERY_VOLTAGE_PIN, INPUT);
	analogSetPinAttenuation(ADC_BATTERY_VOLTAGE_PIN, ADC_11db);

	ledcAttach(PWM_PIN, PWM_FREQUENCY, PWM_RES_BITS);

	// Setup relay pins and ensure they start in OFF state
	pinMode(RELAY_1_PIN, OUTPUT);
	pinMode(RELAY_2_PIN, OUTPUT);
	digitalWrite(RELAY_1_PIN, HIGH); // Ensure relay starts OFF
	digitalWrite(RELAY_2_PIN, HIGH); // Ensure relay starts OFF

	EEPROM.begin(EEPROM_SIZE);
	loadSettingsFromEEPROM();
}

void loop()
{
	time_loop_task();
	if (batteryVoltage < BATTERY_MINIMAL_VOLTAGE)
	{
		lowBatteryTime = lowBatteryTime == 0 ? millis() : lowBatteryTime;
		if (millis() - lowBatteryTime >= LOW_BATTERY_WORK_TIME)
		{
			//battery is too low, so here we do whatever we need to do, before it will shut down
			//and don't forget to add some delay, if making an api call
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
		//do whatever we need to do between send intervals
		// sendData();
		lastReadTime = currentMillis;
	}

	if (WiFi.status() != WL_CONNECTED)
	{
		reconnectToWifi();
	}

	server.handleClient();
	delay(10);
}
