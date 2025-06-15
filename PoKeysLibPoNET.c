/*

Copyright (C) 2013 Matev� Bo�nak (matevz@poscope.com)

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*/


#include "PoKeysLibHal.h"
#include "PoKeysLibCore.h"
#include "PoKeysLibAsync.h"

/**
 * @brief Read overall PoNET bus status.
 *
 * Issues command ::PONET_OP_GET_STATUS using ::PK_CMD_POI2C_COMMUNICATION and
 * stores the returned status byte in ::sPoNETmodule::PoNETstatus.
 *
 * @param device Pointer to an initialized PoKeys device structure.
 * @return ::PK_OK on success, ::PK_ERR_NOT_CONNECTED if @p device is NULL or
 *         ::PK_ERR_TRANSFER on communication failure.
 */
int32_t PK_PoNETGetPoNETStatus(sPoKeysDevice* device)
{
  if (device == NULL) return PK_ERR_NOT_CONNECTED;

  CreateRequest(device->request, PK_CMD_POI2C_COMMUNICATION, PONET_OP_GET_STATUS, 0, 0, 0);
  if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;

  device->PoNETmodule.PoNETstatus = device->response[8];

  return PK_OK;
}

/**
 * @brief Retrieve basic information about the selected PoNET module.
 *
 * Sends ::PONET_OP_GET_MODULE_SETTINGS which returns I\2C address,
 * module type, module size and option flags. The values are stored in the
 * ::sPoNETmodule structure of @p device.
 *
 * @param device Pointer to an initialized PoKeys device structure.
 * @return ::PK_OK on success, ::PK_ERR_NOT_CONNECTED if @p device is NULL or
 *         ::PK_ERR_TRANSFER on communication failure.
 */
int32_t PK_PoNETGetModuleSettings(sPoKeysDevice* device)
{
  if (device == NULL) return PK_ERR_NOT_CONNECTED;

  CreateRequest(device->request, PK_CMD_POI2C_COMMUNICATION, PONET_OP_GET_MODULE_SETTINGS, device->PoNETmodule.moduleID, 0, 0);
  if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;

  device->PoNETmodule.i2cAddress = device->response[8];
  device->PoNETmodule.moduleType = device->response[9];
  device->PoNETmodule.moduleSize = device->response[10];
  device->PoNETmodule.moduleOptions = device->response[11];

  return PK_OK;
}



/**
 * @brief Request status bytes from the PoNET module.
 *
 * Executes ::PONET_OP_GET_MODULE_DATA with subcommand @c 0x10 which instructs
 * the module to prepare its status information for later retrieval using
 * PK_PoNETGetModuleStatus().
 *
 * @param device Pointer to an initialized PoKeys device structure.
 * @return ::PK_OK on success, ::PK_ERR_NOT_CONNECTED if @p device is NULL or
 *         ::PK_ERR_TRANSFER on communication failure.
 */
int32_t PK_PoNETGetModuleStatusRequest(sPoKeysDevice* device)
{
  if (device == NULL) return PK_ERR_NOT_CONNECTED;

  CreateRequest(device->request, PK_CMD_POI2C_COMMUNICATION, PONET_OP_GET_MODULE_DATA, 0x10, device->PoNETmodule.moduleID, 0);
  if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;
  
  return PK_OK;
}

/**
 * @brief Obtain previously requested module status bytes.
 *
 * After a call to PK_PoNETGetModuleStatusRequest(), this function reads the
 * 16 status bytes from the device using ::PONET_OP_GET_MODULE_DATA with
 * subcommand @c 0x30. The bytes are copied to ::sPoNETmodule::statusIn.
 *
 * @param device Pointer to an initialized PoKeys device structure.
 * @return ::PK_OK on success, ::PK_ERR_NOT_CONNECTED if @p device is NULL,
 *         ::PK_ERR_TRANSFER on communication failure or ::PK_ERR_GENERIC when
 *         the response format is invalid.
 */
int32_t PK_PoNETGetModuleStatus(sPoKeysDevice* device)
{
  if (device == NULL) return PK_ERR_NOT_CONNECTED;

  CreateRequest(device->request, PK_CMD_POI2C_COMMUNICATION, PONET_OP_GET_MODULE_DATA, 0x30, 0, 0);
  if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;

  if (device->response[3] != 1) return PK_ERR_GENERIC;
  if (device->response[8] != 0) return PK_ERR_GENERIC;
  memcpy(device->PoNETmodule.statusIn, device->response + 9, 16);

  return PK_OK;
}



