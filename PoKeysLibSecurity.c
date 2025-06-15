#include "PoKeysLibHal.h"
#include "PoKeysLibCore.h"
#include "PoKeysLibAsync.h"
#include <string.h>
/**
 * @file PoKeysLibSecurity.c
 * @brief Security and authorisation helpers (commands 0xE1-0xE3).
 *
 * Provides functions for querying security level, authorising
 * users and changing passwords.
 */

/**
 * @brief Retrieve the device security level (command 0xE1).
 *
 * Reads current authorisation level and 32-byte seed used for
 * password hashing.
 *
 * @param device Pointer to an opened PoKeys device.
 * @param level  Optional pointer to store the security level.
 * @param seed   Optional 32-byte buffer for the random seed.
 * @return PK_OK on success or PK_ERR_* on failure.
 */
int32_t PK_SecurityStatusGet(sPoKeysDevice* device, uint8_t* level, uint8_t* seed)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    CreateRequest(device->request, PK_CMD_SECURITY_STATUS_GET, 0, 0, 0, 0);
    if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;

    if (level) *level = device->response[8];
    if (seed) memcpy(seed, device->response + 9, 32);
    return PK_OK;
}

/**
 * @brief Authorise user with a password hash (command 0xE2).
 *
 * Sends the SHA1 hash of the password at the requested access
 * level and returns the authorisation result.
 *
 * @param device Pointer to an opened PoKeys device.
 * @param level  Requested security level.
 * @param hash   Pointer to a 20-byte password hash.
 * @param status Optional pointer to store the result.
 * @return PK_OK on success or PK_ERR_* on failure.
 */
int32_t PK_UserAuthorise(sPoKeysDevice* device, uint8_t level, const uint8_t* hash, uint8_t* status)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    CreateRequest(device->request, PK_CMD_USER_AUTHORISE, level, 0, 0, 0);
    memcpy(device->request + 8, hash, 20);
    if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;

    if (status) *status = device->response[8];
    return PK_OK;
}

/**
 * @brief Set a new user password (command 0xE3).
 *
 * Stores a SHA1 hash of the password for the selected default
 * security level.
 *
 * @param device       Pointer to an opened PoKeys device.
 * @param defaultLevel Level assigned to the password.
 * @param password     Pointer to 32-byte password hash.
 * @return PK_OK on success or PK_ERR_* on failure.
 */
int32_t PK_UserPasswordSet(sPoKeysDevice* device, uint8_t defaultLevel, const uint8_t* password)
{
    if (device == NULL) return PK_ERR_NOT_CONNECTED;

    CreateRequest(device->request, PK_CMD_USER_PASSWORD_SET, defaultLevel, 0, 0, 0);
    memcpy(device->request + 8, password, 32);
    if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;

    return PK_OK;
}
