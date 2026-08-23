/**
 * @file          CryptoManager.c
 * @brief         Source file containing the cryptographic functions
 * @date          11/08/2026
 * @author        Aditya Bhosale [ASB]
 * @copyright     Bajaj Auto Technology Limited (BATL)
 */

/******************************************************************************/
/*                                                                            */
/*                                  INCLUDES                                  */
/*                                                                            */
/******************************************************************************/
#include "CryptoManager.h"

/******************************************************************************/
/*                                                                            */
/*                                  DEFINES                                   */
/*                                                                            */
/******************************************************************************/
/**
 * @def           BAL_CRYPTO_ORG_NAME
 * @brief         Indicates the organization name for the cryptographic operations.
 */
#define BAL_CRYPTO_ORG_NAME                                 "BAL"

/**
 * @def           BAL_CRYPTO_ORG_UNIT
 * @brief         Indicates the organization unit for the cryptographic operations.
 */
#define BAL_CRYPTO_ORG_UNIT                                 "IntlSys"

/**
 * @def           BAL_CRYPTO_COUNTRY
 * @brief         Indicates the country for the cryptographic operations.
 */
#define BAL_CRYPTO_COUNTRY                                  "IN"

/**
 * @def           BAL_CRYPTO_KEY_GEN
 * @brief         Indicates the key generation string for the cryptographic operations.
 */
#define BAL_CRYPTO_KEY_GEN                                  "bal_crypto_gen"

/**
 * @def           BAL_PKI_LOG_LEVEL
 * @brief         Log level used by the Crypto module debug traces.
 */
#define BAL_PKI_LOG_LEVEL                                   QL_LOG_LEVEL_DEBUG

/**
 * @def           BAL_PKI_LOG
 * @brief         Local logging wrapper for Crypto debug traces.
 */
#define BAL_PKI_LOG(msg, ...)       QL_LOG(BAL_PKI_LOG_LEVEL, "BAL_CRYPTO", msg, ##__VA_ARGS__)

/******************************************************************************/
/*                                                                            */
/*                                 STRUCTURES                                 */
/*                                                                            */
/******************************************************************************/
// Definition of all the structures
/**
 * @struct        CryptoMbedtlsContext_T
 * @brief         Structure containing the mbedTLS context for cryptographic operations
 */
typedef struct
{
   char                        car_SubjectDn[CRYPTO_SUBJECT_DN_SIZE];
   mbedtls_pk_context          st_Key;
   mbedtls_ctr_drbg_context    st_CtrDrbg;
   mbedtls_entropy_context     st_Entropy;
   mbedtls_x509write_csr       st_Csr;
}  CryptoMbedtlsContext_T;

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
static CryptoStatus_E se_Crypto_BuildSubjectDn(
                           char *cpt_SubjectDn,
                           uint16_t u16_SubjectDnSize,
                           const char *ccpt_Vin,
                           const char *ccpt_Imei);

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
 * @var           <Variable name>
 * @brief         <Variable details>.
 */

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
 * @public        ge_Crypto_Init
 * @brief         This function initializes the cryptographic module.
 * @param[in]     none
 * @param[out]    none
 * @return        CryptoStatus_E indicating the result of the operation.
 */
CryptoStatus_E ge_Crypto_Init(void){
   return CRYPTO_OK;
}

/**
 * @public        ge_Crypto_GenerateArtifacts
 * @brief         This function generates a cryptographic key pair and a certificate signing request (CSR).
 * @param[in]     stpt_CryptoIdentity : Pointer to the structure containing the key pair information.
 * @param[out]    stpt_CryptoIdentity : Updated with the generated key pair and CSR.
 * @return        CryptoStatus_E indicating the result of the operation.
 */
