#include "arduino_secrets.h"

// Header files
// Custom header file with neccesary variables, declarations and functions.
// These files should come with the code.
#include "Variables.h"

// Header files for OLED control
// These should be in the standard Arduino Libraries Database
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// LED control header file
#include <Adafruit_NeoPixel.h>

// Declare the OLED display object using I2C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1); // -1 because we're using I2C

// NeoPixel objects for left/right buzzers and the separate 3-LED strip
Adafruit_NeoPixel leftStrip(NUM_LEDS_PER_SIDE, LEFT_LED_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel rightStrip(NUM_LEDS_PER_SIDE, RIGHT_LED_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel boxStrip(3, BOX_STRIP_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  // Initialize the Wire library with custom SDA and SCL pins
  Wire.begin(sdaPin, sclPin);

  // Initialize the display with SSD1306_SWITCHCAPVCC and the correct I2C address
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  // Clear the display buffer
  display.clearDisplay();

  // Set text size and color
  display.setTextSize(1); // Adjust text size
  display.setTextColor(SSD1306_WHITE); // Set text color to white
  
  PrintCenteredText("YELLOW", defaultMessage);

  // Initialize NeoPixel strips
  leftStrip.begin();
  rightStrip.begin();
  boxStrip.begin();

  for (int i = 0; i < buttonCount; i++) {  // This sets all buzzer button pins as inputs with a pulldown resistor
    pinMode(buttonPins[i], INPUT_PULLDOWN);
  }
  
  for (int i = 0; i < optionCount; i++) {  // This sets all option button pins as inputs with a pulldown resistor
    pinMode(optionPins[i], INPUT_PULLDOWN);
  }
  
  pinMode(buzzerPin, OUTPUT);              // This sets the pin for the passive buzzer that makes the beep sound as an output

  clearLEDs();
  
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
    tone(buzzerPin, 1500, 500);  // Short beep
    startTime = millis();
    baseIndex = (whichBuzzer) * BUZZER_LEDS;
    totalTime = BUZZER_LEDS * FADE_TIME;
    redTriggered = false;
    // Set up for swipe effect
    swipeEffectStarted = true;

    BlueScreen();                // Turn the entire blue part of the OLED on

    // Only proceed with animation if buzzerPressed is still true
    // This loop will keep core 1 occupied until buzzerPressed becomes false,
    // so the above two commands only run once each time the buzzer is pressed
    while (buzzerPressed) {
      elapsed = millis() - startTime;
      if (millis() - startTime < totalTime) {
        PlayAnimationWithBuzzer();
      }
      if (millis() - startTime < 3000) {
        ThreeSeconds();          // Run the 3-second wipe animation on the OLED
      }
      else {
        PlayAnimationWithBuzzer();
        PrintCenteredText("BLUE", "3 SECONDS");
      }
    // PrintCenteredText("YELLOW", String(scoreL) + " - " + String(scoreR)); // For scorekeeping
    PrintCenteredText("YELLOW", defaultMessage); // If not scorekeeping
    }
  }
  for (whichBuzzer = 0; whichBuzzer < buttonCount; whichBuzzer++) { // Check all buzzer buttons
    if (digitalRead(buttonPins[whichBuzzer]) == HIGH) {
      buzzerPressed = true; // If one has been pressed, save that info.
      if (whichBuzzer <= 4) {
        whichSide = leftSide;
      }    
      else {
        whichSide = rightSide;
        whichBuzzer -= 5;
      }
      return;
    }
  }
}

void Control() {                      // Checks for any control (clear, plus, minus, bonus) button to be pressed
  if (digitalRead(clearOP) == HIGH) { // Check for clear button
    buzzerPressed = false;            // Pass the buzzerPressed variable to core 1
    display.clearDisplay();
    clearLEDs();
  }
//  if (digitalRead(plusPin) == HIGH) { // Check if +5 button is pressed
//    AddPoints();
//  }
}

void BlueScreen() {   
  // Fill the blue section (rows 16-63) with blue (The call is "...WHITE"
  // because the code doesn't know the difference between a black-white OLED
  // and a black-blue OLED.)
  display.fillRect(0, 16, SCREEN_WIDTH, 48, SSD1306_WHITE);
  display.display();
}

void ThreeSeconds() {
  // Only run the swipe effect once
  if (swipeEffectStarted) {
    // Get the current time and calculate the elapsed time since the effect started
    unsigned long currentTime = millis();
    unsigned long elapsedTime = currentTime - startTime;

    // Ensure the swipe effect lasts for 3 seconds (3000 milliseconds)
    if (elapsedTime < 3000) {
      // Calculate the percentage of progress for the swipe effect
      float progress = float(elapsedTime) / 3000.0; // Value between 0 and 1
      
      // Calculate how much of the blue portion should be revealed
      int swipeHeight = int(progress * BLUE_PORTION_HEIGHT); // From 0 to full height

      // Clear the blue portion gradually (swipe effect)
      display.fillRect(0, BLUE_PORTION_TOP, SCREEN_WIDTH, swipeHeight, SSD1306_BLACK); // Use SSD1306_BLUE or your custom blue color

      // Print some text during the effect
      display.setCursor(10, SCREEN_HEIGHT - 10); // Place it at the bottom

      // Update the display to reflect the changes
      display.display();
    }
    else {
      // After 3 seconds, end the swipe effect
      swipeEffectStarted = false;
      // Finalize the blue portion (just in case)
      display.fillRect(0, BLUE_PORTION_TOP, SCREEN_WIDTH, BLUE_PORTION_HEIGHT, SSD1306_BLACK);
      // Ensure the text stays visible
      display.display();
    }
  }
}

// Function to print centered text on the OLED screen
void PrintCenteredText(String color, String text) {
  int textWidth = text.length() * 6; // Each character is 6 pixels wide in the default font
  int x, y;

  // Determine which section (yellow or blue) to clear and where to position the text
  if (color == "YELLOW") {
    // Clear the yellow portion (top 16 pixels)
    display.fillRect(0, 0, SCREEN_WIDTH, 16, BLACK); // Clear the yellow section

    // Center the text in the yellow section (y = 0 to 15)
    x = (SCREEN_WIDTH - textWidth) / 2; // Horizontal center
    y = (16 - 8) / 2; // Vertical center (16 pixels height, 8 pixel height font)
    
    display.setTextColor(SSD1306_WHITE); // Set text color to white (yellow section)
  } else if (color == "BLUE") {
    // Clear the blue portion (bottom 48 pixels)
    display.fillRect(0, 16, SCREEN_WIDTH, 48, BLACK); // Clear the blue section

    // Center the text in the blue section (y = 16 to 63)
    x = (SCREEN_WIDTH - textWidth) / 2; // Horizontal center
    y = 16 + (48 - 8) / 2; // Vertical center (48 pixels height, 8 pixel height font)
    
    display.setTextColor(SSD1306_WHITE); // Set text color to white (blue section)
  } else {
    return; // Invalid color input
  }

  // Print the text on the screen
  display.setCursor(x, y);
  display.print(text);
  display.display();
}

void AddPoints() {
  PrintCenteredText("YELLOW", "5 Points Added");
  score[whichBuzzer] += 5;
  delay(10);
}

void PlayAnimationWithBuzzer() {
  Adafruit_NeoPixel *animStrip = (whichSide == 0) ? &leftStrip : &rightStrip;
  totalTime = BUZZER_LEDS * FADE_TIME;
  
  if (!redTriggered && elapsed >= FADE_TIME) {
    redTriggered = true;
    setAllBuzzersToRed();
    // Update animated buzzer LEDs:
    for (int i = 0; i < BUZZER_LEDS; i++) {
      int t = elapsed - i * FADE_TIME;
      animStrip->setPixelColor(baseIndex + i, animColor(t));
    }
  }

  if (millis() - startTime >= totalTime) {
    // After animation, set animated buzzer and 3-LED strip to blue.
    for (int i = 0; i < BUZZER_LEDS; i++)
      animStrip->setPixelColor(baseIndex + i, animStrip->Color(0, 0, 200));
    for (int i = 0; i < 3; i++)
      boxStrip.setPixelColor(i, boxStrip.Color(0, 0, 200));
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
void clearLEDs() {
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
    boxStrip.setPixelColor(i, boxStrip.Color(255, 255, 255));  // White
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
void setAllBuzzersToRed() {
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

// Sets default colors: left buzzers violet, right buzzers orange.
void setDefaultColors() {
  for (int i = 0; i < NUM_LEDS_PER_SIDE; i++) {
    leftStrip.setPixelColor(i, leftStrip.Color(Color_Left));  // Violet
    rightStrip.setPixelColor(i, rightStrip.Color(Color_Right));  // Orange
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