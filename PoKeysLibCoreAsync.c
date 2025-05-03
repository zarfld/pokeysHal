/**
 * @file PoKeysLibCoreAsync_USB.c
 * @brief Async-safe USB enumeration logic for PoKeysLib.
 */
#include <stdlib.h>
#include "PoKeysLibCoreAsync_USB.h"
#include <hidapi/hidapi.h>
#include <string.h>
#include "stdio.h"

typedef enum {
    ENUM_IDLE,
    ENUM_ENUMERATING_1001,
    ENUM_ENUMERATING_1002,
    ENUM_DONE
} PK_EnumStage;

typedef struct {
    int32_t numDevices;
    struct hid_device_info *devs;
    struct hid_device_info *cur_dev;
    PK_EnumStage state;
} PKUSBEnumerator;

int32_t PK_EnumerateUSBDevicesAsync(PKUSBEnumerator *ctx)
{
    if (!ctx) return -1;

    switch (ctx->state)
    {
        case ENUM_IDLE:
        case ENUM_DONE:
            ctx->numDevices = 0;
            ctx->devs = NULL;
            ctx->cur_dev = NULL;
            ctx->state = ENUM_ENUMERATING_1001;
            return -2; // in progress

        case ENUM_ENUMERATING_1001:
            if (!ctx->devs && !ctx->cur_dev) {
                ctx->devs = hid_enumerate(0x1DC3, 0x1001);
                ctx->cur_dev = ctx->devs;
            }
            if (ctx->cur_dev) {
                if (PKI_CheckInterface(ctx->cur_dev)) ctx->numDevices++;
                ctx->cur_dev = ctx->cur_dev->next;
                return -2; // still processing
            } else {
                hid_free_enumeration(ctx->devs);
                ctx->devs = NULL;
                ctx->cur_dev = NULL;
                ctx->state = ENUM_ENUMERATING_1002;
                return -2;
            }

        case ENUM_ENUMERATING_1002:
            if (!ctx->devs && !ctx->cur_dev) {
                ctx->devs = hid_enumerate(0x1DC3, 0x1002);
                ctx->cur_dev = ctx->devs;
            }
            if (ctx->cur_dev) {
                if (ctx->cur_dev->interface_number == -1) ctx->numDevices++;
                ctx->cur_dev = ctx->cur_dev->next;
                return -2; // still processing
            } else {
                hid_free_enumeration(ctx->devs);
                ctx->devs = NULL;
                ctx->cur_dev = NULL;
#ifdef POKEYSLIB_USE_LIBUSB
                ctx->numDevices += PK_EnumerateFastUSBDevices();
#endif
                ctx->state = ENUM_DONE;
                return ctx->numDevices;
            }

        default:
            return -1;
    }
}

void PK_InitializeNewDeviceAsync(void *device_ptr)
{
    if (!device_ptr) return;
    sPoKeysDevice *device = (sPoKeysDevice *)device_ptr;
    uint32_t i;

    memset(&device->info, 0, sizeof(sPoKeysDevice_Info));
    memset(&device->DeviceData, 0, sizeof(sPoKeysDevice_Data));

    device->netDeviceData = 0;
    memset(&device->matrixKB, 0, sizeof(sMatrixKeyboard));
    memset(&device->PWM, 0, sizeof(sPoKeysPWM));
    memset(&device->LCD, 0, sizeof(sPoKeysLCD));

    device->FastEncodersConfiguration = 0;
    device->FastEncodersOptions = 0;
    device->UltraFastEncoderConfiguration = 0;
    device->UltraFastEncoderOptions = 0;
    device->UltraFastEncoderFilter = 0;

    memset(device->request, 0, 64);
    memset(device->response, 0, 64);

    device->sendRetries = 3;
    device->readRetries = 10;
    device->socketTimeout = 100;

    PK_DeviceDataGet(device);

    device->Pins = (sPoKeysPinData*)hal_malloc(sizeof(sPoKeysPinData) * device->info.iPinCount);
    memset(device->Pins, 0, sizeof(sPoKeysPinData) * device->info.iPinCount);

    for (i = 0; i < device->info.iPinCount; i++) {
        device->Pins[i].DigitalCounterAvailable = PK_IsCounterAvailable(device, i) ? 1 : 0;
    }

    device->Encoders = (sPoKeysEncoder*)hal_malloc(sizeof(sPoKeysEncoder) * device->info.iEncodersCount);
    memset(device->Encoders, 0, sizeof(sPoKeysEncoder) * device->info.iEncodersCount);

    if (device->info.iEasySensors) {
        device->EasySensors = (sPoKeysEasySensor*)hal_malloc(sizeof(sPoKeysEasySensor) * device->info.iEasySensors);
        memset(device->EasySensors, 0, sizeof(sPoKeysEasySensor) * device->info.iEasySensors);
    } else {
        device->EasySensors = NULL;
    }

    device->PWM.PWMduty = (uint32_t*)hal_malloc(sizeof(uint32_t) * device->info.iPWMCount);
    memset(device->PWM.PWMduty, 0, sizeof(uint32_t) * device->info.iPWMCount);

    if (device->info.iPWMCount > 0) {
        device->PWM.PWMenabledChannels = (unsigned char*)hal_malloc(sizeof(unsigned char) * device->info.iPWMCount);
        memset(device->PWM.PWMenabledChannels, 0, sizeof(unsigned char) * device->info.iPWMCount);
    } else {
        device->PWM.PWMenabledChannels = NULL;
    }

    device->PWM.PWMpinIDs = (unsigned char*)hal_malloc(sizeof(unsigned char) * device->info.iPWMCount);
    memset(device->PWM.PWMpinIDs, 0, sizeof(unsigned char) * device->info.iPWMCount);

    PK_FillPWMPinNumbers(device);

    device->PoExtBusData = (unsigned char*)hal_malloc(sizeof(unsigned char) * device->info.iPoExtBus);

    device->MatrixLED = (sPoKeysMatrixLED*)hal_malloc(sizeof(sPoKeysMatrixLED) * device->info.iMatrixLED);
    memset(device->MatrixLED, 0, sizeof(sPoKeysMatrixLED) * device->info.iMatrixLED);

    memset(&device->PEv2, 0, sizeof(sPoKeysPEv2));

    device->multiPartBuffer = hal_malloc(512);
    if ((intptr_t)device->multiPartBuffer <= 0) device->multiPartBuffer = 0;
}

