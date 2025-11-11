// === Pin Definitions ===
const int led1Pin = 2;
const int led2Pin = 3;

// === Timing and State Variables ===
unsigned long interval1 = 0;
unsigned long interval2 = 0;

unsigned long previousMillis1 = 0;
unsigned long previousMillis2 = 0;

bool led1State = LOW;
bool led2State = LOW;

// === Serial Input State ===
int step = 0;           // 0 = Ask for LED, 1 = Ask for interval
int selectedLED = 0;
String input = "";

// === Function Prototypes ===
void Task_BlinkLED1(void);
void Task_BlinkLED2(void);
void Task_SerialHandler(void);

// === Cyclic Executive Task Table ===
typedef void (*TaskPtr)(void);
TaskPtr taskTable[] = {
  Task_BlinkLED1,
  Task_BlinkLED2,
  Task_SerialHandler
};
const int NUM_TASKS = sizeof(taskTable) / sizeof(taskTable[0]);

// === Setup ===
void setup() {
  pinMode(led1Pin, OUTPUT);
  pinMode(led2Pin, OUTPUT);
  Serial.begin(9600);
  Serial.println("What LED? (1 or 2)");
}

// === Cyclic Executive Loop ===
void loop() {
  for (int i = 0; i < NUM_TASKS; i++) {
    (*taskTable[i])();   // Call each task in round-robin order
  }
}

// === Task Implementations ===

// --- LED1 Blink Task ---
void Task_BlinkLED1(void) {
  unsigned long currentMillis = millis();
  if (interval1 > 0 && (currentMillis - previousMillis1 >= interval1)) {
    previousMillis1 = currentMillis;
    led1State = !led1State;
    digitalWrite(led1Pin, led1State);
  }
}

// --- LED2 Blink Task ---
void Task_BlinkLED2(void) {
  unsigned long currentMillis = millis();
  if (interval2 > 0 && (currentMillis - previousMillis2 >= interval2)) {
    previousMillis2 = currentMillis;
    led2State = !led2State;
    digitalWrite(led2Pin, led2State);
  }
}

// --- Serial Input Task ---
void Task_SerialHandler(void) {
  while (Serial.available()) {
    char read = Serial.read();

    if (read == '\n') {
      if (input.length() > 0) {
        if (step == 0) {  // Selecting LED
          selectedLED = input.toInt();
          if (selectedLED == 1 || selectedLED == 2) {
            Serial.println("What interval (in msec)?");
            step = 1;
          } else {
            Serial.println("Invalid LED. Enter 1 or 2.");
          }
        } else if (step == 1) {  // Setting interval
          unsigned long newInterval = input.toInt();
          if (newInterval > 0) {
            if (selectedLED == 1) interval1 = newInterval;
            else if (selectedLED == 2) interval2 = newInterval;

            Serial.println("What LED? (1 or 2)");
            step = 0;
          } else {
            Serial.println("Invalid interval. Enter a positive number.");
          }
        }
        input = "";
      }
    } else {
      input += read;
    }
  }
}
