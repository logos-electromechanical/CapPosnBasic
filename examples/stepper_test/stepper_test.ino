#include <CapPosnBasic.h>
#include "A4988.h"

// Capacitative pins & constants

#define DRIVE       (A0)
#define COS_A       (A3)
#define COS_B       (A2)
#define SIN_A       (A5)
#define SIN_B       (A4)
#define SIN_GAIN    (1.0f/1.01f)
#define SIN_OFFSET  (0.0f)
#define COS_GAIN    (1.0f/0.85f)
#define COS_OFFSET  (-0.07f)
#define SIN_A_MAX   (2289)
#define SIN_A_MIN   (108)
#define SIN_B_MAX   (2051)
#define SIN_B_MIN   (103)
#define COS_A_MAX   (2979)
#define COS_A_MIN   (422)
#define COS_B_MAX   (2457)
#define COS_B_MIN   (566)
#define READ_DELAY  (10)    // Time to delay after the drive goes high to wait before measuring
#define MEASURE_DELAY (150)  // Time to delay after each measurement before the next
CapPosnBasic position(DRIVE, SIN_A, SIN_B, COS_A, COS_B, READ_DELAY, MEASURE_DELAY, AR_INTERNAL1V65);

// Stepper drive pins and constants

#define MOTOR_STEPS 200
#define RPM 120
#define DIR 5
#define STEP 6
#define SLEEP 9 // optional (just delete SLEEP from everywhere if not used)
#define STEP_ENABLE SCL
#define RESET 10
#define MS1 13
#define MS2 12
#define MS3 11
A4988 stepper(MOTOR_STEPS, DIR, STEP, SLEEP, MS1, MS2, MS3);

// measurement contants
#define MICROSTEP (1)                                 // set the number of microsteps per step (valid values are 1, 2, 4, 8, and 16)
#define MICROSTEPS_PER_REV (MICROSTEP * MOTOR_STEPS)  // the number of microsteps per motor revolution
#define MICROSTEPS_PER_OBSERVATION (1)                // selected so that we'll (eventually) get to all possible positions
#define STEP_DELAY (10)                               // delay this many milliseconds before advancing the next observation
#define REVS_PER_CYCLE (4)
#define TEST_CYCLES (250)                             // number of test cycles (4 revs in each direction)

// globals
int16_t current_step = 0;
int16_t test_cycle = 0;
float sin_max = -1.0;
float cos_max = -1.0;
float sin_min = 1.0;
float cos_min = 1.0;
int32_t sinA_max = 0;
int32_t sinA_min = 4096;
int32_t sinB_max = 0;
int32_t sinB_min = 4096;
int32_t cosA_max = 0;
int32_t cosA_min = 4096;
int32_t cosB_max = 0;
int32_t cosB_min = 4096;

// print the current state to Serial
void print_header() {
  Serial.print("step,");
  Serial.print("raw sine A,");
  Serial.print("raw sine B,");
  Serial.print("raw cosine A,");
  Serial.print("raw cosine B,");
  Serial.print("sine phase,");
  Serial.print("cosine phase,");
  Serial.print("angle (rad),");
  Serial.print("sine max,");
  Serial.print("sine min,");
  Serial.print("cosine max,");
  Serial.print("cosine min,");
  Serial.print("raw sine A max,");
  Serial.print("raw sine A min,");
  Serial.print("raw sine B max,");
  Serial.print("raw sine B min,");
  Serial.print("raw cosine A max,");
  Serial.print("raw cosine A min,");
  Serial.print("raw cosine B max,");
  Serial.print("raw cosine B min");

  Serial.println("");
}

void print_state() {
  Serial.print(current_step); Serial.print(",");
  Serial.print(position.rawSinA()); Serial.print(",");
  Serial.print(position.rawSinB()); Serial.print(",");
  Serial.print(position.rawCosA()); Serial.print(",");
  Serial.print(position.rawCosB()); Serial.print(",");
  Serial.print(position.sinPhase(), 4); Serial.print(",");
  Serial.print(position.cosPhase(), 4); Serial.print(",");
  Serial.print(position.angleRad(),4); Serial.print(",");
  Serial.print(sin_max); Serial.print(",");
  Serial.print(sin_min); Serial.print(",");
  Serial.print(cos_max); Serial.print(",");
  Serial.print(cos_min); Serial.print(",");
  Serial.print(sinA_max); Serial.print(",");
  Serial.print(sinA_min); Serial.print(",");
  Serial.print(sinB_max); Serial.print(",");
  Serial.print(sinB_min); Serial.print(",");
  Serial.print(cosA_max); Serial.print(",");
  Serial.print(cosA_min); Serial.print(",");
  Serial.print(cosB_max); Serial.print(",");
  Serial.print(cosB_min);

  Serial.println("");
}

