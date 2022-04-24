#ifndef WrapperWebServer_h
#define WrapperWebServer_h

#include "BaseHeader.h"
#include <WebServer.h>
#include <EnhancedThread.h>
#include "BoxTimer.h"

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
        loop();
    
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
            sendEvent(const char* eventname, const char* content);
            //sendEventJSON(char* eventname, xyzjsondoc jsonContent);


        bool 
            commandGetFile(String* path, uint32_t read_start, uint32_t read_length, bool download),
            commandGetFlashFile(String* path, uint32_t read_start, uint32_t read_length);

        BoxTimer _sseTimer;
        WebServer* _server;
        FileFs _uploadFile;
        bool _uploadFileOpen = false;
        bool _uploadFlashFileOpen = false;

};

#endif
