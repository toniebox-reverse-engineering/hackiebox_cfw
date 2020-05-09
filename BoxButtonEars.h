#ifndef BoxButtonEars_h
#define BoxButtonEars_h

#include "BaseHeader.h"
#include "JC_Button.h"

class BoxButtonEars {
    public:
        enum PressedTime { NOT, SHORT, LONG, VERY_LONG };
        enum EarButton { NONE, SMALL, BIG, BOTH };

        void
            begin(),
            loop();
        

    private:

        uint16_t _earLongPressMs = 1000;
        uint16_t _earVeryLongPressMs = 10000;

        PressedTime _earSmallPressedTime = NOT;
        PressedTime _earBigPressedTime = NOT;
        PressedTime _earBothPressedTime = NOT;

        Button _earSmall = Button(59, 25, false, true);
        Button _earBig = Button(57, 25, false, true);
};

#endif