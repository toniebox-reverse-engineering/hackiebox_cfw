#ifndef Hackiebox_h
#define Hackiebox_h

#include "BaseHeader.h"

#include "JC_Button.h"

#include "WrapperWiFi.h"
#include "WrapperWebServer.h"


class Hackiebox { 
    public:
    void
        setup(),
        loop();
    private:
        enum ButtonPressedTime { NOT, SHORT, LONG, VERY_LONG };

        WrapperWiFi _wifi;
        WrapperWebServer _server;

        int _earLongPressMs = 1000;
        int _earVeryLongPressMs = 10000;

        ButtonPressedTime _earSmallPressedTime = NOT;
        ButtonPressedTime _earBigPressedTime = NOT;
        ButtonPressedTime _earBothPressedTime = NOT;

        Button _earSmall = Button(59, 25, false, true);
        Button _earBig = Button(57, 25, false, true);
    
    void
        _buttonLoop();

};

#endif
