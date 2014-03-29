FILE(GLOB DRIVERS_SOURCES
    "${ENMICRO_DIR}/kits/common/drivers/display*.c"
)

INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}")
INCLUDE_DIRECTORIES("${ENMICRO_DIR}/kits/common/drivers")
INCLUDE_DIRECTORIES("${ENMICRO_DIR}/reptile/glib")
ADD_EFM32_EXECUTABLE(
	emlcd 
	emlcd.c
	${ENMICRO_DIR}/kits/common/drivers/udelay.c
	${ENMICRO_DIR}/kits/common/drivers/retargettextdisplay.c
	${ENMICRO_DIR}/kits/common/drivers/textdisplay.c
	${ENMICRO_DIR}/kits/common/drivers/retargetio.c
	${DRIVERS_SOURCES}
)