CryptoStatus_E ge_Crypto_GenerateArtifacts(CryptoIdentity_T *stpt_CryptoIdentity)
{
   int i_Ret = 0;
   CryptoStatus_E e_Return = CRYPTO_NOK;
   CryptoMbedtlsContext_T* stpt_CryptoCtx = NULL;

   stpt_CryptoCtx = (CryptoMbedtlsContext_T*)calloc(1,sizeof(*stpt_CryptoCtx));

   if(stpt_CryptoCtx == NULL)
   {
#ifdef DEBUG_ENABLED
         BAL_PKI_LOG("Calloc Failed for mbedTLS context");
#endif
   }
   else
   {
      // Validate parameters.
      if ((stpt_CryptoIdentity == NULL) ||
         (stpt_CryptoIdentity->u8pt_PublicKey == NULL) ||
         (stpt_CryptoIdentity->u8pt_PrivateKey == NULL) ||
         (stpt_CryptoIdentity->u8pt_CSRGen == NULL))
      {
   #ifdef DEBUG_ENABLED
            BAL_PKI_LOG("Invalid parameter");
   #endif
      }
      else
      {
         BAL_PKI_LOG("Valid parameter");

         // Initialize mbedTLS contexts.
         BAL_PKI_LOG("Calling mbedtls_ctr_drbg_init");
         mbedtls_ctr_drbg_init(&stpt_CryptoCtx->st_CtrDrbg);


         BAL_PKI_LOG("Calling mbedtls_pk_init");
         mbedtls_pk_init(&stpt_CryptoCtx->st_Key);


         BAL_PKI_LOG("Calling mbedtls_entropy_init");
         mbedtls_entropy_init(&stpt_CryptoCtx->st_Entropy);


         BAL_PKI_LOG("Calling mbedtls_x509write_csr_init");
         mbedtls_x509write_csr_init(&stpt_CryptoCtx->st_Csr);

         // Populate buffer with subject DN.
         BAL_PKI_LOG("Building CSR subject DN");
         i_Ret = se_Crypto_BuildSubjectDn(
                     stpt_CryptoCtx->car_SubjectDn,
                     sizeof(stpt_CryptoCtx->car_SubjectDn),
                     stpt_CryptoIdentity->car_Vin,
                     stpt_CryptoIdentity->car_Imei);

         // Seed random number generator.
         if (i_Ret == (int)CRYPTO_OK)
         {
            BAL_PKI_LOG("Building CSR subject DN Successful");
            i_Ret = mbedtls_ctr_drbg_seed(
                        &stpt_CryptoCtx->st_CtrDrbg,
                        mbedtls_entropy_func,
                        &stpt_CryptoCtx->st_Entropy,
                        (const unsigned char *)BAL_CRYPTO_KEY_GEN,
                        strlen(BAL_CRYPTO_KEY_GEN));
         }
         else
         {
   #ifdef DEBUG_ENABLED
            BAL_PKI_LOG("Failed to build subject DN return=%d", i_Ret);
            #endif
         }

         if (i_Ret == 0)
         {
            BAL_PKI_LOG("Setting up PK context for key generation");
            // Configure ECC key context.
            i_Ret = mbedtls_pk_setup(
                        &stpt_CryptoCtx->st_Key,
                        mbedtls_pk_info_from_type(MBEDTLS_PK_ECKEY));
         }
         else
         {
   #ifdef DEBUG_ENABLED
            BAL_PKI_LOG("Failed to setup ECC key context return=%d", i_Ret);
   #endif
         }

         if (i_Ret == 0)
         {
            BAL_PKI_LOG("Generating ec keypair curve=secp256r1");
            // Generate ECC P-256 key pair.
            i_Ret = mbedtls_ecp_gen_key(
                        MBEDTLS_ECP_DP_SECP256R1,
                        mbedtls_pk_ec(stpt_CryptoCtx->st_Key),
                        mbedtls_ctr_drbg_random,
                        &stpt_CryptoCtx->st_CtrDrbg);
         }
         else
         {
   #ifdef DEBUG_ENABLED
            BAL_PKI_LOG("Failed to generate ECC key pair return=%d", i_Ret);
   #endif
         }

         if (i_Ret == 0)
         {
            BAL_PKI_LOG("Generating ec keypair Public Key");
            // Export public key.
            i_Ret = mbedtls_pk_write_pubkey_pem(
                        &stpt_CryptoCtx->st_Key,
                        stpt_CryptoIdentity->u8pt_PublicKey,
                        (size_t)(stpt_CryptoIdentity->u16_PublicKeySize));
         }
         else
         {
   #ifdef DEBUG_ENABLED
            BAL_PKI_LOG("Failed to export public key return=%d", i_Ret);
   #endif
         }

         if (i_Ret == 0)
         {
            BAL_PKI_LOG("Generating ec keypair Private Key");
            // Export private key.
            i_Ret = mbedtls_pk_write_key_pem(
                        &stpt_CryptoCtx->st_Key,
                        stpt_CryptoIdentity->u8pt_PrivateKey,
                        (size_t)(stpt_CryptoIdentity->u16_PrivateKeySize));
         }
         else
         {
   #ifdef DEBUG_ENABLED
            BAL_PKI_LOG("Failed to export private key return=%d", i_Ret);
   #endif
         }

         if (i_Ret == 0)
         {
            // Set CSR key.
            mbedtls_x509write_csr_set_key(&stpt_CryptoCtx->st_Csr,
                                          &stpt_CryptoCtx->st_Key);

            // Set CSR message digest algorithm.
            mbedtls_x509write_csr_set_md_alg(&stpt_CryptoCtx->st_Csr,
                                             MBEDTLS_MD_SHA256);
            // Set CSR subject name.
            i_Ret = mbedtls_x509write_csr_set_subject_name(
                        &stpt_CryptoCtx->st_Csr,
                        stpt_CryptoCtx->car_SubjectDn);
         }
         else
         {
   #ifdef DEBUG_ENABLED
            BAL_PKI_LOG("Failed to set CSR key or message digest algorithm return=%d", i_Ret);
   #endif
         }

         if (i_Ret == 0)
         {
            // Export CSR in PEM format.
            i_Ret = mbedtls_x509write_csr_pem(
                        &stpt_CryptoCtx->st_Csr,
                        stpt_CryptoIdentity->u8pt_CSRGen,
                        (size_t)(stpt_CryptoIdentity->u16_CSRGenSize),
                        mbedtls_ctr_drbg_random,
                        &stpt_CryptoCtx->st_CtrDrbg);
         }
         else
         {
   #ifdef DEBUG_ENABLED
            BAL_PKI_LOG("Failed to export CSR return=%d", i_Ret);
   #endif
         }

         // Release all resources.
         mbedtls_pk_free(&stpt_CryptoCtx->st_Key);
         mbedtls_ctr_drbg_free(&stpt_CryptoCtx->st_CtrDrbg);
         mbedtls_entropy_free(&stpt_CryptoCtx->st_Entropy);
         mbedtls_x509write_csr_free(&stpt_CryptoCtx->st_Csr);
         free(stpt_CryptoCtx);

         if (i_Ret != 0)
         {
            e_Return = CRYPTO_KEY_GENERATION_ERROR;
   #ifdef DEBUG_ENABLED
            BAL_PKI_LOG("Failed to Generate Crypto Artifacts");
   #endif
         }
         else
         {
            // Set the actual sizes of the generated keys and CSR.
            stpt_CryptoIdentity->u16_PublicKeySize =
            (uint16_t)strlen((char *)stpt_CryptoIdentity->u8pt_PublicKey);

            stpt_CryptoIdentity->u16_PrivateKeySize =
            (uint16_t)strlen((char *)stpt_CryptoIdentity->u8pt_PrivateKey);

            stpt_CryptoIdentity->u16_CSRGenSize =
            (uint16_t)strlen((char *)stpt_CryptoIdentity->u8pt_CSRGen);

            e_Return = CRYPTO_OK;
   #ifdef DEBUG_ENABLED
            BAL_PKI_LOG("Successfully Generated Crypto Artifacts");
   #endif
         }
      }
   }

   return e_Return;
}

