#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ─── Pin Definitions ───────────────────────────────────────────────────────────
#define LOAD_CELL_DT    2
#define LOAD_CELL_SCK   3
#define H_EFFECT_D      5
#define H_EFFECT_A      A2
#define DEBUG_BUTTON    7
#define START_BUTTON    8
#define SOAP_MOSFET     9
#define WATER_MOSFET    10
#define WATER_LED3      A0
#define SOAP_LED2       A1
#define WATER_FLOW_SENSOR A3
#define I2C_SDA         A4
#define I2C_SCL         A5

// ─── Tunable Parameters ─────────────────────────────────────────────────────────
const unsigned long RINSE_1_DURATION_MS   = 3000;   // First water rinse (ms)
const unsigned long SOAP_DURATION_MS      = 3000;   // Soap cycle (ms)
const unsigned long RINSE_2_DURATION_MS   = 2000;   // Short rinse after soap (ms)
const unsigned long DRAIN_DURATION_MS     = 10000;  // Drain / nothing (ms)
const unsigned long RINSE_3_DURATION_MS   = 5000;   // Third water rinse (ms)
const unsigned long PAUSE_DURATION_MS     = 5000;   // Off pause (ms)
const unsigned long RINSE_4_DURATION_MS   = 5000;   // Final water rinse (ms)

const int   HALL_EFFECT_THRESHOLD         = 512;    // Analog threshold (0–1023)
const bool  HALL_ACTIVE_HIGH              = true;   // true = HIGH means detected

// ─── I2C Config ─────────────────────────────────────────────────────────────────
#define LCD_ADDRESS     0x27
#define ESP32_ADDRESS   0x08

LiquidCrystal_I2C lcd(LCD_ADDRESS, 16, 2);

// ─── State Machine ──────────────────────────────────────────────────────────────
enum CycleState {
  STATE_IDLE,
  STATE_RINSE_1,    // 3s water
  STATE_SOAP,       // 3s soap
  STATE_RINSE_2,    // 2s water
  STATE_DRAIN,      // 10s nothing (drain)
  STATE_RINSE_3,    // 5s water
  STATE_PAUSE,      // 5s nothing
  STATE_RINSE_4,    // 5s water
  STATE_DONE
};

CycleState currentState = STATE_IDLE;
unsigned long stateStartTime = 0;

// ─── Helper: All outputs OFF ────────────────────────────────────────────────────
void allOff() {
  digitalWrite(WATER_MOSFET, LOW);
  digitalWrite(SOAP_MOSFET,  LOW);
  digitalWrite(WATER_LED3,   LOW);
  digitalWrite(SOAP_LED2,    LOW);
}

// ─── Helper: Update LCD ─────────────────────────────────────────────────────────
void setLCDStatus(const char* line1, const char* line2 = "") {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}

// ─── Helper: Read Hall Effect Sensor ────────────────────────────────────────────
bool hallEffectDetected() {
  
    return digitalRead(H_EFFECT_D) == LOW;
}

// ─── Helper: Notify ESP32 via I2C ───────────────────────────────────────────────
void notifyESP32(const char* message) {
  Wire.beginTransmission(ESP32_ADDRESS);
  Wire.write((const uint8_t*)message, strlen(message));
  Wire.endTransmission();
}

// ─── Transition to a new state ──────────────────────────────────────────────────
void enterState(CycleState newState) {
  allOff();
  currentState = newState;
  stateStartTime = millis();

  switch (newState) {
    case STATE_IDLE:
      setLCDStatus("Ready", "Press START");
      break;

    case STATE_RINSE_1:
      setLCDStatus("Cycle: 1/7", "Rinsing...");
      digitalWrite(WATER_MOSFET, HIGH);
      digitalWrite(WATER_LED3,   HIGH);
      break;

    case STATE_SOAP:
      setLCDStatus("Cycle: 2/7", "Soaping...");
      digitalWrite(SOAP_MOSFET, HIGH);
      digitalWrite(SOAP_LED2,   HIGH);
      break;

    case STATE_RINSE_2:
      setLCDStatus("Cycle: 3/7", "Rinsing...");
      digitalWrite(WATER_MOSFET, HIGH);
      digitalWrite(WATER_LED3,   HIGH);
      break;

    case STATE_DRAIN:
      setLCDStatus("Cycle: 4/7", "Draining...");
      // All outputs already off — just wait
      break;

    case STATE_RINSE_3:
      setLCDStatus("Cycle: 5/7", "Rinsing...");
      digitalWrite(WATER_MOSFET, HIGH);
      digitalWrite(WATER_LED3,   HIGH);
      break;

    case STATE_PAUSE:
      setLCDStatus("Cycle: 6/7", "Draining...");
      // All outputs already off — just wait
      break;

    case STATE_RINSE_4:
      setLCDStatus("Cycle: 7/7", "Final Rinse...");
      digitalWrite(WATER_MOSFET, HIGH);
      digitalWrite(WATER_LED3,   HIGH);
      break;

    case STATE_DONE:
      setLCDStatus("Cycle Complete!", "");
      notifyESP32("CYCLE_DONE");
      break;
  }
}

