#include <FastLED.h>

/*
 * Sketch Adalight de la placa Arduino UNO original Reproducido ChatGPT pero con cambios concretos realizados
 * para la tira de LED WS2812B.
 */

/*
 * Adalight / FastLED
 *
 * Reproducción del firmware antiguo:
 *   MCU:         ATmega328P
 *   LED:         WS2812B
 *   Data:        D6
 *   Color:       GRB
 *   Serial:      115200 baud
 *   LEDs:        114
 *
 * Protocolo:
 *   "Ada"
 *   HI
 *   LO
 *   CHECKSUM
 *   RGB RGB RGB...
 */

// -----------------------------------------------------------------------------
// CONFIGURACIÓN
// ============================================================

#define NUM_LEDS       115
#define LED_PIN        6
#define GROUND_PIN     10

#define LED_TYPE       WS2812B
#define COLOR_ORDER    GRB

#define BRIGHTNESS     255
#define SERIAL_RATE    115200

// Apagar los LED tras este tiempo sin recibir frames.
#define OFF_TIMEOUT   2000

// Tiempo máximo para completar un frame interrumpido.
#define FRAME_TIMEOUT 500

CRGB leds[NUM_LEDS];

// ============================================================
// ADALIGHT
// ============================================================

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

uint32_t ledCount = 0;
uint32_t expectedBytes = 0;
uint32_t bytesReceived = 0;

uint32_t lastFrameMillis = 0;
uint32_t lastByteMillis = 0;

bool lightsOn = false;
bool validFrame = false;

// ============================================================
// SETUP
// ============================================================

void setup()
{
  pinMode(GROUND_PIN, OUTPUT);
  digitalWrite(GROUND_PIN, LOW);

  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(
    leds,
    NUM_LEDS
  );

  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();
  FastLED.show();

  Serial.begin(SERIAL_RATE);

  // Handshake Adalight
  Serial.print("Ada\n");

  lastFrameMillis = millis();
}

// ============================================================
// LOOP
// ============================================================

void loop()
{
  uint32_t now = millis();

  // ----------------------------------------------------------
  // TIMEOUT DE RECEPCIÓN
  // ----------------------------------------------------------

  if (state != WAIT_A &&
      (uint32_t)(now - lastByteMillis) >= FRAME_TIMEOUT)
  {
    state = WAIT_A;
  }

  // ----------------------------------------------------------
  // RECIBIR DATOS SERIE
  // ----------------------------------------------------------

  while (Serial.available() > 0)
  {
    uint8_t c = Serial.read();
    lastByteMillis = millis();

    switch (state)
    {
      case WAIT_A:

        if (c == 'A')
        {
          state = WAIT_D;
        }

        break;

      case WAIT_D:

        if (c == 'd')
        {
          state = WAIT_A2;
        }
        else if (c != 'A')
        {
          state = WAIT_A;
        }

        break;

      case WAIT_A2:

        if (c == 'a')
        {
          state = WAIT_HIGH;
        }
        else if (c == 'A')
        {
          state = WAIT_D;
        }
        else
        {
          state = WAIT_A;
        }

        break;

      case WAIT_HIGH:

        hi = c;
        state = WAIT_LOW;

        break;

      case WAIT_LOW:

        lo = c;
        state = WAIT_CHECKSUM;

        break;

      case WAIT_CHECKSUM:

        checksum = c;

        if (checksum == (hi ^ lo ^ 0x55))
        {
          ledCount = (((uint16_t)hi << 8) | lo) + 1;

          expectedBytes = ledCount * 3;
          bytesReceived = 0;

          validFrame = (ledCount <= NUM_LEDS);

          if (validFrame)
          {
            FastLED.clear();
          }

          state = RECEIVE_DATA;
        }
        else
        {
          state = WAIT_A;
        }

        break;

      case RECEIVE_DATA:

        if (validFrame)
        {
          uint32_t ledIndex = bytesReceived / 3;
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
        }

        bytesReceived++;

        // Frame completo
        if (bytesReceived >= expectedBytes)
        {
          if (validFrame)
          {
            FastLED.show();

            lastFrameMillis = millis();
            lightsOn = true;
          }

          state = WAIT_A;
        }

        break;
    }
  }

  // ----------------------------------------------------------
  // APAGADO AUTOMÁTICO POR INACTIVIDAD
  // ----------------------------------------------------------

  if (lightsOn &&
      (uint32_t)(millis() - lastFrameMillis) >= OFF_TIMEOUT)
  {
    FastLED.clear();
    FastLED.show();

    lightsOn = false;
  }
}