/**
 * @public        ge_Crypto_ValidateCksm
 * @brief         This function validates the checksum of the provided data against the expected checksum.
 * @param[in]     u8pt_Data : Pointer to the data for which the checksum needs to be validated.
 * @param[in]     u32_DataLength : Length of the data in bytes.
 * @param[in]     u8_DataExpectedCksum : The expected checksum.
 * @return        CryptoStatus_E indicating the result of the operation.
 */
TelSysResponse_E ge_Crypto_ValidateCksm(
                        uint8_t *u8pt_Data,
                        uint32_t u32_DataLength,
                        uint8_t u8_DataExpectedCksum)
{
   TelSysResponse_E e_Return = eTEL_NOK;
   uint8_t u8_CalculatedCksm;

   if ((u8pt_Data != NULL) && (u32_DataLength > 0U))
   {
      u8_CalculatedCksm = gu8_Crypto_CalcCksm(u8pt_Data,
                                             u32_DataLength);

      if (u8_CalculatedCksm == u8_DataExpectedCksum)
      {
         e_Return = eTEL_OK;
      }
   }

   return e_Return;
}

/**
 * @public        ge_Crypto_Validate16BitCksm
 * @brief         Validates the CRC-16 checksum of the provided data.
 * @param[in]     u8pt_Data : Pointer to the data.
 * @param[in]     u32_DataLength : Length of the data in bytes.
 * @param[in]     u16_DataExpectedCksm : Expected CRC-16 checksum.
 * @return        TelSysResponse_E indicating the result of the operation.
 */
