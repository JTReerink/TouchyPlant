#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "R4_Touch.h"

//Definieer de ledstrip variabelen
#define NeoPIN 4 // de pin waarop het signaal naar de strip gaat
const int NUM_LEDS = 119; // aantal ledjes op de strip
int brightness = 255;
const int aantalSensoren = 4;

// Maak een array van vier TouchSensor objecten aan
TouchSensor sensors[aantalSensoren];

// Initieer ledtrip
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, NeoPIN, NEO_GRB + NEO_KHZ800);

int ledstripParts[aantalSensoren][NUM_LEDS/aantalSensoren];


// Stel tuningswaarden in voor elke sensor
ctsu_pin_settings_t mySettings = {.div=CTSU_CLOCK_DIV_18, .gain=CTSU_ICO_GAIN_100, .ref_current=0, .offset=100, .count=3};


// Drempelwaarden en pinnen voor de vier sensoren
unsigned int threshold = 40000;
int pins[4] = {1, 2, 3, 8};

// Variabelen om de vorige status van de sensor te onthouden
bool lastTouch[aantalSensoren];

void setup() {
  // Start een serialmonitor
  Serial.begin(115200);
  Serial.println("begin");

  // Setup de ledstrip
  strip.setBrightness(brightness);
  strip.begin();
  strip.show();

  Serial.println("voor");

  for (int i=0; i<aantalSensoren; i++){
    for (int j=0; j < NUM_LEDS; j + aantalSensoren) {
      // ledstripParts[i][j];
    }
  }


  // Initialiseer elke sensor met zijn pin en drempelwaarde, en pas de instellingen toe
  for (int i = 0; i < aantalSensoren; i++) {
    sensors[i].begin(pins[i], threshold);
    sensors[i].applyPinSettings(mySettings);
  }

  // Start de capacitieve touch unit
  TouchSensor::start();
  Serial.println("einde setup");
}


// Functie om de lastTouch array te printen
void printLastTouch() {
  for (int i = 0; i < aantalSensoren; i++) {
    Serial.print(lastTouch[i]);
    if (i < aantalSensoren-1) {
      Serial.print(", "); // Komma's tussen de waarden
    }
  }
  Serial.println(); // Nieuwe regel na het printen van de array
}

uint32_t getColor(int i) {
  switch (i) {
    case 0:
      return strip.Color(255, 0, 0);

    case 1:
      return strip.Color(0, 255, 0);

    case 2:
      return strip.Color(0, 0, 255);
  }
}

void handleTouch(int i, bool touch) {
  uint32_t color = getColor(i);

  for(int j=0; j < (NUM_LEDS/aantalSensoren); j + aantalSensoren) {
    if(!touch) {
      strip.setPixelColor(ledstripParts[i][j], strip.Color(0,0,0));
      continue;
    }

    strip.setPixelColor(ledstripParts[i][j], color);
  }
}

void loop() {
  // Lees en controleer elke sensor op aanraking
  for (int i = 0; i < aantalSensoren; i++) {
    bool touch = sensors[i].read();

    // Als de status is veranderd, print dan het resultaat
    if (touch != lastTouch[i]) {
      
      lastTouch[i] = touch;

      printLastTouch();
    }
    
    handleTouch(i, touch);


  }

  
}
