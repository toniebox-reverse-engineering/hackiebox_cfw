#ifndef BoxButtonEars_h
#define BoxButtonEars_h

#include "BaseHeader.h"
#include "JC_Button.h"

#include <EnhancedThread.h>

class BoxButtonEars : public EnhancedThread {
    public:
        enum class PressedTime { NOT, SHORT, LONG, VERY_LONG };
        enum class EarButton { NONE, SMALL, BIG, BOTH };
        enum class PressedType { NONE, PRESS, RELEASE };

        void
            begin(),
            loop(),
            reloadConfig(),
            waitForRelease();

    private:

        uint16_t _earLongPressMs;
        uint16_t _earVeryLongPressMs;

        PressedTime _earSmallPressedTime = PressedTime::NOT;
        PressedTime _earBigPressedTime = PressedTime::NOT;
        PressedTime _earBothPressedTime = PressedTime::NOT;

        Button _earSmall = Button(59, 25, false, true);
        Button _earBig = Button(57, 25, false, true);
};

#endif