void PK_CleanDeviceAsync(void *device_ptr)
{
    if (!device_ptr) return;
    sPoKeysDevice *device = (sPoKeysDevice *)device_ptr;

    hal_free(device->Pins);
    device->Pins = NULL;

    hal_free(device->Encoders);
    device->Encoders = NULL;

    hal_free(device->PWM.PWMduty);
    device->PWM.PWMduty = NULL;

    hal_free(device->PWM.PWMenabledChannels);
    device->PWM.PWMenabledChannels = NULL;

    hal_free(device->PWM.PWMpinIDs);
    device->PWM.PWMpinIDs = NULL;

    hal_free(device->PoExtBusData);
    device->PoExtBusData = NULL;

    hal_free(device->MatrixLED);
    device->MatrixLED = NULL;

    if (device->multiPartBuffer) {
        hal_free(device->multiPartBuffer);
        device->multiPartBuffer = NULL;
    }

    if (device->EasySensors) {
        hal_free(device->EasySensors);
        device->EasySensors = NULL;
    }

    if (device->netDeviceData) {
        hal_free(device->netDeviceData);
        device->netDeviceData = NULL;
    }
}

void PK_CloneDeviceStructureAsync(const sPoKeysDevice *original, sPoKeysDevice *destination)
{
    if (!original || !destination) return;

    destination->Pins = (sPoKeysPinData*)hal_malloc(sizeof(sPoKeysPinData) * original->info.iPinCount);
    destination->Encoders = (sPoKeysEncoder*)hal_malloc(sizeof(sPoKeysEncoder) * original->info.iEncodersCount);
    destination->PWM.PWMduty = (uint32_t*)hal_malloc(sizeof(uint32_t) * original->info.iPWMCount);
    destination->PWM.PWMenabledChannels = original->info.iPWMCount > 0
        ? (unsigned char*)hal_malloc(sizeof(unsigned char) * original->info.iPWMCount) : NULL;
    destination->PWM.PWMpinIDs = (unsigned char*)hal_malloc(sizeof(unsigned char) * original->info.iPWMCount);
    destination->MatrixLED = (sPoKeysMatrixLED*)hal_malloc(sizeof(sPoKeysMatrixLED) * original->info.iMatrixLED);

    destination->EasySensors = original->info.iEasySensors > 0
        ? (sPoKeysEasySensor*)hal_malloc(sizeof(sPoKeysEasySensor) * original->info.iEasySensors) : NULL;

    destination->netDeviceData = original->netDeviceData
        ? (sPoKeysNetworkDeviceInfo *)hal_malloc(sizeof(sPoKeysNetworkDeviceInfo)) : NULL;

    destination->PoExtBusData = (unsigned char*)hal_malloc(sizeof(unsigned char) * original->info.iPoExtBus);

    destination->devHandle = original->devHandle;
    destination->devHandle2 = original->devHandle2;
    destination->info = original->info;
    destination->DeviceData = original->DeviceData;

    memcpy(destination->Pins, original->Pins, original->info.iPinCount * sizeof(sPoKeysPinData));
    memcpy(destination->Encoders, original->Encoders, original->info.iEncodersCount * sizeof(sPoKeysEncoder));

    if (original->info.iEasySensors)
        memcpy(destination->EasySensors, original->EasySensors, original->info.iEasySensors * sizeof(sPoKeysEasySensor));

    destination->matrixKB = original->matrixKB;
    destination->PWM.PWMperiod = original->PWM.PWMperiod;

    memcpy(destination->PWM.PWMduty, original->PWM.PWMduty, sizeof(uint32_t) * original->info.iPWMCount);
    if (destination->PWM.PWMenabledChannels)
        memcpy(destination->PWM.PWMenabledChannels, original->PWM.PWMenabledChannels, sizeof(unsigned char) * original->info.iPWMCount);
    memcpy(destination->PWM.PWMpinIDs, original->PWM.PWMpinIDs, sizeof(unsigned char) * original->info.iPWMCount);
    memcpy(destination->MatrixLED, original->MatrixLED, sizeof(sPoKeysMatrixLED) * original->info.iMatrixLED);

    destination->LCD = original->LCD;
    destination->PoNETmodule = original->PoNETmodule;
    destination->PoIL = original->PoIL;
    destination->RTC = original->RTC;

    destination->FastEncodersConfiguration = original->FastEncodersConfiguration;
    destination->FastEncodersOptions = original->FastEncodersOptions;
    destination->UltraFastEncoderConfiguration = original->UltraFastEncoderConfiguration;
    destination->UltraFastEncoderOptions = original->UltraFastEncoderOptions;
    destination->UltraFastEncoderFilter = original->UltraFastEncoderFilter;

    memcpy(destination->PoExtBusData, original->PoExtBusData, sizeof(unsigned char) * original->info.iPoExtBus);
    destination->connectionType = original->connectionType;
    destination->requestID = original->requestID;
}

