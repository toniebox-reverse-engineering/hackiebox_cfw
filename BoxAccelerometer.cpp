#include "BoxAccelerometer.h"
#include "BoxEvents.h"

void BoxAccelerometer::begin() {  
    pinMode(62, OUTPUT);
    digitalWrite(62, HIGH);
    
    setInterval(250);
    Log.info("Init Accelerometer...");

    if (!_accel.begin())
        Log.error("...not found!");

    _accel.setupPL();

    
    _accel.writeRegister(CTRL_REG1, 0x02); //Original 0x02 //F_READ
    _accel.writeRegister(XYZ_DATA_CFG, 0x02); //Original 0x02 //FS1
    _accel.writeRegister(CTRL_REG2, 0x00); //Original 0x00 //Standby
    _accel.writeRegister(F_SETUP, 0x00); //Original 0x00
    _accel.writeRegister(TRIG_CFG, 0x08); //Original 0x08 //Trig_PULSE/ZSPEFE/ELE
    _accel.writeRegister(PULSE_CFG, 0x54); //Original 0x54 //YSPEFE
    _accel.setupTap(0x1B, 0x3F, 0x3F); //Original 0x1B, 0x3F, 0x3F
    _accel.writeRegister(PULSE_TMLT, 0x28); //Original 0x28 //TMLT3/TMLT5
    _accel.writeRegister(PULSE_LTCY, 0x7F); //Original 0x7F //LTCY6/LTCY5/LTCY4/LTCY3/LTCY2/LTCY1/LTCY0
    _accel.writeRegister(HP_FILTER_CUTOFF, 0x10); //Original 0x10 //Pulse_LPF_EN

    _accel.writeRegister(CTRL_REG3, 0x12); //Original 0x12 //WAKE_PULSE/IPOL
    _accel.writeRegister(CTRL_REG4, 0x40); //Original 0x40 //INT_EN_FIFO
    _accel.writeRegister(CTRL_REG5, 0x40); //Original 0x40 //INT_CFG_FIFO INT1
    _accel.writeRegister(CTRL_REG1, 0x03); //Original 0x03 //F_READ/ACTIVE

    Log.info("...done");
}

