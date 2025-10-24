#ifndef CRON_HELPERS_H
#define CRON_HELPERS_H

// Simple cron expression matcher
// Format: minute hour day month weekday
// Example: "30 8 * * *" = every day at 8:30
// Supports: numbers, *, and ranges (1-5)
bool matchCronExpression(const char* cronExpr, int minute, int hour, int day, int month, int weekday) {
    char cronParts[5][20];
    int partIndex = 0;
    int charIndex = 0;
    
    // Parse cron expression into parts
    for (int i = 0; cronExpr[i] != '\0' && partIndex < 5; i++) {
        if (cronExpr[i] == ' ') {
            cronParts[partIndex][charIndex] = '\0';
            partIndex++;
            charIndex = 0;
        } else {
            cronParts[partIndex][charIndex++] = cronExpr[i];
        }
    }
    cronParts[partIndex][charIndex] = '\0';
    
    // Match values
    int values[] = {minute, hour, day, month, weekday};
    for (int i = 0; i < 5; i++) {
        if (strcmp(cronParts[i], "*") == 0) continue;
        
        // Handle ranges (e.g., "1-5")
        if (strchr(cronParts[i], '-') != NULL) {
            int rangeStart, rangeEnd;
            sscanf(cronParts[i], "%d-%d", &rangeStart, &rangeEnd);
            if (values[i] < rangeStart || values[i] > rangeEnd) return false;
        }
        // Handle single values
        else {
            int value = atoi(cronParts[i]);
            if (values[i] != value) return false;
        }
    }
    
    return true;
}

// Convert weekday from TimeAPI format (1-7, Monday=1) to cron format (0-6, Sunday=0)
int convertWeekdayToCron(int timeApiWeekday) {
    return (timeApiWeekday % 7);
}

bool shouldRelayBeOn(const char* onCron, const char* offCron, const String& currentTime) {
    // Parse time components from TimeAPI response (format: "YYYY-MM-DDTHH:mm:ss.sssZ")
    int hour = currentTime.substring(11, 13).toInt();
    int minute = currentTime.substring(14, 16).toInt();
    int day = currentTime.substring(8, 10).toInt();
    int month = currentTime.substring(5, 7).toInt();
    int weekday = currentTime.substring(17, 18).toInt(); // Assuming TimeAPI provides weekday
    weekday = convertWeekdayToCron(weekday);
    
    bool matchesOnTime = matchCronExpression(onCron, minute, hour, day, month, weekday);
    bool matchesOffTime = matchCronExpression(offCron, minute, hour, day, month, weekday);
    
    // If matches ON time exactly, return true
    if (matchesOnTime) return true;
    // If matches OFF time exactly, return false
    if (matchesOffTime) return false;
    
    // For any other time, we need to determine if we're between ON and OFF times
    // This is a simplified approach - for more complex schedules, you might need a more sophisticated algorithm
    int onHour = atoi(strchr(onCron, ' ') + 1);
    int offHour = atoi(strchr(offCron, ' ') + 1);
    
    if (offHour > onHour) {
        // Simple case: ON at 7:00, OFF at 23:00
        return (hour >= onHour && hour < offHour);
    } else if (offHour < onHour) {
        // Complex case: ON at 23:00, OFF at 7:00 (overnight)
        return (hour >= onHour || hour < offHour);
    }
    
    return false;
}

#endif 