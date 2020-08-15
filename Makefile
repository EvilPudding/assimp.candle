CC = cc
LD = cc

DEPS = -lassimp 

DEPS_EMS = assimp.candle/libassimp.so

DIR = build

SRCS = assimp.c

OBJS_REL = $(patsubst %.c, $(DIR)/%.o, $(SRCS))
OBJS_DEB = $(patsubst %.c, $(DIR)/%.debug.o, $(SRCS))
OBJS_EMS = $(patsubst %.c, $(DIR)/%.emscripten.o, $(SRCS))

CFLAGS = -Wuninitialized $(PARENTCFLAGS)

CFLAGS_REL = $(CFLAGS) -O3

CFLAGS_DEB = $(CFLAGS) -g3

CFLAGS_EMS = $(CFLAGS) -Iassimp/include/ -Iassimp/build/include/ -s USE_SDL=2

##############################################################################

all: $(DIR)/libs

$(DIR)/libs: $(DIR)/export.a
	echo $(DEPS) assimp.candle/$< > $@

$(DIR)/export.a: init $(OBJS_REL)
	$(AR) rs $@ $(OBJS_REL)

$(DIR)/%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS_REL)

##############################################################################

debug: $(DIR)/libs_debug 

$(DIR)/libs_debug: $(DIR)/export_debug.a
	echo $(DEPS) assimp.candle/$< > $@

$(DIR)/export_debug.a: init $(OBJS_DEB)
	$(AR) rs $@ $(OBJS_DEB)

$(DIR)/%.debug.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS_DEB)

##############################################################################

emscripten: $(DIR)/libs_emscripten

$(DIR)/libs_emscripten: $(DIR)/export_emscripten.a
	echo $(DEPS_EMS) assimp.candle/$< > $@

$(DIR)/export_emscripten.a: init $(OBJS_EMS)
	emar rs $@ $(OBJS_EMS)

$(DIR)/%.emscripten.o: %.c
	emcc -o $@ -c $< $(CFLAGS_EMS)

##############################################################################

init:
	git submodule update
	mkdir -p $(DIR)

##############################################################################

clean:
	-rm -r $(DIR)

# vim:ft=make
#

