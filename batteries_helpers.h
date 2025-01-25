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
