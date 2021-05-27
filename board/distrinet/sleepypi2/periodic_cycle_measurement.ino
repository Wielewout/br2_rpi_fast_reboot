#include "Arduino.h"
#include <EDB.h>
#include <EEPROM.h>
#include <SleepyPi2.h>
#include <PCF8523.h>

#define CMD_PI_TO_SHDWN_PIN  17 // PC3 - 0/P Handshake to request the Pi to shutdown  - Active high
#define PI_IS_RUNNING 7         // PD7 - I/P Handshake to show that the Pi is running - Active High
#define ALARM_PIN 0
#define BUTTON_PIN 1
#define LED_PIN 13
#define TIME_TO_SKIP_MEASUREMENT_LOOP 5000
#define TIME_BETWEEN_MEASUREMENTS 1000
#define NB_OF_MEASUREMENT_ITERATIONS 25
#define END_EXEC_AND_SHUT_SIGNAL "shut" // Marks the end of execution and that RasPi wants to shutdown
#define END_EXEC_AND_IDLE_SIGNAL "idle" // Marks the end of execution and that RasPi wants to run idle
#define EXEC_FROM_IDLE_SIGNAL "exec"
#define GET_MEASUREMENTS_SIGNAL "print"
#define CLEAR_MEASUREMENTS_SIGNAL "clear"
#define SEPARATOR " | "
#define MS_TO_HOURS 3600000
#define BLINK_TIME_MS 100
#define LOOP_DELAY_MS 5
#define WAIT_DELAY_MS 1

typedef enum {
  PI_OFF = 0,
  PI_ON = 1,
  PI_BOOTING,
  PI_SHUTTING_DOWN,
  PI_IDLE
} PISTATE;

PISTATE pi_state = PI_OFF;

eTIMER_TIMEBASE PeriodicTimer_Timebase = eTB_HOUR; // Timebase set to eTB_SECOND, eTB_MINUTE, eTB_HOUR
uint8_t PeriodicTimer_Value = 1; // Timer Interval in units of Timebase

tmElements_t tm;

volatile bool is_button_pressed = false;
volatile bool is_cycle_end = false;

unsigned long start_boot_time, start_exec_time, start_shut_time, start_idle_time = 0;
unsigned long boot_time, shut_time = 0;

float current_consumption = 0; // mAh
unsigned long previous_time_current_consumption = 0;

bool shutdown_after_exec = true;

int last_measurement_iteration = 0;
bool is_measuring_finished = false;

struct Measurement {
  int id;
  unsigned long boot_time;
  float boot_cons;
  unsigned long exec_time;
  float exec_cons;
  unsigned long shut_time;
  float shut_cons;
  unsigned long idle_time;
  float idle_cons;
} measurement;

void writer(unsigned long address, byte data) {
  EEPROM.write(address, data);
}

byte reader(unsigned long address) {
  return EEPROM.read(address);
}

EDB db(&writer, &reader);

void setup() {
  Serial.begin(9600);
  
  // Pi Shutdown handshake
  pinMode(PI_IS_RUNNING, INPUT);
  pinMode(CMD_PI_TO_SHDWN_PIN, OUTPUT);
  digitalWrite(CMD_PI_TO_SHDWN_PIN, LOW);	// Don't command to shutdown

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  attachInterrupt(BUTTON_PIN, button_isr, FALLING);
  attachInterrupt(ALARM_PIN, alarm_isr, FALLING);
  
  SleepyPi.enablePiPower(false);
  SleepyPi.enableExtPower(false);

  SleepyPi.rtcInit(true);

 if (db.open(sizeof(measurement)) != EDB_OK) {
    db.create(sizeof(measurement), EEPROM.length(), (unsigned int) sizeof(measurement));
 }
  
  delay(TIME_TO_SKIP_MEASUREMENT_LOOP);
  measureCycleInLoop();
}

void button_isr() {
  is_button_pressed = true;
}

void alarm_isr() {
  is_cycle_end = true;
}

void loop() {
  handleButtonPress();
  handleSerialSignals();
  delay(LOOP_DELAY_MS);
}

void handleButtonPress() {
  if (is_button_pressed && is_measuring_finished) {
    if (pi_state == PI_OFF)
      startRasPi();
    else if (pi_state == PI_ON)
      shutdownRasPi();
    else
      blinkLED();
  }
  is_button_pressed = false;
}

void handleSerialSignals() {
  if (Serial.available() > 0) {
    String incoming = Serial.readString();
    if (incoming == GET_MEASUREMENTS_SIGNAL)
      printMeasurementsOnSerial();
    else if (incoming == CLEAR_MEASUREMENTS_SIGNAL)
      clearMeasurements();
    else
      blinkLED();
  }
}