void setup() {
  Serial.begin(115200);
  pinMode(STEP_ENABLE, OUTPUT);
  pinMode(DRIVE, OUTPUT);
  pinMode(RESET, OUTPUT);
  digitalWrite(STEP_ENABLE, LOW);
  digitalWrite(DRIVE, LOW);
  digitalWrite(RESET, HIGH);
  stepper.begin(RPM);
  stepper.setMicrostep(MICROSTEP); 
  analogReadResolution(12);
  position.setGainsOffsets(SIN_OFFSET, SIN_GAIN, COS_OFFSET, COS_GAIN);
  position.setSinARange(SIN_A_MAX, SIN_A_MIN);
  position.setSinBRange(SIN_B_MAX, SIN_B_MIN);
  position.setCosARange(COS_A_MAX, COS_A_MIN);
  position.setCosBRange(COS_B_MAX, COS_B_MIN);
  print_header();
}

void loop() {
  // check if we should stop the test
  if (test_cycle > TEST_CYCLES) { 
    print_header();
    while(1); 
  }
  test_cycle++;

  // calculate this once
  uint16_t measure_per_rev = round(MICROSTEPS_PER_REV/MICROSTEPS_PER_OBSERVATION);

  // forward two revolutions
  for (uint16_t i = 0; i < REVS_PER_CYCLE * measure_per_rev; i++) {
    // capture and print the measurement
    position.capture();
    
    cos_max   = max(cos_max, position.cosPhase());
    cos_min   = min(cos_min, position.cosPhase());
    sin_max   = max(sin_max, position.sinPhase());
    sin_min   = min(sin_min, position.sinPhase());
    sinA_max  = max(sinA_max, position.rawSinA());
    sinA_min  = min(sinA_min, position.rawSinA());
    sinB_max  = max(sinB_max, position.rawSinB());
    sinB_min  = min(sinB_min, position.rawSinB());
    cosA_max  = max(cosA_max, position.rawCosA());
    cosA_min  = min(cosA_min, position.rawCosA());
    cosB_max  = max(cosB_max, position.rawCosB());
    cosB_min  = min(cosB_min, position.rawCosB());

    print_state();

    // advance motor

    stepper.move(MICROSTEPS_PER_OBSERVATION);
    current_step += MICROSTEPS_PER_OBSERVATION;
    current_step = current_step % MICROSTEPS_PER_REV;
    delay(STEP_DELAY);
  }

  // reverse two revolutions
  for (uint16_t i = 0; i < REVS_PER_CYCLE * measure_per_rev; i++) {
    // capture and print the measurement
    position.capture();
    
    cos_max   = max(cos_max, position.cosPhase());
    cos_min   = min(cos_min, position.cosPhase());
    sin_max   = max(sin_max, position.sinPhase());
    sin_min   = min(sin_min, position.sinPhase());
    sinA_max  = max(sinA_max, position.rawSinA());
    sinA_min  = min(sinA_min, position.rawSinA());
    sinB_max  = max(sinB_max, position.rawSinB());
    sinB_min  = min(sinB_min, position.rawSinB());
    cosA_max  = max(cosA_max, position.rawCosA());
    cosA_min  = min(cosA_min, position.rawCosA());
    cosB_max  = max(cosB_max, position.rawCosB());
    cosB_min  = min(cosB_min, position.rawCosB());
    
    print_state();

    // advance motor

    stepper.move(-MICROSTEPS_PER_OBSERVATION);
    current_step -= MICROSTEPS_PER_OBSERVATION;
    if (current_step < 0) { current_step += MICROSTEPS_PER_REV; }
    delay(STEP_DELAY);
  }

}
