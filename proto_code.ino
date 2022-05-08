




#include <HX711_ADC.h>
#if defined(ESP8266)|| defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif

#include <Servo.h>

int pos = 0;

Servo servo_9;


//pins:
const int HX711_dout_1 = 4; //mcu > HX711 dout pin
const int HX711_sck_1 = 5; //mcu > HX711 sck pin

const int HX711_dout_2 = 8; //mcu > HX711 dout pin
const int HX711_sck_2 = 9; //mcu > HX711 sck pin

//HX711 constructor:
HX711_ADC LoadCell_1(HX711_dout_1, HX711_sck_1);
HX711_ADC LoadCell_2(HX711_dout_2, HX711_sck_2);

const int calVal_eepromAdress = 0;
unsigned long t = 0;

void setup() {
  Serial.begin(57600); delay(10);
  Serial.println();
  Serial.println("Starting...");
  pinMode(LED_BUILTIN, OUTPUT);



  servo_9.attach(9, 500, 2500);

    
  LoadCell_1.begin();
  LoadCell_2.begin();
  //LoadCell_1.setReverseOutput(); //uncomment to turn a negative output value to positive
  float calibrationValue_1; // calibration value (see example file "Calibration.ino")
  calibrationValue_1 = 620; // uncomment this if you want to set the calibration value in the sketch
  float calibrationValue_2; // calibration value (see example file "Calibration.ino")
  calibrationValue_2 = 620; // uncomment this if you want to set the calibration value in the sketch

#if defined(ESP8266)|| defined(ESP32)
  //EEPROM.begin(512); // uncomment this if you use ESP8266/ESP32 and want to fetch the calibration value from eeprom
#endif
  //EEPROM.get(calVal_eepromAdress, calibrationValue); // uncomment this if you want to fetch the calibration value from eeprom

  unsigned long stabilizingtime = 2000; // preciscion right after power-up can be improved by adding a few seconds of stabilizing time
  boolean _tare = true; //set this to false if you don't want tare to be performed in the next step
  LoadCell_1.start(stabilizingtime, _tare);
  LoadCell_2.start(stabilizingtime, _tare);
  if (LoadCell_1.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
    while (1);
  }
  else {
    LoadCell_1.setCalFactor(calibrationValue_1); // set calibration value (float)
    Serial.println("Startup is complete");
  }

  if (LoadCell_2.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
    while (1);
  }
  else {
    LoadCell_2.setCalFactor(calibrationValue_2); // set calibration value (float)
    Serial.println("Startup is complete");
  }
}

void loop() {
  static boolean newDataReady_1 = 0;
  static boolean newDataReady_2 = 0;
  const int serialPrintInterval = 0; //increase value to slow down serial print activity

  // check for new data/start next conversion:
  if (LoadCell_1.update()) newDataReady_1 = true;

  // get smoothed value from the dataset:
  if (newDataReady_1) {
    if (millis() > t + serialPrintInterval) {
      float i = LoadCell_1.getData();
      
      if (i>=100){
        digitalWrite(LED_BUILTIN, HIGH);
      }
      else{
           digitalWrite(LED_BUILTIN, LOW);

          // sweep the servo from 0 to 180 degrees in steps
          // of 1 degrees
          for (pos = 0; pos <= 180; pos += 1) {
            // tell servo to go to position in variable 'pos'
            servo_9.write(pos);
            // wait 15 ms for servo to reach the position
            delay(15); // Wait for 15 millisecond(s)
          }
          for (pos = 180; pos >= 0; pos -= 1) {
            // tell servo to go to position in variable 'pos'
            servo_9.write(pos);
            // wait 15 ms for servo to reach the position
            delay(15); // Wait for 15 millisecond(s)
          }
      }
      
      Serial.print("Load_cell_1 output val: ");
      Serial.println(i);
      newDataReady_1 = 0;
      t = millis();
    }
  }


  // check for new data/start next conversion:  
  if (LoadCell_2.update()) newDataReady_2 = true;

  // get smoothed value from the dataset:
    if (newDataReady_2) {
    if (millis() > t + serialPrintInterval) {
      float j = LoadCell_2.getData();
      Serial.print("Load_cell_2 output val: ");
      Serial.println(j);
      newDataReady_2 = 0;
      t = millis();
    }
  }



/////////////////////////////////////////////////////////////////////////////////
  // receive command from serial terminal, send 't' to initiate tare operation:
  if (Serial.available() > 0) {
    char inByte = Serial.read();
    if (inByte == 't') LoadCell_1.tareNoDelay();
  }

  // check if last tare operation is complete:
  if (LoadCell_1.getTareStatus() == true) {
    Serial.println("Tare complete");
  }

    // receive command from serial terminal, send 't' to initiate tare operation:
  if (Serial.available() > 0) {
    char inByte = Serial.read();
    if (inByte == 't') LoadCell_2.tareNoDelay();
  }

  // check if last tare operation is complete:
  if (LoadCell_2.getTareStatus() == true) {
    Serial.println("Tare complete");
  }
delay(10);
}
