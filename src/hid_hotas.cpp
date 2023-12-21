#include "hid_hotas.h"

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

HOTAS::HOTAS(HOTAS_Axis** axes, uint8_t axesCount, HOTAS_Axis** simulationAxes, uint8_t simulationAxesCount, HOTAS_Buttons* buttons) {
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
        set2(&ptr, 0x85, hidReportId);  // REPORT_ID (3)

        if (axes != NULL && axesCount > 0) {
            set2(&ptr, USAGE, 0x01); // USAGE (Pointer)
            set2(&ptr, 0x15, 0x00); // LOGICAL_MINIMUM (0)
            set5(&ptr, 0x27, 0xFF, 0xFF, 0x00, 0x00); // LOGICAL_MAXIMUM (65535)
            set2(&ptr, 0x75, 0x10); // REPORT_SIZE (16)
            set2(&ptr, 0x95, axesCount); // REPORT_COUNT (axesCount)
            set2(&ptr, COLLECTION, 0x00); // (Physical)
                for (uint8_t i = 0; i < axesCount; i++) {
                    set2(&ptr, USAGE, axes[i]->type);
                }
                set2(&ptr, 0x81, 0x02); // INPUT (Data,Var,Abs)
            set1(&ptr, END_COLLECTION);
        }

        if (simulationAxes != NULL && simulationAxesCount > 0) {
            set2(&ptr, USAGE_PAGE, SIMULATION_CONTROLS);
            set2(&ptr, USAGE, 0x09); // USAGE (Airplane Simulation Device)
            set2(&ptr, 0x15, 0x00); // LOGICAL_MINIMUM (0)
            set5(&ptr, 0x27, 0xFF, 0xFF, 0x00, 0x00); // LOGICAL_MAXIMUM (65535)
            set2(&ptr, 0x75, 0x10); // REPORT_SIZE (16)
            set2(&ptr, 0x95, simulationAxesCount); // REPORT_COUNT (simulationCount)
            set2(&ptr, COLLECTION, 0x00); // (Physical)
                for (uint8_t i = 0; i < simulationAxesCount; i++) {
                    set2(&ptr, USAGE, simulationAxes[i]->type);
                }
                set2(&ptr, 0x81, 0x02); // INPUT (Data,Var,Abs)
            set1(&ptr, END_COLLECTION);
        }

        if (buttons != NULL) {
            set2(&ptr, USAGE_PAGE, 0x09); // (Button)
            set2(&ptr, 0x19, 0x01);          // USAGE_MINIMUM (Button 1)
            set2(&ptr, 0x29, buttons->numberOfButtons);   // USAGE_MAXIMUM (Button 32)
            set2(&ptr, 0x15, 0x00);          // LOGICAL_MINIMUM (0)
            set2(&ptr, 0x25, 0x01);          // LOGICAL_MAXIMUM (1)
            set2(&ptr, 0x75, 0x01);          // REPORT_SIZE (1)
            set2(&ptr, 0x95, buttons->numberOfButtons);   // REPORT_COUNT (# of buttons)
            set2(&ptr, 0x55, 0x00);          // UNIT_EXPONENT (0)
            set2(&ptr, 0x65, 0x00);          // UNIT (None)
            set2(&ptr, 0x81, 0x02);          // INPUT (Data,Var,Abs)
            if (buttons->bitsInLastByte > 0) {
                set2(&ptr, 0x75, 0x01);                  // REPORT_SIZE (1)
                set2(&ptr, 0x95, 8 - buttons->bitsInLastByte); // REPORT_COUNT (# of padding bits)
                set2(&ptr, 0x81, 0x03);                  // INPUT (Const,Var,Abs)
            }
        }
    set1(&ptr, END_COLLECTION);

    int hidReportDescriptorSize = ptr - tempHidReportDescriptor;
    uint8_t *customHidReportDescriptor = new uint8_t[hidReportDescriptorSize];
	memcpy(customHidReportDescriptor, tempHidReportDescriptor, hidReportDescriptorSize);

    DynamicHIDSubDescriptor *node = new DynamicHIDSubDescriptor(customHidReportDescriptor, hidReportDescriptorSize, false);
	DynamicHID().AppendDescriptor(node);

    hidReportSize = buttons->bytesLength;
	hidReportSize += axesCount * 2;
	hidReportSize += simulationAxesCount * 2;
}

void buildAndSet16BitValue(uint8_t **ptr, uint16_t value) {
	uint8_t highByte = (uint8_t)(value >> 8);
	uint8_t lowByte = (uint8_t)(value & 0x00FF);
	**ptr = lowByte;
    *ptr += 1;
	**ptr = highByte;
    *ptr += 1;
}

void HOTAS::sendState() {
	uint8_t data[hidReportSize];
	uint8_t *ptr = data;

    for (uint8_t i = 0; i < axesCount; i++) {
        buildAndSet16BitValue(&ptr, axes[i]->value);
    }

    for (uint8_t i = 0; i < simulationAxesCount; i++) {
        buildAndSet16BitValue(&ptr, simulationAxes[i]->value);
    }

    for (int i = 0; i < buttons->bytesLength; i++) {
		*ptr = buttons->bytes[i];
        ptr += 1;
	}

	DynamicHID().SendReport(hidReportId, data, hidReportSize);
}

#endif