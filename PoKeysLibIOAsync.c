#include "PoKeysLibHal.h"
#include "PoKeysLibAsync.h"
#include "hal_canon.h"


int export_IO_pins(const char *prefix, long comp_id, sPoKeysDevice *device)
{
    if (device == NULL){
        rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: device is NULL\n", __FILE__, __FUNCTION__);
        return -1;
    }


    int r = 0;

    // AnalogOut Pins
     for (int j = 0; j < (device->info.iPWMCount); j++) {

     }

    rtapi_print_msg(RTAPI_MSG_DBG, "PoKeys: %s:%s: canonical %s.adcout.pwm\n", __FILE__, __FUNCTION__, prefix);
    r = hal_param_u32_newf(HAL_RW, &(device->PWM.PWMperiod), comp_id, "%s.adcout.pwm.period", prefix);
    if (r != 0) {
        rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.adcout.pwm.period failed\n", __FILE__, __FUNCTION__, prefix);
        return r;
    }
     // AnalogIn Pins
    int analogInCount = 7;
    for (int j = 0; j < (analogInCount); j++) {
        rtapi_print_msg(RTAPI_MSG_DBG, "PoKeys: %s:%s: canonical %s.adcin.%01d\n", __FILE__, __FUNCTION__, prefix, j);
        r = hal_export_adcin(&device->AnalogInput[j].Canon, prefix, j, comp_id);
        if (r != 0) {
            rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: canonical %s.adcin.%01d failed\n", __FILE__, __FUNCTION__, prefix, j);
            return r;
        }

        // pokeys specific additional pins & options

        rtapi_print_msg(RTAPI_MSG_DBG, "PoKeys: %s:%s: %s.adcin.%01d.in.hw\n", __FILE__, __FUNCTION__, prefix, j);
        r = hal_pin_u32_newf(HAL_OUT, &(device->Pins[40 + j].AnalogValue), comp_id, "%s.adcin.%01d.in.hw", prefix, j);
        if (r != 0) {
            rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.adcin.%01d.in.hw failed\n", __FILE__, __FUNCTION__, prefix, j);
            return r;
        }

        rtapi_print_msg(RTAPI_MSG_DBG, "PoKeys: %s:%s: %s.adcin.%01d.in.raw\n", __FILE__, __FUNCTION__, prefix, j);
        r = hal_pin_float_newf(HAL_OUT, &(device->AnalogInput[j].rawvalue), comp_id, "%s.adcin.%01d.in.raw", prefix, j);
        if (r != 0) {
            rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.adcin.%01d.in.raw failed\n", __FILE__, __FUNCTION__, prefix, j);
            return r;
        }


        rtapi_print_msg(RTAPI_MSG_DBG, "PoKeys: %s:%s: %s.adcin.%01d.ReferenceVoltage\n", __FILE__, __FUNCTION__, prefix, j);
        r = hal_param_float_newf(HAL_RO, &(device->AnalogInput[j].ReferenceVoltage), comp_id, "%s.adcin.%01d.ReferenceVoltage", prefix, j);
        if (r != 0) {
            rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.adcin.%01d.ReferenceVoltage failed\n", __FILE__, __FUNCTION__, prefix, j);
            return r;
        }
        device->AnalogInput[j].ReferenceVoltage = 3.3; // default reference voltage
    }

    // Digital Pins
    for (int j = 0; j < (device->info.iPinCount); j++) {

        // PinFunction
        rtapi_print_msg(RTAPI_MSG_DBG, "PoKeys: %s:%s: %s.Pins.%01d.PinFunction\n", __FILE__, __FUNCTION__, prefix, j);
        r = hal_param_u32_newf(HAL_RW, &(device->Pins[j].PinFunction), comp_id, "%s.pins.%01d.PinFunction", prefix, j);
        if (r != 0) {
            rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.pins.%01d.PinFunction failed\n", __FILE__, __FUNCTION__, prefix, j);
            return r;
        }

        if (PK_CheckPinCapability(device, j, PK_AllPinCap_digitalInput) == 1) {
            //hal_digin_t digin = device->Pins[j].DigitalValueGet;
            rtapi_print_msg(RTAPI_MSG_DBG, "PoKeys: %s:%s: canonical %s.digin.%01d\n", __FILE__, __FUNCTION__, prefix, j);
            r = hal_export_digin(&device->Pins[j].DigitalValueGet, prefix, j, comp_id);
            if (r != 0) {
                rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: canonical %s.digin.%01d failed\n", __FILE__, __FUNCTION__, prefix, j);
                return r;
            }

            // pokeys specific additional options
            rtapi_print_msg(RTAPI_MSG_DBG, "PoKeys: %s:%s: %s.digin.%01d.invert\n", __FILE__, __FUNCTION__, prefix, j);
            r = hal_param_bit_newf(HAL_RW, &(device->Pins[j].PinCap_invertPin), comp_id, "%s.digin.%01d.invert", prefix, j);
            if (r != 0) {
                rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: %s.digin.%01d.invert failed\n", __FILE__, __FUNCTION__, prefix, j);
                return r;
            }
        }

        if (PK_CheckPinCapability(device, j, PK_AllPinCap_digitalOutput) == 1) {
            // ensure &device->Pins[j].PinCap_invertPin is mapped/cloned to device->Pins[j].DigitalValueSet.invert 
            rtapi_print_msg(RTAPI_MSG_DBG, "PoKeys: %s:%s: canonical %s.digout.%01d\n", __FILE__, __FUNCTION__, prefix, j);
            r = hal_export_digout(&device->Pins[j].DigitalValueSet, prefix, j,  comp_id);
            if (r != 0) {
                rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: canonical %s.digout.%01d failed\n", __FILE__, __FUNCTION__, prefix, j);
                return r;
            }
        }
    }

    return r;
}

