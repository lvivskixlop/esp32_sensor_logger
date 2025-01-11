// #include <ArduinoJson.h>

// StaticJsonDocument<4096> jsonDocument;
// char buffer[4096];

// void create_env_json(float temperature, float humidity, float voltage)
// {
//     jsonDocument.clear();
//     add_json_object("temperature", temperature, "°C");
//     add_json_object("humidity", humidity, "%");
//     add_json_object("voltage", voltage, "V");
//     serializeJson(jsonDocument, buffer);
// }

// void add_json_object(char *tag, float value, char *unit)
// {
//     JsonObject obj = jsonDocument.createNestedObject();
//     obj["type"] = tag;
//     obj["value"] = value;
//     obj["unit"] = unit;
// }