sPoKeysDevice* PK_ConnectToDeviceAsync(uint32_t deviceIndex)
{
    static struct hid_device_info *devs = NULL;
    static struct hid_device_info *cur_dev = NULL;
    static uint32_t numDevices = 0;
    static ConnectAsyncStage stage = CONN_IDLE;

    sPoKeysDevice *tmpDevice = NULL;

    if (stage == CONN_IDLE) {
        devs = hid_enumerate(0x1DC3, 0x1001);
        cur_dev = devs;
        numDevices = 0;
        stage = CONN_ENUM_1001;
        return NULL;
    }

    while (cur_dev) {
        if ((stage == CONN_ENUM_1001 && cur_dev->interface_number == 1) ||
            (stage == CONN_ENUM_1002 && cur_dev->interface_number == -1)) {
            
            if (numDevices == deviceIndex) {
                tmpDevice = hal_malloc(sizeof(sPoKeysDevice));
                if (!tmpDevice) break;

                tmpDevice->devHandle = (void*)hid_open_path(cur_dev->path);
                tmpDevice->devHandle2 = NULL;
                tmpDevice->connectionType = PK_DeviceType_USBDevice;

                if (tmpDevice->devHandle != NULL) {
                    PK_InitializeNewDeviceAsync(tmpDevice);
                } else {
                    hal_free(tmpDevice);
                    tmpDevice = NULL;
                }

                hid_free_enumeration(devs);
                devs = cur_dev = NULL;
                stage = CONN_IDLE;
                return tmpDevice;
            }
            numDevices++;
        }
        cur_dev = cur_dev->next;
        return NULL; // progress one device per call
    }

    if (stage == CONN_ENUM_1001) {
        hid_free_enumeration(devs);
        devs = hid_enumerate(0x1DC3, 0x1002);
        cur_dev = devs;
        stage = CONN_ENUM_1002;
        return NULL;
    }

    if (stage == CONN_ENUM_1002) {
        hid_free_enumeration(devs);
        devs = NULL;
        stage = CONN_IDLE;
    }

    return NULL;
}

