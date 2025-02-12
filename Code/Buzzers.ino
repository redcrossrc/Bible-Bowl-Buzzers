// Libraries
#include <FastLED.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Left side buzzer pin numbers
const int anaBP = 8;
const int nathanaelBP = 18;
const int moomooBP = 17;
const int juniorTerroristBP = 16;
const int popoBP = 15;

// Right side buzzer pin numbers
const int quinnBP = 35;
const int tylerBP = 36;
const int kolbyBP = 37;
const int jkroppBP = 38;
const int shishiBP = 39;

// Option Pins
const int clearOP = 42;
const int boxClearOP = 2;
const int buzzerPin = 3; // Passive buzzer pin

// OLED Pins
const int sdaPin = 20;
const int sclPin = 19;

// LED Information
#define LED_PIN_LEFT   6
#define LED_PIN_RIGHT  5
#define LED_PIN_BOX    4
#define NUM_LEDS       20
#define BOX_LEDS       6
#define BRIGHTNESS_ON  255
#define BRIGHTNESS_OFF 0
#define LED_TYPE       WS2812
#define COLOR_ORDER    GRB

// Create an array to hold the LED data
CRGB leds[NUM_LEDS];
CRGB boxLeds[BOX_LEDS];

// OLED display size
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

// Create display object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Initial message
String defaultMessage = "NBB Buzzers V1.0";

const int buttonPins[] = {anaBP, nathanaelBP, moomooBP, juniorTerroristBP, popoBP, quinnBP, tylerBP, kolbyBP, jkroppBP, shishiBP};
const int buttonCount = sizeof(buttonPins) / sizeof(buttonPins[0]);

volatile bool clearPressed = false;
bool buzzerPressed = false;
int buzzerID = 0;

void IRAM_ATTR clearISR() {
    clearPressed = true;
    buzzerPressed = false;
}

void setup() {
    Serial.begin(115200);
    Wire.begin(sdaPin, sclPin);
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    printCenteredText(defaultMessage);
    
    FastLED.addLeds<LED_TYPE, LED_PIN_LEFT, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.addLeds<LED_TYPE, LED_PIN_RIGHT, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.addLeds<LED_TYPE, LED_PIN_BOX, COLOR_ORDER>(boxLeds, BOX_LEDS).setCorrection(TypicalLEDStrip);
    
    for (int i = 0; i < buttonCount; i++) {
        pinMode(buttonPins[i], INPUT_PULLDOWN);
    }
    pinMode(clearOP, INPUT_PULLDOWN);
    pinMode(buzzerPin, OUTPUT);
    attachInterrupt(digitalPinToInterrupt(clearOP), clearISR, CHANGE);
}

void loop() {
    if (clearPressed) {
        clearPressed = false;
        buzzerLoop();
    }
}

void buzzerLoop() {
    FastLED.setBrightness(BRIGHTNESS_OFF);
    buzzerID = 0;
    while (!buzzerPressed) {
        for (int i = 0; i < buttonCount; i++) {
            if (digitalRead(buttonPins[i]) == HIGH) {
                buzzerPressed = false;
                buzzerID = buttonPins[i];
                tone(buzzerPin, 1000, 100); // Beep for 100ms
                whiteScreen();
                control();
                return;
            }
        }
        delay(10);
    }
}

void control() {
    FastLED.setBrightness(BRIGHTNESS_ON);
    int color = (buzzerID >= quinnBP) ? CRGB::Blue : CRGB::Yellow;
    int startIdx = (buzzerID % 5) * 4; 
    for (int i = startIdx; i <= startIdx + 3; i++) {
        leds[i] = color;
    }
    for (int i = 0; i < BOX_LEDS; i++) {
        boxLeds[i] = color;
    }
    FastLED.show();
    threeSeconds();
    printCenteredText("3 SECONDS UP");
}

void threeSeconds() {
    unsigned long startTime = millis();
    while (millis() - startTime < 3000) {
        display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_BLACK);
        display.display();
    }
    
    FastLED.clear(); // Turn off LEDs after 3 seconds
    FastLED.show();
}

void whiteScreen() {
    display.clearDisplay();
    display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
    display.display();
}

void printCenteredText(String text) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);

    int16_t x1, y1;
    uint16_t width, height;
    display.getTextBounds(text, 0, 0, &x1, &y1, &width, &height);
    
    int x = (SCREEN_WIDTH - width) / 2;
    int y = (SCREEN_HEIGHT - height) / 2;
    display.setCursor(x, y);
    display.println(text);
    display.display();
}
