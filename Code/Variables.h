// Left side buzzer pin numbers
const int anaBP = 21;
const int nathanaelBP = 22;
const int juniorTerroristBP = 15;
const int popoBP = 13;

// Right side buzzer pin numbers
const int tylerBP = 16;
const int kolbyBP = 17;
const int jkroppBP = 14; // Pin 5 is cursed.
const int shishiBP = 18;

// Buzzer pin array for digitalRead
const int buttonPins[] = {
anaBP, nathanaelBP, juniorTerroristBP, popoBP,
tylerBP, kolbyBP, jkroppBP, shishiBP
};
const int buttonCount = sizeof(buttonPins) / sizeof(buttonPins[0]);

bool buzzerPressed = false; // State variable to pass between cores

int whichBuzzer;

// Option Pins
const int clearOP = 27;   // Clear button on option
const int buzzerPin = 32;  // Passive buzzer for beep sound

#define LEFT_LED_PIN       23
#define RIGHT_LED_PIN      4
#define BOX_STRIP_PIN      26
#define NUM_LEDS_PER_SIDE  15  // 5 buzzers * 3 LEDs per buzzer
#define BUZZER_LEDS        3
#define COUNT_TIME         3.5
#define FADE_TIME          1000  // ms per LED transition
#define Color_Right        150, 0, 125
#define Color_Left         255, 98, 0
#define CLEAR_COLOR        0, 0, 255
#define OPTION_COLOR       0, 213, 255

#define LED_BRIGHTNESS     255

const int leftSide = 0;
const int rightSide = 1;
int whichSide;

unsigned long startTime;
unsigned long elapsed;
int totalTime = BUZZER_LEDS * FADE_TIME;
int baseIndex;
bool redTriggered;

// Variable to alternate tie-breaker results fairly
int lastTieBreakerWinner = -1;  // -1 = none yet, 0 = left, 1 = right