/**
 * @brief Parser for pin function configuration response (CMD 0xC0, param1=0).
 */
int PK_ParsePinFunctionsResponse(sPoKeysDevice *dev, const uint8_t *resp) {
    if (!dev || !resp) return PK_ERR_GENERIC;
    for (uint32_t i = 0; i < dev->info.iPinCount; ++i) {
        dev->Pins[i].PinFunction = resp[8 + i];    // writing uint8_t to hal_u32_t

        // hal_bit_t PinCap_invertPin;           // PinFunction flag 'PK_PinCap_invertPin': Invert digital pin polarity (set together with digital input, output or triggered input)
        if (dev->Pins[i].PinFunction & PK_PinCap_invertPin) {

            dev->Pins[i].PinCap_invertPin = 1;
            dev->Pins[i].DigitalValueSet.invert =1;
        } else {
            dev->Pins[i].PinCap_invertPin = 0;
            dev->Pins[i].DigitalValueSet.invert =0;
        }
    }
    return PK_OK;
}

/**
 * @brief Starts async pin function configuration request (CMD 0xC0, param1=0).
 */
int PK_StartPinFunctionsRequestAsync(sPoKeysDevice *dev) {
    if (!dev) return PK_ERR_NOT_CONNECTED;
    return CreateRequestAsync(dev, 0xC0, (const uint8_t[]){0}, 1,
                               NULL, 0, PK_ParsePinFunctionsResponse);
}

/**
 * @brief Parser for digital counter configuration response (CMD 0xC0, param1=1).
 */
int PK_PinCounterConfigurationParse(sPoKeysDevice *dev, const uint8_t *resp) {
    if (!dev || !resp) return PK_ERR_GENERIC;
    for (uint32_t i = 0; i < dev->info.iPinCount; ++i) {
        if (PK_IsCounterAvailable(dev, (uint8_t)i)) {
            dev->Pins[i].DigitalCounterAvailable = 1;
            dev->Pins[i].CounterOptions = resp[8 + i];
        } else {
            dev->Pins[i].DigitalCounterAvailable = 0;
        }
    }
    return PK_OK;
}

/**
 * @brief Parser for pin key mapping type response (CMD 0xC1).
 */