void printMeasurementsOnSerial() {
  // number of measurements
  Serial.println(db.count(), DEC);
  
  for (int recno = 1; recno <= db.count(); recno++) {
    db.readRec(recno, EDB_REC measurement);
    if (measurement.id < 10) Serial.print("  ");
    else if (measurement.id < 100) Serial.print(" ");
    Serial.print(measurement.id);
    Serial.print(SEPARATOR);
    
    Serial.print("BOOT: ");
    padTimeMeasurement(measurement.boot_time);
    Serial.print("BPWR: ");
    padPowerMeasurement(measurement.boot_cons);

    Serial.print("EXEC: ");
    padTimeMeasurement(measurement.exec_time);
    Serial.print("EPWR: ");
    padPowerMeasurement(measurement.exec_cons);

    Serial.print("SHUT: ");
    padTimeMeasurement(measurement.shut_time);
    Serial.print("SPWR: ");
    padPowerMeasurement(measurement.shut_cons);

    Serial.print("IDLE: ");
    padTimeMeasurement(measurement.idle_time);
    Serial.print("IPWR: ");
    padPowerMeasurement(measurement.idle_cons);
    
    Serial.println();
    delay(10); // let finish writing to serial
  }
}

void padTimeMeasurement(unsigned long measurement) {
  if (measurement < 10) Serial.print("    ");
  else if (measurement < 100) Serial.print("   ");
  else if (measurement < 1000) Serial.print("  ");
  else if (measurement < 10000) Serial.print(" ");
  Serial.print(measurement, DEC);
  Serial.print(" ms");
  Serial.print(SEPARATOR);
}

void padPowerMeasurement(float measurement) {
  if (measurement < 10) Serial.print("  ");
  else if (measurement < 100) Serial.print(" ");
  Serial.print(measurement, DEC);
  Serial.print(" mAh");
  Serial.print(SEPARATOR);
}

void clearMeasurements() {
  Serial.print("clearing ");
  Serial.print(db.count());
  Serial.println(" measurements from the database");

  db.clear();
}

void measureCycleInLoop() {
  for (
      last_measurement_iteration = 0;
      last_measurement_iteration < NB_OF_MEASUREMENT_ITERATIONS && !is_button_pressed;
      last_measurement_iteration++) {
    startPeriodicAlarm();
    
    unsigned long start_elapsed_time = millis();

    startRasPi();
    measurement.boot_time = getBootStopwatchTime();
    measurement.boot_cons = getCurrentConsumption();

    execRasPi();
    measurement.exec_cons = getCurrentConsumption();

    if (shutdown_after_exec) {
      shutdownRasPi();
      measurement.shut_time = getShutdownStopwatchTime();
      measurement.shut_cons = getCurrentConsumption();
    } else {
      measurement.shut_time = 0;
      measurement.shut_cons = 0;

      pi_state = PI_IDLE;
    }

    idleRasPi();
    measurement.idle_cons = getCurrentConsumption();

    finishPeriodicAlarm();

    measurement.id = db.count();
    db.appendRec(EDB_REC measurement);
    
    if (is_button_pressed) {
      last_measurement_iteration++; // this iteration still considered finished
      is_button_pressed = false; // otherwise immediate startup
      break;
    }
    unsigned long elapsed_time = millis() - start_elapsed_time;
    delay(LOOP_DELAY_MS);
  }
  is_measuring_finished = true;
  detachInterrupt(ALARM_PIN);

  if (pi_state == PI_IDLE) {
    shutdownRasPi();
  }
}

void startPeriodicAlarm() {
  SleepyPi.rtcClearInterrupts();
  is_cycle_end = false;
  SleepyPi.setTimer1(PeriodicTimer_Timebase, PeriodicTimer_Value);
}

void finishPeriodicAlarm() {
  SleepyPi.ackTimer1();
}

void startRasPi() {
  if (pi_state == PI_OFF) {
    startPiStartup();
    waitUntilPiStartup();
    stopBootStopwatch();
    pi_state = PI_ON;
  } else {
    boot_time = 0;
    resetCurrentConsumption();
  }
}

void startPiStartup() {
  SleepyPi.enablePiPower(true);
  resetCurrentConsumption();
  startBootStopwatch();
  pi_state = PI_BOOTING;
}

void waitUntilPiStartup() {
  digitalWrite(LED_PIN, HIGH);
  while (!isPiRunning()) {
    calculateCurrentConsumption();
    delay(WAIT_DELAY_MS);
  }
  digitalWrite(LED_PIN, LOW);
}

bool isPiRunning() {
  return digitalRead(PI_IS_RUNNING) > 0;
}

