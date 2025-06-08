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

#pragma once
#include <Arduino.h>

class CapPosnBasic {
public:
    CapPosnBasic(
        uint32_t drivePin,                  // The pin used to drive the system
        uint32_t sinApin,                   // The A phase of the sin half of the sense pads
        uint32_t sinBpin,                   // The B phase of the sin half of the sense pads
        uint32_t cosApin,                   // The A phase of the cos half of the sense pads
        uint32_t cosBpin,                   // The B phase of the cos half of the sense pads
        uint16_t readDelayMicros = 30,      // Delay between charging cap & start of measurement
        uint16_t measureDelayMicros = 150,  // Delay between phase measurements
        eAnalogReference analogRef = AR_DEFAULT);    // Reference source for analog measurements 

    /*
     * Set offsets and gains 
     */
    void setGainsOffsets(float sinOffset, float sinGain, float cosOffset, float cosGain);

    /*
     *  Capture an angle measurement
     */
    void capture();

    /*
     * Read the last measured angles, in degrees 
     */
    float angleDeg() { return ((float)(_angle * cRadDeg)/cPlaces); }

    /*
     * Read the last measured angles, in radians 
     */
    float angleRad() { return ((float)_angle/cPlaces); }

    void setReadDelayMicros(uint16_t delay) { _readDelay = delay; }
    void setMeasureDelayMicros(uint16_t delay) { _measureDelay = delay; }

    // Read internal values 
    int16_t rawSinA() { return _sinA; }
    int16_t rawSinB() { return _sinB; }
    int16_t rawCosA() { return _cosA; }
    int16_t rawCosB() { return _cosB; }
    float sinPhase() { return (float)_sin/cPlaces; }
    float cosPhase() { return (float)_cos/cPlaces; }

private:
    // constants 
    static const int16_t cPlaces;
    static const int32_t cRadDeg;
    static const int32_t cHalfPi;
    static const int32_t cPi;
    static const int32_t cOneOver2Pi;

    // state variables
    int16_t _sinA;
    int16_t _sinB;
    int16_t _cosA;
    int16_t _cosB;
    int32_t _sin;
    int32_t _cos;
    int32_t _angle;

    // configuration variables 
    uint32_t _drivePin;
    uint32_t _sinApin;
    uint32_t _sinBpin;
    uint32_t _cosApin;
    uint32_t _cosBpin;
    uint16_t _readDelay;
    uint16_t _measureDelay;
    eAnalogReference _ref;
    int32_t _sinGain;
    int32_t _sinOffset;
    int32_t _cosGain;
    int32_t _cosOffset;

    // functions
    int32_t calcAngle();
    int32_t bound(int32_t x, int32_t max, int32_t min);
};