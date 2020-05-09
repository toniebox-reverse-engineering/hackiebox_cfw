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
            loop(),
            reloadConfig();

    private:

        uint16_t _earLongPressMs;
        uint16_t _earVeryLongPressMs;

        PressedTime _earSmallPressedTime = NOT;
        PressedTime _earBigPressedTime = NOT;
        PressedTime _earBothPressedTime = NOT;

        Button _earSmall = Button(59, 25, false, true);
        Button _earBig = Button(57, 25, false, true);
};

#endif