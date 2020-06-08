#include "LogStream.h"

#include "Hackiebox.h"
#include "WrapperWebServer.h"

size_t LogStream::write(uint8_t character)  {
    if (Box.webServer.subscriptionCount == 0 || _ssePaused)
        return 0;
        
    for (uint8_t i = 0; i < SSE_MAX_CHANNELS; i++) {
        WrapperWebServer::SSESubscription* subscription = &Box.webServer.subscription[i];
        WiFiClient* client = &(subscription->client);
        if (!(subscription->clientIP) || !client->connected()) 
            continue;
        
        if (_lineFinished) {
            client->print("data: { \"type\":\"");
            client->print("log");
            client->print("\", \"data\":\"");
            _lineFinished = false;
        }
        client->print(character); //TODO escape ";
    }
}

size_t LogStream::println() {
    if (Box.webServer.subscriptionCount == 0 || _ssePaused)
        return 0;

    print("\" }\n\n"); // Extra newline required by SSE standard
    _lineFinished = true;
}
bool LogStream::isLineFinished() {
    return _lineFinished;
}
void LogStream::setSsePaused(bool paused) {
    _ssePaused = paused;
}