// ─── setup() ────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(9600);

  pinMode(WATER_MOSFET, OUTPUT);
  pinMode(SOAP_MOSFET,  OUTPUT);
  pinMode(WATER_LED3,   OUTPUT);
  pinMode(SOAP_LED2,    OUTPUT);

  pinMode(START_BUTTON,  INPUT);
  pinMode(DEBUG_BUTTON,  INPUT);
  pinMode(H_EFFECT_D,    INPUT_PULLUP);

  allOff();

  Wire.begin();
  lcd.init();
  lcd.backlight();

  enterState(STATE_IDLE);
  Serial.println("System ready.");
}

// ─── loop() ─────────────────────────────────────────────────────────────────────
void loop() {
  unsigned long now = millis();

  switch (currentState) {

    case STATE_IDLE: {
      bool hallOk   = hallEffectDetected();
      bool btnPress = (digitalRead(START_BUTTON) == HIGH);
      bool debugBtn = (digitalRead(DEBUG_BUTTON) == HIGH);

      lcd.setCursor(0, 1);
      if (!hallOk && !debugBtn) {
        lcd.setCursor(0, 0);
        lcd.print("Not Ready            ");
        lcd.setCursor(0, 1);
        lcd.print("Door Open            ");
      } else if (hallOk || debugBtn){
        lcd.setCursor(0, 0);
        lcd.print("Ready - Press GO     ");
        lcd.setCursor(0, 1);
        lcd.print("Door Closed          ");
      }

      if ((hallOk && btnPress) || (debugBtn && btnPress)) {
        Serial.println("Starting cycle...");
        delay(50);
        enterState(STATE_RINSE_1);
      }
      break;
    }

    case STATE_RINSE_1:
      if (now - stateStartTime >= RINSE_1_DURATION_MS) {
        Serial.println("Rinse 1 done -> Soap");
        enterState(STATE_SOAP);
      }
      break;

    case STATE_SOAP:
      if (now - stateStartTime >= SOAP_DURATION_MS) {
        Serial.println("Soap done -> Rinse 2");
        enterState(STATE_RINSE_2);
      }
      break;

    case STATE_RINSE_2:
      if (now - stateStartTime >= RINSE_2_DURATION_MS) {
        Serial.println("Rinse 2 done -> Drain");
        enterState(STATE_DRAIN);
      }
      break;

    case STATE_DRAIN:
      if (now - stateStartTime >= DRAIN_DURATION_MS) {
        Serial.println("Drain done -> Rinse 3");
        enterState(STATE_RINSE_3);
      }
      break;

    case STATE_RINSE_3:
      if (now - stateStartTime >= RINSE_3_DURATION_MS) {
        Serial.println("Rinse 3 done -> Pause");
        enterState(STATE_PAUSE);
      }
      break;

    case STATE_PAUSE:
      if (now - stateStartTime >= PAUSE_DURATION_MS) {
        Serial.println("Pause done -> Rinse 4");
        enterState(STATE_RINSE_4);
      }
      break;

    case STATE_RINSE_4:
      if (now - stateStartTime >= RINSE_4_DURATION_MS) {
        Serial.println("Rinse 4 done -> Complete");
        enterState(STATE_DONE);
      }
      break;

    case STATE_DONE:
      if (now - stateStartTime >= 3000) {
        Serial.println("Returning to idle.");
        enterState(STATE_IDLE);
      }
      break;
  }
}