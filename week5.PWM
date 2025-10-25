#include <Arduino.h>

// --- Pin definitions ---
#define LED1_PIN 18      // LED 1
#define LED2_PIN 19      // LED 2
#define BUZZER_PIN 27    // Buzzer

// --- PWM channels ---
#define LED1_CH 0
#define LED2_CH 1
#define BUZZER_CH 2

// --- PWM settings ---
#define LED_FREQ 5000
#define LED_RES 8           // 8-bit resolution (0–255)
#define BUZZER_FREQ 2000
#define BUZZER_RES 10       // 10-bit resolution (0–1023)

void setup() {
  // --- Setup LEDs ---
  ledcSetup(LED1_CH, LED_FREQ, LED_RES);
  ledcAttachPin(LED1_PIN, LED1_CH);

  ledcSetup(LED2_CH, LED_FREQ, LED_RES);
  ledcAttachPin(LED2_PIN, LED2_CH);

  // --- Setup Buzzer ---
  ledcSetup(BUZZER_CH, BUZZER_FREQ, BUZZER_RES);
  ledcAttachPin(BUZZER_PIN, BUZZER_CH);
  ledcWrite(BUZZER_CH, 512);  // 50% duty cycle (for passive buzzer)

  Serial.begin(115200);
  Serial.println("Starting 2-LED + Buzzer demo...");
}

void loop() {
  // Fade both LEDs and beep the buzzer
  for (int d = 0; d <= 255; d++) {
    ledcWrite(LED1_CH, d);           // LED1 fades up
    ledcWrite(LED2_CH, 255 - d);     // LED2 fades down (opposite)
    
    // Make buzzer beep every ~50 brightness steps
    if (d % 50 == 0) {
      ledcWriteTone(BUZZER_CH, 1500); // Play tone
      delay(100);
      ledcWrite(BUZZER_CH, 0);        // Stop tone
    }

    delay(10);
  }

  for (int d = 255; d >= 0; d--) {
    ledcWrite(LED1_CH, d);
    ledcWrite(LED2_CH, 255 - d);
    
    if (d % 50 == 0) {
      ledcWriteTone(BUZZER_CH, 1200);
      delay(100);
      ledcWrite(BUZZER_CH, 0);
    }

    delay(10);
  }
}