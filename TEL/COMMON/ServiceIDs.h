/**
 * @file          ServiceIDs.h
 * @brief         Header file containing all the service ids associated with
 *                UART based TX-RX communication between RH850 and Modem.
 *                These are supposed to be application specific IDs and will be
 *                mostly generated in collaboration with Application developers
 *                and Diagnostics Team.
 *                This file should primarily be used in conjunction with UART
 *                Service.
 * @date          21/07/2026
 * @author        Yash Giramkar [YSG], Aditya Bhosale [ASB]
 * @copyright     Bajaj Auto Technology Limited (BATL)
 */

#ifndef _SERVICE_IDS_H
#define _SERVICE_IDS_H

/******************************************************************************/
/*                                                                            */
/*                                  INCLUDES                                  */
/*                                                                            */
/******************************************************************************/
#include <stdint.h>
#include <stdbool.h>

/******************************************************************************/
/*                                                                            */
/*                                  DEFINES                                   */
/*                                                                            */
/******************************************************************************/


/******************************************************************************/
/*                                                                            */
/*                                 TYPEDEFS                                   */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/*                                   ENUMS                                    */
/*                                                                            */
/******************************************************************************/
/**
 * @enum          ServiceIDs_E
 * @brief         Global Enum containing all the service IDs.
 */
typedef enum
{
   /**************************************************************************
    * CTS for all Metadata Requests
    **************************************************************************/
   eSERVICE_ID_CTS = 0,
   eSERVICE_ID_DATA_SEND_ACK,
   eSERVICE_ID_DATA_SEND_NACK,
   /**************************************************************************
    * General Application
    **************************************************************************/
   eSERVICE_ID_GENERAL_GET_STATUS,
   eSERVICE_ID_GENERAL_SET_DEBUG_MODE,
   eSERVICE_ID_GENERAL_GET_IMEI,
   eSERVICE_ID_GENERAL_GET_ICCID,
   eSERVICE_ID_GENERAL_GET_SERIAL_NUMBER,
   eSERVICE_ID_GENERAL_GET_PRODUCT_ID,
   eSERVICE_ID_GENERAL_GET_MODEL_INFO,
   eSERVICE_ID_GENERAL_GET_FIRMWARE_VERSION,
   eSERVICE_ID_GENERAL_GET_SUBVERSION,
   eSERVICE_ID_GENERAL_GET_TEMPERATURE,
   eSERVICE_ID_GENERAL_GET_VIN,
   eSERVICE_ID_GENERAL_GET_SIM_STATUS,
   eSERVICE_ID_GENERAL_GET_NW_STATUS,
   eSERVICE_ID_GENERAL_GET_NW_STRENGTH,
   eSERVICE_ID_GENERAL_GET_NW_TIME,

   /**************************************************************************
    * FOTA Application
    **************************************************************************/
   eSERVICE_ID_FOTA_NOTIFY,
   eSERVICE_ID_FOTA_INSTALL_REQ,
   eSERVICE_ID_FOTA_DEPLOY_REQ,
   eSERVICE_ID_FOTA_REBOOT_REQ,

   /**************************************************************************
    * OA Applciation
    **************************************************************************/
   eSERVICE_ID_OA_DATA_NOTIFY,
   eSERVICE_ID_OA_DATA_PUSH_TO_MODEM,
   eSERVICE_ID_OA_DATA_PUSH_TO_CLOUD,

   /**************************************************************************
    * Manifest Application
    **************************************************************************/
   eSERVICE_ID_MANIFEST_REQ,


   /**************************************************************************
    * Provisioning
    **************************************************************************/
   eSERVICE_ID_PROV_VIN_WRITE,
   eSERVICE_ID_PROV_GEN_PKI,
   eSERVICE_ID_PROV_CSR_BLK_REQ,
   eSERVICE_ID_PROV_ROOT_CA_SIZE_INFO,
   eSERVICE_ID_PROV_ROOT_CA_BLK_RECP,
   eSERVICE_ID_PROV_DEV_CERT_SIZE_INFO,
   eSERVICE_ID_PROV_DEV_CERT_BLK_RECP,
   eSERVICE_ID_PROV_VIN_ALIAS_WRITE,
   eSERVICE_ID_PROV_DEV_ID_WRITE,
   eSERVICE_ID_PROV_DEV_ID_REPROVISION,


   /**************************************************************************
    * MAX Servie ID Configured
    **************************************************************************/
   eSERVICE_ID_MAX_CONFIGURED_ID,


   /**************************************************************************
    * Last Service ID
    **************************************************************************/
   eSERVICE_ID_LAST_SERVICE_ID = 0xFFFF,


} ServiceIDs_E;


/**
 * @enum          ServiceMgrIDs_E
 * @brief         Global Enum containing all the service manager IDs.
 */
typedef enum
{
   eSERVICE_MGR_ID_UART = 0,
   eSERVICE_MGR_ID_HTTPS,
   eSERVICE_MGR_ID_MQTT,
   eSERVICE_MGR_ID_NW,

   /**************************************************************************
    * Last Service ID
    **************************************************************************/
   eSERVICE_LAST_MGR_ID,

} ServiceMgrIDs_E;

/******************************************************************************/
/*                                                                            */
/*                                 STRUCTURES                                 */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/*                                   UNIONS                                   */
/*                                                                            */
/******************************************************************************/
/**
 * @union         <Union name>
 * @brief         <Union details>.
 */

/******************************************************************************/
/*                                                                            */
/*                              EXTERN VARIABLES                              */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/*                              EXTERN FUNCTIONS                              */
/*                                                                            */
/******************************************************************************/

#endif //!_SERVICE_IDS_H

/**
 * Copyright(c) Bajaj Auto Technology Limited (BATL) as an unpublished work.
 * THIS SOFTWARE AND/OR MATERIAL IS THE PROPERTY OF BATL.
 * ALL USE, DISCLOSURE, AND/OR REPRODUCTION NOT SPECIFICALLY AUTHORIZED BY
 * BATL IS PROHIBITED.
 *
 * @author:Yash Giramkar [YSG], Aditya Bhosale [ASB]
 */
