#include <Arduino.h>
#include <SoftwareSerial.h>
#include <FastLED.h>

const int PIN_SERIAL_RX = 15;
const int PIN_SERIAL_TX = 14;

const static uint8_t NUM_LEDS = 73;

const static uint8_t LEDS_FOOD_READY[] = {
  1, 2, 3, 4, 5, 6, 7, 8,
              53, 54, 55, 56, 57, 58, 59,
  60, 61, 62, 63, 64, 65, 66, 67, 68, 69,
  70, 71, 72
};

const static uint8_t LEDS_LOGO[] = {
  10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
  20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
  30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
  40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
  50, 51
};

SoftwareSerial rs485(PIN_SERIAL_RX, PIN_SERIAL_TX);

uint8_t hue = 0;
bool show_food_ready = 0;
bool power = 1;

String inputCommand = "";
bool commandReady = false;

CRGB leds[NUM_LEDS];

void set_group(const uint8_t* led_group, uint8_t length, CHSV value) {
  for (uint8_t i = 0; i < length; i++) {
    leds[led_group[i]] = value;
  }
}

void handle_command() {
  if (inputCommand.length() < 3 || inputCommand.charAt(1) != ':') {
    return;
  }

  char command = inputCommand.charAt(0);
  String param = inputCommand.substring(2);

  switch (command) {
    case 'P':
      if (param == "on") {
        power = true;
      } else if (param == "off") {
        power = false;
      }
    break;

    case 'S':
      if (param == "on") {
        show_food_ready = true;
      } else if (param == "off") {
        show_food_ready = false;
      }
    break;
  }
}

void setup() {
  FastLED.addLeds<APA102, MOSI, SCK, BGR>(leds, NUM_LEDS);
  FastLED.setDither(0);
  FastLED.clear();
  rs485.begin(115200);
}

void softSerialEvent() {
  while (rs485.available()) {
    char inChar = (char)rs485.read();

    if (inChar == '\r') {
      continue;
    }

    if (inChar == '\n') {
      commandReady = true;
      break;
    }

    inputCommand += inChar;
  }
}

void loop() {
  softSerialEvent();

  if (commandReady) {
    handle_command();
    inputCommand = "";
    commandReady = false;
  }

  EVERY_N_MILLIS(64) {
    if (power) {
      set_group(LEDS_LOGO, sizeof(LEDS_LOGO), CHSV(hue, 255, 255));

      if (show_food_ready) {
        set_group(LEDS_FOOD_READY, sizeof(LEDS_FOOD_READY),
                  CHSV(hue + 128 /* wrap */, 255, 255));
      } else {
        set_group(LEDS_FOOD_READY, sizeof(LEDS_FOOD_READY),
                  CHSV(0, 0, 0));
      }
      FastLED.show();
      hue += 1; // wrap
    } else {
      FastLED.clear(true);
    }
  }
}
