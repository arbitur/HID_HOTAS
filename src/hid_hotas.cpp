#include "hid_hotas.h"

#define USAGE_PAGE ( 0x05 )
#define USAGE ( 0x09 )
#define USAGE_MINIMUM ( 0x19 )
#define USAGE_MAXIMUM ( 0x29 )
#define LOGICAL_MINIMUM ( 0x15 )
#define LOGICAL_MAXIMUM ( 0x25 )
#define LOGICAL_MAXIMUM_LONG ( 0x27 )
#define REPORT_SIZE ( 0x75 )
#define REPORT_ID ( 0x85 )
#define REPORT_COUNT ( 0x95 )
#define UNIT_EXPONENT ( 0x55 )
#define UNIT ( 0x65 )
#define D_INPUT ( 0x81 )
#define COLLECTION ( 0xA1 )
#define END_COLLECTION ( 0xC0 )

#define GENERAL_DESKTOP ( 0x01 )
#define SIMULATION_CONTROLS ( 0x02 )

#define JOYSTICK ( 0x04 )
#define AIRPLANE_SIMULATION_DEVICE ( 0x09 )

#if defined(_USING_DYNAMIC_HID)

void set1(uint8_t **ptr, uint8_t v) {
    **ptr = v;
    *ptr += 1;
}

void set2(uint8_t **ptr, uint8_t v1, uint8_t v2) {
    set1(ptr, v1);
    set1(ptr, v2);
}

void set5(uint8_t **ptr, uint8_t v1, uint8_t v2, uint8_t v3, uint8_t v4, uint8_t v5) {
    set1(ptr, v1);
    set2(ptr, v2, v3);
    set2(ptr, v4, v5);
}

HOTAS::HOTAS(uint8_t hidReportId, HOTAS_Axis** axes, uint8_t axesCount, HOTAS_Axis** simulationAxes, uint8_t simulationAxesCount, HOTAS_Buttons* buttons) {
    this->hidReportId = hidReportId;
    this->axes = axes;
    this->axesCount = axesCount;
    this->simulationAxes = simulationAxes;
    this->simulationAxesCount = simulationAxesCount;
    this->buttons = buttons;

    uint8_t tempHidReportDescriptor[150];
    uint8_t *ptr = tempHidReportDescriptor;

    set2(&ptr, USAGE_PAGE, GENERAL_DESKTOP);
    set2(&ptr, USAGE, JOYSTICK);
    set2(&ptr, COLLECTION, 0x01); // (Application)
        set2(&ptr, REPORT_ID, hidReportId);

        if (buttons != NULL) {
            set2(&ptr, USAGE_PAGE, 0x09); // (Button)
            set2(&ptr, USAGE_MINIMUM, 0x01); //  (Button 1)
            set2(&ptr, USAGE_MAXIMUM, buttons->numberOfButtons); // (Button 32)
            set2(&ptr, LOGICAL_MINIMUM, 0); // (0)
            set2(&ptr, LOGICAL_MAXIMUM, 1); // (1)
            set2(&ptr, REPORT_SIZE, 1); // (1)
            set2(&ptr, REPORT_COUNT, buttons->numberOfButtons); // (# of buttons)
            set2(&ptr, UNIT_EXPONENT, 0x00); // (0)
            set2(&ptr, UNIT, 0x00); // (None)
            set2(&ptr, D_INPUT, 0x02); // (Data,Var,Abs)
            if (buttons->bitsInLastByte > 0) {
                set2(&ptr, REPORT_SIZE, 1); // (1)
                set2(&ptr, REPORT_COUNT, 8 - buttons->bitsInLastByte); // (# of padding bits)
                set2(&ptr, D_INPUT, 0x03); // (Const,Var,Abs)
            }
        }


        if (axes != NULL && axesCount > 0) {
            set2(&ptr, USAGE_PAGE, GENERAL_DESKTOP);
            set2(&ptr, USAGE, 0x01); // (Pointer)
            set2(&ptr, LOGICAL_MINIMUM, 0x00); // (0)
            set5(&ptr, LOGICAL_MAXIMUM_LONG, 0xFF, 0xFF, 0x00, 0x00); // (65535)
            set2(&ptr, REPORT_SIZE, 16); // (16)
            set2(&ptr, REPORT_COUNT, axesCount); // (# axis)
            set2(&ptr, COLLECTION, 0x00); // (Physical)
                for (uint8_t i = 0; i < axesCount; i++) {
                    set2(&ptr, USAGE, axes[i]->type);
                }
                set2(&ptr, D_INPUT, 0x02); // (Data,Var,Abs)
            set1(&ptr, END_COLLECTION);
        }

        if (simulationAxes != NULL && simulationAxesCount > 0) {
            set2(&ptr, USAGE_PAGE, SIMULATION_CONTROLS);
            // set2(&ptr, USAGE, AIRPLANE_SIMULATION_DEVICE);
            set2(&ptr, LOGICAL_MINIMUM, 0x00); // (0)
            set5(&ptr, LOGICAL_MAXIMUM_LONG, 0xFF, 0xFF, 0x00, 0x00); // (65535)
            set2(&ptr, REPORT_SIZE, 16); // (16)
            set2(&ptr, REPORT_COUNT, simulationAxesCount); // (# of simulation axis)
            set2(&ptr, COLLECTION, 0x00); // (Physical)
                for (uint8_t i = 0; i < simulationAxesCount; i++) {
                    set2(&ptr, USAGE, simulationAxes[i]->type);
                }
                set2(&ptr, D_INPUT, 0x02); // (Data,Var,Abs)
            set1(&ptr, END_COLLECTION);
        }
    set1(&ptr, END_COLLECTION);

    int hidReportDescriptorSize = ptr - tempHidReportDescriptor;
    uint8_t *customHidReportDescriptor = new uint8_t[hidReportDescriptorSize];
	memcpy(customHidReportDescriptor, tempHidReportDescriptor, hidReportDescriptorSize);

    DynamicHIDSubDescriptor *node = new DynamicHIDSubDescriptor(customHidReportDescriptor, hidReportDescriptorSize, false);
	DynamicHID().AppendDescriptor(node);

    this->hidReportSize = 0;
    if (buttons != NULL) {
        this->hidReportSize += buttons->bytesLength;
    }
	this->hidReportSize += axesCount * 2;
	this->hidReportSize += simulationAxesCount * 2;
}

void buildAndSet16BitValue(uint8_t **ptr, uint16_t value) {
	uint8_t highByte = (uint8_t)(value >> 8);
	uint8_t lowByte = (uint8_t)(value & 0x00FF);
    set1(ptr, lowByte);
    set1(ptr, highByte);
}

void HOTAS::sendState() {
	uint8_t data[hidReportSize];
	uint8_t *ptr = data;

    if (buttons != NULL) {
        for (int i = 0; i < buttons->bytesLength; i++) {
            set1(&ptr, buttons->bytes[i]);
        }
    }

    for (uint8_t i = 0; i < axesCount; i++) {
        buildAndSet16BitValue(&ptr, axes[i]->value * 0xFFFF);
    }

    for (uint8_t i = 0; i < simulationAxesCount; i++) {
        buildAndSet16BitValue(&ptr, simulationAxes[i]->value * 0xFFFF);
    }

	DynamicHID().SendReport(hidReportId, data, hidReportSize);
}

#endif