int PK_PinKeyMappingTypeParse(sPoKeysDevice *dev, const uint8_t *resp) {
    if (!dev || !resp) return PK_ERR_GENERIC;
    for (uint32_t i = 0; i < dev->info.iPinCount; ++i) {
        if ((uint8_t)(dev->Pins[i].PinFunction)& PK_PinCap_digitalInput) {
            dev->Pins[i].MappingType = resp[8 + i];
        } else {
            dev->Pins[i].MappingType = 0;
        }
    }
    return PK_OK;
}

/**
 * @brief Parser for pin key mapping codes response (CMD 0xC2).
 */
int PK_PinKeyMappingCodesParse(sPoKeysDevice *dev, const uint8_t *resp) {
    if (!dev || !resp) return PK_ERR_GENERIC;
    for (uint32_t i = 0; i < dev->info.iPinCount; ++i) {
        if ((uint8_t)(dev->Pins[i].PinFunction) & PK_PinCap_digitalInput) {
            dev->Pins[i].KeyCodeMacroID = resp[8 + i];
        } else {
            dev->Pins[i].KeyCodeMacroID = 0;
        }
    }
    return PK_OK;
}


/**
 * @brief Parser for pin key mapping modifiers response (CMD 0xC3).
 */
int PK_PinKeyMappingModifiersParse(sPoKeysDevice *dev, const uint8_t *resp) {
    if (!dev || !resp) return PK_ERR_GENERIC;
    for (uint32_t i = 0; i < dev->info.iPinCount; ++i) {
        if ((uint8_t)(dev->Pins[i].PinFunction) & PK_PinCap_digitalInput) {
            dev->Pins[i].KeyModifier = resp[8 + i];
        } else {
            dev->Pins[i].KeyModifier = 0;
        }
    }
    return PK_OK;
}

/**
 * @brief Parser for triggered key down macro ID mapping (CMD 0xD7, param1=11).
 */
int PK_PinTriggeredDownKeyCodeParse(sPoKeysDevice *dev, const uint8_t *resp) {
    if (!dev || !resp) return PK_ERR_GENERIC;
    for (uint32_t i = 0; i < dev->info.iPinCount; ++i) {
        if ((uint8_t)(dev->Pins[i].PinFunction) & PK_PinCap_triggeredInput) {
            dev->Pins[i].downKeyCodeMacroID = resp[8 + i];
        } else {
            dev->Pins[i].downKeyCodeMacroID = 0;
        }
    }
    return PK_OK;
}

/**
 * @brief Parser for triggered key down modifiers (CMD 0xD7, param1=12).
 */
int PK_PinTriggeredDownKeyModifierParse(sPoKeysDevice *dev, const uint8_t *resp) {
    if (!dev || !resp) return PK_ERR_GENERIC;
    for (uint32_t i = 0; i < dev->info.iPinCount; ++i) {
        if ((uint8_t)(dev->Pins[i].PinFunction) & PK_PinCap_triggeredInput) {
            dev->Pins[i].downKeyModifier = resp[8 + i];
        } else {
            dev->Pins[i].downKeyModifier = 0;
        }
    }
    return PK_OK;
}

/**
 * @brief Parser for triggered key up modifiers (CMD 0xD7, param1=14).
 */
int PK_PinTriggeredUpKeyModifierParse(sPoKeysDevice *dev, const uint8_t *resp) {
    if (!dev || !resp) return PK_ERR_GENERIC;
    for (uint32_t i = 0; i < dev->info.iPinCount; ++i) {
        if ((uint8_t)(dev->Pins[i].PinFunction) & PK_PinCap_triggeredInput) {
            dev->Pins[i].upKeyModifier = resp[8 + i];
        } else {
            dev->Pins[i].upKeyModifier = 0;
        }
    }
    return PK_OK;
}

/**
 * @brief Parser for triggered key up macro IDs (CMD 0xD7, param1=13).
 */
