#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <debounce.h>

#define redLedPin 0
#define greenLedPin 1
#define blueLedPin 4
#define nextButtonPin 3
#define prevButtonPin 2

#define PRESET_OFF   0
#define PRESET_COLOR 1
#define PRESET_CYCLE 2

#define offPreset 0
#define cyclePreset 13
#define cycleDelay 5000
#define debounceTime 10

#define idleDelay 3000

#define maxPreset 14

// Set this to "true" to enable debugging blinks
#define enableBlink false

unsigned long idleLastEvent;

int currentPreset = 0;

int cyclePresetId = -1;
int cycleStartPreset = 0;
int cycleEndPreset = 0;
int cycleCurrentPreset = 0;
unsigned long cycleStepDelay;
unsigned long cycleLastStepTime;

// The color presets table
int presets[][4] = {
  {0,   0,   0,   PRESET_OFF  }, // Off
  {255, 0,   0,   PRESET_COLOR}, // Red
  {255, 37,  0,   PRESET_COLOR}, // Orange
  {255, 255, 0,   PRESET_COLOR}, // Yellow
  {37,  255, 37,  PRESET_COLOR}, // Light Green
  {0,   255, 0,   PRESET_COLOR}, // Green
  {0,   0,   255, PRESET_COLOR}, // Blue
  {37,  37,  255, PRESET_COLOR}, // Light Blue
  {115, 0,   255, PRESET_COLOR}, // Purple
  {128, 37,  255, PRESET_COLOR}, // Violet
  {255, 0,   37,  PRESET_COLOR}, // Pink
  {255, 37,  114, PRESET_COLOR}, // Light Pink
  {255, 255, 255, PRESET_COLOR}, // White
  {1,   12, 2000, PRESET_CYCLE}, // Slow color cycle
  {1,   12,  500, PRESET_CYCLE}, // Fast color cycle
};

void applyLedPreset(int preset) {
  int red = presets[preset][0];
  int green = presets[preset][1];
  int blue = presets[preset][2];

  analogWrite(redLedPin, red);
  analogWrite(greenLedPin, green);
  analogWrite(blueLedPin, blue);
}

void runPreset(int preset) {
  int presetType = presets[preset][3];
  if (presetType == PRESET_COLOR) {
    applyLedPreset(preset);
  }
  if (presetType == PRESET_OFF) {
    applyLedPreset(preset);  // With the current sleep mechanism, we don't behave any differently when we're off
  }
  if (presetType == PRESET_CYCLE) {
    doCycler(preset);
  }
  else {
    // If we're not doing a cycler, make sure to reset the current cycler id
    cyclePresetId = -1;
  }
}

static void updateActivity() {  // Update our activity for idle detection
  idleLastEvent = millis();
}

bool isIdle() {
  unsigned long now = millis();
  unsigned long elapsed = now - idleLastEvent;
  return (elapsed > idleDelay);
}

static void doCycler(int preset) {
  if(preset == cyclePresetId) {
    // This is an already-running cycler, so handle it appropriately
    // Check if it's time to move on to the next preset
    unsigned long now = millis();
    unsigned long elapsed = now - cycleLastStepTime;
    if (elapsed >= cycleStepDelay) {
      cycleCurrentPreset += 1;
      cycleLastStepTime = now;
    }
    if (cycleCurrentPreset > cycleEndPreset) {
      cycleCurrentPreset = cycleStartPreset;
    }
  }
  else {
    // This is a new cycler, so let's set it up
    cycleStartPreset = presets[preset][0];
    cycleEndPreset = presets[preset][1];
    cycleStepDelay = presets[preset][2];
    cycleCurrentPreset = cycleStartPreset;
    cycleLastStepTime = millis();
    cyclePresetId = preset;
  }
  updateActivity();
  applyLedPreset(cycleCurrentPreset);
}

static void incrementCurrentPreset() {
  currentPreset += 1;
  if (currentPreset > maxPreset) {
    currentPreset = 0;
  }
  runPreset(currentPreset);
}

static void decrementCurrentPreset() {
  if(currentPreset == 0) {
    currentPreset = maxPreset;
  }
  else {
    currentPreset -= 1;
  }
  runPreset(currentPreset);
}

