#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "R4_Touch.h"

// Definieer de ledstrip variabelen
#define NeoPIN 4 // de pin waarop het signaal naar de strip gaat
const int NUM_LEDS = 119; // aantal ledjes op de strip
int stripBrightness = 255;
const int aantalSensoren = 4;
bool audience = true;

// Dit bepaalt de snelheid van de golfbeweging
float waveSpeed = 0.1; // Hoe sneller de golf, hoe hoger het getal
float phase = 0; // Faseverschuiving voor de golf


// Maak een array van vier TouchSensor objecten aan
TouchSensor sensors[aantalSensoren];

// Initieer ledtrip
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, NeoPIN, NEO_GRB + NEO_KHZ800);

int ledstripParts[aantalSensoren][NUM_LEDS/aantalSensoren];
int number[aantalSensoren];


// Stel tuningswaarden in voor elke sensor
ctsu_pin_settings_t mySettings = {.div=CTSU_CLOCK_DIV_18, .gain=CTSU_ICO_GAIN_100, .ref_current=0, .offset=100, .count=3};


// Drempelwaarden en pinnen voor de vier sensoren
unsigned int threshold = 40000;
int pins[4] = {1, 2, 3, 8};

// Variabelen om de vorige status van de sensor te onthouden
bool lastTouch[aantalSensoren];

void shuffleArray(int* array, int size) {
  for (int i = size - 1; i > 0; i--) {
    int j = random(0, i + 1); // Kies een willekeurige index
    // Verwissel array[i] en array[j]
    int temp = array[i];
    array[i] = array[j];
    array[j] = temp;
  }
}

void startSequence() {
  audience = false;
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(0,255,0));
    strip.show();
    delay(100);
  }

}

void setup() {
  // Start een serialmonitor
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB
  }

  // Setup de ledstrip
  strip.setBrightness(stripBrightness);
  strip.begin();
  strip.show();


  for (int i=0; i < aantalSensoren; i++) {
    for(int j=0; j < NUM_LEDS/aantalSensoren + 1; j++) {
      if(i + j * aantalSensoren >= NUM_LEDS) {
        ledstripParts[i][j] = i + (j - 1) * aantalSensoren;
        continue;
      }
      ledstripParts[i][j] = i + j * aantalSensoren;
    }
    shuffleArray(ledstripParts[i], NUM_LEDS/aantalSensoren);

  }


  // Initialiseer elke sensor met zijn pin en drempelwaarde, en pas de instellingen toe
  for (int i = 0; i < aantalSensoren; i++) {
    sensors[i].begin(pins[i], threshold);
    sensors[i].applyPinSettings(mySettings);
  }

  // Start de capacitieve touch unit
  TouchSensor::start();

  Serial.println("setup done");
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

      
    case 3:
      return strip.Color(255, 0, 150);
  }
}

void handleTouch(int i, bool touch) {
  uint32_t color = getColor(i);

  int currentNumber = number[i];
  int maxLedsPerSensor = NUM_LEDS / aantalSensoren;

  if (!touch) {
    // Zet alle LEDs van dit segment uit als de sensor niet wordt aangeraakt
    for (int x = 0; x < maxLedsPerSensor; x++) {
      strip.setPixelColor(ledstripParts[i][x], strip.Color(0, 0, 0));
    }
  } else {
    // Zet de vorige LED uit, ook als currentNumber op 0 wordt gezet
    int previousNumber = currentNumber - 1;
    if (currentNumber == 0) {
      previousNumber = maxLedsPerSensor - 1; // als we op LED 0 zijn, zet de laatste LED uit
    }

    // Zet de vorige LED uit
    strip.setPixelColor(ledstripParts[i][previousNumber], strip.Color(0, 0, 0));

    // Zet de huidige LED aan
    strip.setPixelColor(ledstripParts[i][currentNumber], color);

    // Verhoog currentNumber en reset naar 0 als het einde is bereikt
    currentNumber++;
    if (currentNumber >= maxLedsPerSensor) {
      currentNumber = 0;
    }

    // Sla de nieuwe waarde op
    number[i] = currentNumber;
  }

  // Update de strip
  strip.show();
  delay(1);
}


void loop() {
  // Lees en controleer elke sensor op aanraking

  if (audience) {
    for (int i = 0; i < aantalSensoren; i++) {
    bool touch = sensors[i].read();

    // Als de status is veranderd, print dan het resultaat
    if (touch != lastTouch[i]) {
      
      lastTouch[i] = touch;

      printLastTouch();
    }
    
    handleTouch(i, touch);
  } 
  if (Serial.available() > 0) {
    // Lees de inkomende string tot een nieuwe lijn (\n) of een ander teken
    String input = Serial.readStringUntil('\n'); // Je kunt '\n' vervangen door een ander terminatiekarakter

    // Controleer of de ontvangen string "start" is
    if (input == "start") {
      startSequence();
    }
  }
  } else {
    
   // Loop over alle LEDs
  for (int led = 0; led < NUM_LEDS; led++) {
    // Bepaal de helderheid voor elke LED gebaseerd op de sinusgolf en de positie
    // Maak gebruik van een fase om de golf te laten bewegen
    int ledBrightness = (sin(led * 0.1 + phase) * 127 + 128); // Helderheid varieert van 0 tot 255

    // Zorg ervoor dat de helderheid binnen de grenzen blijft (5-255)
    if (ledBrightness < 5) {
      ledBrightness = 5;
    } else if (ledBrightness > 255) {
      ledBrightness = 255;
    }

    // Stel de kleur voor de LED in
    strip.setPixelColor(led, strip.Color(0, ledBrightness, 0));
  }

  // Update de fase voor de volgende iteratie, dit zorgt voor beweging in de golf
  phase -= waveSpeed; // Verander hier de richting van de golf

  // Update de LED-strip
  strip.show();
  delay(150); // Delay voor de snelheid van de animatie
  }


    // for(int i = 255; i>10; i--){
    //   strip.setBrightness(i);
    //   strip.show();
    //   i<150 ? delay(20) : delay(10);
    // }
    // for(int i = 10; i<255; i++){
    //   strip.setBrightness(i);
    //   strip.show();
    //   i<150 ? delay(20) : delay(10);
    // }
  

  
}
