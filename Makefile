CC = cc
LD = cc

EMSDK = /home/pudds/projects/third_party/emsdk
EMS_TOOLCHAIN = $(EMSDK)/emscripten/master/cmake/Modules/Platform/Emscripten.cmake
EMS_MODULE = $(EMSDK)/emscripten/master/cmake/Modules

ASSIMP_OPTS  = -DASSIMP_NO_EXPORT=ON -DASSIMP_BUILD_TESTS=OFF
ASSIMP_OPTS := $(ASSIMP_OPTS) -DASSIMP_BUILD_ASSIMP_TOOLS=OFF
ASSIMP_OPTS := $(ASSIMP_OPTS) -DCMAKE_C_FLAGS=-Wno-implicit-function-declaration
ASSIMP_OPTS := $(ASSIMP_OPTS) -DCMAKE_CXX_FLAGS=-Wno-implicit-function-declaration
ASSIMP_OPTS := $(ASSIMP_OPTS) -DCMAKE_BUILD_TYPE=Release

ASSIMP_OPTS_EMS  = $(ASSIMP_OPTS) -DBUILD_SHARED_LIBS=OFF
ASSIMP_OPTS_EMS := $(ASSIMP_OPTS_EMS) -DCMAKE_TOOLCHAIN_FILE="$(EMS_TOOLCHAIN)"
ASSIMP_OPTS_EMS := $(ASSIMP_OPTS_EMS) -DCMAKE_MODULE_PATH="$(EMS_MODULE)"

DIR = build

SRCS = assimp.c

OBJS_REL = $(patsubst %.c, $(DIR)/%.o, $(SRCS))
OBJS_DEB = $(patsubst %.c, $(DIR)/%.debug.o, $(SRCS))
OBJS_EMS = $(patsubst %.c, $(DIR)/%.emscripten.o, $(SRCS))

DEPS_REL = assimp.candle/build/assimp/code/libassimp.so
DEPS_EMS = assimp.candle/build/assimp_emscripten/code/libassimp.a \
assimp.candle/build/assimp_emscripten/contrib/zlib/libzlib.a \
assimp.candle/build/assimp_emscripten/contrib/zlib/libzlibstatic.a \
assimp.candle/build/assimp_emscripten/contrib/irrXML/libIrrXML.a

CFLAGS = -Iassimp/include -Wuninitialized $(PARENTCFLAGS)

CFLAGS_REL = $(CFLAGS) -Ibuild/assimp/include -O3

CFLAGS_DEB = $(CFLAGS) -Ibuild/assimp/include -g3

CFLAGS_EMS = $(CFLAGS) -Ibuild/assimp_emscripten/include -s USE_SDL=2

##############################################################################

all: init $(DIR)/assimp/code/libassimp.so $(DIR)/libs

$(DIR)/libs: $(DIR)/export.a
	echo assimp.candle/$< $(DEPS_REL) > $@

$(DIR)/export.a: $(OBJS_REL)
	$(AR) rs $@ $(OBJS_REL)

$(DIR)/%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS_REL)

##############################################################################

$(DIR)/assimp_emscripten/code/libassimp.a:
	cmake -B $(DIR)/assimp_emscripten assimp $(ASSIMP_OPTS_EMS)
	cmake --build $(DIR)/assimp_emscripten

$(DIR)/assimp/code/libassimp.so:
	cmake -B $(DIR)/assimp assimp $(ASSIMP_OPTS)
	cmake --build $(DIR)/assimp

##############################################################################

debug: init $(DIR)/libs_debug 

$(DIR)/libs_debug: $(DIR)/export_debug.a
	echo $(DEPS_REL) assimp.candle/$< > $@

$(DIR)/export_debug.a: $(OBJS_DEB)
	$(AR) rs $@ $(OBJS_DEB)

$(DIR)/%.debug.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS_DEB)

##############################################################################

emscripten: init $(DIR)/assimp_emscripten/code/libassimp.a $(DIR)/libs_emscripten

$(DIR)/libs_emscripten: $(DIR)/export_emscripten.a
	echo $(DEPS_EMS) assimp.candle/$< > $@

$(DIR)/export_emscripten.a: $(OBJS_EMS)
	emar rs $@ $(OBJS_EMS)

$(DIR)/%.emscripten.o: %.c
	emcc -o $@ -c $< $(CFLAGS_EMS)

##############################################################################

init:
	mkdir -p $(DIR)

##############################################################################

clean:
	-rm -r $(DIR)

# vim:ft=make
#

.PHONY: $(DIR)/assimp_emscripten/code/libassimp.a
