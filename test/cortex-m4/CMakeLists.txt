CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
SET (CMAKE_VERBOSE_MAKEFILE OFF)

PROJECT (benchmark-cortex-m4)
ENABLE_LANGUAGE (ASM-ATT)

INCLUDE_DIRECTORIES("src/")
INCLUDE_DIRECTORIES("../../src/")
INCLUDE_DIRECTORIES("../../deps/etl/include/")
include_directories ("../../deps/GSL/include")

# +-----------------+
# | My code         |
# +-----------------+
ADD_EXECUTABLE(${CMAKE_PROJECT_NAME}.elf
    "src/benchmark.cc"
)

# +--------------+
# | STM Cube     |
# +--------------+
# Workaround : rozdzielenie kodu C++ i C powoduje, że QTCreator dobrze parsuje nagłówki C++. Bez tego podziału QTCreator się gubi.
add_library ("stm" STATIC
        "${STARTUP_CODE}"
        "src/stm32l4xx_it.c"
        "src/stm32l4xx_hal_conf.h"
        "src/system_stm32l4xx.c"



)


TARGET_LINK_LIBRARIES (${CMAKE_PROJECT_NAME}.elf -Wl,--whole-archive stm -Wl,--no-whole-archive)

FIND_PROGRAM (OPENOCD openocd)
ADD_CUSTOM_TARGET("upload" DEPENDS ${CMAKE_PROJECT_NAME}.elf COMMAND ${OPENOCD} -f /home/iwasz/local/share/openocd/scripts/interface/stlink.cfg -f /home/iwasz/local/share/openocd/scripts/target/stm32l4x.cfg -c 'program ${CMAKE_PROJECT_NAME}.elf verify reset exit')
ADD_CUSTOM_TARGET("debug" DEPENDS ${CMAKE_PROJECT_NAME}.elf COMMAND ${OPENOCD} -f /home/iwasz/local/share/openocd/scripts/interface/stlink.cfg -f /home/iwasz/local/share/openocd/scripts/target/stm32l4x.cfg)

ADD_CUSTOM_TARGET(${CMAKE_PROJECT_NAME}.bin ALL DEPENDS ${CMAKE_PROJECT_NAME}.elf COMMAND ${CMAKE_OBJCOPY} -Obinary ${CMAKE_PROJECT_NAME}.elf ${CMAKE_PROJECT_NAME}.bin)

FIND_PROGRAM (DD dd)
# WARNING! dd parameters has to match linker script memory areas sizes which in turn has to match sector sizes in the µC flash.
ADD_CUSTOM_TARGET(main.bin ALL DEPENDS ${CMAKE_PROJECT_NAME}.bin COMMAND ${DD} if=${CMAKE_PROJECT_NAME}.bin of=main.bin bs=1 count=32k)
ADD_CUSTOM_TARGET(radio.bin ALL DEPENDS ${CMAKE_PROJECT_NAME}.bin COMMAND ${DD} if=${CMAKE_PROJECT_NAME}.bin of=radio.bin skip=32k bs=1 count=32k)
