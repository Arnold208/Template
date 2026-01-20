# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set(SOURCES
    startup/system_stm32f4xx.c
    startup/startup_stm32f412rx.s
    startup/tx_initialize_low_level.S
    stm32cubef4/stm32f4xx_hal_msp.c
    board_init.c
    console.c
    screen.c
    main.c
    simple_sensor.c
    sensor_display.c
    mqtt_manager.c
    wwd_networking.c
    images.c
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME}
    azrtos::threadx
    azrtos::netxduo

    stm32cubef4
    mxchip_bsp
    wiced_sdk
    app_common
    # jsmn
)

target_include_directories(${PROJECT_NAME} 
    PUBLIC 
        .
        ../../../shared/src
        ../../../shared/lib/netxduo/addons/mqtt
        ../../../shared/lib/netxduo/common/inc
        ../../../shared/lib/netxduo/nx_secure
        ../../../shared/lib/netxduo/nx_secure/inc
        ../lib/wiced_sdk/inc
)

target_link_directories(${PROJECT_NAME} 
    PUBLIC 
        startup
)


if(CMAKE_C_COMPILER_ID STREQUAL "IAR")
    message(FATAL_ERROR, "IAR is not currently implemented for this device")
else()
    set_target_linker(${PROJECT_NAME} MXChip_AZ3166.ld)
endif()

post_build(${PROJECT_NAME})
