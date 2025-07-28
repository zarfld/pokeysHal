/*
 * PoKeysLibSPIAsync.c
 *
 * Asynchronous SPI helpers using PoKeysLibAsync framework.
 * These mirror the blocking functions found in PoKeysLibSPI.c
 * but avoid any blocking socket operations for RT compatibility.
 *
 * Copyright (C) 2025 - RT-safe implementation for LinuxCNC compatibility
 */

#include "PoKeysLibHal.h"
#include "PoKeysLibAsync.h"
#include <string.h>

/**
 * @brief Configure the SPI interface asynchronously (command 0xE5/0x01).
 *
 * Non-blocking version of PK_SPIConfigure that schedules the request
 * using the async framework.
 *
 * @param device      Pointer to an initialised device structure.
 * @param prescaler   SPI clock prescaler value.
 * @param frameFormat Frame configuration (mode, bit length etc.).
 *
 * @return Request ID on success or negative error code on failure.
 */
int PK_SPIConfigureAsync(sPoKeysDevice* device, uint8_t prescaler, uint8_t frameFormat)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    
    uint8_t params[4] = {0x01, prescaler, frameFormat, 0};
    
    int req_id = CreateRequestAsync(device, PK_CMD_SPI_COMMUNICATION,
                                    params, 4, NULL, 0, NULL);
    if (req_id < 0) return req_id;
    
    return SendRequestAsync(device, (uint8_t)req_id);
}

/**
 * @brief Write data to SPI bus asynchronously (command 0xE5/0x02).
 *
 * Non-blocking version of PK_SPIWrite that schedules the write operation
 * using the async framework.
 *
 * @param device      Pointer to device structure.
 * @param buffer      Data buffer to transmit.
 * @param dataLength  Number of bytes to transmit (max 54).
 * @param pinCS       Chip select pin number.
 *
 * @return Request ID on success or negative error code on failure.
 */
int PK_SPIWriteAsync(sPoKeysDevice* device, const uint8_t* buffer, uint8_t dataLength, uint8_t pinCS)
{
    if (!device || !buffer) return PK_ERR_NOT_CONNECTED;
    if (dataLength > 54) return PK_ERR_GENERIC; // Protocol limitation
    
    uint8_t params[4] = {0x02, dataLength, pinCS, 0};
    
    int req_id = CreateRequestAsyncWithPayload(device, PK_CMD_SPI_COMMUNICATION,
                                               params, 4, buffer, dataLength, NULL);
    if (req_id < 0) return req_id;
    
    return SendRequestAsync(device, (uint8_t)req_id);
}

/**
 * @brief Read data from SPI bus asynchronously (command 0xE5/0x03).
 *
 * Non-blocking version of PK_SPIRead that schedules the read operation.
 * The response data will be available in the device response buffer after
 * PK_ReceiveAndDispatch processes the reply.
 *
 * @param device      Pointer to device structure.
 * @param buffer      Buffer to receive data (can be NULL for async operation).
 * @param dataLength  Number of bytes to read.
 *
 * @return Request ID on success or negative error code on failure.
 */
int PK_SPIReadAsync(sPoKeysDevice* device, uint8_t* buffer, uint8_t dataLength)
{
    if (!device) return PK_ERR_NOT_CONNECTED;
    if (dataLength > 54) return PK_ERR_GENERIC; // Protocol limitation
    
    uint8_t params[4] = {0x03, dataLength, 0, 0};
    
    int req_id = CreateRequestAsync(device, PK_CMD_SPI_COMMUNICATION,
                                    params, 4, buffer, dataLength, NULL);
    if (req_id < 0) return req_id;
    
    return SendRequestAsync(device, (uint8_t)req_id);
}

/**
 * @brief Perform SPI write/read transaction asynchronously (command 0xE5/0x04).
 *
 * Combined write/read operation that transmits data while simultaneously
 * receiving data from the SPI device. This is useful for SPI devices that
 * continuously clock out data during transactions.
 *
 * @param device      Pointer to device structure.
 * @param txBuffer    Buffer containing data to transmit.
 * @param rxBuffer    Buffer to receive data (can be NULL for async operation).
 * @param dataLength  Number of bytes to transfer.
 * @param pinCS       Chip select pin number.
 *
 * @return Request ID on success or negative error code on failure.
 */
int PK_SPITransferAsync(sPoKeysDevice* device, const uint8_t* txBuffer, 
                        uint8_t* rxBuffer, uint8_t dataLength, uint8_t pinCS)
{
    if (!device || !txBuffer) return PK_ERR_NOT_CONNECTED;
    if (dataLength > 54) return PK_ERR_GENERIC; // Protocol limitation
    
    uint8_t params[4] = {0x04, dataLength, pinCS, 0};
    
    int req_id = CreateRequestAsyncWithPayload(device, PK_CMD_SPI_COMMUNICATION,
                                               params, 4, txBuffer, dataLength, NULL);
    if (req_id < 0) return req_id;
    
    return SendRequestAsync(device, (uint8_t)req_id);
}
