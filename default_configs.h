// pins
int ADC_BATTERY_VOLTAGE_PIN = 36;
int SOIL_MOISTURE_PIN = 33;
int RELAY_1_PIN = 13;
int RELAY_2_PIN = 14;
int DHTPIN = 27;
int CLOCK_CLK = 34;
int CLOCK_DAT = 35;
int CLOCK_RST = 32;

// Relay timing settings (24-hour format)
int RELAY1_ON_HOUR = 7;
int RELAY1_OFF_HOUR = 23;
int RELAY2_ON_HOUR = 7;
int RELAY2_OFF_HOUR = 23;

float BATTERY_VOLTAGE_DIVIDER_RATIO = 3.17;
float BATTERY_VOLTAGE_CORRECTION = 1.064;
float BATTERY_MINIMAL_VOLTAGE = 6.0;        // lifepo4 min is 2,5. So better don't make it below 3. And since batteries are 2S, 3.0 + 3.0 = 6.0
int SENSOR_READ_AND_SEND_INTERVAL = 600000; // 10 minutes
int REQUEST_TIMEOUT = 30000;                // 30 seconds
int RECONNECT_INTERVAL = 30000;             // 30 seconds
int LOW_BATTERY_WORK_TIME = 300000;         // 5 minutes
char *SSID = "TRDLviv_2.4";
char *WIFI_PASSWORD = "0963739595";
char *GOOGLE_APPS_SCRIPT_URL = "https://script.google.com/macros/s/AKfycbyAQvx4EzeSrPjetWynwa0HkYIdZgZagqO9Tzxt-kktKQ8MR-JJrsaclHwfoL1hSurM/exec";
char *TIME_API_URL = "https://timeapi.io/api/time/current/zone?timeZone=Europe/Kyiv";
char *WEBHOOK_SITE = "https://webhook.site/e4144c5f-539c-4694-bb0b-54cd62b1eff0";
IPAddress LOCAL_IP(192, 168, 88, 101);
IPAddress GATEWAY(192, 168, 88, 1);
IPAddress SUBNET(255, 255, 255, 0);
IPAddress PRIMARY_DNS(8, 8, 8, 8);
IPAddress SECONDARY_DNS(8, 8, 4, 4);
