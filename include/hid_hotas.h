#ifndef HOTAS_H
#define HOTAS_H

#include "DynamicHID/DynamicHID.h"

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

// General desktop usages
#define USAGE_X ( 0x30 )
#define USAGE_Y ( 0x31 )
#define USAGE_Z ( 0x32 )
#define USAGE_RX ( 0x33 )
#define USAGE_RY ( 0x34 )
#define USAGE_RZ ( 0x35 )
#define USAGE_SLIDER ( 0x36 )
#define USAGE_DIAL ( 0x37 )
#define USAGE_WHEEL ( 0x38 )

// Simulation usages
#define USAGE_THROTTLE ( 0xBB )
#define USAGE_RUDDER ( 0xBA )

struct HOTAS_Axis {
    HOTAS_Axis(uint8_t type) {
        this->type = type;
    }
    uint8_t type;
    /// 0>=<1
    float value = 0;
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
        int b = button % 8;
        bitWrite(bytes[i], b, pressed);
    }

    int8_t getButtonPressed(uint8_t button) {
        if (button > numberOfButtons - 1) return -1;
        int i = button / 8;
        int b = button % 8;
        return bitRead(bytes[i], b);
    }
};

class HOTAS {
public:
    HOTAS(uint8_t hidReportId, HOTAS_Axis** axes, uint8_t axesCount, HOTAS_Axis** simulationAxes, uint8_t simulationAxesCount, HOTAS_Buttons* buttons);

    void sendState();

private:
    uint8_t hidReportId;
    uint8_t hidReportSize;

    HOTAS_Axis** axes = NULL;
    uint8_t axesCount;

    HOTAS_Axis** simulationAxes = NULL;
    uint8_t simulationAxesCount;

    HOTAS_Buttons* buttons = NULL;
};

#endif
#endif