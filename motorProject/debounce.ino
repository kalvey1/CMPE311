#define buttonInput 2 // pin 2 button input
#define LED 3         // pin 3 = OC2B

// Duty cycle sequence
int dutyCycle[] = {0, 64, 128, 191, 255, 191, 128, 64};
int dutyCycleArrLen = 8;
int dutyCycleIndex = 0;

volatile uint8_t currentDuty = 0;
bool flagUpdate = false;

// -------------------- Debounce variables --------------------
bool lastButtonState = HIGH;   // pull-up, so HIGH when unpressed
bool stableButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50; // ms

// -------------------- PWM ISR --------------------
ISR(TIMER2_OVF_vect) {
  if (currentDuty > 0) PORTD |= (1 << PD3);  // LED ON at overflow
}

ISR(TIMER2_COMPB_vect) {
  if (currentDuty < 255) PORTD &= ~(1 << PD3); // LED OFF at compare match
}

// -------------------- Timer2 setup for ISR PWM --------------------
void setupTimer2PWM() {
  cli();
  DDRD |= (1 << PD3);  // pin 3 output

  // Fast PWM mode, TOP = 255
  TCCR2A = (1 << WGM20) | (1 << WGM21);
  TCCR2B = (1 << WGM22);

  // Clock prescaler = 64 (~976 Hz PWM)
  TCCR2B |= (1 << CS22);

  // Enable interrupts
  TIMSK2 |= (1 << TOIE2);    // overflow interrupt
  TIMSK2 |= (1 << OCIE2B);   // compare match B

  OCR2A = 255; // TOP
  OCR2B = 0;   // start at 0% duty

  sei();
}

// -------------------- Duty cycle update --------------------
void dutyCycleUpdate() {
  if (flagUpdate) {
    dutyCycleIndex++;
    if (dutyCycleIndex == dutyCycleArrLen) dutyCycleIndex = 0;

    currentDuty = dutyCycle[dutyCycleIndex];
    OCR2B = currentDuty;

    Serial.print("Duty Cycle set to ");
    Serial.print(currentDuty);
    Serial.println("/255");

    flagUpdate = false;
  }
}

// -------------------- Non-blocking debounce --------------------
void buttonUpdates() {
  bool reading = digitalRead(buttonInput);

  // Reset debounce timer if state changes
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
    lastButtonState = reading;
  }

  // If stable longer than debounceDelay
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != stableButtonState) {
      stableButtonState = reading;

      // Trigger on button press (LOW because using pull-up)
      if (stableButtonState == LOW) {
        flagUpdate = true;
      }
    }
  }
}

// -------------------- Task list --------------------
typedef void (*tasks)();
tasks taskList[] = { dutyCycleUpdate, buttonUpdates };

// -------------------- Setup --------------------
void setup() {
  Serial.begin(9600);
  pinMode(buttonInput, INPUT_PULLUP); // internal pull-up
  pinMode(LED, OUTPUT);

  // Ensure LED starts OFF at startup
  PORTD &= ~(1 << PD3);

  // Start at first duty cycle = 0
  dutyCycleIndex = 0;
  currentDuty = dutyCycle[dutyCycleIndex];
  OCR2B = currentDuty;

  // Initialize debounce state
  stableButtonState = digitalRead(buttonInput);
  lastButtonState = stableButtonState;
  flagUpdate = false;

  setupTimer2PWM(); // enable ISR PWM

  Serial.print("Duty Cycle set to ");
  Serial.print(currentDuty);
  Serial.println("/255");
}

// -------------------- Main loop --------------------
void loop() {
  for (int i = 0; i < 2; i++) {
    taskList[i]();
  }
}