/**
 * @brief Write output status bytes to the PoNET module.
 *
 * Transmits the 16 bytes stored in ::sPoNETmodule::statusOut using
 * ::PONET_OP_SET_MODULE_DATA.
 *
 * @param device Pointer to an initialized PoKeys device structure.
 * @return ::PK_OK on success, ::PK_ERR_NOT_CONNECTED if @p device is NULL or
 *         ::PK_ERR_TRANSFER on communication failure.
 */
int32_t PK_PoNETSetModuleStatus(sPoKeysDevice* device)
{
  uint32_t i;

  if (device == NULL) return PK_ERR_NOT_CONNECTED;

  CreateRequest(device->request, PK_CMD_POI2C_COMMUNICATION, PONET_OP_SET_MODULE_DATA, device->PoNETmodule.moduleID, 0, 0);
  for (i = 0; i < 16; i++)
  {
    device->request[8 + i] = device->PoNETmodule.statusOut[i];
  }
  
  if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;
  return PK_OK;
}



/**
 * @brief Set the PWM duty cycle of a PoNET module.
 *
 * Uses ::PONET_OP_SET_PWM_VALUE to write the value stored in
 * ::sPoNETmodule::PWMduty to the module identified by ::sPoNETmodule::moduleID.
 *
 * @param device Pointer to an initialized PoKeys device structure.
 * @return ::PK_OK on success, ::PK_ERR_NOT_CONNECTED if @p device is NULL or
 *         ::PK_ERR_TRANSFER on communication failure.
 */
int32_t PK_PoNETSetModulePWM(sPoKeysDevice* device)
{
  if (device == NULL) return PK_ERR_NOT_CONNECTED;

  CreateRequest(device->request, PK_CMD_POI2C_COMMUNICATION, PONET_OP_SET_PWM_VALUE, device->PoNETmodule.moduleID, device->PoNETmodule.PWMduty, 0);
  
  if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;
  return PK_OK;
}

/**
 * @brief Request the current value of the module's light sensor.
 *
 * Sends ::PONET_OP_GET_LIGHT_SENSOR with subcommand @c 0x10 to trigger the
 * reading of the light sensor. The value can later be read by
 * PK_PoNETGetModuleLight().
 *
 * @param device Pointer to an initialized PoKeys device structure.
 * @return ::PK_OK on success, ::PK_ERR_NOT_CONNECTED if @p device is NULL or
 *         ::PK_ERR_TRANSFER on communication failure.
 */
int32_t PK_PoNETGetModuleLightRequest(sPoKeysDevice* device)
{
  if (device == NULL) return PK_ERR_NOT_CONNECTED;

  CreateRequest(device->request, PK_CMD_POI2C_COMMUNICATION, PONET_OP_GET_LIGHT_SENSOR, 0x10, device->PoNETmodule.moduleID, 0);
  
  if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;
  return PK_OK;
}

/**
 * @brief Read the light sensor value previously requested.
 *
 * Executes ::PONET_OP_GET_LIGHT_SENSOR with subcommand @c 0x30 to obtain the
 * measured light value which is stored in ::sPoNETmodule::lightValue.
 *
 * @param device Pointer to an initialized PoKeys device structure.
 * @return ::PK_OK on success, ::PK_ERR_NOT_CONNECTED if @p device is NULL,
 *         ::PK_ERR_TRANSFER on communication failure or ::PK_ERR_GENERIC when
 *         the returned status indicates an error.
 */
int32_t PK_PoNETGetModuleLight(sPoKeysDevice* device)
{
  if (device == NULL) return PK_ERR_NOT_CONNECTED;

  CreateRequest(device->request, PK_CMD_POI2C_COMMUNICATION, PONET_OP_GET_LIGHT_SENSOR, 0x30, device->PoNETmodule.moduleID, 0);
  
  if (SendRequest(device) != PK_OK) return PK_ERR_TRANSFER;
  if (device->response[8] != 0) return PK_ERR_GENERIC;

  device->PoNETmodule.lightValue = device->response[9];

  return PK_OK;
}