int PK_PinTriggeredUpKeyCodeParse(sPoKeysDevice *dev, const uint8_t *resp) {
    if (!dev || !resp) return PK_ERR_GENERIC;
    for (uint32_t i = 0; i < dev->info.iPinCount; ++i) {
        if ((uint8_t)(dev->Pins[i].PinFunction) & PK_PinCap_triggeredInput) {
            dev->Pins[i].upKeyCodeMacroID = resp[8 + i];
        } else {
            dev->Pins[i].upKeyCodeMacroID = 0;
        }
    }
    return PK_OK;
}

int32_t PK_PinConfigurationGetAsync(sPoKeysDevice* device)
{

    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    // Get all pin configuration
    CreateRequestAsync(device, 0xC0, (const uint8_t[]){0}, 1,
                               NULL, 0, PK_ParsePinFunctionsResponse);

    // If device supports digital counters, get the settings for them
    if (device->info.iDigitalCounters) {
        // Get all pin configuration - counter setup
        CreateRequestAsync(device, 0xC0, (const uint8_t[]){1}, 1,
                           NULL, 0, PK_PinCounterConfigurationParse);
    }

    // If the device supports key mapping, get the settings
    if (device->info.iKeyMapping) {
        // Get all pin key mapping - type
        CreateRequestAsync(device, 0xC1, NULL, 0,
                           NULL, 0, PK_PinKeyMappingTypeParse);
        // Get all pin key mapping - key codes
        CreateRequestAsync(device, 0xC2, NULL, 0,
                           NULL, 0, PK_PinKeyMappingCodesParse);
        // Get all pin key mapping - modifiers
        CreateRequestAsync(device, 0xC3, NULL, 0,
                           NULL, 0, PK_PinKeyMappingModifiersParse);
    }

    // Again, check if device supports triggered key mapping
	if (device->info.iTriggeredKeyMapping)
	{
        // Get all pin key mapping - triggered inputs
        CreateRequestAsync(device, 0xD7, (const uint8_t[]){11}, 1,
                              NULL, 0, PK_PinTriggeredDownKeyCodeParse);

        CreateRequestAsync(device, 0xD7, (const uint8_t[]){12}, 1,
                              NULL, 0, PK_PinTriggeredDownKeyModifierParse);

        CreateRequestAsync(device, 0xD7, (const uint8_t[]){13}, 1,
                              NULL, 0, PK_PinTriggeredUpKeyCodeParse);

        CreateRequestAsync(device, 0xD7, (const uint8_t[]){14}, 1,
                              NULL, 0, PK_PinTriggeredUpKeyModifierParse);
    }
    return PK_OK;
}

/**
 * @brief Sets all pin configuration fields asynchronously.
 */
