FILE(GLOB DRIVERS_SOURCES
    "${ENMICRO_DIR}/kits/common/drivers/display*.c"
)

INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/../qpn_5.2.0/include")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/../qpn_5.2.0/ports/arm-cm/vanilla/gnu/")
INCLUDE_DIRECTORIES("${ENMICRO_DIR}/kits/common/drivers")
INCLUDE_DIRECTORIES("${ENMICRO_DIR}/reptile/glib")
ADD_EFM32_EXECUTABLE(
	display_test_qpn 
	main.c 
	display_test.c
	bsp.c
	${ENMICRO_DIR}/kits/common/drivers/udelay.c
	${ENMICRO_DIR}/kits/common/drivers/retargetio.c
	${ENMICRO_DIR}/kits/common/drivers/retargettextdisplay.c
	${ENMICRO_DIR}/kits/common/drivers/textdisplay.c
	${DRIVERS_SOURCES}
    ${CMAKE_SOURCE_DIR}/../qpn_5.2.0/source/qepn.c
    ${CMAKE_SOURCE_DIR}/../qpn_5.2.0/source/qfn.c

)
