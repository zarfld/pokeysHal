/**
 * @file PoKeysLibCoreAsync.c
 * @brief Asynchronous variants of core communication helpers.
 *
 * This file provides non-blocking versions of the USB enumeration and
 * request sending routines. They adhere to the same packet format as the
 * synchronous functions defined in PoKeysLibCore.c.
 */
#include <stdlib.h>
//#include "PoKeysLibCoreAsync_USB.h"
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

/**
 * @brief Begin or continue asynchronous USB enumeration.
 *
 * The enumeration context maintains its own state machine. Calling this
 * function repeatedly will progress through device discovery according to
 * the PoKeys protocol. When enumeration completes the function returns the
 * number of detected USB devices.
 *
 * @param ctx Enumeration context structure. Pass NULL to abort.
 * @return -2 while enumeration is in progress, a non-negative device
 *         count on completion or -1 on invalid parameters.
 */
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
    rtapi_print_msg(RTAPI_MSG_ERR, "PoKeys: %s:%s: Initializing new device...\n", __FILE__, __FUNCTION__);
    
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
//sPoKeysAnalogData
    device->AnalogInput = (sPoKeysAnalogData*)hal_malloc(sizeof(sPoKeysAnalogData) * 7);
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

    device->Pins = NULL;

    device->Encoders = NULL;

    device->PWM.PWMduty = NULL;

    device->PWM.PWMenabledChannels = NULL;

    device->PWM.PWMpinIDs = NULL;

    device->PoExtBusData = NULL;

    device->MatrixLED = NULL;

    if (device->multiPartBuffer) {
        device->multiPartBuffer = NULL;
    }

    if (device->EasySensors) {
        device->EasySensors = NULL;
    }

    if (device->netDeviceData) {
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
                    //free(tmpDevice);
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
                        //free(tmpDevice);
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
                    //free(tmpDevice);
                } else {
                    //free(tmpDevice);
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
                //free(devices);
                devices = NULL;
                return tmpDevice;
            }
        }
        currentIndex++;
    }

    if (devices) {
        //free(devices);
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

/**
 * @brief Enumerate PoKeys USB devices (blocking helper).
 *
 * This function performs the same enumeration as
 * PK_EnumerateUSBDevicesAsync() but in a blocking manner.
 *
 * @return Number of detected devices.
 */
int32_t PK_EnumerateUSBDevices()
{
    int32_t numDevices = 0;
    struct hid_device_info *devs, *cur_dev;

    devs = hid_enumerate(0x1DC3, 0x1001);
    cur_dev = devs;

    while (cur_dev)
    {
        if (PKI_CheckInterface(cur_dev))
        {
            numDevices++;
        }

        cur_dev = cur_dev->next;
    }
    hid_free_enumeration(devs);

    devs = hid_enumerate(0x1DC3, 0x1002);
    cur_dev = devs;

    while (cur_dev)
    {
        if (cur_dev->interface_number == -1) numDevices++;
        cur_dev = cur_dev->next;
    }
    hid_free_enumeration(devs);

#ifdef POKEYSLIB_USE_LIBUSB
    numDevices += PK_EnumerateFastUSBDevices();
#endif
    return numDevices;
}

/**
 * @brief Return the currently used connection type.
 *
 * This is an inline helper used by async routines to determine whether
 * communication is performed over USB, network or fast USB.
 *
 * @param device PoKeys device handle.
 * @return Connection type constant from ::ePK_DeviceConnectionType.
 */
int32_t PK_GetCurrentDeviceConnectionType(sPoKeysDevice* device)
{
    return device->connectionType;
}

void InitializeNewDevice(sPoKeysDevice* device)
{
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


    for (i = 0; i < device->info.iPinCount; i++)
    {
        if (PK_IsCounterAvailable(device, i))
        {
            device->Pins[i].DigitalCounterAvailable = 1;
        } else
        {
            device->Pins[i].DigitalCounterAvailable = 0;
        }
    }

    device->Encoders = (sPoKeysEncoder*)hal_malloc(sizeof(sPoKeysEncoder) * device->info.iEncodersCount);
    memset(device->Encoders, 0, sizeof(sPoKeysEncoder) * device->info.iEncodersCount);

    if (device->info.iEasySensors)
    {
        device->EasySensors = (sPoKeysEasySensor*)hal_malloc(sizeof(sPoKeysEasySensor) * device->info.iEasySensors);
        memset(device->EasySensors, 0, sizeof(sPoKeysEasySensor) * device->info.iEasySensors);
    } else
    {
        device->EasySensors = NULL;
    }

    device->PWM.PWMduty = (hal_u32_t*)hal_malloc(sizeof(hal_u32_t) * device->info.iPWMCount);
    memset(device->PWM.PWMduty, 0, sizeof(hal_u32_t) * device->info.iPWMCount);

    device->AnalogOutput = (hal_adcout_t*)hal_malloc(sizeof(hal_adcout_t) * device->info.iPWMCount);
    memset(device->AnalogOutput, 0, sizeof(hal_adcout_t) * device->info.iPWMCount);
    
    if (device->info.iPWMCount > 0)
    {
        device->PWM.PWMenabledChannels = (unsigned char*)hal_malloc(sizeof(unsigned char) * device->info.iPWMCount);
        memset(device->PWM.PWMenabledChannels, 0, sizeof(unsigned char) * device->info.iPWMCount);
    }
    else
    {
        device->PWM.PWMenabledChannels = NULL;
    }
    device->PWM.PWMpinIDs = (unsigned char*)hal_malloc(sizeof(unsigned char) * device->info.iPWMCount);
    memset(device->PWM.PWMpinIDs, 0, sizeof(unsigned char) * device->info.iPWMCount);

    PK_FillPWMPinNumbers(device);

    device->PoExtBusData = (unsigned char*)hal_malloc(sizeof(unsigned char) * device->info.iPoExtBus);

    device->MatrixLED = (sPoKeysMatrixLED*)hal_malloc(sizeof(sPoKeysMatrixLED) * device->info.iMatrixLED);
    memset(device->MatrixLED, 0, sizeof(sPoKeysMatrixLED) * device->info.iMatrixLED);

    memset(&device->PEv2, 0, sizeof(sPoKeysPEv2));

    device-> multiPartBuffer = hal_malloc(512);
    if (device->multiPartBuffer <= 0) device->multiPartBuffer = 0;

#ifdef USE_ALIGN_TEST
    device->alignTest1 = 1;
    device->alignTest2 = 2;
    device->alignTest3 = 3;
    device->alignTest4 = 4;
    device->alignTest5 = 5;
    device->alignTest6 = 6;
    device->alignTest7 = 7;
    device->alignTest8 = 8;
    device->alignTest9 = 9;
    device->alignTest10 = 10;
    device->alignTest11 = 11;
#endif
}

void CleanDevice(sPoKeysDevice* device)
{    
    device->Pins = NULL;
    device->Encoders = NULL;
    device->PWM.PWMduty = NULL;
    device->PWM.PWMenabledChannels = NULL;
    device->PWM.PWMpinIDs = NULL;
    device->PoExtBusData = NULL;
    device->MatrixLED = NULL;

    if (device->multiPartBuffer != NULL)
    {
        device->multiPartBuffer = NULL;
    }

    if (device->EasySensors != NULL)
    {
        device->EasySensors = NULL;
    }

    if (device->netDeviceData != NULL)
    {
        device->netDeviceData = NULL;
    }
}

void PK_ReleaseDeviceStructure(sPoKeysDevice* device)
{
    CleanDevice(device);
}

void PK_CloneDeviceStructure(sPoKeysDevice* original, sPoKeysDevice *destination)
{
    destination->Pins = (sPoKeysPinData*)hal_malloc(sizeof(sPoKeysPinData) * original->info.iPinCount);
    destination->Encoders = (sPoKeysEncoder*)hal_malloc(sizeof(sPoKeysEncoder) * original->info.iEncodersCount);
    destination->PWM.PWMduty = (uint32_t*)hal_malloc(sizeof(uint32_t) * original->info.iPWMCount);
    destination->PWM.PWMenabledChannels = (unsigned char*)hal_malloc(sizeof(unsigned char) * original->info.iPWMCount);
    if (original->info.iPWMCount == 0) destination->PWM.PWMenabledChannels = NULL;
    destination->PWM.PWMpinIDs = (unsigned char*)hal_malloc(sizeof(unsigned char) * original->info.iPWMCount);
    destination->MatrixLED = (sPoKeysMatrixLED*)hal_malloc(sizeof(sPoKeysMatrixLED) * original->info.iMatrixLED);

    if (original->info.iEasySensors)
    {
        destination->EasySensors = (sPoKeysEasySensor*)hal_malloc(sizeof(sPoKeysEasySensor) * original->info.iEasySensors);
    } else
    {
        destination->EasySensors = 0;
    }

    if (original->netDeviceData != 0)
    {
        destination->netDeviceData = (sPoKeysNetworkDeviceInfo *)hal_malloc(sizeof(sPoKeysNetworkDeviceInfo));
        memcpy(destination->netDeviceData, original->netDeviceData, sizeof(sPoKeysNetworkDeviceInfo));
    } else
    {
        destination->netDeviceData = 0;
    }
    destination->PoExtBusData = (unsigned char*)hal_malloc(sizeof(unsigned char) * original->info.iPoExtBus);

    destination->devHandle = original->devHandle;
    destination->devHandle2 = original->devHandle2;

    destination->info = original->info;
    destination->DeviceData = original->DeviceData;

    memcpy(&destination->Pins[0], &original->Pins[0],
            original->info.iPinCount * sizeof(sPoKeysPinData));
    memcpy(&destination->Encoders[0], &original->Encoders[0],
            original->info.iEncodersCount * sizeof(sPoKeysEncoder));

    if (original->info.iEasySensors)
    {
        memcpy(&destination->EasySensors[0], &original->EasySensors[0],
                original->info.iEasySensors * sizeof(sPoKeysEasySensor));
    }

    destination->matrixKB = original->matrixKB;

    destination->PWM.PWMperiod = original->PWM.PWMperiod;
    memcpy(destination->PWM.PWMduty, original->PWM.PWMduty,
           sizeof(uint32_t) * original->info.iPWMCount);
    memcpy(destination->PWM.PWMenabledChannels, original->PWM.PWMenabledChannels,
           sizeof(unsigned char) * original->info.iPWMCount);
    memcpy(destination->PWM.PWMpinIDs, original->PWM.PWMpinIDs,
           sizeof(unsigned char) * original->info.iPWMCount);

    memcpy(destination->MatrixLED, original->MatrixLED,
           sizeof(sPoKeysMatrixLED) * original->info.iMatrixLED);

    destination->LCD = original->LCD;

    destination->PoNETmodule = original->PoNETmodule;
    destination->PoIL = original->PoIL;
    destination->RTC = original->RTC;

    destination->FastEncodersConfiguration =    original->FastEncodersConfiguration;
    destination->FastEncodersOptions =          original->FastEncodersOptions;
    destination->UltraFastEncoderConfiguration =original->UltraFastEncoderConfiguration;
    destination->UltraFastEncoderOptions =      original->UltraFastEncoderOptions;
    destination->UltraFastEncoderFilter =       original->UltraFastEncoderFilter;

    memcpy(destination->PoExtBusData, original->PoExtBusData, sizeof(unsigned char) * original->info.iPoExtBus);

    destination->connectionType = original->connectionType;
    destination->requestID = original->requestID;

}

void * PK_FastUSBConnectToDevice(uint32_t deviceIndex);

sPoKeysDevice* PK_ConnectToDevice(uint32_t deviceIndex)
{
    int32_t numDevices = 0;
    struct hid_device_info *devs, *cur_dev;
    sPoKeysDevice* tmpDevice;
    void * devData;

    devs = hid_enumerate(0x1DC3, 0x1001);
    cur_dev = devs;

    while (cur_dev)
    {
        if (cur_dev->interface_number == 1)
        {
            if (numDevices == deviceIndex)
            {
                tmpDevice = (sPoKeysDevice*)hal_malloc(sizeof(sPoKeysDevice));

                tmpDevice->devHandle = (void*)hid_open_path(cur_dev->path);
                tmpDevice->devHandle2 = NULL;

                tmpDevice->connectionType = PK_DeviceType_USBDevice;


                if (tmpDevice->devHandle != NULL)
                {
                    InitializeNewDevice(tmpDevice);
                } else
                {
                    //free(tmpDevice);
                    tmpDevice = NULL;
                }
                hid_free_enumeration(devs);
                return tmpDevice;
            }
            numDevices++;
        }
        cur_dev = cur_dev->next;
    }
    hid_free_enumeration(devs);

    devs = hid_enumerate(0x1DC3, 0x1002);
    cur_dev = devs;

    while (cur_dev)
    {
        if (cur_dev->interface_number == -1)
        {
            if (numDevices == deviceIndex)
            {
                tmpDevice = (sPoKeysDevice*)hal_malloc(sizeof(sPoKeysDevice));

                tmpDevice->devHandle = (void*)hid_open_path(cur_dev->path);
                tmpDevice->devHandle2 = NULL;

                tmpDevice->connectionType = PK_DeviceType_USBDevice;

                if (tmpDevice->devHandle != NULL)
                {
                    InitializeNewDevice(tmpDevice);
                } else
                {
                    //free(tmpDevice);
                    tmpDevice = NULL;
                }
                hid_free_enumeration(devs);
                return tmpDevice;
            }
            numDevices++;
        }
        cur_dev = cur_dev->next;
    }
    hid_free_enumeration(devs);

#ifdef POKEYSLIB_USE_LIBUSB
    devData = PK_FastUSBConnectToDevice(deviceIndex - numDevices);

    if (devData != NULL)
    {
        tmpDevice = (sPoKeysDevice*)hal_malloc(sizeof(sPoKeysDevice));

        tmpDevice->devHandle = NULL;
        tmpDevice->devHandle2 = devData;

        tmpDevice->connectionType = PK_DeviceType_FastUSBDevice;
        InitializeNewDevice(tmpDevice);
        return tmpDevice;
    }
#endif
    return NULL;
}

sPoKeysDevice* PK_ConnectToPoKeysDevice_USB(uint32_t serialNumber, uint32_t flags)
{
    int32_t numDevices = 0;
    struct hid_device_info *devs, *cur_dev;
    int32_t k;
    sPoKeysDevice* tmpDevice;
    uint8_t serialSearch[8];

    int devRange = 0;
    uint8_t deviceTypeRequested = (flags >> 1) & 0x7F;

#ifdef POKEYSLIB_USE_LIBUSB
    void * devData = ConnectToFastUSBInterface(serialNumber);
    if (devData != NULL)
    {
        tmpDevice = (sPoKeysDevice*)hal_malloc(sizeof(sPoKeysDevice));

        tmpDevice->devHandle = NULL;
        tmpDevice->devHandle2 = devData;

        tmpDevice->connectionType = PK_DeviceType_FastUSBDevice;
        InitializeNewDevice(tmpDevice);    
        return tmpDevice;
    }
#endif

    devs = hid_enumerate(0x1DC3, 0x1001);
    cur_dev = devs;

    sprintf((char*)serialSearch, "x.%05u", serialNumber % 100000);

    while (cur_dev)
    {
        if ((cur_dev->interface_number == 1 && devRange == 0) ||
            (cur_dev->interface_number == 0 && devRange == 1))
        {
            if (cur_dev->serial_number != 0 && cur_dev->serial_number[0] != 'P')
            {
                for (k = 1; k < 8 && cur_dev->serial_number[k] != 0; k++)
                {
                    if (cur_dev->serial_number[k] != serialSearch[k]) break;
                }

                if (deviceTypeRequested == 2 && cur_dev->serial_number[0] != '2') k = 0;
                if (deviceTypeRequested == 3 && cur_dev->serial_number[0] != '3') k = 0;
                if (deviceTypeRequested == 4 && cur_dev->serial_number[0] != '4') k = 0;

                if (k == 7)
                {
                    tmpDevice = (sPoKeysDevice*)hal_malloc(sizeof(sPoKeysDevice));

                    tmpDevice->devHandle = (void*)hid_open_path(cur_dev->path);
                    tmpDevice->devHandle2 = 0;

                    tmpDevice->connectionType = PK_DeviceType_USBDevice;
                    if (tmpDevice->devHandle != NULL)
                    {
                        InitializeNewDevice(tmpDevice);
                    }
                    else
                    {
                        //free(tmpDevice);
                        tmpDevice = NULL;
                    }
                    hid_free_enumeration(devs);
                    return tmpDevice;
                }
            }
            else
            {
                tmpDevice = (sPoKeysDevice*)hal_malloc(sizeof(sPoKeysDevice));
                tmpDevice->devHandle = (void*)hid_open_path(cur_dev->path);
                tmpDevice->devHandle2 = 0;

                if (tmpDevice->devHandle != NULL)
                {
                    InitializeNewDevice(tmpDevice);
                }
                else
                {
                    //free(tmpDevice);
                    tmpDevice = NULL;
                    hid_free_enumeration(devs);
                    return NULL;
                }

                tmpDevice->connectionType = PK_DeviceType_USBDevice;
                if (tmpDevice->DeviceData.SerialNumber == serialNumber)
                {
                    hid_free_enumeration(devs);
                    return tmpDevice;
                }
                else
                {
                    CleanDevice(tmpDevice);
                    //free(tmpDevice);
                }
            }

            numDevices++;
        }
        cur_dev = cur_dev->next;

        if (cur_dev == NULL)
        {
            devRange++;
            switch (devRange)
            {
                case 1:
                    hid_free_enumeration(devs);
                    devs = hid_enumerate(0x1DC3, 0x1001);
                    cur_dev = devs;
                    break;
            }
        }
    }
    hid_free_enumeration(devs);
    return NULL;
}

sPoKeysDevice* PK_ConnectToPoKeysDevice_Ethernet(uint32_t serialNumber, uint32_t checkForNetworkDevicesAndTimeout, uint32_t flags)
{
    int32_t k;
    sPoKeysDevice* tmpDevice;
    sPoKeysNetworkDeviceSummary * devices;
    int32_t iNet;

    if (checkForNetworkDevicesAndTimeout)
    {
        devices = (sPoKeysNetworkDeviceSummary*)hal_malloc(sizeof(sPoKeysNetworkDeviceSummary) * 16);
        iNet = PK_SearchNetworkDevices(devices, checkForNetworkDevicesAndTimeout, serialNumber);

        if (iNet > 16) iNet = 16;

        for (k = 0; k < iNet; k++)
        {
            if (devices[k].SerialNumber == serialNumber)
            {
                if (flags & 1) devices[k].useUDP = 1;
                tmpDevice = PK_ConnectToNetworkDevice(&devices[k]);
                if (tmpDevice == NULL)
                {
                    //free(tmpDevice);
                }
                else
                {
                    //free(devices);
                    InitializeNewDevice(tmpDevice);
                    return tmpDevice;
                }
            }
        }
        //free(devices);
    }

    return NULL;
}

sPoKeysDevice* PK_ConnectToPoKeysDevice(uint32_t serialNumber, uint32_t checkForNetworkDevicesAndTimeout, uint32_t flags)
{
    sPoKeysDevice* tmpDevice = NULL;

    if ((flags & (1 << 8)) && checkForNetworkDevicesAndTimeout > 0)
    {
        tmpDevice = PK_ConnectToPoKeysDevice_Ethernet(serialNumber, checkForNetworkDevicesAndTimeout, flags);
    }

    if (tmpDevice == NULL)
    {
        tmpDevice = PK_ConnectToPoKeysDevice_USB(serialNumber, flags);
    }
    if (tmpDevice == NULL && ((flags & (1 << 8)) == 0) && checkForNetworkDevicesAndTimeout > 0)
    {
        tmpDevice = PK_ConnectToPoKeysDevice_Ethernet(serialNumber, checkForNetworkDevicesAndTimeout, flags);
    }
    return tmpDevice;
}

sPoKeysDevice* PK_ConnectToDeviceWSerial(uint32_t serialNumber, uint32_t checkForNetworkDevicesAndTimeout)
{
    return PK_ConnectToPoKeysDevice(serialNumber, checkForNetworkDevicesAndTimeout, 0);
}

sPoKeysDevice* PK_ConnectToDeviceWSerial_UDP(uint32_t serialNumber, uint32_t checkForNetworkDevicesAndTimeout)
{
    return PK_ConnectToPoKeysDevice(serialNumber, checkForNetworkDevicesAndTimeout, 1);
}

void PK_DisconnectDevice(sPoKeysDevice* device)
{
    if (device != NULL)
    {
        if (device->connectionType == PK_DeviceType_NetworkDevice) 
        {
            PK_DisconnectNetworkDevice(device);
        } else
        {
#ifdef POKEYSLIB_USE_LIBUSB
            DisconnectFromFastUSBInterface(device->devHandle2);
            device->devHandle2 = NULL;
#endif

            if ((hid_device*)device->devHandle != NULL)
            {
                hid_close((hid_device*)device->devHandle);
            }
        }

        CleanDevice(device);
        //free(device);
    }
}

/**
 * @brief Fill a request buffer with a PoKeys command.
 *
 * This helper mirrors the synchronous CreateRequest() implementation
 * and prepares a 64 byte packet header according to the protocol.
 *
 * @param request Buffer that will receive the command bytes.
 * @param type    Command ID (PK_CMD_* constant).
 * @param param1  Parameter 1.
 * @param param2  Parameter 2.
 * @param param3  Parameter 3.
 * @param param4  Parameter 4.
 * @return PK_OK on success or PK_ERR_NOT_CONNECTED when @p request is NULL.
 */
int32_t CreateRequest(unsigned char * request, unsigned char type, unsigned char param1, unsigned char param2, unsigned char param3, unsigned char param4)
{
    if (request == NULL) return PK_ERR_NOT_CONNECTED;

    memset(request, 0, 64);

    request[1] = type;
    request[2] = param1;
    request[3] = param2;
    request[4] = param3;
    request[5] = param4;

    return PK_OK;
}

/**
 * @brief Send a command composed from the provided parameters.
 *
 * A convenience wrapper that fills @p device->request using
 * CreateRequest() semantics and then calls SendRequest().
 *
 * @param device Target device instance.
 * @param type   Command ID.
 * @param param1 Parameter 1.
 * @param param2 Parameter 2.
 * @param param3 Parameter 3.
 * @param param4 Parameter 4.
 * @return Result of SendRequest().
 */
int32_t PK_CustomRequest(sPoKeysDevice* device, unsigned char type, unsigned char param1, unsigned char param2, unsigned char param3, unsigned char param4)
{
    device->request[1] = type;
    device->request[2] = param1;
    device->request[3] = param2;
    device->request[4] = param3;
    device->request[5] = param4;

    return SendRequest(device);
}

uint8_t getChecksum(uint8_t * data)
{
    uint8_t temp = 0;
    uint32_t i;

    for (i = 0; i < 7; i++)
    {
            temp += data[i];
    }
    return temp;
}

int32_t LastRetryCount = 0;
int32_t LastWaitCount = 0;

/**
 * @brief Send a multi-part request buffer.
 *
 * For large data transfers the packet is split into 8 blocks and
 * transmitted using network or fast USB helpers. Behaviour matches the
 * synchronous SendRequest_multiPart() routine.
 *
 * @param device Device handle containing prepared multipart data.
 * @return PK_OK on success or PK_ERR_TRANSFER on failure.
 */
int32_t SendRequest_multiPart(sPoKeysDevice* device)
{
    if (device == NULL) return PK_ERR_GENERIC;
    if (device->connectionType == PK_DeviceType_NetworkDevice)
    {
        return SendEthRequestBig(device);
    }

#ifdef POKEYSLIB_USE_LIBUSB
    if (device->connectionType == PK_DeviceType_FastUSBDevice)
        return SendRequestFastUSB_multiPart(device);
    else
        return PK_ERR_TRANSFER;
#else
    return PK_ERR_TRANSFER;
#endif
}

/**
 * @brief Send a request and wait for a response (async context).
 *
 * Formats the current request buffer, transmits it over the selected
 * interface and waits for the reply in a blocking loop. Functionally
 * identical to the synchronous version.
 *
 * @param device Target device instance.
 * @return PK_OK on success or PK_ERR_TRANSFER on failure.
 */
int32_t SendRequest(sPoKeysDevice* device)
{
    uint32_t waits = 0;
    uint32_t retries = 0;
    int32_t result = 0;
    uint8_t bufferOut[65] = {0};

    hid_device * devHandle;

    if (device == NULL) return PK_ERR_GENERIC;
    if (device->connectionType == PK_DeviceType_NetworkDevice)
    {
        return SendEthRequest(device);
    }
#ifdef POKEYSLIB_USE_LIBUSB
        if (device->connectionType == PK_DeviceType_FastUSBDevice)
            return SendRequestFastUSB(device);
#endif
    devHandle = (hid_device*)device->devHandle;

    if (devHandle == NULL) return PK_ERR_GENERIC;

    while (retries++ < 2)
    {
        device->request[0] = 0xBB;
        device->request[6] = ++device->requestID;
        device->request[7] = getChecksum(device->request);

        memcpy(bufferOut + 1, device->request, 64);

        result = hid_write(devHandle, bufferOut, 65);

        if (result < 0)
        {
            retries++;
            continue;
        }

        waits = 0;

        while (waits++ < 50)
        {
            result = hid_read(devHandle, device->response, 65);

            if (result < 0)
            {
                    break;
            }

                if (device->response[0] == 0xAA && device->response[6] == device->requestID)
                {
                    if (device->response[7] == getChecksum(device->response))
                    {
                        LastRetryCount = retries;
                        LastWaitCount = waits;
                        return PK_OK;
                    }
                }
            }
    }

    return PK_ERR_TRANSFER;
}

/**
 * @brief Transmit a request without reading any reply.
 *
 * The request buffer is formatted the same as in SendRequest() but the
 * function returns immediately after sending the packet. Used for
 * commands that do not generate a direct response.
 *
 * @param device Target device instance.
 * @return PK_OK on success or PK_ERR_TRANSFER on failure.
 */
int32_t SendRequest_NoResponse(sPoKeysDevice* device)
{
    uint32_t waits = 0;
    uint32_t retries = 0;
    int32_t result = 0;
    uint8_t bufferOut[65] = {0};

    hid_device * devHandle;

    if (device == NULL) return PK_ERR_GENERIC;
    if (device->connectionType == PK_DeviceType_NetworkDevice)
    {
        return SendEthRequest_NoResponse(device);
    }
#ifdef POKEYSLIB_USE_LIBUSB
        if (device->connectionType == PK_DeviceType_FastUSBDevice)
            return SendRequestFastUSB_NoResponse(device);
#endif
    devHandle = (hid_device*)device->devHandle;

    if (devHandle == NULL) return PK_ERR_GENERIC;

    while (retries++ < 2)
    {
        device->request[0] = 0xBB;
        device->request[6] = ++device->requestID;
        device->request[7] = getChecksum(device->request);

        memcpy(bufferOut + 1, device->request, 64);

        result = hid_write(devHandle, bufferOut, 65);

        if (result < 0)
        {
            retries++;
            continue;
        }
    }

    return PK_ERR_TRANSFER;
}

void PK_SL_SetPinFunction(sPoKeysDevice* device, uint8_t pin, uint8_t function)
{
    device->Pins[pin].PinFunction = function;
}

uint8_t PK_SL_GetPinFunction(sPoKeysDevice* device, uint8_t pin)
{
    return device->Pins[pin].PinFunction;
}

void PK_SL_DigitalOutputSet(sPoKeysDevice* device, uint8_t pin, uint8_t value)
{
    device->Pins[pin].DigitalValueSet = value;
}

uint8_t PK_SL_DigitalInputGet(sPoKeysDevice* device, uint8_t pin)
{
    return device->Pins[pin].DigitalValueGet;
}

uint32_t PK_SL_AnalogInputGet(sPoKeysDevice* device, uint8_t pin)
{
    return device->Pins[pin].AnalogValue;
}