int32_t PK_PinConfigurationSetAsync(sPoKeysDevice* device) {
    if (!device) return PK_ERR_NOT_CONNECTED;

    // CMD 0xC0, param1=1: Set all pin functions
    uint8_t bufferC0[56] = {0};
    for (uint32_t i = 0; i < device->info.iPinCount && i < 56; i++) {

        if (device->Pins[i].PinFunction & PK_PinCap_digitalInput || device->Pins[i].PinFunction & PK_PinCap_triggeredInput) {
            if (device->Pins[i].PinCap_invertPin == 1){
                device->Pins[i].PinFunction |= PK_PinCap_invertPin;
            } else {
                device->Pins[i].PinFunction &= ~PK_PinCap_invertPin;
            }

        } else if (device->Pins[i].PinFunction & PK_PinCap_digitalOutput) {
            if (device->Pins[i].DigitalValueSet.invert == 1){
                device->Pins[i].PinFunction |= PK_PinCap_invertPin;
            } else {
                device->Pins[i].PinFunction &= ~PK_PinCap_invertPin;
            }
        }
        bufferC0[i] = (uint8_t)(device->Pins[i].PinFunction);  // write (PinFunction)hal_u32_t to bufferC0 (uint8_t)
    }
    CreateRequestAsync(device, 0xC0, (const uint8_t[]){1}, 1,
                       bufferC0, 56, NULL);

    // CMD 0xC0, param2=2: Set counter options
    if (device->info.iDigitalCounters) {
        uint8_t bufferCounters[56] = {0};
        for (uint32_t i = 0; i < device->info.iPinCount && i < 56; i++) {
            bufferCounters[i] = device->Pins[i].CounterOptions;
        }
        CreateRequestAsync(device, 0xC0, (const uint8_t[]){0, 2}, 2,
                           bufferCounters, 56, NULL);
    }

    if (device->info.iKeyMapping) {
        uint8_t keyType[56] = {0};
        uint8_t keyCode[56] = {0};
        uint8_t keyMod[56] = {0};

        for (uint32_t i = 0; i < device->info.iPinCount && i < 56; i++) {
            if ((uint8_t)(device->Pins[i].PinFunction) & PK_PinCap_digitalInput) {
                keyType[i] = device->Pins[i].MappingType;
                keyCode[i] = device->Pins[i].KeyCodeMacroID;
                keyMod[i] = device->Pins[i].KeyModifier;
            }
        }

        CreateRequestAsync(device, 0xC1, (const uint8_t[]){1}, 1,
                           keyType, 56, NULL);
        CreateRequestAsync(device, 0xC2, (const uint8_t[]){1}, 1,
                           keyCode, 56, NULL);
        CreateRequestAsync(device, 0xC3, (const uint8_t[]){1}, 1,
                           keyMod, 56, NULL);

        if (device->info.iTriggeredKeyMapping) {
            uint8_t downCode[56] = {0}, downMod[56] = {0}, upCode[56] = {0}, upMod[56] = {0};

            for (uint32_t i = 0; i < device->info.iPinCount && i < 56; i++) {
                if ((uint8_t)(device->Pins[i].PinFunction) & PK_PinCap_triggeredInput) {
                    downCode[i] = device->Pins[i].downKeyCodeMacroID;
                    downMod[i] = device->Pins[i].downKeyModifier;
                    upCode[i] = device->Pins[i].upKeyCodeMacroID;
                    upMod[i] = device->Pins[i].upKeyModifier;
                }
            }

            CreateRequestAsync(device, 0xD7, (const uint8_t[]){1}, 1,
                               downCode, 56, NULL);
            CreateRequestAsync(device, 0xD7, (const uint8_t[]){2}, 1,
                               downMod, 56, NULL);
            CreateRequestAsync(device, 0xD7, (const uint8_t[]){3}, 1,
                               upCode, 56, NULL);
            CreateRequestAsync(device, 0xD7, (const uint8_t[]){4}, 1,
                               upMod, 56, NULL);
        }
    }

    return PK_OK;
}

/**
 * @brief Asynchronously sets the digital IO values.
 */
int32_t PK_DigitalIOSetAsync(sPoKeysDevice* device) {
    if (!device) return PK_ERR_NOT_CONNECTED;

    uint8_t dio[56] = {0};
    uint8_t mask[56] = {0};

    for (uint32_t i = 0; i < device->info.iPinCount && i < 56; i++) {
        if (device->Pins[i].preventUpdate > 0) {
            mask[i / 8] |= (1 << (i % 8));
        } else if (*(device->Pins[i].DigitalValueSet.out) > 0) {
            dio[i / 8] |= (1 << (i % 8));
        }
    }

    for (uint8_t i = 0; i < 7; ++i) {
        device->request[8 + i] = dio[i];
        device->request[20 + i] = mask[i];
    }

    return CreateRequestAsync(device, 0xCC, (const uint8_t[]){1}, 1,
                               device->request + 8, 56, NULL);
}

/**
 * @brief Parser for digital IO input values (CMD 0xCC, param1=0)
 */
int PK_DigitalIOGetParse(sPoKeysDevice* device, const uint8_t* response) {
    if (!device || !response) return PK_ERR_GENERIC;
    for (uint32_t i = 0; i < device->info.iPinCount && i < 56; i++) {
        *(device->Pins[i].DigitalValueGet.in) = ((response[8 + i / 8] & (1 << (i % 8))) != 0);
        *(device->Pins[i].DigitalValueGet.in_not) = ((response[8 + i / 8] & (1 << (i % 8))) == 0);
    }
    return PK_OK;
}

/**
 * @brief Asynchronously requests digital IO input values.
 */
