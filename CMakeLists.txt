cmake_minimum_required(VERSION 3.5)
project(DMM_STM32)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

add_definitions(-DSTM32F40_41xxx
                -DUSE_STDPERIPH_DRIVER
                -DSTM32F4XX
                -DKEIL_IDE
                -DHSE_VALUE=8000000
                -DPLL_M=8
                -DPLL_N=336
                -DPLL_P=2
                -DPLL_Q=7 )

include_directories(
        Libraries/CMSIS/Include/
        Libraries/STM32F4xx_StdPeriph_Driver/inc/
        Libraries/CMSIS/STM32F4xx/Include/
        Utilities/STM32F407-Discovery/
        RTE/
)

set(SOURCE_FILES
        adc.c
        adc.h
        capacitance.c
        capacitance.h
        dac.c
        dac.h
        diode.c
        diode.h
        dmm.c
        dmm.h
        dmm_config.h
        lcd_buffer.c
        lcd_buffer.h
        lcd_driver.c
        lcd_driver.h
        packet.c
        packet.h
        queue.c
        queue.h
        serial.c
        serial.h
        switches.c
        switches.h
        utils.c
        utils.h)

add_executable(DMM_STM32 ${SOURCE_FILES})