void BoxAccelerometer::loop() {  
    if (_accel.available()) { 
        //Log.verbose("Accelerometer=(%i, %i, %i), Orient=%i", _accel.getX(), _accel.getY(), _accel.getZ(), _accel.readOrientation());

        Orientation orient = (Orientation)_accel.readOrientation();
        if (orient == Orientation::EARS_UP2) {
            orient = Orientation::EARS_UP;
        } else if (orient == Orientation::EARS_DOWN2) {
            orient = Orientation::EARS_DOWN;
        }
        
        if (_orientation != orient) {
            _orientation = orient;
            Events.handleAccelerometerOrientationEvent(_orientation);
        }

        uint8_t tap = _accel.readTap();
        if (tap) {
            bool AxZ  = (tap&0b1000000)>0; //event on axis
            bool AxY  = (tap&0b0100000)>0;
            bool AxX  = (tap&0b0010000)>0;
            //bool DPE  = (tap&0b0001000)>0; //double
            bool PolZ = !((tap&0b0000100)>0); //0=positive 1=negative
            bool PolY = !((tap&0b0000010)>0);
            bool PolX = !((tap&0b0000001)>0);

            //X+ = box bottom
            //X- = box top
            //Y+ = box back left (big ear)
            //Y- = box front right (speaker, small ear)
            //Z+ = box back right (small ear)
            //Z- = box front left (speaker, big ear)

            //Something wrong, only blinks red or greenyellow
            TapOn tapOn = TapOn::NONE;
            if (AxX) {
                if (PolX) {
                    tapOn = TapOn::BOTTOM;
                    Box.boxLEDs.setActiveAnimationByIteration(BoxLEDs::ANIMATION_TYPE::BLINK, BoxLEDs::CRGB::Red, 2);
                } else {
                    tapOn = TapOn::TOP;
                    Box.boxLEDs.setActiveAnimationByIteration(BoxLEDs::ANIMATION_TYPE::BLINK, BoxLEDs::CRGB::Orange, 2);
                }
            }
            if (AxY && AxZ) {
                if (PolY && PolZ) {
                    tapOn = TapOn::BACK;
                    Box.boxLEDs.setActiveAnimationByIteration(BoxLEDs::ANIMATION_TYPE::BLINK, BoxLEDs::CRGB::Blue, 3);
                } else if (!PolY && !PolZ) {
                    tapOn = TapOn::FRONT;
                    Box.boxLEDs.setActiveAnimationByIteration(BoxLEDs::ANIMATION_TYPE::BLINK, BoxLEDs::CRGB::Violet, 3);
                } else if (PolY && !PolZ) {
                    tapOn = TapOn::LEFT;
                    Box.boxLEDs.setActiveAnimationByIteration(BoxLEDs::ANIMATION_TYPE::BLINK, BoxLEDs::CRGB::Green, 3);
                } else if (!PolY && PolZ) {
                    tapOn = TapOn::RIGHT;
                    Box.boxLEDs.setActiveAnimationByIteration(BoxLEDs::ANIMATION_TYPE::BLINK, BoxLEDs::CRGB::GreenYellow, 3);
                }
            } else if (AxY) {
                if (PolY) {
                    tapOn = TapOn::LEFT_BACK;
                    Box.boxLEDs.setActiveAnimationByIteration(BoxLEDs::ANIMATION_TYPE::BLINK, BoxLEDs::CRGB::Blue, 3);
                } else {
                    tapOn = TapOn::LEFT_FRONT;
                    Box.boxLEDs.setActiveAnimationByIteration(BoxLEDs::ANIMATION_TYPE::BLINK, BoxLEDs::CRGB::Violet, 3);
                }
            } else if (AxZ) {
                if (PolZ) {
                    tapOn = TapOn::RIGHT_BACK;
                    Box.boxLEDs.setActiveAnimationByIteration(BoxLEDs::ANIMATION_TYPE::BLINK, BoxLEDs::CRGB::Green, 3);
                } else {
                    tapOn = TapOn::RIGHT_FRONT;
                    Box.boxLEDs.setActiveAnimationByIteration(BoxLEDs::ANIMATION_TYPE::BLINK, BoxLEDs::CRGB::GreenYellow, 3);
                }
            }

            Log.disableNewline(true);
            Log.verbose("Tap recieved %B, direction=%X, ", tap, tapOn);
            Log.disableNewline(false);
            switch (tapOn)
            {
            case TapOn::LEFT:
                Log.printfln("LEFT");
                break;
            
            case TapOn::RIGHT:
                Log.printfln("RIGHT");
                break;
            
            case TapOn::FRONT:
                Log.printfln("FRONT");
                break;
            
            case TapOn::BACK:
                Log.printfln("BACK");
                break;
            
            case TapOn::TOP:
                Log.printfln("TOP");
                break;
            
            case TapOn::BOTTOM:
                Log.printfln("BOTTOM");
                break;

            case TapOn::LEFT_FRONT:
                Log.printfln("LEFT_FRONT");
                break;
            
            case TapOn::RIGHT_FRONT:
                Log.printfln("RIGHT_FRONT");
                break;
            
            case TapOn::LEFT_BACK:
                Log.printfln("LEFT_BACK");
                break;
            
            case TapOn::RIGHT_BACK:
                Log.printfln("RIGHT_BACK");
                break;
            
            default:
                break;
                Log.printfln("OTHER");
            }

        }
        
    }
}

void BoxAccelerometer::reloadConfig() {  

}

BoxAccelerometer::Orientation BoxAccelerometer::getOrientation() {
    return _orientation;
}