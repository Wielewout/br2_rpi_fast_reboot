program = localizer

CC = gcc
CXX = g++

csrc = $(wildcard src/*.c)
ccsrc = $(wildcard src/*.cpp)

OBJDIR = obj
obj = $(addprefix $(OBJDIR)/, $(csrc:.c=.o) $(ccsrc:.cpp=.o))
dep = $(obj:.o=.d)
deps = dep

VC_LIB = /opt/vc/lib
USERLAND_ROOT = /home/pi/Camera/userland

CFLAGS = -O3 -Wall -g

INCLUDE_PATHS += -I./inc
INCLUDE_PATHS += -I$(VC_LIB)
INCLUDE_PATHS += -I$(USERLAND_ROOT)
INCLUDE_PATHS += -I$(USERLAND_ROOT)/interface/vcos
INCLUDE_PATHS += -I$(USERLAND_ROOT)/interface/vcos/pthreads
INCLUDE_PATHS += -I$(USERLAND_ROOT)/interface/vmcs_host/linux
INCLUDE_PATHS += -I$(USERLAND_ROOT)/host_applications/linux/libs/sm
INCLUDE_PATHS += -I$(USERLAND_ROOT)/host_applications/linux/libs/bcm_host/include

LIB_PATHS = -L$(VC_LIB)

MMAL_LIBS = -lmmal_core -lmmal_util -lmmal_vc_client
GL_LIBS = -lvcos -lvcsm -lbcm_host -lbrcmGLESv2 -lbrcmEGL -lGLESv2 -lEGL
COMMON_LIBS = -lpthread -lm -ldl
LIBS = $(MMAL_LIBS) $(GL_LIBS) $(COMMON_LIBS)

.PHONY: all
all: obj obj/src $(program)

obj:
	@mkdir obj

obj/src:
	@mkdir obj/src

DEPDIR := .d
$(shell mkdir -p $(DEPDIR) >/dev/null)
$(shell mkdir -p $(DEPDIR)/src >/dev/null)
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td

COMPILE.c = $(CC) $(DEPFLAGS) $(CFLAGS) $(INCLUDE_PATHS) $(CPPFLAGS) $(TARGET_ARCH) -c
COMPILE.cc = $(CXX) $(DEPFLAGS) $(CXXFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c
POSTCOMPILE = @mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d && touch $@

$(OBJDIR)/%.o : %.c
$(OBJDIR)/%.o : %.c $(DEPDIR)/%.d
	@echo "compile $< ..."
	@$(COMPILE.c) $(OUTPUT_OPTION) $<
	$(POSTCOMPILE)

$(OBJDIR)/%.o : %.cpp
$(OBJDIR)/%.o : %.cpp $(DEPDIR)/%.d
	@echo "compile $< ..."
	@$(COMPILE.cc) $(OUTPUT_OPTION) $<
	$(POSTCOMPILE)

$(DEPDIR)/%.d: ;
.PRECIOUS: $(DEPDIR)/%.d

include $(wildcard $(patsubst %,$(DEPDIR)/%.d,$(basename $(csrc))))

$(program): $(obj)
	@echo "build $@ ..."
	@$(CXX) -o $@ $^ $(LIB_PATHS) $(LIBS)

.PHONY: clean
clean:
	@echo "clean all ..."
	@rm -rf $(dep) $(obj) obj/src obj $(program)
