#include <FastLED.h>

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
// -----------------------------------------------------------------------------

#define NUM_LEDS       114
#define LED_PIN        6
#define LED_TYPE       WS2812B
#define COLOR_ORDER    GRB

#define BRIGHTNESS     255
#define SERIAL_RATE    115200

// Pin usado por algunas implementaciones antiguas como GND auxiliar.
// El firmware clásico de referencia utilizaba D10 para esto.
#define GROUND_PIN     10

// -----------------------------------------------------------------------------
// ADALIGHT
// -----------------------------------------------------------------------------

static const uint8_t prefix[] = { 'A', 'd', 'a' };

uint8_t hi;
uint8_t lo;
uint8_t chk;

CRGB leds[NUM_LEDS];

void setup()
{
    // -------------------------------------------------------------------------
    // GND auxiliar
    // -------------------------------------------------------------------------

    pinMode(GROUND_PIN, OUTPUT);
    digitalWrite(GROUND_PIN, LOW);

    // -------------------------------------------------------------------------
    // LEDs
    // -------------------------------------------------------------------------

    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(
        leds,
        NUM_LEDS
    );

    FastLED.setBrightness(BRIGHTNESS);

    // -------------------------------------------------------------------------
    // Inicialización LED
    // -------------------------------------------------------------------------

    // memset(leds, 0, sizeof(leds)); // Primera propuesta por ChatGPT, pero no funcionaba
    FastLED.clear(); // Fix propuesto por Gemini
    FastLED.show();

    // -------------------------------------------------------------------------
    // Serial
    // -------------------------------------------------------------------------

    Serial.begin(SERIAL_RATE);

    // Adalight handshake
    Serial.print("Ada\n");
}

void loop()
{
    uint8_t i;

    // -------------------------------------------------------------------------
    // Buscar "Ada"
    // -------------------------------------------------------------------------

    for (i = 0; i < sizeof(prefix); ++i)
    {
        while (!Serial.available())
        {
            // Esperar datos
        }

        if (prefix[i] != Serial.read())
        {
            // No coincide: volver a buscar desde el principio
            i = 0;
        }
    }

    // -------------------------------------------------------------------------
    // Leer HI, LO y CHECKSUM
    // -------------------------------------------------------------------------

    while (!Serial.available())
    {
    }

    hi = Serial.read();

    while (!Serial.available())
    {
    }

    lo = Serial.read();

    while (!Serial.available())
    {
    }

    chk = Serial.read();

    // -------------------------------------------------------------------------
    // Comprobar checksum
    // -------------------------------------------------------------------------

    if (chk != (hi ^ lo ^ 0x55))
    {
        return;
    }

    // -------------------------------------------------------------------------
    // Número de LEDs enviado por Hyperion
    //
    // Adalight:
    //
    //   LED count = (HI << 8 | LO) + 1
    //
    // -------------------------------------------------------------------------

    uint16_t ledCount =
        ((uint16_t)hi << 8 | lo) + 1;

    // No permitir escribir fuera del array
    if (ledCount > NUM_LEDS)
    {
        ledCount = NUM_LEDS;
    }

    // -------------------------------------------------------------------------
    // Limpiar buffer
    // -------------------------------------------------------------------------

    // memset(leds, 0, sizeof(leds)); // Primera propuesta por ChatGPT, pero no funcionaba
    FastLED.clear(); // Fix propuesto por Gemini
    FastLED.clear();

    // -------------------------------------------------------------------------
    // Recibir RGB
    // -------------------------------------------------------------------------

    for (uint16_t i = 0; i < ledCount; ++i)
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;

        while (!Serial.available())
        {
        }

        r = Serial.read();

        while (!Serial.available())
        {
        }

        g = Serial.read();

        while (!Serial.available())
        {
        }

        b = Serial.read();

        leds[i].r = r;
        leds[i].g = g;
        leds[i].b = b;
    }

    // -------------------------------------------------------------------------
    // Mostrar
    // -------------------------------------------------------------------------

    FastLED.show();
}