# directory declaration
LIB_CMSISDSP_DIR = $(LIBRARIES_ROOT)/cmsisdsp

LIB_CMSISDSP_ASMSRCDIR = $(LIB_CMSISDSP_DIR)
LIB_CMSISDSP_CSRCDIR = $(LIB_CMSISDSP_DIR)
LIB_CMSISDSP_CXXSRCSDIR    = $(LIB_CMSISDSP_DIR)
LIB_CMSISDSP_INCDIR  = $(LIB_CMSISDSP_DIR) 

# find all the source files in the target directories
#LIB_CMSISDSP_CSRCS = $(call get_csrcs, $(LIB_CMSISDSP_CSRCDIR))
#LIB_CMSISDSP_CXXSRCS = $(call get_cxxsrcs, $(LIB_CMSISDSP_CXXSRCSDIR))
LIB_CMSISDSP_CXXSRCS = 
LIB_CMSISDSP_ASMSRCS = $(call get_asmsrcs, $(LIB_CMSISDSP_ASMSRCDIR))

LIB_CMSISDSP_CSRCS = $(LIB_CMSISDSP_DIR)/Source/BasicMathFunctions/BasicMathFunctions.c
LIB_CMSISDSP_CSRCS += $(LIB_CMSISDSP_DIR)/Source/CommonTables/CommonTables.c
LIB_CMSISDSP_CSRCS += $(LIB_CMSISDSP_DIR)/Source/InterpolationFunctions/InterpolationFunctions.c
LIB_CMSISDSP_CSRCS += $(LIB_CMSISDSP_DIR)/Source/BayesFunctions/BayesFunctions.c
LIB_CMSISDSP_CSRCS += $(LIB_CMSISDSP_DIR)/Source/MatrixFunctions/MatrixFunctions.c
LIB_CMSISDSP_CSRCS += $(LIB_CMSISDSP_DIR)/Source/ComplexMathFunctions/ComplexMathFunctions.c
LIB_CMSISDSP_CSRCS += $(LIB_CMSISDSP_DIR)/Source/QuaternionMathFunctions/QuaternionMathFunctions.c
LIB_CMSISDSP_CSRCS += $(LIB_CMSISDSP_DIR)/Source/ControllerFunctions/ControllerFunctions.c
LIB_CMSISDSP_CSRCS += $(LIB_CMSISDSP_DIR)/Source/SVMFunctions/SVMFunctions.c
LIB_CMSISDSP_CSRCS += $(LIB_CMSISDSP_DIR)/Source/DistanceFunctions/DistanceFunctions.c
LIB_CMSISDSP_CSRCS += $(LIB_CMSISDSP_DIR)/Source/StatisticsFunctions/StatisticsFunctions.c
LIB_CMSISDSP_CSRCS += $(LIB_CMSISDSP_DIR)/Source/FastMathFunctions/FastMathFunctions.c
LIB_CMSISDSP_CSRCS += $(LIB_CMSISDSP_DIR)/Source/SupportFunctions/SupportFunctions.c
LIB_CMSISDSP_CSRCS += $(LIB_CMSISDSP_DIR)/Source/FilteringFunctions/FilteringFunctions.c
LIB_CMSISDSP_CSRCS += $(LIB_CMSISDSP_DIR)/Source/TransformFunctions/TransformFunctions.c
LIB_CMSISDSP_CSRCS += $(LIB_CMSISDSP_DIR)/Source/WindowFunctions/WindowFunctions.c


LIB_CMSISDSP_INCDIR = $(LIB_CMSISDSP_DIR)/Include
LIB_CMSISDSP_INCDIR += $(LIB_CMSISDSP_DIR)/PrivateInclude


# get object files
LIB_CMSISDSP_COBJS = $(call get_relobjs, $(LIB_CMSISDSP_CSRCS))
LIB_CMSISDSP_CXXOBJS = $(call get_relobjs, $(LIB_CMSISDSP_CXXSRCS))
LIB_CMSISDSP_ASMOBJS = $(call get_relobjs, $(LIB_CMSISDSP_ASMSRCS))
LIB_CMSISDSP_OBJS = $(LIB_CMSISDSP_COBJS) $(LIB_CMSISDSP_ASMOBJS) $(LIB_CMSISDSP_CXXOBJS)

# get dependency files
LIB_CMSISDSP_DEPS = $(call get_deps, $(LIB_CMSISDSP_OBJS))

# extra macros to be defined
LIB_CMSISDSP_DEFINES = -DLIB_CMSISDSP

# genearte library
ifeq ($(CMSISDSP_LIB_FORCE_PREBUILT), y)
override LIB_CMSISDSP_OBJS:=
endif
CMSISDSP_LIB_NAME = libcmsisdsp.a
LIB_CMSISDSP := $(subst /,$(PS), $(strip $(OUT_DIR)/$(CMSISDSP_LIB_NAME)))

#$(info $$LIB_CMSISDSP_OBJS is [${LIB_CMSISDSP_OBJS}])

# library generation rule
$(LIB_CMSISDSP): $(LIB_CMSISDSP_OBJS)
	$(TRACE_ARCHIVE)
ifeq "$(strip $(LIB_CMSISDSP_OBJS))" ""
	$(CP) $(PREBUILT_LIB)$(CMSISDSP_LIB_NAME) $(LIB_CMSISDSP)
else
	$(Q)$(AR) $(AR_OPT) $@ $(LIB_CMSISDSP_OBJS)
	$(CP) $(LIB_CMSISDSP) $(PREBUILT_LIB)$(CMSISDSP_LIB_NAME)
endif

# specific compile rules
# user can add rules to compile this middleware
# if not rules specified to this middleware, it will use default compiling rules

# Middleware Definitions
LIB_INCDIR += $(LIB_CMSISDSP_INCDIR)
LIB_CSRCDIR += $(LIB_CMSISDSP_CSRCDIR)
LIB_CXXSRCDIR += $(LIB_CMSISDSP_CXXSRCDIR)
LIB_ASMSRCDIR += $(LIB_CMSISDSP_ASMSRCDIR)

LIB_CSRCS += $(LIB_CMSISDSP_CSRCS)
LIB_CXXSRCS += $(LIB_CMSISDSP_CXXSRCS)
LIB_ASMSRCS += $(LIB_CMSISDSP_ASMSRCS)
LIB_ALLSRCS += $(LIB_CMSISDSP_CSRCS) $(LIB_CMSISDSP_ASMSRCS)

LIB_COBJS += $(LIB_CMSISDSP_COBJS)
LIB_CXXOBJS += $(LIB_CMSISDSP_CXXOBJS)
LIB_ASMOBJS += $(LIB_CMSISDSP_ASMOBJS)
LIB_ALLOBJS += $(LIB_CMSISDSP_OBJS)

LIB_DEFINES += $(LIB_CMSISDSP_DEFINES)
LIB_DEPS += $(LIB_CMSISDSP_DEPS)
LIB_LIBS += $(LIB_CMSISDSP)