/**
 * @file          CryptoManager.h
 * @brief         Header file containing Configuration for Crypto Initialization.
 * @date          11/08/2026
 * @author        Aditya Bhosale [ASB]
 * @copyright     Bajaj Auto Technology Limited (BATL)
 */

#ifndef _CRYPTO_H
#define _CRYPTO_H

/******************************************************************************/
/*                                                                            */
/*                                  INCLUDES                                  */
/*                                                                            */
/******************************************************************************/
#include "Generics.h"

#include "mbedtls/pk.h"
#include "mbedtls/ecp.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "mbedtls/x509_csr.h"

/******************************************************************************/
/*                                                                            */
/*                                  DEFINES                                   */
/*                                                                            */
/******************************************************************************/
/**
 * @def           CRYPTO_SUBJECT_DN_SIZE
 * @brief         Indicates the maximum size of the subject DN buffer.
 */
#define CRYPTO_SUBJECT_DN_SIZE                              (256U)

/**
 * @def           CRYPTO_VIN_SIZE
 * @brief         Indicates the maximum size of the VIN buffer.
 */
#define CRYPTO_VIN_SIZE                                     (18U)

/**
 * @def           CRYPTO_IMEI_SIZE
 * @brief         Indicates the maximum size of the IMEI buffer.
 */
#define CRYPTO_IMEI_SIZE                                    (16U)

/**
 * @def           CRC8_INIT_VALUE
 * @brief         Indicates the CRC-8 Init value.
 */
#define CRC8_INIT_VALUE                                     ((uint8_t)0x00U)

/**
 * @def           CRC8_POLYNOMIAL
 * @brief         Indicates the CRC-8 polynomial.
 */
#define CRC8_POLYNOMIAL                                     ((uint8_t)0x1DU)

/**
 * @def           CRC8_MSB_MASK
 * @brief         Indicates the CRC-8 polynomial mask.
 */
#define CRC8_MSB_MASK                                       ((uint8_t)0x80U)

/**
 * @def           CRC16_INIT_VALUE
 * @brief         Indicates the CRC-16 Init value.
 */
#define CRC16_INIT_VALUE                                    ((uint16_t)0x0000U)

/**
 * @def           CRC16_POLYNOMIAL
 * @brief         Indicates the CRC-16 polynomial.
 */
#define CRC16_POLYNOMIAL                                    ((uint16_t)0x1021U)

/**
 * @def           CRC16_MSB_MASK
 * @brief         Indicates the CRC-16 polynomial mask.
 */
#define CRC16_MSB_MASK                                      ((uint16_t)0x8000U)

/**
 * @def           CRC32_INIT_VALUE
 * @brief         Indicates the CRC-32 Init value.
 */
#define CRC32_INIT_VALUE                                    ((uint16_t)0x00000000UL)

/**
 * @def           CRC32_POLYNOMIAL
 * @brief         Indicates the CRC-32 polynomial.
 */
#define CRC32_POLYNOMIAL                                    ((uint32_t)0x04C11DB7UL)

/**
 * @def           CRC32_MSB_MASK
 * @brief         Indicates the CRC-32 polynomial mask.
 */
#define CRC32_MSB_MASK                                      ((uint32_t)0x80000000UL)

/******************************************************************************/
/*                                                                            */
/*                                   ENUMS                                    */
/*                                                                            */
/******************************************************************************/
/**
 * @enum          CryptoStatus_E
 * @brief         Enum representing the status of cryptographic operations.
 */
typedef enum
{
   CRYPTO_OK                     = 0,
   CRYPTO_NOK                    = -1,
   CRYPTO_INVALID_PARAMETER      = -2,
   CRYPTO_BUFFER_TOO_SMALL       = -3,
   CRYPTO_KEY_GENERATION_ERROR   = -4,
} CryptoStatus_E;

/******************************************************************************/
/*                                                                            */
/*                                 STRUCTURES                                 */
/*                                                                            */
/******************************************************************************/
/**
 * @struct        CryptoIdentity_T
 * @brief         Structure containing the cryptographic key pair information.
 */
typedef struct
{
   char     car_Vin[CRYPTO_VIN_SIZE];  // Vehicle Identification Number (VIN) for the device.
   char     car_Imei[CRYPTO_IMEI_SIZE];// International Mobile Equipment Identity (IMEI) for the device.
   uint8_t  *u8pt_PublicKey;           // Pointer to the buffer for the public key.
   uint16_t u16_PublicKeySize;      // In-Out parameter: updated with actual size after generation.
   uint8_t  *u8pt_PrivateKey;          // Pointer to the buffer for the private key.
   uint16_t u16_PrivateKeySize;     // In-Out parameter: updated with actual size after generation.
   uint8_t  *u8pt_CSRGen;              // Pointer to the buffer for the generated Certificate Signing Request (CSR).
   uint16_t u16_CSRGenSize;         // In-Out parameter: updated with actual size after generation.
} CryptoIdentity_T;

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

extern CryptoStatus_E ge_Crypto_Init(void);

extern CryptoStatus_E ge_Crypto_GenerateArtifacts(
                        CryptoIdentity_T *stpt_CryptoIdentity);

extern TelSysResponse_E ge_Crypto_ValidateCksm(
                        uint8_t* u8pt_Data,
                        uint32_t u32_DataLength,
                        uint8_t u8_DataExpectedCksum);

extern TelSysResponse_E ge_Crypto_Validate16BitCksm(
                        uint8_t *u8pt_Data,
                        uint32_t u32_DataLength,
                        uint16_t u16_DataExpectedCksm);

extern TelSysResponse_E ge_Crypto_Validate32BitCksm(
                        uint8_t *u8pt_Data,
                        uint32_t u32_DataLength,
                        uint32_t u32_DataExpectedCksm);

extern uint8_t  gu8_Crypto_CalcCksm(uint8_t* u8pt_Data,
                                    uint32_t u32_DataLength);

extern uint16_t gu16_Crypto_CalcCksm(uint8_t* u8pt_Data,
                                     uint32_t u32_DataLength);

extern uint32_t gu32_Crypto_CalcCksm(uint8_t* u8pt_Data,
                                     uint32_t u32_DataLength);

#endif //!_CRYPTO_H

/**
 * Copyright(c) Bajaj Auto Technology Limited (BATL) as an unpublished work.
 * THIS SOFTWARE AND/OR MATERIAL IS THE PROPERTY OF BATL.
 * ALL USE, DISCLOSURE, AND/OR REPRODUCTION NOT SPECIFICALLY AUTHORIZED BY
 * BATL IS PROHIBITED.
 *
 * @author: Aditya Bhosale [ASB]
 */