int PK_DigitalIOGetAsync(sPoKeysDevice* device) {
    if (!device) return PK_ERR_NOT_CONNECTED;
    return CreateRequestAsync(device, 0xCC, (const uint8_t[]){0}, 1,
                              NULL, 0, PK_DigitalIOGetParse);
}

/**
 * @brief Combined Set and Get digital IO in one async call.
 */
int PK_DigitalIOSetGetAsync(sPoKeysDevice* device) {
    if (!device) return PK_ERR_NOT_CONNECTED;

    uint8_t dio[56] = {0};
    uint8_t mask[56] = {0};

    for (uint32_t i = 0; i < device->info.iPinCount && i < 56; i++) {
        if (device->Pins[i].preventUpdate > 0) {
            mask[i / 8] |= (1 << (i % 8));
        } else if (*(device->Pins[i].DigitalValueSet.out) > 0) {
            dio[i / 8] |= (1 << (i % 8));
        }
    }

    for (uint8_t i = 0; i < 7; ++i) {
        device->request[8 + i] = dio[i];
        device->request[20 + i] = mask[i];
    }

    return CreateRequestAsync(device, 0xCC, (const uint8_t[]){1}, 1,
                               device->request + 8, 
                                 56, PK_DigitalIOGetParse);
}

/**
 * @brief Parser for analog inputs (CMD 0x3A, param1=1).
 */
int PK_AnalogIOParse(sPoKeysDevice* device, const uint8_t* response) {
    if (!device || !response) return PK_ERR_GENERIC;
    if (device->info.iAnalogInputs == 0) return PK_ERR_NOT_SUPPORTED;

    for (uint32_t i = 0; i < 7 && (40 + i) < device->info.iPinCount; ++i) {
         *(device->Pins[40 + i].AnalogValue) = ((uint32_t)device->response[8 + i * 2] << 8) + (long)device->response[9 + i * 2];
		*(device->AnalogInput[i].rawvalue) = *(device->Pins[40 + i].AnalogValue) * 4095 / device->AnalogInput[i].ReferenceVoltage;
		*(device->AnalogInput[i].Canon.value) = *(device->AnalogInput[i].rawvalue) * device->AnalogInput[i].Canon.scale + device->AnalogInput[i].Canon.offset;
    }
    return PK_OK;
}

/**
 * @brief Starts async request for analog inputs (CMD 0x3A, param1=1).
 */
int PK_AnalogIOGetAsync(sPoKeysDevice* device) {
    if (!device) return PK_ERR_NOT_CONNECTED;
    if (device->info.iAnalogInputs == 0) return PK_ERR_NOT_SUPPORTED;

    return CreateRequestAsync(device, 0x3A, (const uint8_t[]){1}, 1,
                              NULL, 0, PK_AnalogIOParse);
}

/**
 * @brief Parser for RC analog filter configuration (CMD 0x38).
 */
int PK_AnalogRCFilterParse(sPoKeysDevice* device, const uint8_t* response) {
    if (!device || !response) return PK_ERR_GENERIC;
    if (device->info.iAnalogFiltering == 0) return PK_ERR_NOT_SUPPORTED;

    device->otherPeripherals.AnalogRCFilter =
        ((uint32_t)response[2]) |
        ((uint32_t)response[3] << 8) |
        ((uint32_t)response[4] << 16) |
        ((uint32_t)response[5] << 24);

    return PK_OK;
}

/**
 * @brief Starts async request for RC analog filter value (CMD 0x38).
 */
int PK_AnalogRCFilterGetAsync(sPoKeysDevice* device) {
    if (!device) return PK_ERR_NOT_CONNECTED;
    if (device->info.iAnalogFiltering == 0) return PK_ERR_NOT_SUPPORTED;

    return CreateRequestAsync(device, 0x38, NULL, 0, NULL, 0, PK_AnalogRCFilterParse);
}

/**
 * @brief Starts async command to set RC analog filter (CMD 0x39).
 */
