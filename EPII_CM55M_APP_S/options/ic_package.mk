##
#  IC_PACKAGE_SEL
##
ifeq ($(IC_PACKAGE_SEL), LQFP128)
APPL_DEFINES += -DIC_PACKAGE_LQFP128
endif
ifeq ($(IC_PACKAGE_SEL), WLCSP65)
APPL_DEFINES += -DIC_PACKAGE_WLCSP65
endif
ifeq ($(IC_PACKAGE_SEL), QFN88)
APPL_DEFINES += -DIC_PACKAGE_QFN88
endif
ifeq ($(IC_PACKAGE_SEL), BGA64)
APPL_DEFINES += -DIC_PACKAGE_BGA64
endif