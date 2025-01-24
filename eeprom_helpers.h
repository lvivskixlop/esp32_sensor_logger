#define EEPROM_SIZE 1024

void saveSettingsToEEPROM()
{
    String jsonString = createJsonStringFromSettings();

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

    DynamicJsonDocument jsonDocument(1024);
    DeserializationError error = deserializeJson(jsonDocument, jsonString);

    if (error)
        return;

    setSettingsFromJson(jsonDocument);
}