int PK_AnalogRCFilterSetAsync(sPoKeysDevice* device) {
    if (!device) return PK_ERR_NOT_CONNECTED;
    if (device->info.iAnalogFiltering == 0) return PK_ERR_NOT_SUPPORTED;

    uint32_t tmp = device->otherPeripherals.AnalogRCFilter;
    uint8_t payload[4] = {
        (uint8_t)(tmp & 0xFF),
        (uint8_t)((tmp >> 8) & 0xFF),
        (uint8_t)((tmp >> 16) & 0xFF),
        (uint8_t)((tmp >> 24) & 0xFF)
    };

    return CreateRequestAsync(device, 0x39, NULL, 0, payload, 4, NULL);
}


/**
 * @brief Parser for digital counter values (CMD 0xD8).
 */
int PK_DigitalCounterParse(sPoKeysDevice* device, const uint8_t* response) {
    if (!device || !response) return PK_ERR_GENERIC;
    if (device->info.iDigitalCounters == 0) return PK_ERR_NOT_SUPPORTED;

    for (uint32_t j = 0; j < 13; j++) {
        uint8_t pinIndex = device->request[8 + j];
        if (PK_IsCounterAvailable(device, pinIndex)) {
            device->Pins[pinIndex].DigitalCounterValue = *(int32_t*)(&response[8 + j * 4]);
        }
    }
    return PK_OK;
}

/**
 * @brief Starts async request to get digital counter values (CMD 0xD8).
 */
int PK_DigitalCounterGetAsync(sPoKeysDevice* device) {
    if (!device) return PK_ERR_NOT_CONNECTED;
    if (device->info.iDigitalCounters == 0) return PK_ERR_NOT_SUPPORTED;

    uint8_t pinIndices[13];
    size_t k = 0;
    for (uint32_t i = 0; i < device->info.iPinCount && k < 13; i++) {
        if (PK_IsCounterAvailable(device, i)) {
            pinIndices[k++] = (uint8_t)i;
        }
    }
    if (k == 0) return PK_OK; // no counters to query

    return CreateRequestAsync(device, 0xD8, NULL, 0, pinIndices, k, PK_DigitalCounterParse);
}

/**
 * @brief Starts async request to clear digital counters (CMD 0x1D).
 */
int PK_DigitalCounterClearAsync(sPoKeysDevice* device) {
    if (!device) return PK_ERR_NOT_CONNECTED;
    return CreateRequestAsync(device, 0x1D, NULL, 0, NULL, 0, NULL);
}

/**
 * @brief Starts async command to configure PWM duty cycle and period (CMD 0xCB).
 */
int PK_PWMConfigurationSetAsync(sPoKeysDevice* device) {
    if (!device) return PK_ERR_NOT_CONNECTED;

    uint8_t payload[37] = {0};
    for (uint32_t n = 0; n < 6; n++) {
        if (device->PWM.PWMenabledChannels[n]) {
            payload[0] |= (uint8_t)(1 << n);
        }
        uint32_t duty = device->PWM.PWMduty[n];
        payload[1 + n * 4] = (uint8_t)(duty & 0xFF);
        payload[2 + n * 4] = (uint8_t)((duty >> 8) & 0xFF);
        payload[3 + n * 4] = (uint8_t)((duty >> 16) & 0xFF);
        payload[4 + n * 4] = (uint8_t)((duty >> 24) & 0xFF);
    }
    uint32_t period = device->PWM.PWMperiod;
    payload[33] = (uint8_t)(period & 0xFF);
    payload[34] = (uint8_t)((period >> 8) & 0xFF);
    payload[35] = (uint8_t)((period >> 16) & 0xFF);
    payload[36] = (uint8_t)((period >> 24) & 0xFF);

    return CreateRequestAsync(device, 0xCB, (const uint8_t[]){1}, 1, payload, sizeof(payload), NULL);
}


/**
 * @brief Parser for reading PWM configuration (CMD 0xCB, param1=0)
 */
