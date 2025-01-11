// #include <Arduino.h>
// #include <HTTPClient.h>
// #include <default_configs.h>

// String getHostHeaderFromUrl(String url)
// {
//     int index = url.indexOf("://");
//     if (index != -1)
//     {
//         String host = url.substring(index + 3);
//         index = host.indexOf("/");
//         if (index != -1)
//         {
//             host = host.substring(0, index);
//         }
//         return host;
//     }
//     return "";
// }