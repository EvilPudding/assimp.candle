CC = emcc
LD = emcc
AR = emar

DEPS = assimp.candle/libassimp.so

DIR = build

SRCS = assimp.c

OBJS_REL = $(patsubst %.c, $(DIR)/%.o, $(SRCS))
OBJS_DEB = $(patsubst %.c, $(DIR)/%.debug.o, $(SRCS))

CFLAGS = -Wuninitialized $(PARENTCFLAGS) -I../candle -Iassimp/include/ -Iassimp/build/include/ -s USE_SDL=2

CFLAGS_REL = $(CFLAGS) -O3

CFLAGS_DEB = $(CFLAGS) -g3

##############################################################################

all: $(DIR)/export.a
	echo -n $(DEPS) > $(DIR)/deps

$(DIR)/export.a: init $(OBJS_REL)
	$(AR) rs build/export.a $(OBJS_REL)

$(DIR)/%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS_REL)

##############################################################################

debug: $(DIR)/export_debug.a
	echo $(DEPS) > $(DIR)/deps

$(DIR)/export_debug.a: init $(OBJS_DEB)
	$(AR) rs build/export_debug.a $(OBJS_DEB)

$(DIR)/%.debug.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS_DEB)

##############################################################################

init:
	git submodule update
	mkdir -p $(DIR)

##############################################################################

clean:
	-rm -r $(DIR)

# vim:ft=make
#

