/*
 * Basic Capacitative Position Driver
 * Uses the charge and sense method across four sectors to find position
 *
 * Copyright (c)2025 Pierce Nichols
 *
 * This file may be redistributed under the terms of the MIT license.
 * A copy of this license has been included with this distribution in the file LICENSE.
 *
 */

 #include "CapPosnBasic.h"

CapPosnBasic::CapPosnBasic(
    uint32_t drivePin,                  // The pin used to drive the system
    uint32_t sinApin,                   // The A phase of the sin half of the sense pads
    uint32_t sinBpin,                   // The B phase of the sin half of the sense pads
    uint32_t cosApin,                   // The A phase of the cos half of the sense pads
    uint32_t cosBpin,                   // The B phase of the cos half of the sense pads
    uint16_t readDelayMicros = 30,      // Delay between charging cap & start of measurement
    uint16_t measureDelayMicros = 150,  // Delay between phase measurements
    eAnalogReference analogRef = AR_DEFAULT) 
{
    
    _drivePin       = drivePin;
    _sinApin        = sinApin;
    _sinBpin        = sinBpin;
    _cosApin        = cosApin;
    _cosBpin        = cosBpin;
    _readDelay      = readDelayMicros;
    _measureDelay   = measureDelayMicros;
    _ref            = analogRef;
    _sinGain        = cPlaces;   
    _sinOffset      = 0;
    _cosGain        = cPlaces;    
    _cosOffset      = 0;
    pinMode(_drivePin, OUTPUT);
    analogReference(_ref);
}

void CapPosnBasic::setGainsOffsets(float sinOffset, float sinGain, float cosOffset, float cosGain) {
    _sinGain    = sinGain * cPlaces;
    _sinOffset  = sinOffset * cPlaces;
    _cosGain    = cosGain * cPlaces;
    _cosOffset  = cosOffset * cPlaces;
}


int32_t CapPosnBasic::bound(int32_t x, int32_t max, int32_t min) {
    if (x > max) { return max; }
    if (x < min) { return min; }
    return x;
}

void CapPosnBasic::capture() {
    analogReference(_ref);
    pinMode(_drivePin, OUTPUT);

    // prep system -- make sure all of the measurements are under identical conditions
    digitalWrite(_drivePin, HIGH);
    delayMicroseconds(_readDelay);
    _sinA = analogRead(_sinApin);
    digitalWrite(_drivePin, LOW);
    delayMicroseconds(_measureDelay);

    // Read first sin phase 
    digitalWrite(_drivePin, HIGH);
    delayMicroseconds(_readDelay);
    _sinA = analogRead(_sinApin);
    digitalWrite(_drivePin, LOW);
    delayMicroseconds(_measureDelay);

    // Read second sin phase 
    digitalWrite(_drivePin, HIGH);
    delayMicroseconds(_readDelay);
    _sinB = analogRead(_sinBpin);
    digitalWrite(_drivePin, LOW);
    delayMicroseconds(_measureDelay);

    // Read first cos phase 
    digitalWrite(_drivePin, HIGH);
    delayMicroseconds(_readDelay);
    _cosA = analogRead(_cosApin);
    digitalWrite(_drivePin, LOW);
    delayMicroseconds(_measureDelay);

    // Read second cos phase 
    digitalWrite(_drivePin, HIGH);
    delayMicroseconds(_readDelay);
    _cosB = analogRead(_cosBpin);
    digitalWrite(_drivePin, LOW);

    _angle = calcAngle();

}

int32_t CapPosnBasic::calcAngle() {
    _sin = bound((((_sinA - _sinB) * _sinGain)/(_sinA + _sinB)) + _sinOffset);
    _cos = bound((((_cosA - _cosB) * _cosGain)/(_cosA + _cosB)) + _cosOffset);

    int32_t sinAngle, cosAngle;

    if ((_sin >= 0) and (_cos >= 0)) {      // angle from 0-90 deg
        sinAngle = _sin * cOneOver2Pi;
        cosAngle = -(_cos - 1) * cOneOver2Pi;
    }
    else if ((_sin >= 0) and (_cos < 0)) {  // angle from 90-180 degrees
        sinAngle = -(_sin - 2) * cOneOver2Pi;
        cosAngle = -(_cos - 1) * cOneOver2Pi;
    }
    else if ((_sin < 0) and (_cos >= 0)) {  // angle from -90 to 0 degrees
        sinAngle = _sin * cOneOver2Pi;        
        cosAngle = (_cos - 1) * cOneOver2Pi;
    }
    else {                                  // angle from -180 to -90
        sinAngle = -(_sin + 2) * cOneOver2Pi;
        cosAngle = (_cos - 1) * cOneOver2Pi;
    }
    return (sinAngle + cosAngle)/2;
}

const int16_t CapPosnBasic::cPlaces = 1000;
const int32_t CapPosnBasic::cRadDeg = (180.0f/M_PI) * CapPosnBasic::cPlaces;
const int32_t CapPosnBasic::cPi = M_PI * CapPosnBasic::cPlaces;
const int32_t CapPosnBasic::cHalfPi = CapPosnBasic::cPi/2;
const int32_t CapPosnBasic::cOneOver2Pi = (CapPosnBasic::cPlaces)/(2 * M_PI);