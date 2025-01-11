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