sPoKeysDevice* PK_ConnectToPoKeysDevice_USB_Async(uint32_t serialNumber, uint32_t flags)
{
    static sPoKeysDevice* tmpDevice = NULL;
    static struct hid_device_info *devs = NULL, *cur_dev = NULL;
    static int devRange = 0;
    static uint8_t serialSearch[8];
    static uint8_t deviceTypeRequested;

    if (!devs && !cur_dev) {
        deviceTypeRequested = (flags >> 1) & 0x7F;
        sprintf((char*)serialSearch, "x.%05u", serialNumber % 100000);
        devs = hid_enumerate(0x1DC3, 0x1001);
        cur_dev = devs;
        devRange = 0;
    }

    while (cur_dev) {
        if ((cur_dev->interface_number == 1 && devRange == 0) ||
            (cur_dev->interface_number == 0 && devRange == 1)) {

            if (cur_dev->serial_number && cur_dev->serial_number[0] != 'P') {
                int k;
                for (k = 1; k < 8 && cur_dev->serial_number[k] != 0; k++) {
                    if (cur_dev->serial_number[k] != serialSearch[k]) break;
                }

                if ((deviceTypeRequested == 2 && cur_dev->serial_number[0] != '2') ||
                    (deviceTypeRequested == 3 && cur_dev->serial_number[0] != '3') ||
                    (deviceTypeRequested == 4 && cur_dev->serial_number[0] != '4')) k = 0;

                if (k == 7) {
                    tmpDevice = hal_malloc(sizeof(sPoKeysDevice));
                    tmpDevice->devHandle = (void*)hid_open_path(cur_dev->path);
                    tmpDevice->devHandle2 = 0;
                    tmpDevice->connectionType = PK_DeviceType_USBDevice;

                    if (tmpDevice->devHandle) {
                        PK_InitializeNewDeviceAsync(tmpDevice);
                    } else {
                        hal_free(tmpDevice);
                        tmpDevice = NULL;
                    }
                    hid_free_enumeration(devs);
                    devs = NULL;
                    cur_dev = NULL;
                    return tmpDevice;
                }
            } else {
                tmpDevice = hal_malloc(sizeof(sPoKeysDevice));
                tmpDevice->devHandle = (void*)hid_open_path(cur_dev->path);
                tmpDevice->devHandle2 = 0;
                tmpDevice->connectionType = PK_DeviceType_USBDevice;

                if (tmpDevice->devHandle) {
                    PK_InitializeNewDeviceAsync(tmpDevice);
                    if (tmpDevice->DeviceData.SerialNumber == serialNumber) {
                        hid_free_enumeration(devs);
                        devs = NULL;
                        cur_dev = NULL;
                        return tmpDevice;
                    }
                    PK_CleanDeviceAsync(tmpDevice);
                    hal_free(tmpDevice);
                } else {
                    hal_free(tmpDevice);
                }
            }
        }
        cur_dev = cur_dev->next;

        if (!cur_dev && ++devRange == 1) {
            hid_free_enumeration(devs);
            devs = hid_enumerate(0x1DC3, 0x1001);
            cur_dev = devs;
        }
    }

    if (devs) {
        hid_free_enumeration(devs);
        devs = NULL;
    }

    return NULL; // not yet found
}

sPoKeysDevice* PK_ConnectToPoKeysDevice_Ethernet_Async(uint32_t serialNumber, uint32_t checkForNetworkDevicesAndTimeout, uint32_t flags)
{
    static sPoKeysNetworkDeviceSummary *devices = NULL;
    static int32_t numDevices = -1;
    static int currentIndex = 0;

    if (devices == NULL) {
        devices = hal_malloc(sizeof(sPoKeysNetworkDeviceSummary) * 16);
        numDevices = PK_SearchNetworkDevices(devices, checkForNetworkDevicesAndTimeout, serialNumber);
        if (numDevices > 16) numDevices = 16;
        currentIndex = 0;
    }

    while (currentIndex < numDevices) {
        if (devices[currentIndex].SerialNumber == serialNumber) {
            if (flags & 1) devices[currentIndex].useUDP = 1;
            sPoKeysDevice* tmpDevice = PK_ConnectToNetworkDevice(&devices[currentIndex]);
            if (tmpDevice) {
                PK_InitializeNewDeviceAsync(tmpDevice);
                hal_free(devices);
                devices = NULL;
                return tmpDevice;
            }
        }
        currentIndex++;
    }

    if (devices) {
        hal_free(devices);
        devices = NULL;
    }

    return NULL;
}

sPoKeysDevice *PK_ConnectToDeviceWSerial_Async(uint32_t serial, uint32_t timeout_ms)
{
    static sPoKeysDevice *dev = NULL;
    static connection_state_t state = IDLE;
    static uint64_t start_time_us = 0;

    switch (state)
    {
        case IDLE:
            dev = hal_malloc(sizeof(sPoKeysDevice));
            // Start discovery, initialize fields
            state = DISCOVERY_START;
            start_time_us = get_current_time_us();
            break;

        case DISCOVERY_START:
            // Start async search (if not already running)
            if (PK_SearchNetworkDevicesAsync_Start(...)) {
                state = DISCOVERY_RUNNING;
            }
            break;

        case DISCOVERY_RUNNING:
            if (PK_SearchNetworkDevicesAsync_Process(...) == PK_OK_FOUND) {
                // Got device
                state = CONNECTING;
            } else if (timeout expired) {
                state = IDLE;
                return NULL;
            }
            break;

        case CONNECTING:
            dev = PK_ConnectToNetworkDeviceAsync(...); // internally also non-blocking
            if (dev != NULL) {
                state = DONE;
            }
            break;

        case DONE:
            state = IDLE;
            return dev;
    }

    return NULL; // still connecting
}
