#include <Arduino.h>
#include <time.h>

static bool g_timeSynced = false;
static unsigned long g_lastResyncMillis = 0;
static const unsigned long RESYNC_INTERVAL_MS = 6UL * 60UL * 60UL * 1000UL; // 6 hours
static const int NTP_TIMEOUT_MS = 5000;

// POSIX TZ string for Kyiv (handles DST). This is the typical expression:
// EET (UTC+2) standard, EEST (UTC+3) DST rules: last part sets DST start/end.
// Note: ESP32's C library understands this POSIX-style TZ string.
static const char* KYIV_TZ = "EET-2EEST,M3.5.0/3,M10.5.0/4";

static bool try_fetch_time(int timeoutMs)
{
    struct tm timeinfo;
    // wait up to timeoutMs for SNTP to sync
    unsigned long start = millis();
    while (millis() - start < (unsigned long)timeoutMs)
    {
        if (getLocalTime(&timeinfo, 1000))
        {
            return true;
        }
        delay(50);
    }
    return false;
}

void time_init()
{
    // configure SNTP servers (0 offset, we'll use timezone via TZ envvar)
    configTime(0, 0, "pool.ntp.org", "time.google.com");

    // set timezone environment (Kyiv) and apply
    setenv("TZ", KYIV_TZ, 1);
    tzset();

    // try an initial fetch (non-blocking-ish with timeout)
    g_timeSynced = try_fetch_time(NTP_TIMEOUT_MS);
    g_lastResyncMillis = millis();
}

void time_loop_task()
{
    // If not synced yet, attempt to sync more aggressively
    if (!g_timeSynced)
    {
        g_timeSynced = try_fetch_time(NTP_TIMEOUT_MS);
        g_lastResyncMillis = millis();
        return;
    }

    // Periodic resync when online
    if (millis() - g_lastResyncMillis >= RESYNC_INTERVAL_MS)
    {
        // try a quick re-sync but don't block loop for long
        bool ok = try_fetch_time(NTP_TIMEOUT_MS);
        if (ok)
        {
            g_timeSynced = true;
        }
        g_lastResyncMillis = millis();
    }
}

bool time_is_synced()
{
    return g_timeSynced;
}

String time_get_iso8601()
{
    if (!g_timeSynced)
        return String();

    struct tm timeinfo;
    if (!getLocalTime(&timeinfo, 1000))
        return String();

    char buf[32];
    // Provide local-time ISO without fractional seconds. The trailing 'Z' is used
    // in your previous code — we keep it for compatibility even though 'Z' usually denotes UTC.
    // If you want explicit timezone offset string instead, we can change this easily.
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", &timeinfo);
    String out = String(buf) + "Z";
    return out;
}

int time_get_hour()
{
    if (!g_timeSynced)
        return -1;

    struct tm timeinfo;
    if (!getLocalTime(&timeinfo, 1000))
        return -1;
    return timeinfo.tm_hour;
}
