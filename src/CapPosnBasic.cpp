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

#define ANALOG_RESOLUTION (12) // This will work for any board, but may be weird for ones with only 10 bit resolution
#define INPUT_MAX (2000)
#define INPUT_MIN (0)

CapPosnBasic::CapPosnBasic(
    uint32_t drivePin,                  // The pin used to drive the system
    uint32_t sinApin,                   // The A phase of the sin half of the sense pads
    uint32_t sinBpin,                   // The B phase of the sin half of the sense pads
    uint32_t cosApin,                   // The A phase of the cos half of the sense pads
    uint32_t cosBpin,                   // The B phase of the cos half of the sense pads
    uint16_t readDelayMicros,           // Delay between charging cap & start of measurement
    uint16_t measureDelayMicros,        // Delay between phase measurements
    eAnalogReference analogRef)         // Analog reference to use 
{
    
    _drivePin       = drivePin;
    _sinApin        = sinApin;
    _sinBpin        = sinBpin;
    _cosApin        = cosApin;
    _cosBpin        = cosBpin;
    _readDelay      = readDelayMicros;
    _measureDelay   = measureDelayMicros;
    _ref            = analogRef;
    _sinGain        = 1.0;   
    _sinOffset      = 0;
    _cosGain        = 1.0;    
    _cosOffset      = 0;
    pinMode(_drivePin, OUTPUT);
    analogReference(_ref);
    analogReadResolution(ANALOG_RESOLUTION);
    setSinARange((1 << ANALOG_RESOLUTION), 0);
    setSinBRange((1 << ANALOG_RESOLUTION), 0);
    setCosARange((1 << ANALOG_RESOLUTION), 0);
    setCosBRange((1 << ANALOG_RESOLUTION), 0);
}

void CapPosnBasic::setGainsOffsets(float sinOffset, float sinGain, float cosOffset, float cosGain) {
    _sinGain    = sinGain;
    _sinOffset  = sinOffset;
    _cosGain    = cosGain;
    _cosOffset  = cosOffset;
}


float CapPosnBasic::bound(float x, float max, float min) {
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

void CapPosnBasic::setSinARange(int32_t max, int32_t min) {
    _maxSinA = max;
    _minSinA = min;
}

void CapPosnBasic::setSinBRange(int32_t max, int32_t min) {
    _maxSinB = max;
    _minSinB = min;
}

void CapPosnBasic::setCosARange(int32_t max, int32_t min) {
    _maxCosA = max;
    _minCosA = min;
}

void CapPosnBasic::setCosBRange(int32_t max, int32_t min) {
    _maxCosB = max;
    _minCosB = min;
}


float CapPosnBasic::calcAngle() {
    // map all inputs to the same ranges
    int32_t sa = map(_sinA, _minSinA, _maxSinA, INPUT_MIN, INPUT_MAX);
    int32_t sb = map(_sinB, _minSinB, _maxSinB, INPUT_MIN, INPUT_MAX);
    int32_t ca = map(_cosA, _minCosA, _maxCosA, INPUT_MIN, INPUT_MAX);
    int32_t cb = map(_cosB, _minCosB, _maxCosB, INPUT_MIN, INPUT_MAX);

    // calculate angle
    _sin = ((((sa - sb) * _sinGain)/(sa + sb)) + _sinOffset);
    _cos = ((((ca - cb) * _cosGain)/(ca + cb)) + _cosOffset);
    return atan2(_sin, _cos);
}

const float CapPosnBasic::cRadDeg = (180.0f/M_PI);
const float CapPosnBasic::cHalfPi = M_PI/2;
const float CapPosnBasic::cOneOver2Pi = 1/(2 * M_PI);