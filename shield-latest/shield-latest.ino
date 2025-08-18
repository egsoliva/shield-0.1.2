/**********************************************************************************************************
*
*   SHIELD v1.2 - Smart Hard Hat with Impact Emergency Location Detector for Instant Disaster Response
*
*   COMMENTS:
*     - This version has no threshold and only focuses on setting up the SIM800L EVB module, GPS module,
*       and the ADXL345 module (a temporary threshold is placed to test SMS and calls)
*     - The pins for the modules and the buzzer are defined in the code
*     - The range set in the code varies from -16 to +16g. This is to allow readings from heavy impact due
*       to debris or objects during an emergency
*     - The data rate set in the code is 100 Hz
*     - ADXL345 Datasheet: https://www.analog.com/media/en/technical-documentation/data-sheets/adxl345.pdf
*     - Debug thresholds and determine when the fall occurs (as of now it appears at abnormally high peaks)
*
*   ADDED:
*     - Added printDebugInfo() function to print out accel, tilt, and state
*     - Added thresholds for free-fall and impact, durations are also defined
*     - Added stages of fall and used switch statements for fall state
*
**********************************************************************************************************/

#include <Wire.h>
#include <ADXL345.h>
#include <TinyGPS++.h>
#include <Filters.h>
#include <AH/STL/cmath>     
#include <AH/Timing/MillisMicrosTimer.hpp>
#include <Filters/Butterworth.hpp>
#include <Filters/MedianFilter.hpp>

#define BUZZER 4
#define SDA 23
#define SCL 22
#define GPS_RX 17
#define GPS_TX 16
#define SIM800L_RX 19
#define SIM800L_TX 18
#define BUTTON 5
#define GPS_BAUD 9600
#define SIM800L_BAUD 9600
#define IMPACT_THRESHOLD 16.00f  
#define FREE_FALL_THRESHOLD 9.00f  
#define ORIENTATION_THRESHOLD 20.0f 

enum FallState {
  NORMAL, 
  FREE_FALL_DETECTED, 
  IMPACT_DETECTED, 
  FALL_CONFIRMED
};

FallState fallState = NORMAL;
unsigned long stateStartTime = 0;
unsigned long freeFallTime = 0;
unsigned long impactTime = 0;
float impactPeak = 0;

float current_tilt = 0;
float prev_tilt = 0;

unsigned long startMillis, currentMillis;

float x_accel_raw, y_accel_raw, z_accel_raw, accel_raw, x_filtered, 
      y_filtered, z_filtered, x_medfilt, y_medfilt, z_medfilt, accel, 
      z_accel_raw_g;

double pitch, roll, yaw, filt_pitch, filt_roll, tilt, tilt_medfilt;

const char *numbers[] = {"adviserNumber", "guardianNumber", "nurseNumber"}; // Place their numbers in the respective places

TinyGPSPlus gps;
HardwareSerial gpsSerial(2);
HardwareSerial sim800Serial(1);

ADXL345 accelerometer;

double latitude = gps.location.lat();
double longitude = gps.location.lng();

int lastState = HIGH;
int currentState;
int ctr_state = 0;

const double f_s = 45; // Sample frequency (Hz)
const double f_c = 10; // Cut-off frequency (Hz)
const double f_n = 2 * f_c / f_s; // Normalized cut-off frequency (Hz)

MedianFilter<3, float> medfilt_X = {0}; // Median filter 
MedianFilter<4, float> medfilt_Y = {0};
MedianFilter<3, float> medfilt_Z = {0};
MedianFilter<3, float> medfilt_tilt = {0};

bool detect_fall(float accel, float tilt) {
  static float prev_tilt = tilt;
  float tilt_change = abs(tilt - prev_tilt);
  prev_tilt = tilt;

  switch(fallState) {
    case NORMAL:
      if(accel < FREE_FALL_THRESHOLD) {
        freeFallTime = millis();
        fallState = FREE_FALL_DETECTED;
      }
      break;

    case FREE_FALL_DETECTED:
      if(millis() - freeFallTime > 300) {
        if(accel > IMPACT_THRESHOLD) {
          impactTime = millis();
          impactPeak = accel;
          fallState = IMPACT_DETECTED;
        }
        else if(millis() - freeFallTime > 1000) {
          fallState = NORMAL;
        }
      }
      break;

    case IMPACT_DETECTED:
      if(millis() - impactTime > 300) {
        if(tilt_change > ORIENTATION_THRESHOLD) {
          fallState = FALL_CONFIRMED;
          return true;
        } 
        else {
          fallState = NORMAL;
        }
      }
      break;
      
    case FALL_CONFIRMED:
      fallState = NORMAL;
      break;
  }
  
  return false;
}

