#ifndef WrapperWebServer_h
#define WrapperWebServer_h

#include "BaseHeader.h"
#include <WebServer.h>

class WrapperWebServer { 
    public:
    void
        begin(),
        handle(void);

    private:
        void
            handleNotFound(void),
            handleRoot(void),
            handleSse(void),
            handleAjax(void),
            handleUploadFile(void),
            handleUploadFlashFile(void);

        void 
            sendJsonSuccess();

        bool 
            commandGetFile(String* path, long read_start, long read_length),
            commandGetFlashFile(String* path, long read_start, long read_length);

        WebServer* _server;
        FileFs _uploadFile;
        bool _uploadFileOpen = false;

};

#endif
