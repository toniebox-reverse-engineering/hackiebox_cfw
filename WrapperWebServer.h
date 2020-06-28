#ifndef WrapperWebServer_h
#define WrapperWebServer_h

#include "BaseHeader.h"
#include <WebServer.h>
#include <EnhancedThread.h>

#define SSE_MAX_CHANNELS 5 

class WrapperWebServer : public EnhancedThread { 
    public:
        struct SSESubscription {
            IPAddress clientIP;
            WiFiClient client;
        //Ticker keepAliveTimer;
        } subscription[SSE_MAX_CHANNELS];
        uint8_t subscriptionCount = 0;
        
    void
        begin(),
        loop(),
        handle(void);
    
    private:
        void
            handleNotFound(void),
            handleUnknown(void),
            handleRoot(void),
            handleFile(const char* path, const char* type),
            handleSseSub(void),
            handleAjax(void),
            handleUploadFile(void),
            handleUploadFlashFile(void);

        void 
            sendJsonSuccess();

        void
            sseHandler(uint8_t channel),
            sseKeepAlive(),
            sendEvent(char* eventname, char* content);
            //sendEventJSON(char* eventname, xyzjsondoc jsonContent);


        bool 
            commandGetFile(String* path, long read_start, long read_length, bool download),
            commandGetFlashFile(String* path, long read_start, long read_length);

        WebServer* _server;
        FileFs _uploadFile;
        bool _uploadFileOpen = false;

};

#endif
