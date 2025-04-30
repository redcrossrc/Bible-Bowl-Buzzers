#include "arduino_secrets.h"

// Header files
// Custom header file with neccesary variables, declarations and functions.
// These files should come with the code.
#include "Variables.h"

// LED control header file
#include <Adafruit_NeoPixel.h>

// NeoPixel objects for left/right buzzers and the separate 3-LED strip
Adafruit_NeoPixel leftStrip(NUM_LEDS_PER_SIDE, LEFT_LED_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel rightStrip(NUM_LEDS_PER_SIDE, RIGHT_LED_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel boxStrip(3, BOX_STRIP_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  // Initialize NeoPixel strips
  leftStrip.setBrightness (LED_BRIGHTNESS);
  leftStrip.begin();
  rightStrip.setBrightness (LED_BRIGHTNESS);
  rightStrip.begin();
  boxStrip.setBrightness (LED_BRIGHTNESS);
  boxStrip.begin();

  for (int i = 0; i < buttonCount; i++) {  // This sets all buzzer button pins as inputs with a pulldown resistor
    pinMode(buttonPins[i], INPUT_PULLDOWN);
  }
  
  pinMode(clearOP, INPUT_PULLDOWN);
  
  pinMode(buzzerPin, OUTPUT);              // This sets the pin for the passive buzzer that makes the beep sound as an output

  ClearLeds();
  
  // Start the FreeRTOS task on core 0 to run control()
  xTaskCreatePinnedToCore(
    ControlTask,           // Function to run
    "Control Task",        // Task name
    2048,                  // Stack size
    NULL,                  // Parameters
    1,                     // Task priority
    NULL,                  // Task handle
    0                      // Run on core 0
  );
  
  // Start the FreeRTOS task on core 1 to run buzzerLoop()
  xTaskCreatePinnedToCore(
    BuzzerTask,            // Function to run
    "Buzzer Task",         // Task name
    2048,                  // Stack size
    NULL,                  // Parameters
    1,                     // Task priority
    NULL,                  // Task handle
    1                      // Run on core 1
  );

}

void loop(){ // Empty Loop. Arduino will not compile without loop(), because stupidity.
  // I guess I could use this for something, but it makes more sense to name my functions myself.
}

void ControlTask(void *pvParameters) {
  while (true) {
    Control();       // Continuously run Control in one task
    vTaskDelay(10);  // Small delay to yield control
  }
}

void BuzzerTask(void *pvParameters) {
  while (true) {
    BuzzerLoop();  // Continuously run BuzzerLoop in one task
    vTaskDelay(10);  // Small delay to yield control
  }
}

void BuzzerLoop() {
  if (buzzerPressed) {
    startTime = millis();
    baseIndex = (whichBuzzer) * BUZZER_LEDS;
    redTriggered = false;
    
    elapsed = millis() - startTime;
    PlayLedAnimation();
    
    digitalWrite(buzzerPin, HIGH);
    delay(500);
    digitalWrite(buzzerPin, LOW);
    
    // Only proceed with animation if buzzerPressed is still true
    // This loop will keep core 1 occupied until buzzerPressed becomes false,
    // so the above commands only run once each time the buzzer is pressed
    while (buzzerPressed) {
      elapsed = millis() - startTime;
      PlayLedAnimation();
    }
  }

  int leftPressed[4];
  int rightPressed[4];
  int leftCount = 0;
  int rightCount = 0;
  
  // Separate pressed buzzers by side
  for (int i = 0; i < buttonCount; i++) {
    if (digitalRead(buttonPins[i]) == HIGH) {
      if (i <= 3) {
        leftPressed[leftCount++] = i;
      } else {
        rightPressed[rightCount++] = i - 4;
      }
    }
  }

  if (leftCount == 0 && rightCount == 0) return; // No buzzers pressed

  // Handle tie-break condition (both sides have at least one buzzer pressed)
  if (leftCount > 0 && rightCount > 0) {
    int winnerSide;

    if (lastTieBreakerWinner == -1) {
      // First time: choose randomly
      winnerSide = esp_random() % 2;
    } else {
      // Alternate based on last winner
      winnerSide = 1 - lastTieBreakerWinner;
    }

    lastTieBreakerWinner = winnerSide;

    if (winnerSide == 0) { // Left wins tie
      whichSide = leftSide;
      whichBuzzer = leftPressed[esp_random() % leftCount];
    } else { // Right wins tie
      whichSide = rightSide;
      whichBuzzer = rightPressed[esp_random() % rightCount];
    }

    buzzerPressed = true;
    return;
  }

  // Only one side has buzzer(s) pressed
  if (leftCount > 0) {
    whichSide = leftSide;
    whichBuzzer = leftPressed[esp_random() % leftCount];
    buzzerPressed = true;
    return;
  }

  if (rightCount > 0) {
    whichSide = rightSide;
    whichBuzzer = rightPressed[esp_random() % rightCount];
    buzzerPressed = true;
    return;
  }
}

void Control() {                      // Checks for any control (clear, plus, minus, bonus) button to be pressed
  if (digitalRead(clearOP) == HIGH) { // Check for clear button
    buzzerPressed = false;            // Pass the buzzerPressed variable to core 1
    ClearLeds();
  }
}

void PlayLedAnimation() {
  Adafruit_NeoPixel *animStrip = (whichSide == 0) ? &leftStrip : &rightStrip;
  
  if (!redTriggered && elapsed >= FADE_TIME) {
    redTriggered = true;
    SetAllToRed();
    // Update animated buzzer LEDs:
    for (int i = 0; i < BUZZER_LEDS; i++) {
      int t = elapsed - i * FADE_TIME;
      animStrip->setPixelColor(baseIndex + i, animColor(t));
    }
  }

  if (millis() - startTime >= totalTime) {
    // After animation, set animated buzzer and 3-LED strip to blue.
    for (int i = 0; i < BUZZER_LEDS; i++)
      animStrip->setPixelColor(baseIndex + i, animStrip->Color(CLEAR_COLOR));
    for (int i = 0; i < 3; i++)
      boxStrip.setPixelColor(i, boxStrip.Color(CLEAR_COLOR));
      animStrip->show();
      boxStrip.show();
    return;
  }

  // Update animated buzzer LEDs:
  for (int i = 0; i < BUZZER_LEDS; i++) {
    int t = elapsed - i * FADE_TIME;
    animStrip->setPixelColor(baseIndex + i, animColor(t));
  }
  // Update the separate 3-LED strip:
  for (int i = 0; i < 3; i++) {
    int t = elapsed - i * FADE_TIME;
    boxStrip.setPixelColor(i, animColor(t));
  }
  animStrip->show();
  boxStrip.show();
}

// Clear LED animation function
void ClearLeds() {
  // 1. Wipe back to blank (off) with delay every 3 LEDs
  for (int i = NUM_LEDS_PER_SIDE - 1; i >= 0; i--) {
    if (i % 3 == 2 && i != NUM_LEDS_PER_SIDE - 1) {
      delay(150);  // Delay every 3 LEDs to match the spreading effect
    }
    leftStrip.setPixelColor(i, leftStrip.Color(Color_Left));  // Purple
    rightStrip.setPixelColor(i, rightStrip.Color(Color_Right));  // Orange
    leftStrip.show();
    rightStrip.show();
    delay(25);  // Small delay for the wipe effect
  }

  // 2. Box LED strip turns white and wipes from the middle to blank
  for (int i = 0; i < 3; i++) {
    boxStrip.setPixelColor(i, boxStrip.Color(OPTION_COLOR));  // White
    boxStrip.show();
    delay(100);  // Wipe effect from left to right
  }

  // Final state: All LEDs stay purple and orange, 3-LED strip is cleared
  for (int i = 0; i < NUM_LEDS_PER_SIDE; i++) {
    leftStrip.setPixelColor(i, leftStrip.Color(Color_Left));  // Purple
    rightStrip.setPixelColor(i, rightStrip.Color(Color_Right));  // Orange
  }

  leftStrip.show();
  rightStrip.show();
}

// Sets all buzzers (including the animated one) to red for both sides.
void SetAllToRed() {
  // Set left side buzzers to red
  for (int i = 0; i < NUM_LEDS_PER_SIDE; i++) {
    leftStrip.setPixelColor(i, leftStrip.Color(255, 0, 0));  // Red
  }
  
  // Set right side buzzers to red
  for (int i = 0; i < NUM_LEDS_PER_SIDE; i++) {
    rightStrip.setPixelColor(i, rightStrip.Color(255, 0, 0));  // Red
  }

  leftStrip.show();
  rightStrip.show();
}

// Returns the animation color based on time offset t (ms):
// t < 0: solid green; 0 <= t < FADE_TIME: fade green->red; t >= FADE_TIME: off.
uint32_t animColor(int t) {
  if (t < 0) return leftStrip.Color(0, 255, 0);
  if (t < FADE_TIME) {
    float p = (float)t / FADE_TIME;
    return leftStrip.Color(p * 255, (1.0 - p) * 255, 0);
  }
  return leftStrip.Color(0, 0, 0);
}