TelSysResponse_E ge_Crypto_Validate16BitCksm(
                        uint8_t *u8pt_Data,
                        uint32_t u32_DataLength,
                        uint16_t u16_DataExpectedCksm)
{
   TelSysResponse_E e_Return = eTEL_NOK;
   uint16_t u16_CalculatedCksm;

   if ((u8pt_Data != NULL) && (u32_DataLength > 0U))
   {
      u16_CalculatedCksm = gu16_Crypto_CalcCksm(u8pt_Data,
                                                u32_DataLength);

      if (u16_CalculatedCksm == u16_DataExpectedCksm)
      {
         e_Return = eTEL_OK;
      }
   }

   return e_Return;
}

/**
 * @public        ge_Crypto_Validate32BitCksm
 * @brief         Validates the CRC-32 checksum of the provided data.
 * @param[in]     u8pt_Data : Pointer to the data.
 * @param[in]     u32_DataLength : Length of the data in bytes.
 * @param[in]     u32_DataExpectedCksm : Expected CRC-32 checksum.
 * @return        TelSysResponse_E indicating the result of the operation.
 */
TelSysResponse_E ge_Crypto_Validate32BitCksm(
                        uint8_t *u8pt_Data,
                        uint32_t u32_DataLength,
                        uint32_t u32_DataExpectedCksm)
{
   TelSysResponse_E e_Return = eTEL_NOK;
   uint32_t u32_CalculatedCksm;

   if ((u8pt_Data != NULL) && (u32_DataLength > 0U))
   {
      u32_CalculatedCksm = gu32_Crypto_CalcCksm(u8pt_Data,
                                                u32_DataLength);

      if (u32_CalculatedCksm == u32_DataExpectedCksm)
      {
         e_Return = eTEL_OK;
      }
   }

   return e_Return;
}

/**
 * @public        gu8_Crypto_CalcCksm
 * @brief         This function calculates the 8bit checksum of the provided data.
 * @param[in]     u8pt_Data : Pointer to the data for which the checksum needs to be calculated.
 * @param[in]     u32_DataLength : Length of the data in bytes.
 * @return        The calculated checksum.
 */
uint8_t gu8_Crypto_CalcCksm(uint8_t *u8pt_Data,
                            uint32_t u32_DataLength)
{
   uint32_t u32_Index = 0U;
   uint8_t  u8_Bit;
   uint8_t  u8_Crc = CRC8_INIT_VALUE;

   if ((u8pt_Data == NULL) || (u32_DataLength == 0U))
   {
      u8_Crc = 0U;
   }
   else{
      for (u32_Index = 0U; u32_Index < u32_DataLength; u32_Index++)
      {
         // XOR the current byte with the CRC value
         u8_Crc ^= u8pt_Data[u32_Index];

         // Process each bit of the byte
         for (u8_Bit = 0U; u8_Bit < 8U; u8_Bit++)
         {
            // If the MSB is set, shift left and XOR with polynomial 0x1D
            if ((u8_Crc & CRC8_MSB_MASK) != 0U)
            {
                  u8_Crc = (uint8_t)((u8_Crc << 1U) ^ CRC8_POLYNOMIAL);
            }
            else
            {
                  u8_Crc <<= 1U;
            }
         }
      }
   }

   return u8_Crc;
}

/**
 * @public        gu16_Crypto_CalcCksm
 * @brief         This function calculates the 16bit checksum of the provided data.
 * @param[in]     u8pt_Data : Pointer to the data for which the checksum needs to be calculated.
 * @param[in]     u32_DataLength : Length of the data in bytes.
 * @return        The calculated checksum.
 */
uint16_t gu16_Crypto_CalcCksm(uint8_t *u8pt_Data,
                              uint32_t u32_DataLength)
{
   uint32_t u32_Index;
   uint8_t u8_Bit;
   uint16_t u16_Crc = CRC16_INIT_VALUE;

   if ((u8pt_Data == NULL) || (u32_DataLength == 0U))
   {
      u16_Crc = 0U;
   }
   else{
      for (u32_Index = 0U; u32_Index < u32_DataLength; u32_Index++)
      {
         /* XOR current byte into upper byte of CRC */
         u16_Crc ^= ((uint16_t)u8pt_Data[u32_Index] << 8U);

         for (u8_Bit = 0U; u8_Bit < 8U; u8_Bit++)
         {
            if ((u16_Crc & CRC16_MSB_MASK) != 0U)
            {
                  u16_Crc = (uint16_t)((u16_Crc << 1U) ^ CRC16_POLYNOMIAL);
            }
            else
            {
                  u16_Crc <<= 1U;
            }
         }
      }
   }

   return u16_Crc;
}

