#include "LogStreamSse.h"

#include "Hackiebox.h"
#include "WrapperWebServer.h"

size_t LogStreamSse::write(uint8_t character)  {
    if (Box.webServer.subscriptionCount == 0 || _ssePaused)
        return 0;
    
    for (uint8_t i = 0; i < SSE_MAX_CHANNELS; i++) {
        WrapperWebServer::SSESubscription* subscription = &Box.webServer.subscription[i];
        WiFiClient* client = &(subscription->client);
        if (!(subscription->clientIP) || !client->connected()) 
            continue;
        
        bool tagIsOpen = _tagIsOpen;
        if (character == '\n') {
            if (_tagIsOpen) {
                client->print("\" }\n\n"); // Extra newline required by SSE standard
                tagIsOpen = false;
            }
        } else {
            if (!_tagIsOpen) {
                client->print("data: { \"type\":\"");
                client->print("log");
                client->print("\", \"data\":\"");
                tagIsOpen = true;
            }
            switch (character) {
            case '\r':
            case '\b':
            case '\f':
                break;
            case '\t':
                break;
                client->print("\\t");
            case '\"':
                break;
                client->print("\\\"");
            default:
                client->print((char)character);
                break;
            }
        }
        _tagIsOpen = tagIsOpen;
    }
    return 1;
}

size_t LogStreamSse::println() {
    if (Box.webServer.subscriptionCount == 0 || _ssePaused)
        return 0;

    return print("\n");
}
void LogStreamSse::setSsePaused(bool paused) {
    _ssePaused = paused;
}