int PK_PWMConfigurationParse(sPoKeysDevice* device, const uint8_t* response) {
    if (!device || !response) return PK_ERR_GENERIC;

    for (uint32_t n = 0; n < 6; n++) {
        device->PWM.PWMenabledChannels[n] = (response[8] & (1 << n)) ? 1 : 0;
        device->PWM.PWMduty[n] =
            ((uint32_t)response[9 + n * 4]) |
            ((uint32_t)response[10 + n * 4] << 8) |
            ((uint32_t)response[11 + n * 4] << 16) |
            ((uint32_t)response[12 + n * 4] << 24);
    }
    device->PWM.PWMperiod =
        ((uint32_t)response[33]) |
        ((uint32_t)response[34] << 8) |
        ((uint32_t)response[35] << 16) |
        ((uint32_t)response[36] << 24);

    return PK_OK;
}

/**
 * @brief Starts async request to get PWM configuration (CMD 0xCB, param1=0)
 */
int PK_PWMConfigurationGetAsync(sPoKeysDevice* device) {
    if (!device) return PK_ERR_NOT_CONNECTED;

    return CreateRequestAsync(device, 0xCB, (const uint8_t[]){0}, 1, NULL, 0, PK_PWMConfigurationParse);
}


/**
 * @brief Starts async command to update PWM values only (CMD 0xCB, param1=1, param2=1).
 */
int PK_PWMUpdateAsync(sPoKeysDevice* device) {
    if (!device) return PK_ERR_NOT_CONNECTED;

    uint8_t payload[37] = {0};
    for (uint32_t n = 0; n < 6; n++) {
        if (device->PWM.PWMenabledChannels[n]) {
            payload[0] |= (uint8_t)(1 << n);
        }
        uint32_t duty = device->PWM.PWMduty[n];
        payload[1 + n * 4] = (uint8_t)(duty & 0xFF);
        payload[2 + n * 4] = (uint8_t)((duty >> 8) & 0xFF);
        payload[3 + n * 4] = (uint8_t)((duty >> 16) & 0xFF);
        payload[4 + n * 4] = (uint8_t)((duty >> 24) & 0xFF);
    }
    uint32_t period = device->PWM.PWMperiod;
    payload[33] = (uint8_t)(period & 0xFF);
    payload[34] = (uint8_t)((period >> 8) & 0xFF);
    payload[35] = (uint8_t)((period >> 16) & 0xFF);
    payload[36] = (uint8_t)((period >> 24) & 0xFF);

    return CreateRequestAsync(device, 0xCB, (const uint8_t[]){1, 1}, 2, payload, sizeof(payload), NULL);
}

    int parse_PoExtBusGet(sPoKeysDevice* device, const uint8_t* resp) {
        uint32_t len = device->info.iPoExtBus;

        for (int i = 0; i < device->info.iPoExtBus; i++)
            {
                device->PoExtBusData[i] = resp[8 + i];
            }
        return PK_OK;
    }
/**
 * @brief Starts async request to set PoExtBus data (CMD 0xDA).
 */
int PK_PoExtBusSetAsync(sPoKeysDevice* device) {
    if (!device) return PK_ERR_NOT_CONNECTED;

    uint32_t len = device->info.iPoExtBus;
    if (len == 0 || !device->PoExtBusData) return PK_ERR_NOT_SUPPORTED;

    // Prepare payload
    uint8_t payload[64] = {0};
  //  memcpy(payload, device->PoExtBusData, len);
    for (uint32_t i = 0; i < len && i < sizeof(payload); ++i){
        payload[i] = device->PoExtBusData[i];
    }


    return CreateRequestAsync(device, 0xDA, (const uint8_t[]){1, 0}, 2, payload, len, parse_PoExtBusGet);
}



/**
 * @brief Starts async request to get PoExtBus data (CMD 0xDA, param1=2).
 */
int PK_PoExtBusGetAsync(sPoKeysDevice* device) {
    if (!device) return PK_ERR_NOT_CONNECTED;

    uint32_t len = device->info.iPoExtBus;
    if (len == 0 || !device->PoExtBusData) return PK_ERR_NOT_SUPPORTED;



    return CreateRequestAsync(device, 0xDA, (const uint8_t[]){2, 0}, 2, NULL, 0, parse_PoExtBusGet);
}