/**
 * @file          PROVData.c
 * @brief         Source file containing Provisioning Data associated variable
 *                handling.
 * @date          10/08/26
 * @author        Yash Sunil Giramkar [YSG]
 * @copyright     Bajaj Auto Technology Limited (BATL)
 */

/******************************************************************************/
/*                                                                            */
/*                                  INCLUDES                                  */
/*                                                                            */
/******************************************************************************/
#include "PROVData.h"

/******************************************************************************/
/*                                                                            */
/*                                  DEFINES                                   */
/*                                                                            */
/******************************************************************************/
/**
 * @def           <Define name>
 * @brief         <Define details>.
 */

/******************************************************************************/
/*                                                                            */
/*                                   ENUMS                                    */
/*                                                                            */
/******************************************************************************/
// Definition of all the enums
/**
 * @enum          <Enum name>
 * @brief         <Enum details>.
 */

// Declarations of all the enum variables
/**
 * @var           <Variable name>
 * @brief         <Variable details>.
 */

/******************************************************************************/
/*                                                                            */
/*                                 STRUCTURES                                 */
/*                                                                            */
/******************************************************************************/
// Definition of all the structures
/**
 * @struct        <Structure name>
 * @brief         <Structure details>.
 */

// Declarations of all the structure variables
/**
 * @var           <Variable name>
 * @brief         <Variable details>.
 */

/******************************************************************************/
/*                                                                            */
/*                                   UNIONS                                   */
/*                                                                            */
/******************************************************************************/
// Definition of all the unions
/**
 * @union         <Union name>
 * @brief         <Union details>.
 */

// Declarations of all the union variables
/**
 * @var           <Variable name>
 * @brief         <Variable details>.
 */

/******************************************************************************/
/*                                                                            */
/*                       PRIVATE FUNCTION DECLARATIONS                        */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/*                              EXTERN VARIABLES                              */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/*                              PUBLIC VARIABLES                              */
/*                                                                            */
/******************************************************************************/
/**
 * @var           gca_VIN
 * @brief         RAM Reserved for VIN.
 */
char gca_VIN[PROV_VIN_MAX_SIZE] = {0};

/**
 * @var           gca_publicKey
 * @brief         RAM Reserved for public key.
 */
char gca_publicKey[PROV_PUBLIC_KEY_MAX_SIZE] = {0};

/**
 * @var           gca_privateKey
 * @brief         RAM Reserved for private key.
 */
char gca_privateKey[PROV_PRIVATE_KEY_MAX_SIZE] = {0};

/**
 * @var           gca_CSR
 * @brief         RAM Reserved for CSR.
 */
char gca_CSR[PROV_CSR_MAX_SIZE] = {0};

/**
 * @var           gca_rootCA
 * @brief         RAM Reserved for Root CA
 */
char gca_rootCA[PROV_ROOT_CA_MAX_SIZE] = {0};

/**
 * @var           gca_deviceCert
 * @brief         RAM Reserved for device certificate.
 */
char gca_deviceCert[PROV_DEV_CERT_MAX_SIZE] = {0};


/**
 * @var           gca_devID
 * @brief         RAM Reserved for Device ID.
 */
char gca_devID[PROV_DEV_ID_MAX_SIZE] = {0};


/**
 * @var           gca_vinAlias
 * @brief         RAM Reserved for VIN Alias.
 */
char gca_vinAlias[PROV_VIN_ALIAS_MAX_SIZE] = {0};

/**
 * @var           gu16_sizeOfCSR
 * @brief         Size of CSR
 */
uint16_t gu16_sizeOfCSR = 0;

/**
 * @var           gu16_sizeOfCSR
 * @brief         Size of CSR
 */
uint8_t gu8_totalCSRBlocks = 0;


/**
 * @var           gu16_sizeOfRootCA
 * @brief         Variable to hold size of Root CA
 */
uint16_t gu16_sizeOfRootCA = 0;

/**
 * @var           gu8_totalRootCABlocks
 * @brief         Variable to hold number of blocks in Root CA
 */
uint8_t gu8_totalRootCABlocks = 0;

/**
 * @var           gu8_currRootCABlock
 * @brief         Variable to hold number of blocks in Root CA
 */
uint8_t gu8_currRootCABlock = 0;

/**
 * @var           gu16_sizeOfDevCert
 * @brief         Variable to hold size of device certificate
 */
uint16_t gu16_sizeOfDevCert = 0;

/**
 * @var           gu8_totalDevCertBlocks
 * @brief         Variable to hold number of device certificate blocks
 */
uint8_t gu8_totalDevCertBlocks = 0;

/**
 * @var           gu8_currDevCertBlocks
 * @brief         Variable to hold number of device certificate blocks
 */
uint8_t gu8_currDevCertBlock = 0;

/**
 * @var           ge_provStatus
 * @brief         Variable to hold provisioning status
 */
PROVStatus_E ge_provStatus = eDEV_UNPROVISIONED;

/******************************************************************************/
/*                                                                            */
/*                             PRIVATE VARIABLES                              */
/*                                                                            */
/******************************************************************************/
/**
 * @var           <Variable name>
 * @brief         <Variable details>.
 */

/******************************************************************************/
/*                                                                            */
/*                              EXTERN FUNCTIONS                              */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/*                        PRIVATE FUNCTION DEFINITIONS                        */
/*                                                                            */
/******************************************************************************/
/**
 * @private       <Function name>
 * @brief         <Function details>.
 * @param[in]     <Input parameter details>.
 * @param[out]    <Output parameter details>.
 * @param[inout]  <Input-Output parameter details>.
 * @return        <Return details>.
 */

/******************************************************************************/
/*                                                                            */
/*                        PUBLIC FUNCTION DEFINITIONS                         */
/*                                                                            */
/******************************************************************************/
/**
 * @public        <Function name>
 * @brief         <Function details>.
 * @param[in]     <Input parameter details>.
 * @param[out]    <Output parameter details>.
 * @param[inout]  <Input-Output parameter details>.
 * @return        <Return details>.
 */

/**
 * Copyright(c) Bajaj Auto Technology Limited (BATL) as an unpublished work.
 * THIS SOFTWARE AND/OR MATERIAL IS THE PROPERTY OF BATL.
 * ALL USE, DISCLOSURE, AND/OR REPRODUCTION NOT SPECIFICALLY AUTHORIZED BY
 * BATL IS PROHIBITED.
 *
 * @author: Yash Sunil Giramkar [YSG]
 */
