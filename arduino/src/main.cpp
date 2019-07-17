#include <Arduino.h>
#include <SoftwareSerial.h>
#include <FastLED.h>

const int PIN_SERIAL_RX = 15;
const int PIN_SERIAL_TX = 14;

const static uint8_t NUM_LEDS = 12;

const static uint8_t LEDS_FRONT[] = {0, 2, 4, 6, 8, 10};

const static uint8_t LEDS_BACK[] = {1, 3, 5, 7, 9, 11};

SoftwareSerial rs485(PIN_SERIAL_RX, PIN_SERIAL_TX);

uint8_t hue = 0;
bool power = 1;
bool show_front = 1;
bool show_back = 1;

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

    case 'B':
      if (param == "on") {
        show_back = true;
      } else if (param == "off") {
        show_back = false;
      }
    break;

    case 'F':
      if (param == "on") {
        show_front = true;
      } else if (param == "off") {
        show_front = false;
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
      if (show_front) {
        set_group(LEDS_FRONT, sizeof(LEDS_FRONT), CHSV(hue, 255, 255));
      } else {
        set_group(LEDS_FRONT, sizeof(LEDS_FRONT), CHSV(0, 0, 0));
      }

      if (show_back) {
        set_group(LEDS_BACK, sizeof(LEDS_BACK),
                  CHSV(hue + 128 /* wrap */, 255, 255));
      } else {
        set_group(LEDS_BACK, sizeof(LEDS_BACK),
                  CHSV(0, 0, 0));
      }
      FastLED.show();
      hue += 1; // wrap
    } else {
      FastLED.clear(true);
    }
  }
}
