PROJECT(zero-gecko)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
ENABLE_LANGUAGE(ASM)

SET(CMAKE_EXE_LINKER_FLAGS "--specs=nano.specs -Wl,--gc-sections -mthumb" CACHE INTERNAL "exe link flags")

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
	"../cmake/Modules/")

FIND_PACKAGE(EmLib REQUIRED)

IF(NOT ENMICRO_DIR)
	MESSAGE(FATAL_ERROR "No ENMICRO_DIR specified")
ENDIF()

FIND_PROGRAM(EACOMMANDER start-eACommander.sh HINTS ${ENMICRO_DIR}/eACommander)

EMLIB_CONFIGURE_LINKER_SCRIPT()
SET(CMAKE_EXE_LINKER_FLAGS
	"${CMAKE_EXE_LINKER_FLAGS} -Wl,-Map=${CMAKE_PROJECT_NAME}.map")

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${EMLIB_INCLUDE_DIR}
    ${EMLIB_FAMILY_INCLUDE_DIR}
	${ENMICRO_DIR}/CMSIS/Include
	${ENMICRO_DIR}/kits/common/bsp
	${ENMICRO_DIR}/kits/EFM32ZG_STK3200/config
)

FILE(GLOB BSP_SOURCES
    "${ENMICRO_DIR}/kits/common/bsp/bsp_bcc.c"
    "${ENMICRO_DIR}/kits/common/bsp/bsp_stk*.c"
)

FUNCTION(ADD_EFM32_EXECUTABLE EXECUTABLE_NAME)
	SET(elf_file ${EXECUTABLE_NAME}.elf)
	SET(lst_file ${EXECUTABLE_NAME}.lst)
	SET(bin_file ${EXECUTABLE_NAME}.bin)
	ADD_EXECUTABLE(${elf_file} ${ARGN} ${BSP_SOURCES} 
		${EMLIB_SYSTEM_SOURCE} ${EMLIB_STARTUP_SOURCE})

	TARGET_LINK_LIBRARIES(${elf_file} ${EMLIB_LIBRARIES} -Wl,--start-group -lgcc -lc_s -lnosys   -Wl,--end-group)

	ADD_CUSTOM_COMMAND(
		OUTPUT ${lst_file}
		POST_BUILD COMMAND
		${CMAKE_OBJDUMP} ARGS -S ${elf_file} > ${lst_file}
	)
	ADD_CUSTOM_COMMAND(
		OUTPUT ${bin_file}
		POST_BUILD COMMAND
		${CMAKE_OBJCOPY} ARGS -Obinary ${elf_file} ${bin_file}
	)
    ADD_CUSTOM_TARGET(
        upload_${EXECUTABLE_NAME}
        ${EACOMMANDER} --flash ${bin_file}
        DEPENDS ${bin_file} ${elf_file} ${lst_file}
        COMMENT "Uploading ${bin_file}"
    )
ENDFUNCTION(ADD_EFM32_EXECUTABLE)

ADD_SUBDIRECTORY(blink)
ADD_SUBDIRECTORY(blink_qpn)
ADD_SUBDIRECTORY(emlcd)
ADD_SUBDIRECTORY(display_test)
