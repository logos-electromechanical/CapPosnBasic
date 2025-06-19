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
    float angleDeg() { return (_angle * cRadDeg); }

    /*
     * Read the last measured angles, in radians 
     */
    float angleRad() { return _angle; }

    void setReadDelayMicros(uint16_t delay) { _readDelay = delay; }
    void setMeasureDelayMicros(uint16_t delay) { _measureDelay = delay; }

    void setSinARange(int32_t max, int32_t min);
    void setSinBRange(int32_t max, int32_t min);
    void setCosARange(int32_t max, int32_t min);
    void setCosBRange(int32_t max, int32_t min);

    // Read internal values 
    int16_t rawSinA() { return _sinA; }
    int16_t rawSinB() { return _sinB; }
    int16_t rawCosA() { return _cosA; }
    int16_t rawCosB() { return _cosB; }
    float sinPhase() { return _sin; }
    float cosPhase() { return _cos; }

private:
    // constants 
    // static const int16_t cPlaces;
    static const float cRadDeg;
    static const float cHalfPi;
    // static const int32_t cPi;
    static const float cOneOver2Pi;

    // state variables
    int16_t _sinA;
    int16_t _sinB;
    int16_t _cosA;
    int16_t _cosB;
    float _sin;
    float _cos;
    float _angle;

    // configuration variables 
    uint32_t _drivePin;
    uint32_t _sinApin;
    uint32_t _sinBpin;
    uint32_t _cosApin;
    uint32_t _cosBpin;
    uint16_t _readDelay;
    uint16_t _measureDelay;
    eAnalogReference _ref;
    float _sinGain;
    float _sinOffset;
    float _cosGain;
    float _cosOffset;
    int32_t _maxSinA;
    int32_t _minSinA;
    int32_t _maxSinB;
    int32_t _minSinB;
    int32_t _maxCosA;
    int32_t _minCosA;
    int32_t _maxCosB;
    int32_t _minCosB;

    // functions
    float calcAngle();
    float bound(float x, float max = 1.0f, float min = -1.0f);
};