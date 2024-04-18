# directory declaration
LIB_CMSISCV_DIR = $(LIBRARIES_ROOT)/cmsiscv

LIB_CMSISCV_ASMSRCDIR = $(LIB_CMSISCV_DIR)
LIB_CMSISCV_CSRCDIR = $(LIB_CMSISCV_DIR)
LIB_CMSISCV_CXXSRCSDIR    = $(LIB_CMSISCV_DIR)
LIB_CMSISCV_INCDIR  = $(LIB_CMSISCV_DIR) 

# find all the source files in the target directories
LIB_CMSISCV_CSRCS = $(call get_csrcs, $(LIB_CMSISCV_CSRCDIR)/Source)
#LIB_CMSISCV_CXXSRCS = $(call get_cxxsrcs, $(LIB_CMSISCV_CXXSRCSDIR))
LIB_CMSISCV_CXXSRCS = 
LIB_CMSISCV_ASMSRCS = $(call get_asmsrcs, $(LIB_CMSISCV_ASMSRCDIR))

LIB_CMSISCV_INCDIR = $(LIB_CMSISCV_DIR)/Include

# get object files
LIB_CMSISCV_COBJS = $(call get_relobjs, $(LIB_CMSISCV_CSRCS))
LIB_CMSISCV_CXXOBJS = $(call get_relobjs, $(LIB_CMSISCV_CXXSRCS))
LIB_CMSISCV_ASMOBJS = $(call get_relobjs, $(LIB_CMSISCV_ASMSRCS))
LIB_CMSISCV_OBJS = $(LIB_CMSISCV_COBJS) $(LIB_CMSISCV_ASMOBJS) $(LIB_CMSISCV_CXXOBJS)

# get dependency files
LIB_CMSISCV_DEPS = $(call get_deps, $(LIB_CMSISCV_OBJS))

# extra macros to be defined
LIB_CMSISCV_DEFINES = -DLIB_CMSISCV

# genearte library
ifeq ($(CMSISCV_LIB_FORCE_PREBUILT), y)
override LIB_CMSISCV_OBJS:=
endif
CMSISCV_LIB_NAME = libcmsiscv.a
LIB_CMSISCV := $(subst /,$(PS), $(strip $(OUT_DIR)/$(CMSISCV_LIB_NAME)))

#$(info $$LIB_CMSISCV_OBJS is [${LIB_CMSISCV_OBJS}])

# library generation rule
$(LIB_CMSISCV): $(LIB_CMSISCV_OBJS)
	$(TRACE_ARCHIVE)
ifeq "$(strip $(LIB_CMSISCV_OBJS))" ""
	$(CP) $(PREBUILT_LIB)$(CMSISCV_LIB_NAME) $(LIB_CMSISCV)
else
	$(Q)$(AR) $(AR_OPT) $@ $(LIB_CMSISCV_OBJS)
	$(CP) $(LIB_CMSISCV) $(PREBUILT_LIB)$(CMSISCV_LIB_NAME)
endif

# specific compile rules
# user can add rules to compile this middleware
# if not rules specified to this middleware, it will use default compiling rules

# Middleware Definitions
LIB_INCDIR += $(LIB_CMSISCV_INCDIR)
LIB_CSRCDIR += $(LIB_CMSISCV_CSRCDIR)
LIB_CXXSRCDIR += $(LIB_CMSISCV_CXXSRCDIR)
LIB_ASMSRCDIR += $(LIB_CMSISCV_ASMSRCDIR)

LIB_CSRCS += $(LIB_CMSISCV_CSRCS)
LIB_CXXSRCS += $(LIB_CMSISCV_CXXSRCS)
LIB_ASMSRCS += $(LIB_CMSISCV_ASMSRCS)
LIB_ALLSRCS += $(LIB_CMSISCV_CSRCS) $(LIB_CMSISCV_ASMSRCS)

LIB_COBJS += $(LIB_CMSISCV_COBJS)
LIB_CXXOBJS += $(LIB_CMSISCV_CXXOBJS)
LIB_ASMOBJS += $(LIB_CMSISCV_ASMOBJS)
LIB_ALLOBJS += $(LIB_CMSISCV_OBJS)

LIB_DEFINES += $(LIB_CMSISCV_DEFINES)
LIB_DEPS += $(LIB_CMSISCV_DEPS)
LIB_LIBS += $(LIB_CMSISCV)