#include "BoxAccelerometer.h"
#include "BoxEvents.h"

void BoxAccelerometer::begin() {  
    pinMode(62, OUTPUT);
    digitalWrite(62, HIGH);
    Wire.begin();
    
    setInterval(250);
    Log.info("Initialize Accelerometer...");

    if (!_accel.begin())
        Log.error("... not found!");

    _accel.setupPL();

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
    }
}

void BoxAccelerometer::reloadConfig() {  

}

BoxAccelerometer::Orientation BoxAccelerometer::getOrientation() {
    return _orientation;
}