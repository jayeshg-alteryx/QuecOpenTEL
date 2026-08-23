# 1. Define the component name
set(target BAL_TEL)

# 2. Register the static library for target
add_library(${target} STATIC)

# 3. Expose the structural include paths to your components and external SDK links
set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${out_app_lib_dir})
target_compile_definitions(${target} PRIVATE OSI_LOG_TAG=LOG_TAG_QUEC)
target_include_directories(${target} PRIVATE ../../net/mbedtls/include)
target_include_directories(${target} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../LinkSDK/components/subdev
)
target_include_directories(${target} PUBLIC

    ${CMAKE_CURRENT_SOURCE_DIR}/CONFIG
    ${CMAKE_CURRENT_SOURCE_DIR}/COMMON

   # Framework Layer
   ${CMAKE_CURRENT_SOURCE_DIR}/FRMWRK/SERVICE_MGR
   ${CMAKE_CURRENT_SOURCE_DIR}/FRMWRK/APP_MONITOR

   # Services (Passive)
   ${CMAKE_CURRENT_SOURCE_DIR}/SERVICES/PASSIVE/FS
   ${CMAKE_CURRENT_SOURCE_DIR}/SERVICES/PASSIVE/CRYPTO

   # Services (Active)
   ${CMAKE_CURRENT_SOURCE_DIR}/SERVICES/ACTIVE/UART
   ${CMAKE_CURRENT_SOURCE_DIR}/SERVICES/ACTIVE/NW
   ${CMAKE_CURRENT_SOURCE_DIR}/SERVICES/ACTIVE/MQTT
   ${CMAKE_CURRENT_SOURCE_DIR}/SERVICES/ACTIVE/HTTPS

   # Applications
   ${CMAKE_CURRENT_SOURCE_DIR}/APP/GENERAL
   ${CMAKE_CURRENT_SOURCE_DIR}/APP/PROV
   ${CMAKE_CURRENT_SOURCE_DIR}/APP/HRTBT
   ${CMAKE_CURRENT_SOURCE_DIR}/APP/OA
   ${CMAKE_CURRENT_SOURCE_DIR}/APP/FOTA


   # System (Init, Reboot and Shutdown)
    ${CMAKE_CURRENT_SOURCE_DIR}/SYSTEM

)


target_link_libraries(${target} PRIVATE ql_app_peripheral)

# 4 Include path for all the source files
target_sources(${target} PRIVATE

    # Common Layer
    "COMMON/Generics.c"

    # Framework Layer
    "FRMWRK/SERVICE_MGR/ServiceManager.c"
    "FRMWRK/APP_MONITOR/AppMonitor.c"

    # Service Layer(Passive)
    "SERVICES/PASSIVE/FS/FS_Manager.c"
    "SERVICES/PASSIVE/FS/FS_Port.c"
    "SERVICES/PASSIVE/CRYPTO/CryptoManager.c"

    # Service Layer(Active)
    "SERVICES/ACTIVE/UART/UARTConfig.c"
    "SERVICES/ACTIVE/UART/UARTManager.c"
    "SERVICES/ACTIVE/UART/UARTPort.c"
    "SERVICES/ACTIVE/UART/UARTRouter.c"
    "SERVICES/ACTIVE/HTTPS/HTTPSManager.c"
    "SERVICES/ACTIVE/MQTT/MQTTManager.c"
    "SERVICES/ACTIVE/NW/NWManager.c"

    #  Application Monitors
    "APP/GENERAL/GENERALManager.c"
    "APP/FOTA/FOTAManager.c"
    "APP/FOTA/FOTA_SM.c"
    "APP/FOTA/FOTADownloadSM.c"
    "APP/FOTA/FOTAJobParser.c"
    "APP/HRTBT/HRTBTManager.c"
    "APP/OA/OAManager.c"
    "APP/PROV/PROVManager.c"
    "APP/PROV/PROVData.c"

   #  System Start up and Shutdown
    "SYSTEM/SysInit.c"
    "SYSTEM/Reboot.c"
    "SYSTEM/Shutdown.c"

    # QuecOpen cJSON library for FOTA JSON parsing and status creation
    "../LinkSDK/components/subdev/cJSON.c"
)

# 5. Link with standard SDK compiler definitions/flags inherited from the top-level target
relative_glob(srcs include/*.h src/*.c inc/*.h)
beautify_c_code(${target} ${srcs})
