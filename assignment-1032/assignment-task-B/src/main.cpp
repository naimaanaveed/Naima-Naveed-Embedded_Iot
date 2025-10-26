// Naima Naveed
// 23-NTU-CS-1032
//TASK-B


#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ==== Pin Definitions (FINAL) ====
#define BTN_MODE 16      // Push Button on GPIO 16
#define LED1 13          // LED1 (Red)
#define LED2 12          // LED2 (Blue)
#define LED3 25          // PWM LED for Fade Mode
#define BUZZER 26        // Optional: Buzzer for fade mode

// ==== OLED Setup ====
Adafruit_SSD1306 oled(128, 64, &Wire, -1);

// ==== Global Variables ====
int mode = 0;
unsigned long previousMillis = 0;
int brightness = 0;
int fadeAmount = 5;

void setup() {
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(BTN_MODE, INPUT_PULLUP);

  // Initialize OLED
  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  oled.clearDisplay();
  oled.setTextSize(2);
  oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(0, 0);
  oled.println("Mode: 0");
  oled.display();
}

void loop() {
  // Button press check
  if (digitalRead(BTN_MODE) == LOW) {
    delay(200); // debounce
    mode = (mode + 1) % 4;

    oled.clearDisplay();
    oled.setCursor(0, 0);
    oled.print("Mode: ");
    oled.println(mode);
    oled.display();
  }

  switch (mode) {
    case 0: // Both OFF
      digitalWrite(LED1, LOW);
      digitalWrite(LED2, LOW);
      digitalWrite(LED3, LOW);
      digitalWrite(BUZZER, LOW);
      break;

    case 1: // Alternate Blink
      if (millis() - previousMillis >= 500) {
        previousMillis = millis();
        digitalWrite(LED1, !digitalRead(LED1));
        digitalWrite(LED2, !digitalRead(LED2));
      }
      break;

    case 2: // Both ON
      digitalWrite(LED1, HIGH);
      digitalWrite(LED2, HIGH);
      digitalWrite(LED3, HIGH);
      digitalWrite(BUZZER, LOW); // Optional: keep off
      break;

    case 3: // PWM Fade
      analogWrite(LED3, brightness);
      analogWrite(BUZZER, brightness / 8); // Very soft tone, optional

      brightness += fadeAmount;
      if (brightness <= 0 || brightness >= 255) {
        fadeAmount = -fadeAmount;
      }
      delay(30);
      break;
  }
}