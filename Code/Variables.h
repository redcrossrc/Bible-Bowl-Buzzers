// Left side buzzer pin numbers
const int anaBP = 4;
const int nathanaelBP = 5;
const int moomooBP = 6;
const int juniorTerroristBP = 7;
const int popoBP = 15;

// Right side buzzer pin numbers
const int quinnBP = 42;
const int tylerBP = 41;
const int kolbyBP = 40;
const int jkroppBP = 39;
const int shishiBP = 38;

// Buzzer pin array for digitalRead
const int buttonPins[] = {
anaBP, nathanaelBP, moomooBP, juniorTerroristBP, popoBP,
quinnBP, tylerBP, kolbyBP, jkroppBP, shishiBP
};
const int buttonCount = sizeof(buttonPins) / sizeof(buttonPins[0]);

bool buzzerPressed = false; // State variable to pass between cores

int whichBuzzer;

int anaScore = 0;
int nathanaelScore = 0;
int moomooScore = 0;
int juniorTerroristScore = 0;
int popoScore = 0;
int scoreL = anaScore + nathanaelScore + moomooScore + juniorTerroristScore + popoScore;

int quinnScore = 0;
int tylerScore = 0;
int kolbyScore = 0;
int jkroppScore = 0;
int shishiScore = 0;
int scoreR = quinnScore + tylerScore + kolbyScore + jkroppScore + shishiScore;

int score[] = { // Array to handle scores
anaScore, nathanaelScore, moomooScore, juniorTerroristScore, popoScore,
quinnScore, tylerScore, kolbyScore, jkroppScore, shishiScore
};

// Option Pins
const int clearOP = 9;   // Clear button on option
const int boxClearOP = 36; // Clear button on center box
const int plusPin = 46;   // INPUT_PULLDOWN pin for detecting score addition
const int minusPin = 8;  // INPUT_PULLDOWN pin for detecting score subtraction
const int bonusPin = 3;  // INPUT_PULLDOWN pin for detecting bonuses

// Array for option buttons
const int optionPins[] = {clearOP, boxClearOP, plusPin, minusPin, bonusPin};
const int optionCount = sizeof(optionPins) / sizeof(optionPins[0]);

const int buzzerPin = 35;  // Passive buzzer for beep sound

#define LEFT_LED_PIN       1
#define RIGHT_LED_PIN      2
#define BOX_STRIP_PIN      37
#define NUM_LEDS_PER_SIDE  15  // 5 buzzers * 3 LEDs per buzzer
#define BUZZER_LEDS        3
#define FADE_TIME          1000  // ms per LED transition
#define Color_Right        255, 65, 0
#define Color_Left         255, 0, 200

const int leftSide = 0;
const int rightSide = 1;
int whichSide;

// OLED Pins
const int sdaPin = 18;
const int sclPin = 17;

// Define the I2C address of your display (0x3C is the most common)
#define OLED_ADDR 0x3C

// Define the yellow portion's height and position
#define YELLOW_PORTION_TOP 0
#define YELLOW_PORTION_HEIGHT 16
#define BLUE_PORTION_TOP YELLOW_PORTION_HEIGHT
#define BLUE_PORTION_HEIGHT (SCREEN_HEIGHT - YELLOW_PORTION_HEIGHT)

bool swipeEffectStarted = false;

// OLED display dimensions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Default message for OLED to display.
String defaultMessage = "NBB Buzzer V.1.0";

unsigned long startTime;
unsigned long elapsed;
int totalTime = BUZZER_LEDS * FADE_TIME;
int baseIndex;
bool redTriggered;