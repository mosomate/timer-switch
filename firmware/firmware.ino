const int RELAY_PIN = 15;

const int LED1_PIN = 9;
const int LED2_PIN = 11;

const int BUTTON_PIN = 8;
unsigned int buttonDebouncerTime = 0;
boolean buttonPressed = false;

// Inverted logic, HIGH means idle state
int lastButtonState = HIGH;

const int MODE_OFF = 0;

const int MODE_PRESET1 = 1;
const int PRESET1_ON_TIME = 60;
const int PRESET1_OFF_TIME = 900;

const int MODE_PRESET2 = 2;
const int PRESET2_ON_TIME = 1800;
const int PRESET2_OFF_TIME = 1800;

const int MODE_ON = 3;

int currentMode = MODE_OFF;
unsigned long lastModeRanTime = 0;
int presetSecondsElapsed = 0;

unsigned long currentTime;

void setup() {
  // Set relay pin as output
  pinMode(RELAY_PIN, OUTPUT);

  // Set LED pins as outputs
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);

  // Set button pin
  pinMode(BUTTON_PIN, INPUT);

  // Enable watchdog
  _PROTECTED_WRITE(WDT.CTRLA, WDT_PERIOD_2KCLK_gc);
}

void loop() {
  currentTime = millis();

  handleModeChange();
  handleButton();
}

void handleModeChange() {
  if (buttonPressed) {
    buttonPressed = false;

    currentMode = ( currentMode < MODE_ON ? currentMode+1 : MODE_OFF );

    presetSecondsElapsed = 0;
  }

  if (currentTime < (lastModeRanTime + 1000)) return;
  
  // Run mode code in every second

  int led1State = digitalRead(LED1_PIN);
  int led2State = digitalRead(LED2_PIN);
  
  switch(currentMode) {
    case MODE_OFF:
      // Turn off relay
      digitalWrite(RELAY_PIN, LOW);

      // Blink LEDs like a crossroad
      digitalWrite(LED1_PIN, led1State == HIGH ? LOW : HIGH );
      digitalWrite(LED2_PIN, led1State);
      break;

    case MODE_PRESET1:
      digitalWrite(RELAY_PIN, presetSecondsElapsed < PRESET1_ON_TIME ? HIGH : LOW );

      // Blink only first LED
      digitalWrite(LED1_PIN, led1State == HIGH ? LOW : HIGH );
      digitalWrite(LED2_PIN, HIGH);

      // Reset or step counter
      if (presetSecondsElapsed < (PRESET1_ON_TIME + PRESET1_OFF_TIME))
        presetSecondsElapsed++;
      else
        presetSecondsElapsed = 0;
      
      break;

    case MODE_PRESET2:
      digitalWrite(RELAY_PIN, presetSecondsElapsed < PRESET2_ON_TIME ? HIGH : LOW );

      // Blink only second LED
      digitalWrite(LED1_PIN, HIGH);
      digitalWrite(LED2_PIN, led2State == HIGH ? LOW : HIGH );

      // Reset or step counter
      if (presetSecondsElapsed < (PRESET2_ON_TIME + PRESET2_OFF_TIME))
        presetSecondsElapsed++;
      else
        presetSecondsElapsed = 0;
      
      break;

    case MODE_ON:
      // Turn on relay
      digitalWrite(RELAY_PIN, HIGH);

      // Blink LEDs together
      digitalWrite(LED1_PIN, led1State == HIGH ? LOW : HIGH );
      digitalWrite(LED2_PIN, led1State == HIGH ? LOW : HIGH );
      break;

    default:
      currentMode = MODE_OFF;
      presetSecondsElapsed = 0;
      break;
  }

  lastModeRanTime = currentTime;

  // Reset watchdog
  __asm__ __volatile__ ("wdr"::);
}

void handleButton() {
  // Return if less than 10ms has elapsed from the last change of button state
  if (currentTime < (buttonDebouncerTime + 10)) return;

  // Register button press
  buttonDebouncerTime = currentTime;

  int newButtonState = digitalRead(BUTTON_PIN);

  if (lastButtonState == HIGH && newButtonState == LOW)
    buttonPressed = true;

  lastButtonState = newButtonState;
}
