#include <FastLED.h>

// ============================================================
// CONFIGURACIÓN
// ============================================================

#define LED_PIN       6
#define NUM_LEDS      115
#define BAUD_RATE     115200

#define LED_TYPE      WS2812B
#define COLOR_ORDER   GRB

CRGB leds[NUM_LEDS];

// ============================================================
// ADALIGHT
// ============================================================

#define HEADER_A      'A'
#define HEADER_d      'd'
#define HEADER_a      'a'

// Estado del receptor
enum AdalightState
{
  WAIT_A,
  WAIT_D,
  WAIT_A2,
  WAIT_HIGH,
  WAIT_LOW,
  WAIT_CHECKSUM,
  RECEIVE_DATA
};

AdalightState state = WAIT_A;

uint8_t hi;
uint8_t lo;
uint8_t checksum;

uint16_t dataLength;
uint16_t bytesReceived;

// ============================================================
// SETUP
// ============================================================

void setup()
{
  // Inicializar LEDs
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(
    leds,
    NUM_LEDS
  );

  FastLED.setBrightness(255);

  // Apagar todos los LEDs al arrancar
  FastLED.clear();
  FastLED.show();

  // Puerto serie
  Serial.begin(BAUD_RATE);
}

// ============================================================
// LOOP
// ============================================================

void loop()
{
  while (Serial.available() > 0)
  {
    uint8_t c = Serial.read();

    switch (state)
    {
      // ------------------------------------------------------
      // Esperar 'A'
      // ------------------------------------------------------

      case WAIT_A:

        if (c == HEADER_A)
        {
          state = WAIT_D;
        }

        break;

      // ------------------------------------------------------
      // Esperar 'd'
      // ------------------------------------------------------

      case WAIT_D:

        if (c == HEADER_d)
        {
          state = WAIT_A2;
        }
        else
        {
          state = WAIT_A;
        }

        break;

      // ------------------------------------------------------
      // Esperar segundo 'a'
      // ------------------------------------------------------

      case WAIT_A2:

        if (c == HEADER_a)
        {
          state = WAIT_HIGH;
        }
        else
        {
          state = WAIT_A;
        }

        break;

      // ------------------------------------------------------
      // Byte alto de longitud
      // ------------------------------------------------------

      case WAIT_HIGH:

        hi = c;
        state = WAIT_LOW;

        break;

      // ------------------------------------------------------
      // Byte bajo de longitud
      // ------------------------------------------------------

      case WAIT_LOW:

        lo = c;
        state = WAIT_CHECKSUM;

        break;

      // ------------------------------------------------------
      // Checksum
      // ------------------------------------------------------

      case WAIT_CHECKSUM:

        checksum = c;

        if ((hi ^ lo ^ 0x55) == checksum)
        {
          dataLength = (((uint16_t)hi << 8) | lo) + 1;

          // Esperamos exactamente 3 bytes por LED
          if (dataLength == NUM_LEDS)
          {
            bytesReceived = 0;
            state = RECEIVE_DATA;
          }
          else
          {
            state = WAIT_A;
          }
        }
        else
        {
          state = WAIT_A;
        }

        break;

      // ------------------------------------------------------
      // Datos de los LEDs
      // ------------------------------------------------------

      case RECEIVE_DATA:

        if (bytesReceived < NUM_LEDS * 3)
        {
          uint16_t ledIndex = bytesReceived / 3;
          uint8_t colorIndex = bytesReceived % 3;

          switch (colorIndex)
          {
            case 0:
              leds[ledIndex].r = c;
              break;

            case 1:
              leds[ledIndex].g = c;
              break;

            case 2:
              leds[ledIndex].b = c;
              break;
          }

          bytesReceived++;

          // Frame completo
          if (bytesReceived == NUM_LEDS * 3)
          {
            FastLED.show();
            state = WAIT_A;
          }
        }
        else
        {
          state = WAIT_A;
        }

        break;
    }
  }
}