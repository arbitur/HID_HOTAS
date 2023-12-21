#ifndef HOTAS_H
#define HOTAS_H

#include "DynamicHID/DynamicHID.h"

#define USAGE_PAGE 0x05
#define USAGE 0x09
#define COLLECTION 0xA1
#define END_COLLECTION 0xC0

#define GENERAL_DESKTOP 0x01
#define SIMULATION_CONTROLS 0x02

#define JOYSTICK 0x04
#define AIRPLANE_SIMULATION_DEVICE 0x09

#if ARDUINO < 10606
#error The Joystick library requires Arduino IDE 1.6.6 or greater. Please update your IDE.
#endif // ARDUINO < 10606

#if ARDUINO > 10606
#if !defined(USBCON)
#error The Joystick library can only be used with a USB MCU (e.g. Arduino Leonardo, Arduino Micro, etc.).
#endif // !defined(USBCON)
#endif // ARDUINO > 10606

#if !defined(_USING_DYNAMIC_HID)
#warning "Using legacy HID core (non pluggable)"
#else // !defined(_USING_DYNAMIC_HID)

struct HOTAS_Axis {
    HOTAS_Axis(uint8_t type) {
        this->type = type;
    }
    uint8_t type;
    uint16_t value = 0;
};

struct HOTAS_Buttons {
    HOTAS_Buttons(uint8_t numberOfButtons) {
        this->numberOfButtons = numberOfButtons;
        this->bitsInLastByte = numberOfButtons % 8;
        this->bytesLength = numberOfButtons / 8 + (bitsInLastByte > 0);
        this->bytes = new uint8_t[bytesLength];
    };

    uint8_t numberOfButtons;
    uint8_t bitsInLastByte;
    uint8_t *bytes;
    uint8_t bytesLength;

    void setButtonPressed(uint8_t button, bool pressed) {
        if (button > numberOfButtons - 1) return;
        int i = button / 8;
        int bit = button % 8;
        bitWrite(bytes[i], bit, pressed);
    }

    void getButtonPressed(uint8_t button) {
        if (button > numberOfButtons - 1) return;
        int i = button / 8;
        int bit = button % 8;
        bitRead(bytes[i], bit);
    }
};

class HOTAS {
public:
    HOTAS(HOTAS_Axis** axes, uint8_t axesCount, HOTAS_Axis** simulationAxes, uint8_t simulationAxesCount, HOTAS_Buttons* buttons);

    void sendState();

private:
    uint8_t hidReportId = 3;
    uint8_t hidReportSize;

    HOTAS_Axis** axes = NULL;
    uint8_t axesCount;

    HOTAS_Axis** simulationAxes = NULL;
    uint8_t simulationAxesCount;

    HOTAS_Buttons* buttons = NULL;
};

#endif
#endif