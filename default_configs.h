float VOLTAGE_DIVIDER_RATIO = 3.17;
float VOLTAGE_CORRECTION = 1.064;
float BATTERY_MINIMAL_VOLTAGE = 6.0;        // lifepo4 min is 2,5. So better don't make it below 3. And since batteries are 2S, 3.0 + 3.0 = 6.0
int SENSOR_READ_AND_SEND_INTERVAL = 600000; // 10 minutes
int REQUEST_TIMEOUT = 30000;                // 30 seconds
int RECONNECT_INTERVAL = 30000;             // 30 seconds
int LOW_BATTERY_WORK_TIME = 300000;         // 5 minutes
char *SSID = "TRDLviv_2.4";
char *WIFI_PASSWORD = "0963739595";
char *GOOGLE_APPS_SCRIPT_URL = "https://script.google.com/macros/s/AKfycbzlhPt5KgMKRvYqPHTS6Qfln0peAIw5FWOxu_rNcW4eCGuDorw57CDuWHINWZ66R6mJ/exec";
char *TIME_API_URL = "https://timeapi.io/api/time/current/zone?timeZone=Europe/Kyiv";
char *WEBHOOK_SITE = "https://webhook.site/e4144c5f-539c-4694-bb0b-54cd62b1eff0";
IPAddress LOCAL_IP(192, 168, 88, 101);
IPAddress GATEWAY(192, 168, 88, 1);
IPAddress SUBNET(255, 255, 255, 0);
IPAddress PRIMARY_DNS(8, 8, 8, 8);
IPAddress SECONDARY_DNS(8, 8, 4, 4);