/**
 * @public        gu32_Crypto_CalcCksm
 * @brief         Calculates the CRC-32 checksum of the provided data.
 * @param[in]     u8pt_Data : Pointer to the data.
 * @param[in]     u32_DataLength : Length of the data in bytes.
 * @return        Calculated CRC-32 checksum.
 */
uint32_t gu32_Crypto_CalcCksm(uint8_t *u8pt_Data,
                              uint32_t u32_DataLength)
{
   uint32_t u32_Index;
   uint8_t  u8_Bit;
   uint32_t u32_Crc = CRC32_INIT_VALUE;

   if ((u8pt_Data == NULL) || (u32_DataLength == 0U))
   {
      u32_Crc = 0U;
   }
   else{
      for (u32_Index = 0U; u32_Index < u32_DataLength; u32_Index++)
      {
         /* XOR current byte into the upper byte of the CRC register */
         u32_Crc ^= ((uint32_t)u8pt_Data[u32_Index] << 24U);

         /* Process each bit */
         for (u8_Bit = 0U; u8_Bit < 8U; u8_Bit++)
         {
            if ((u32_Crc & CRC32_MSB_MASK) != 0U)
            {
                  u32_Crc = (u32_Crc << 1U) ^ CRC32_POLYNOMIAL;
            }
            else
            {
                  u32_Crc <<= 1U;
            }
         }
      }
   }

   return u32_Crc;
}

/******************************************************************************/
/*                                                                            */
/*                        PRIVATE FUNCTION DEFINITIONS                        */
/*                                                                            */
/******************************************************************************/
/**
 * @private       se_Crypto_BuildSubjectDn
 * @brief         This function builds the subject distinguished name (DN) for the certificate signing request (CSR) based on the provided VIN and IMEI.
 * @param[in]     ccpt_Vin : Pointer to the vehicle identification number (VIN).
 * @param[in]     ccpt_Imei : Pointer to the international mobile equipment identity (IMEI).
 * @param[out]    cpt_SubjectDn : Pointer to the buffer where the constructed subject DN will be stored.
 * @param[in]     u16_SubjectDnSize : Size of the subject DN buffer.
 * @return        CryptoStatus_E indicating the result of the operation.
 */

static CryptoStatus_E se_Crypto_BuildSubjectDn(
                           char *cpt_SubjectDn,
                           uint16_t u16_SubjectDnSize,
                           const char *ccpt_Vin,
                           const char *ccpt_Imei)
{
   int i32_BufferSize;
   CryptoStatus_E e_Return;

   if ((cpt_SubjectDn == NULL) ||
      (ccpt_Vin == NULL) ||
      (ccpt_Imei == NULL))
   {
      e_Return = CRYPTO_INVALID_PARAMETER;
      return e_Return;
   }

   // Build the subject DN string in the format
   // "C=IN,O=BAL,OU=IntlSys,CN=<VIN>,serialNumber=<IMEI>"
   i32_BufferSize = snprintf( cpt_SubjectDn,
                              u16_SubjectDnSize,
                              "C=%s,O=%s,OU=%s,CN=%s,serialNumber=%s",
                              BAL_CRYPTO_COUNTRY,
                              BAL_CRYPTO_ORG_NAME,
                              BAL_CRYPTO_ORG_UNIT,
                              ccpt_Vin,
                              ccpt_Imei);

   if ((i32_BufferSize < 0) ||
      (i32_BufferSize >= u16_SubjectDnSize))
   {
      e_Return = CRYPTO_BUFFER_TOO_SMALL;
   }
   else
   {
      e_Return = CRYPTO_OK;
   }

   return e_Return;
}

/**
 * Copyright(c) Bajaj Auto Technology Limited (BATL) as an unpublished work.
 * THIS SOFTWARE AND/OR MATERIAL IS THE PROPERTY OF BATL.
 * ALL USE, DISCLOSURE, AND/OR REPRODUCTION NOT SPECIFICALLY AUTHORIZED BY
 * BATL IS PROHIBITED.
 *
 * @author: Aditya Bhosale [ASB]
 */
