// Pin Definitions
const int led1Pin = 2; //LED1 digital output 2
const int led2Pin = 3; //LED2 digital output 3

unsigned long interval1 = 0;  // Default interval for LED1
unsigned long interval2 = 0;  // Default interval for LED2

unsigned long previousMillis1 = 0; //stores the previous time
unsigned long previousMillis2 = 0; //stries the previous time

bool led1State = LOW; //initial state
bool led2State = LOW; //initial state

// Serial Input State
int step = 0;           // 0 = Ask for LED, 1 = Ask for interval
int selectedLED = 0;
String input = ""; //empty string to store inputs later

// Function Prototypes
void Task_BlinkLED1(void);
void Task_BlinkLED2(void);
void Task_SerialHandler(void);

// Cyclic Executive Task Table
typedef void (*TaskPtr)(void);
TaskPtr taskTable[] = {
  Task_BlinkLED1,
  Task_BlinkLED2,
  Task_SerialHandler
};
const int NUM_TASKS = sizeof(taskTable) / sizeof(taskTable[0]);

void setup() {
  pinMode(led1Pin, OUTPUT);
  pinMode(led2Pin, OUTPUT);
  Serial.begin(9600);
  Serial.println("What LED? (1 or 2)");
}

// Cyclic Executive Loop
void loop() {
  for (int i = 0; i < NUM_TASKS; i++) {
    (*taskTable[i])();   // Call each task in round-robin order
  }
}

// Task Implementations

// LED1 Blink Task
void Task_BlinkLED1(void) {
  unsigned long currentMillis = millis();
  if (interval1 > 0 && (currentMillis - previousMillis1 >= interval1)) {
    previousMillis1 = currentMillis;
    led1State = !led1State; //toggle the LED state
    digitalWrite(led1Pin, led1State); //output the toggled state
  }
}

// LED2 Blink Task
void Task_BlinkLED2(void) {
  unsigned long currentMillis = millis();
  if (interval2 > 0 && (currentMillis - previousMillis2 >= interval2)) {
    previousMillis2 = currentMillis;
    led2State = !led2State; //toggle the LED
    digitalWrite(led2Pin, led2State); //output the toggled state
  }
}

// Serial Input Task
void Task_SerialHandler(void) {
  while (Serial.available()) {
    char read = Serial.read();

    if (read == '\n') { //if the charcacter is newline, we have read the whole input
      if (input.length() > 0) { //make sure input is not empty
        if (step == 0) { //user is selecting which LED
          selectedLED = input.toInt(); //storing as an int rather than a character
          if (selectedLED == 1 || selectedLED == 2) { //makes sure they only input 1 or 2
            Serial.println("What interval (in msec)?"); //asks for interval
            step = 1; //changes step to 1 indicating that the user will be inputing an interval
          } else {
            Serial.println("Invalid LED. Enter 1 or 2.");
          }
        } else if (step == 1) { //user is inputting an interval
          unsigned long newInterval = input.toInt(); //stores interval as an integer
          if (newInterval > 0) { //makes sure it's greater than 0
            if (selectedLED == 1) { 
              interval1 = newInterval; //stores interval in variable
            } else if (selectedLED == 2) {
              interval2 = newInterval; //stores interval in variable
            }
            Serial.println("What LED? (1 or 2)"); //re-asks for an LED and loop starts over
            step = 0; //changes step back to 0
          } else {
            Serial.println("Invalid interval. Enter a positive number.");
          }
        }
        input = ""; //resets inout string
      }
    } else {
      input += read; //adds read string into input when we have not reached a newline
    }
  }
}
