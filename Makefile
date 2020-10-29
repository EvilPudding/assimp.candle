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
ASSIMP_OPTS := $(ASSIMP_OPTS) -DASSIMP_BUILD_ZLIB=ON
ASSIMP_OPTS := $(ASSIMP_OPTS) -DASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT=OFF -DASSIMP_BUILD_FBX_IMPORTER=ON

ASSIMP_OPTS_EMS  = $(ASSIMP_OPTS) -DBUILD_SHARED_LIBS=OFF
ASSIMP_OPTS_EMS := $(ASSIMP_OPTS_EMS) -DCMAKE_TOOLCHAIN_FILE="$(EMS_TOOLCHAIN)"
ASSIMP_OPTS_EMS := $(ASSIMP_OPTS_EMS) -DCMAKE_MODULE_PATH="$(EMS_MODULE)"

DIR = build

SRCS = assimp.c aiw.c

OBJS_REL = $(patsubst %.c, $(DIR)/%.o, $(SRCS))
OBJS_DEB = $(patsubst %.c, $(DIR)/%.debug.o, $(SRCS))
OBJS_EMS = $(patsubst %.c, $(DIR)/%.emscripten.o, $(SRCS))

DEPS_REL = assimp.candle/build/assimp_release/contrib/zlib/libzlibstatic.a

DEPS_EMS = assimp.candle/build/assimp_emscripten/lib/libassimp.a \
assimp.candle/build/assimp_emscripten/contrib/zlib/libzlibstatic.a

PLUGIN_SAUCES_REL = $(DIR)/libassimp.so

CFLAGS = -Iassimp/include -Wuninitialized $(PARENTCFLAGS)

CFLAGS_REL = $(CFLAGS) -Ibuild/assimp/include -O3

CFLAGS_DEB = $(CFLAGS) -Ibuild/assimp/include -g3

CFLAGS_EMS = $(CFLAGS) -Ibuild/assimp_emscripten/include -s USE_SDL=2

##############################################################################

all: init $(DIR)/assimp_release/bin/libassimp.so $(DIR)/libs
	cp $(DIR)/assimp_release/bin/libassimp.so $(DIR)/
	echo $(PLUGIN_SAUCES_REL) > $(DIR)/res

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

$(DIR)/assimp_release/bin/libassimp.so:
	cmake -B $(DIR)/assimp_release assimp $(ASSIMP_OPTS)
	cmake --build $(DIR)/assimp_release

##############################################################################

debug: init $(DIR)/assimp_release/bin/libassimp.so $(DIR)/libs_debug
	cp $(DIR)/assimp_release/bin/libassimp.so $(DIR)/
	echo $(PLUGIN_SAUCES_REL) > $(DIR)/res

$(DIR)/libs_debug: $(DIR)/export_debug.a
	echo assimp.candle/$< $(DEPS_REL) > $@

$(DIR)/export_debug.a: $(OBJS_DEB)
	$(AR) rs $@ $(OBJS_DEB)

$(DIR)/%.debug.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS_DEB)

##############################################################################

emscripten: init $(DIR)/assimp_emscripten/code/libassimp.a $(DIR)/libs_emscripten
	echo "" > $(DIR)/res

$(DIR)/libs_emscripten: $(DIR)/export_emscripten.a
	echo assimp.candle/$< $(DEPS_EMS) > $@

$(DIR)/export_emscripten.a: $(OBJS_EMS)
	emar rs $@ $(OBJS_EMS)

$(DIR)/%.emscripten.o: %.c
	emcc -o $@ -c $< $(CFLAGS_EMS)

##############################################################################

init:
	mkdir -p $(DIR)
	rm -f $(DIR)/res
	rm -f $(DIR)/libs
	rm -f $(DIR)/res
	rm -f $(DIR)/libs_debug
	rm -f $(DIR)/libs_emscripten

##############################################################################

clean:
	-cd $(DIR) && ls | grep -v assimp_release | xargs rm -r

# vim:ft=make
#