void setup() {
  Serial.begin(115200);  
  pinMode(BUZZER, OUTPUT);
  pinMODE(BUTTON, INPUT_PULLUP);
  Wire.begin(SDA, SCL);
  accelerometer.begin();
  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX, GPS_TX);
  sim800Serial.begin(SIM800L_BAUD, SERIAL_8N1, SIM800L_RX, SIM800L_TX);
  setAccelerometerSettings();
  startMillis = millis();
}

void loop() {
  buttonState();
  readAccelerometerData();
  readGyroscopeData();
  printDebugInfo();

  if (detect_fall(accel, tilt)) {
    if ctr_state % 2 == 1 {
      tone(BUZZER, 4000, 5000);
    }
    sendEmergency();
  }
  delay(10);
}

// Create a counter that tracks the button state (whether pressed on or off)
void buttonState() {
  currentState = digitalRead(BUTTON);

  if (lastState == LOW && currentState == HIGH) {
    ctr_state ++;
  }

  lastState = currentState;
  
}

// You may modify the range and data rate (other settings can be found on the ADXL345 library)
void setAccelerometerSettings() {
  accelerometer.setRange(ADXL345_RANGE_16G);
  accelerometer.setDataRate(ADXL345_DATARATE_100HZ);
}

// Read data from accelerometer and filter noise
void readAccelerometerData() {
  Vector norm = accelerometer.readNormalize();

  x_accel_raw = norm.XAxis;
  y_accel_raw = norm.YAxis;
  z_accel_raw = norm.ZAxis + 2.00;

  x_medfilt = medfilt_X(x_accel_raw);
  y_medfilt = medfilt_Y(y_accel_raw);
  z_medfilt = medfilt_Z(z_accel_raw);
  accel = sqrt(pow(x_medfilt, 2) + pow(y_medfilt, 2) + pow(z_medfilt, 2));

  //Serial.print("X:"); Serial.print(x_medfilt, 2);
  //Serial.print(" Y:"); Serial.print(y_medfilt, 2);
  //Serial.print(" Z:"); Serial.print(z_medfilt, 2);
  //Serial.print(" Accel:"); Serial.println(accel, 2);
  //delay(10); // Send data every 10ms or at 100 Hz
}

void readGyroscopeData() {
  Vector norm = accelerometer.readNormalize();
  
  x_accel_raw = norm.XAxis;
  y_accel_raw = norm.YAxis;
  z_accel_raw = norm.ZAxis + 2.00;

  x_medfilt = medfilt_X(x_accel_raw);
  y_medfilt = medfilt_Y(y_accel_raw);
  z_medfilt = medfilt_Z(z_accel_raw);

  double ratio = z_medfilt / accel;
  ratio = constrain(ratio, -1.0, 1.0);
  tilt = acos(ratio) * 180.0 / M_PI;

  current_tilt = tilt;

  //Serial.print("Tilt:"); Serial.println(tilt, 3);
}

// Read data from GPS module (from TinyGPS++ library)
void readGPSData() {
  while(gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
    if (gps.location.isUpdated()) {
      Serial.print("Latitude:"); Serial.print(latitude, 6);
      Serial.print(" Longitude:"); Serial.print(longitude, 6);
    }
  }
}

void sendEmergency() {
  sendSMS(numbers);
  makeCall(numbers);
}

// SMS sending function
void sendSMS(const char **numbers) {
  for(int i = 0; i < 3; i++) {
    sim800Serial.println("AT+CSQ");
    delay(1000);

    sim800Serial.println("AT");
    delay(1000);
    
    sim800Serial.println("AT+CMGF=1"); // Set SMS mode to text
    delay(1000);
    
    sim800Serial.print("AT+CMGS=\""); 
    sim800Serial.print(numbers[i]); 
    sim800Serial.println("\"");
    delay(1000);

    String latstr = String(latitude, 6);
    String lngstr = String(longitude, 6);

    String text = "EMERGENCY MESSAGE ALERT";
    text += "SEND HELP IMMEDIATELY to my current location: ";
    text += "Latitude= " + latstr + "\n";
    text += "Longitude= " + lngstr + "\n";
    text += "This is an automated message. Please do not reply. meow (^.v.^)/";

    sim800Serial.print(text);
    delay(500);
    sim800Serial.write(26); // End message with Ctrl+Z
    delay(1000);
  }
}

// Send call function
void makeCall(const char **numbers) {
  for(int i = 0; i < 3; i++) {
    sim800Serial.print("ATD");
    sim800Serial.print(numbers[i]);
    sim800Serial.println(";\r\n");

    Serial.println("Making call...");
    delay(8000);

    // End call
    sim800Serial.println(F("ATH"));
    delay(1000);
  }
}

void printDebugInfo() {
  Serial.print("Accel:"); Serial.print(accel);
  Serial.print(" Tilt:"); Serial.print(tilt);
  Serial.print(" State:"); Serial.println(fallState);
}