static void buttonHandler(uint8_t btnId, uint8_t btnState) {
  updateActivity();
  if (btnState == BTN_PRESSED) {
    //blinkColor(0,0,255);
    if (btnId == nextButtonPin) {
      incrementCurrentPreset();
    }
    else {
      decrementCurrentPreset();
    }
  }
}

// Define my debounced button objects
static Button nextButton(nextButtonPin, buttonHandler);
static Button prevButton(prevButtonPin, buttonHandler);

static void pollButtons() { // !!!!!!!!!!!!!!!!!!!!! This isn't working because the controller is going back to sleep before the button handler can trigger!!!!!!!!!!!!!!!!!!!!!
  nextButton.update(digitalRead(nextButtonPin));
  prevButton.update(digitalRead(prevButtonPin));
}

void blinkOnce() {  // Blink a predetermined color
  blinkColor(255, 255, 255);
}

void blinkColor(int red, int green, int blue) {
  if(enableBlink) {
    analogWrite(redLedPin, red);
    analogWrite(greenLedPin, green);
    analogWrite(blueLedPin, blue);
    delay(100);
    analogWrite(redLedPin, 0);
    analogWrite(greenLedPin, 0);
    analogWrite(blueLedPin, 0);
  }
}

void setup() {
  // put your setup code here, to run once:
  pinMode(redLedPin, OUTPUT);
  pinMode(greenLedPin, OUTPUT);
  pinMode(blueLedPin, OUTPUT);
  pinMode(nextButtonPin, INPUT_PULLUP);
  pinMode(prevButtonPin, INPUT_PULLUP);

  attachInterrupt(nextButtonPin, pollButtons, CHANGE);
  attachInterrupt(prevButtonPin, pollButtons, CHANGE);

  updateActivity();

  // Blink once to confirm startup
  blinkOnce();
}

ISR(PCINT0_vect) {
  pollButtons();
  updateActivity();
  //blinkColor(255,0,0);
}

void idle() {
  GIMSK |= _BV(PCIE);
  PCMSK |= _BV(PCINT3); // Use pin 3 as an interrupt pin
  PCMSK |= _BV(PCINT2); // Use pin 2 as an interrupt pin
  ADCSRA &= ~_BV(ADEN); // Turn off ADC
  set_sleep_mode(SLEEP_MODE_IDLE);

  sleep_enable();
  sei();
  sleep_cpu();
  // System is now asleep until woken by a button
  
  // We've woken back up!
  cli();
  PCMSK &= ~_BV(PCINT3); // Disable interrupts on pin 3
  PCMSK &= ~_BV(PCINT2); // Disable interrupts on pin 2
  sleep_disable();
  ADCSRA |= _BV(ADEN);  // Turn ADC on
  sei();
}

void sleep() {

  //analogWrite(greenLedPin, 255);

  GIMSK |= _BV(PCIE);
  PCMSK |= _BV(PCINT3); // Use pin 3 as an interrupt pin
  PCMSK |= _BV(PCINT2); // Use pin 2 as an interrupt pin
  ADCSRA &= ~_BV(ADEN); // Turn off ADC
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);

  sleep_enable();
  sei();
  sleep_cpu();
  // System is now asleep until woken by a button
  
  // We've woken back up!
  cli();
  PCMSK &= ~_BV(PCINT3); // Disable interrupts on pin 3
  PCMSK &= ~_BV(PCINT2); // Disable interrupts on pin 2
  sleep_disable();
  ADCSRA |= _BV(ADEN);  // Turn ADC on
  sei();
}

void loop() {
  // put your main code here, to run repeatedly:

  // if not running a cycler, go to sleep
  if(isIdle()) {
    //blinkOnce();
    if(presets[currentPreset][3] == PRESET_OFF) {
      runPreset(currentPreset);
      blinkOnce();
      sleep();
    }
    else {
      runPreset(currentPreset);
      idle();
    }
  }
  //blinkOnce();
  pollButtons();
  runPreset(currentPreset);
  if(currentPreset < 0) {
    analogWrite(redLedPin, 0);
    analogWrite(greenLedPin, 255);
    analogWrite(blueLedPin, 0);
    delay(3000);
    //blinkOnce();
  }
  if(currentPreset > maxPreset) {
    analogWrite(redLedPin, 0);
    analogWrite(greenLedPin, 0);
    analogWrite(blueLedPin, 255);
    delay(3000);
    //blinkOnce();
  }
}
