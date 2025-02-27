/* Comment this out to disable prints and save space */

/* Define MQTT host */
#define DEFAULT_MQTT_HOST "mqtt1.eoh.io"

// You should get Auth Token in the ERa App or ERa Dashboard
#define ERA_AUTH_TOKEN "ec4dbd24-8562-4129-9d46-4c3fb278c8b2"

#define PIN_SEN0 14
#define PIN_SEN1 27
#define PIN_SEN2 26
#define PIN_SERVO1 13
#define PIN_SERVO2 12
#define PIN_LED 2    // Pin out LED
#define PIN_BUTTON 0 // Pin out nút FLASH
#define PIN_MOTOR 23
#define LED_ON() digitalWrite(PIN_LED, HIGH) // bật LED
#define LED_OFF() digitalWrite(PIN_LED, LOW) // tắt LED
#define MOTOR_ON() digitalWrite(PIN_MOTOR, LOW)
#define MOTOR_OFF() digitalWrite(PIN_MOTOR, HIGH)
#define MOTOR_TONGLE() ((digitalRead(PIN_MOTOR) == 1) ? digitalWrite(PIN_MOTOR, LOW) : digitalWrite(PIN_MOTOR, HIGH))
#define EEPROM_SIZE 512


#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>
#include <ESP32Servo.h>
#include "Adafruit_TCS34725.h"
#include <Preferences.h>

extern void wifiServerHandle();
extern void wifiServerSetup();

extern Preferences preferences;
extern char *ssid_ap;
extern char *password_ap;
extern int port;