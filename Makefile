MAKEFLAGS += --no-builtin-rules --warn-undefined-variables

CCFLAGS                  = -fpic -IlibMmkJsonWriter/include -Wall -Wextra -Wpedantic -Werror
# TODO:
#    Thoroughly audit https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html for more warnings to turn on.
#    Per-project include paths...?

ifeq ($(shell getconf LONG_BIT),32)
LOCAL_ARCH :=x86
else
LOCAL_ARCH :=x64
endif



# $(1)	Compiler family	(gcc, clang)
CXX_TOOLSET_gcc          = g++
CXX_TOOLSET_clang        = clang++
LXX_TOOLSET_gcc          = $(CXX_TOOLSET_gcc)
LXX_TOOLSET_clang        = $(CXX_TOOLSET_clang)
CCFLAGS_TOOLSET_gcc      = -Wno-long-long
CCFLAGS_TOOLSET_clang    = -Wno-c++11-long-long

# $(2)	Arch			(x86, x64)
CCFLAGS_ARCH_x86         = -m32
CCFLAGS_ARCH_x64         = -m64
LDFLAGS_ARCH_x86         = -m32
LDFLAGS_ARCH_x64         = -m64

# $(3)	Build			(debug, release)
CCFLAGS_BUILD_debug      = -O0 -D_DEBUG
CCFLAGS_BUILD_release    = -O3 -DNDEBUG

# $(4)	Language		(c++03, c++11, gnu++11)

define DEFINE_SINGLE_CONFIG

bin/$(1)-$(2)-$(3)-$(4)/libMmkJsonWriter.so : $$(patsubst %.cpp,obj/$(1)-$(2)-$(3)-$(4)/%.o,$$(shell find libMmkJsonWriter/ -type f -name '*.cpp'))
	@mkdir -p $$(dir $$@)
	$$(LXX_TOOLSET_$(1)) $$(LDFLAGS_ARCH_$(2)) -shared -L$$(dir $$@) $$(patsubst $$(dir $$@)lib%.so,-l%,$$(filter %.so,$$^)) $$^ -o $$@

bin/$(1)-$(2)-$(3)-$(4)/tests               : bin/$(1)-$(2)-$(3)-$(4)/libMmkJsonWriter.so $$(patsubst %.cpp,obj/$(1)-$(2)-$(3)-$(4)/%.o,$$(shell find tests/ -type f -name '*.cpp'))
	@mkdir -p $$(dir $$@)
	$$(LXX_TOOLSET_$(1)) $$(LDFLAGS_ARCH_$(2)) -L$$(dir $$@) $$^ $$(patsubst $$(dir $$@)lib%.so,-l%,$$(filter %.so,$$^)) -o $$@

obj/$(1)-$(2)-$(3)-$(4)/%.o           : %.cpp
	@mkdir -p $$(dir $$@)
	$$(CXX_TOOLSET_$(1)) -c $$^ $(CCFLAGS) $$(CCFLAGS_TOOLSET_$(1)) $$(CCFLAGS_ARCH_$(2)) $$(CCFLAGS_BUILD_$(3)) -std=$(4) -MMD -MF $$(patsubst %.o,%.d,$$@) -o $$@
-include obj$(1)-$(2)-$(3)-$(4)/%.d

ifeq ($(LOCAL_ARCH),$(2))

build-libs::  bin/$(1)-$(2)-$(3)-$(4)/libMmkJsonWriter.so
build-tests:: bin/$(1)-$(2)-$(3)-$(4)/tests
run-tests::   bin/$(1)-$(2)-$(3)-$(4)/tests;   LD_LIBRARY_PATH=$$(dir $$^) ./$$^

endif
endef



TOOLSETS := gcc clang
ARCHS    := x86 x64
BUILDS   := debug release
LANGS    := c++03 c++11 gnu++11


default : run-tests

$(foreach toolset,$(TOOLSETS),$(foreach arch,$(ARCHS),$(foreach build,$(BUILDS),$(foreach lang,$(LANGS), $(eval $(call DEFINE_SINGLE_CONFIG,$(toolset),$(arch),$(build),$(lang)))))))

clean :
	rm -f bin/**/*.so
	rm -f bin/**/mmdebugtest
	rm -f obj/**/*.o
	rm -f obj/**/*.d

.PHONY : build-libs build-tests build-all run-tests default

.SUFFIXES:
