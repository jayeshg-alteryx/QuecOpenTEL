/**
 * @file          PROVData.h
 * @brief         Header file containing user defined data types for
 *                provisioning manager.
 * @date          10/08/26
 * @author        Yash Sunil Giramkar [YSG]
 * @copyright     Bajaj Auto Technology Limited (BATL)
 */

#ifndef _PROV_DATA_H
#define _PROV_DATA_H

/******************************************************************************/
/*                                                                            */
/*                                  INCLUDES                                  */
/*                                                                            */
/******************************************************************************/
#include "Generics.h"
#include "PROVTypes.h"
/******************************************************************************/
/*                                                                            */
/*                                  DEFINES                                   */
/*                                                                            */
/******************************************************************************/
/**
 * @def           PROV_VIN_MAX_SIZE
 * @brief         Maximum size of VIN as per ISO 3779 standard is 17 characters
 */
#define PROV_VIN_MAX_SIZE                    17

 /**
 * @def           PROV_PUBLIC_KEY_MAX_SIZE
 * @brief         Maximum size of public key as per ECC secp256r1(P-256) curve
 *                PEM encoded maximum can be ~170-190 bytes.
 */
#define PROV_PUBLIC_KEY_MAX_SIZE             512

 /**
 * @def           PROV_PRIVATE_KEY_MAX_SIZE
 * @brief         Maximum size of private key as per ECC secp256r1(P-256) curve
 *                PEM encoded maximum can be ~220-250 bytes.
 */
#define PROV_PRIVATE_KEY_MAX_SIZE            512

 /**
 * @def           PROV_CSR_MAX_SIZE
 * @brief         Maximum size of CSR as per ECC secp256r1(P-256) curve
 *                PEM encoded maximum can be ~400-700 bytes.
 */
#define PROV_CSR_MAX_SIZE                    1024

 /**
 * @def           PROV_ROOT_CA_MAX_SIZE
 * @brief         Maximum size of Root CA as per ECC secp256r1(P-256) curve
 *                PEM encoded maximum can be ~550-800 bytes.
 */
#define PROV_ROOT_CA_MAX_SIZE                1024

 /**
 * @def           PROV_DEV_CERT_MAX_SIZE
 * @brief         Maximum size of Device certificate as per ECC secp256r1(P-256)
 *                curve PEM encoded maximum can be ~650-1000 bytes.
 */
#define PROV_DEV_CERT_MAX_SIZE               1024

 /**
 * @def           PROV_DEV_ID_MAX_SIZE
 * @brief         Maximum size of Device ID decided along with cloud team
 */
#define PROV_DEV_ID_MAX_SIZE                 50

 /**
 * @def           PROV_VIN_ALIAS_MAX_SIZE
 * @brief         Maximum size of VIN Alias decided along with cloud team
 */
#define PROV_VIN_ALIAS_MAX_SIZE              50

/******************************************************************************/
/*                                                                            */
/*                                   ENUMS                                    */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/*                                 STRUCTURES                                 */
/*                                                                            */
/******************************************************************************/
/**
 * @struct        <Structure name>
 * @brief         <Structure details>.
 */

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
extern char gca_VIN[PROV_VIN_MAX_SIZE];
extern char gca_publicKey[PROV_PUBLIC_KEY_MAX_SIZE];
extern char gca_privateKey[PROV_PRIVATE_KEY_MAX_SIZE];
extern char gca_CSR[PROV_CSR_MAX_SIZE];
extern char gca_rootCA[PROV_ROOT_CA_MAX_SIZE];
extern char gca_deviceCert[PROV_DEV_CERT_MAX_SIZE];
extern char gca_devID[PROV_DEV_ID_MAX_SIZE];
extern char gca_vinAlias[PROV_VIN_ALIAS_MAX_SIZE];

extern uint16_t gu16_sizeOfCSR;
extern uint8_t gu8_totalCSRBlocks;

extern uint16_t gu16_sizeOfRootCA;
extern uint8_t gu8_totalRootCABlocks;
extern uint8_t gu8_currRootCABlock;

extern uint16_t gu16_sizeOfDevCert;
extern uint8_t gu8_totalDevCertBlocks;
extern uint8_t gu8_currDevCertBlock;

extern PROVStatus_E ge_provStatus;

/******************************************************************************/
/*                                                                            */
/*                              EXTERN FUNCTIONS                              */
/*                                                                            */
/******************************************************************************/

#endif //!_PROV_DATA_H

/**
 * Copyright(c) Bajaj Auto Technology Limited (BATL) as an unpublished work.
 * THIS SOFTWARE AND/OR MATERIAL IS THE PROPERTY OF BATL.
 * ALL USE, DISCLOSURE, AND/OR REPRODUCTION NOT SPECIFICALLY AUTHORIZED BY
 * BATL IS PROHIBITED.
 *
 * @author:Yash Sunil Giramkar [YSG]
 */