void execRasPi() {
  startPiExec();
  waitUntilExecDone();
  stopExecStopwatch();
  pi_state = PI_IDLE;
}

void startPiExec() {
  if (pi_state == PI_IDLE) {
    Serial.println(EXEC_FROM_IDLE_SIGNAL);
    pi_state == PI_ON;
  }
  resetCurrentConsumption();
  startExecStopwatch();
}

void waitUntilExecDone() {
  for(;;) {
    calculateCurrentConsumption();
    if (Serial.available() > 0) {
      String incoming = Serial.readString();
      if (incoming == END_EXEC_AND_SHUT_SIGNAL) {
        shutdown_after_exec = true;
        break;
      } else if (incoming == END_EXEC_AND_IDLE_SIGNAL) {
        shutdown_after_exec = false;
        break;
      }
    }
    delay(WAIT_DELAY_MS);
  }
}

void shutdownRasPi() {
  startPiShutdown();
  waitUntilPiShutdown();
  stopShutdownStopwatch();
  pi_state = PI_OFF;
  finishPiShutdown();
}

void startPiShutdown() {
  digitalWrite(CMD_PI_TO_SHDWN_PIN,HIGH);
  resetCurrentConsumption();
  startShutdownStopwatch();
  pi_state = PI_SHUTTING_DOWN;
}

void waitUntilPiShutdown() {
  digitalWrite(LED_PIN, HIGH);
  while (SleepyPi.checkPiStatus(eZero, false)) {
    calculateCurrentConsumption();
    delay(WAIT_DELAY_MS);
  }
  digitalWrite(LED_PIN, LOW);
}

void finishPiShutdown() {
  digitalWrite(LED_PIN, HIGH);
  delay(5000); // make sure finished shutting down
  SleepyPi.enablePiPower(false);
  digitalWrite(CMD_PI_TO_SHDWN_PIN,LOW);
  digitalWrite(LED_PIN, LOW);
}

void idleRasPi() {
  startPiIdle();
  waitUntilCycleEnd();
  stopIdleStopwatch();
}

void startPiIdle() {
  resetCurrentConsumption();
  startIdleStopwatch();
}

void waitUntilCycleEnd() {
  while (!is_cycle_end) {
    calculateCurrentConsumption();
    delay(WAIT_DELAY_MS);
  }
  is_cycle_end = false;
}

void rebootRasPi() {
  shutdownRasPi();
  delay(1000); // delay to startup the pi (as this works via power)
  startRasPi();
}

void startBootStopwatch() {
    start_boot_time = millis();
}

void stopBootStopwatch() {
  unsigned long stop_boot_time = millis();

  boot_time = 0;
  if (stop_boot_time >= start_boot_time)
    boot_time = stop_boot_time - start_boot_time;
}

unsigned long getBootStopwatchTime() {
  return boot_time;
}

void startExecStopwatch() {
    start_exec_time = millis();
}

void stopExecStopwatch() {
  unsigned long stop_exec_time = millis();

  unsigned long exec_time = 0;
  if (stop_exec_time >= start_exec_time)
    exec_time = stop_exec_time - start_exec_time;

  measurement.exec_time = exec_time;
}

unsigned long getExecStopwatchTime() {
  return measurement.exec_time;
}

void startShutdownStopwatch() {
  start_shut_time = millis();
}

void stopShutdownStopwatch() {
  unsigned long stop_shut_time = millis();

  shut_time = 0;
  if (stop_shut_time >= start_shut_time)
    shut_time = stop_shut_time - start_shut_time;
}

unsigned long getShutdownStopwatchTime() {
  return shut_time;
}

void startIdleStopwatch() {
    start_idle_time = millis();
}

void stopIdleStopwatch() {
  unsigned long stop_idle_time = millis();

  unsigned long idle_time = 0;
  if (stop_idle_time >= start_idle_time)
    idle_time = stop_idle_time - start_idle_time;

  measurement.idle_time = idle_time;
}

unsigned long getIdleStopwatchTime() {
  return measurement.idle_time;
}

void calculateCurrentConsumption() {
  if (previous_time_current_consumption == 0) {
    previous_time_current_consumption = millis();
  } else {
    float current_draw = SleepyPi.rpiCurrent();
    
    unsigned long now = millis();
    float elapsed_time_ms = now - previous_time_current_consumption;
    previous_time_current_consumption = now;
    
    current_consumption += (current_draw * elapsed_time_ms) / MS_TO_HOURS;
  }
}

float getCurrentConsumption() {
  return current_consumption;
}

void resetCurrentConsumption() {
  current_consumption = 0;
}

void blinkLED() {
  digitalWrite(LED_PIN, HIGH);
  delay(BLINK_TIME_MS);
  digitalWrite(LED_PIN, LOW);
}