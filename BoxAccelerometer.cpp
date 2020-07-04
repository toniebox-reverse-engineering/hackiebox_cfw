#include "BoxAccelerometer.h"
#include "BoxEvents.h"

void BoxAccelerometer::begin() {  
    pinMode(62, OUTPUT);
    digitalWrite(62, HIGH);
    
    setInterval(250);
    Log.info("Initialize Accelerometer...");

    if (!_accel.begin())
        Log.error("... not found!");

    _accel.setupPL();

    
    _accel.writeRegister(CTRL_REG1, 0x02); //Original 0x02
    _accel.writeRegister(XYZ_DATA_CFG, 0x02); //Original 0x02
    _accel.writeRegister(CTRL_REG2, 0x00); //Original 0x00
    _accel.writeRegister(F_SETUP, 0x00); //Original 0x00
    _accel.writeRegister(TRIG_CFG, 0x08); //Original 0x08
    _accel.writeRegister(PULSE_CFG, 0x54); //Original 0x54
    _accel.setupTap(0x1B, 0x3F, 0x3F); //Original 0x1B, 0x3F, 0x3F
    _accel.writeRegister(PULSE_TMLT, 0x28); //Original 0x28
    _accel.writeRegister(PULSE_LTCY, 0x7F); //Original 0x7F
    _accel.writeRegister(HP_FILTER_CUTOFF, 0x10); //Original 0x10

    _accel.writeRegister(CTRL_REG3, 0x12); //Original 0x12
    _accel.writeRegister(CTRL_REG4, 0x40); //Original 0x40
    _accel.writeRegister(CTRL_REG5, 0x40); //Original 0x40
    _accel.writeRegister(CTRL_REG1, 0x03); //Original 0x03

    Log.info("... initialized");
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

        byte tap = _accel.readTap();
        if (tap) {
            Log.verbose("Tap recieved %i", tap);
        }
        
    }
}

void BoxAccelerometer::reloadConfig() {  

}

BoxAccelerometer::Orientation BoxAccelerometer::getOrientation() {
    return _orientation;
}