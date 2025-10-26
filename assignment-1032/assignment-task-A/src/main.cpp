//Naima Naveed
//23-NTU-CS-1032
//task - A



#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ==== Pin Definitions (UPDATED as per final wiring) ====
#define BTN_MODE 16     // Updated from 15 → 16 (same as Task B)
#define BTN_RESET 4     // No change
#define LED1 13         // No change (Red LED)
#define LED2 12         // No change (Blue LED)
#define LED3 25         // Updated from 14 → 25 (Fade LED)

// ==== OLED Setup ====
Adafruit_SSD1306 oled(128, 64, &Wire, -1);

// ==== Variables ====
int mode = 0;
unsigned long timer = 0;
bool blink = false;
int fade = 0;
int dir = 5;  // direction of fade change (+/-)

// ==== Function Prototypes ====
void showMode();
void setLEDs(int val);

void setup() {
  pinMode(BTN_MODE, INPUT_PULLUP);
  pinMode(BTN_RESET, INPUT_PULLUP);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);

  // Initialize I2C & OLED
  Wire.begin(21, 22);  // SDA=21, SCL=22 (same as before)
  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  showMode();
}

void loop() {
  // Mode change button
  if (digitalRead(BTN_MODE) == LOW) {
    delay(200);
    mode = (mode + 1) % 4;
    showMode();
  }

  // Reset button
  if (digitalRead(BTN_RESET) == LOW) {
    delay(200);
    mode = 0;
    setLEDs(0);
    showMode();
  }

  // Mode behaviors
  if (mode == 0) {
    // Mode 1: Both OFF
    setLEDs(0);
  } 
  else if (mode == 1) {
    // Mode 2: Alternate Blink
    if (millis() - timer > 500) {
      timer = millis();
      blink = !blink;
      if (blink) {
        analogWrite(LED1, 255);
        analogWrite(LED2, 0);
        analogWrite(LED3, 255);
      } else {
        analogWrite(LED1, 0);
        analogWrite(LED2, 255);
        analogWrite(LED3, 0);
      }
    }
  } 
  else if (mode == 2) {
    // Mode 3: Both ON
    setLEDs(255);
  } 
  else if (mode == 3) {
    // Mode 4: PWM Fade
    if (millis() - timer > 20) {
      timer = millis();
      fade += dir;
      if (fade >= 255 || fade <= 0) dir = -dir;
      analogWrite(LED1, fade);
      analogWrite(LED2, fade);
      analogWrite(LED3, fade);
    }
  }
}

// ==== Helper Functions ====
void setLEDs(int val) {
  analogWrite(LED1, val);
  analogWrite(LED2, val);
  analogWrite(LED3, val);
}

void showMode() {
  oled.clearDisplay();
  oled.setTextSize(2);
  oled.setTextColor(WHITE);
  oled.setCursor(0, 10);
  oled.print("MODE ");
  oled.println(mode + 1);
  oled.setTextSize(1);
  oled.setCursor(0, 40);

  if (mode == 0) oled.print("OFF");
  if (mode == 1) oled.print("BLINK");
  if (mode == 2) oled.print("ON");
  if (mode == 3) oled.print("FADE");

  oled.display();
}