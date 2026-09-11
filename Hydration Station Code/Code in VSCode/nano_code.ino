#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define LOAD_CELL_DT 2
#define LOAD_CELL_SCK 3

#define H_EFFECT_D 5
#define H_EFFECT_A A2

#define DEBUG_BUTTON 7
#define START_BUTTON 8

#define SOAP_MOSFET 9
#define WATER_MOSFET 10

#define WATER_LED3 A0
#define SOAP_LED2 A1

#define WATER_FLOW_SENSOR A3


#define I2C_SDA A4
#define I2C_SCL A5

bool debug_flag = 0;

double flowRate = 0;
volatile int flowCounter = 0; // FIX: Must be volatile since it's modified in an ISR

double flowRateCoefficient = 2.25 * 60 * (1.0/1000.0); // FIX: Use 1.0/1000.0 for double division, not int


unsigned long prevMillis = 0;
unsigned long currentMillis = 0;

int hallEffectState = 0;
double hallEffectAnalog = 0;

double hallEffectThreshold = 200;

int debugBtnState = 0;
int startBtnState = 0;

bool cleaningFlag = 0;
unsigned long cleaningStartMillis;

unsigned long firstRinseTime = 5000;
unsigned long soapCleanTime = 5000;
unsigned long secondRinseTime = 5000;


double waterPerCycle = 0.1;
double powerDrawPerCycle = 0.1;

void prints(String Desc, double value){
  if (debug_flag){
    Serial.print(Desc);
    Serial.println(value);
  }
}

void flowSensorISR(){
  flowCounter++;
}

void setup() {
 
  Wire.begin();

  pinMode(LOAD_CELL_DT, INPUT);
  pinMode(LOAD_CELL_SCK, INPUT);
  
  pinMode(H_EFFECT_D, INPUT);
  pinMode(H_EFFECT_A, INPUT);
  
  pinMode(DEBUG_BUTTON, INPUT);
  pinMode(START_BUTTON, INPUT);

  pinMode(WATER_FLOW_SENSOR, INPUT);
  
  attachInterrupt(digitalPinToInterrupt(WATER_FLOW_SENSOR), flowSensorISR, RISING);

  pinMode(SOAP_MOSFET, OUTPUT);
  pinMode(WATER_MOSFET, OUTPUT);

  pinMode(WATER_LED3, OUTPUT);
  pinMode(SOAP_LED2, OUTPUT);

  delay(100);

  if (debug_flag){
    Serial.begin(9600);
    // FIX: Missing semicolon
    Serial.println("Ready!");
  }
  
  delay(500);  
}

void cleaningCycle(unsigned long startTime){    
    unsigned long elapsedTime = currentMillis - startTime 
    // Step 1: Water ON
    if (elapsedTime < firstRinseTime){
        digitalWrite(WATER_MOSFET, HIGH);
    }
    // Step 2: Water + Soap ON
    if (firstRinseTime < elapsedTime && elapsedTime < (firstRinseTime + soapCleanTime)){
        digitalWrite(SOAP_MOSFET, HIGH);    
    }
    // Step 3: Water ON, Soap OFF
    if ((firstRinseTime + soapCleanTime) < elapsedTime && elapsedTime < (firstRinseTime + soapCleanTime + secondRinseTime)){
        digitalWrite(SOAP_MOSFET, LOW);
    }
    // Step 4: All OFF
    if((firstRinseTime + soapCleanTime + secondRinseTime) < elapsedTime){
        digitalWrite(WATER_MOSFET, LOW);
        cleaningFlag = 0;
    }
    
    
    
}

void loop() {
  hallEffectAnalog = analogRead(H_EFFECT_A);

  startBtnState = digitalRead(START_BUTTON);
  debugBtnState = digitalRead(DEBUG_BUTTON);

  
  currentMillis = millis();

  if (currentMillis - prevMillis >= 1000){
    prevMillis = currentMillis;
    
    flowRate = flowRateCoefficient * flowCounter;
    prints("Flowrate is ", flowRate);
    prints("Flowrate counter is ", flowCounter);
    flowCounter = 0;    
  }
  
  if (hallEffectAnalog >= hallEffectThreshold && hallEffectState == 0){
    hallEffectState = 1;
  } else {
    hallEffectState = 0;
  }

  if (startBtnState){
    cleaningStartMillis = millis()
    cleaningFlag = 1;  
  }
  if (startBtnState || cleaningFlag == 1){
    cleaningCycle();
  }
}