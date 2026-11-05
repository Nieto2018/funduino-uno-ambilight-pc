
#include <FastLED.h>

#define DATA_PIN 6        // Cambia este número si tu cable verde va a otro pin
#define NUM_LEDS 115
#define LED_TYPE WS2812B  // Si tu tira es SK6812 u otra, cámbialo aquí
#define COLOR_ORDER GRB   // Orden de color típico en WS2812B; si sale mal, prueba RGB

CRGB leds[NUM_LEDS];

void setup() {
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(100); // Brillo moderado para no forzar la alimentación
}

void loop() {
  /*
  fill_solid(leds, NUM_LEDS, CRGB::Red);
  FastLED.show();
  delay(1000);

  fill_solid(leds, NUM_LEDS, CRGB::Green);
  FastLED.show();
  delay(1000);
  */

  fill_solid(leds, NUM_LEDS, CRGB::Blue);
  FastLED.show();
  delay(1000);
}