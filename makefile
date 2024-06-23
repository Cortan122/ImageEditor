BUILD_DIR=build
SRC_DIRS=./ src/ external/

LDLIBS=-lraylib -lm -lglfw -lGL
WARNINGS=-Wall -Wextra -Wno-parentheses -Wno-unused-value -Wno-unused-variable -Wno-unused-parameter -Wno-unused-function
CFLAGS=-MMD -fdollars-in-identifiers -funsigned-char $(WARNINGS) -Werror=vla
CPPFLAGS=-Iexternal/ -I$(BUILD_DIR)/
EXENAME=main

TARGETS=install debug
ifeq "$(MAKECMDGOALS)" "install"
  OBJ_EXT=-O
  CFLAGS += -O3 -DNDEBUG
else # ifeq "$(MAKECMDGOALS)" "debug"
  OBJ_EXT=-g
  CFLAGS += -Og -g -fsanitize=address
  LDFLAGS += -fsanitize=address
endif

EXECUTABLE_SYMLINK=$(EXENAME)
EXECUTABLE=$(BUILD_DIR)/$(EXENAME)$(OBJ_EXT)
SRC_EXTS=c cpp
SRC_FILES=$(foreach dir, $(SRC_DIRS), $(foreach ext, $(SRC_EXTS), $(wildcard $(dir)*.$(ext))))
OBJS=$(foreach ext, $(SRC_EXTS), $(patsubst %.$(ext), $(BUILD_DIR)/%$(OBJ_EXT).o, $(filter %.$(ext), $(SRC_FILES))))

DEFAULT_TARGET=debug
.PHONY: clean $(TARGETS) default
default: $(DEFAULT_TARGET)
$(TARGETS): $(EXECUTABLE)
$(OBJS): makefile
MAKEFLAGS += -j4

OBJS:=$(filter-out $(BUILD_DIR)/./resources$(OBJ_EXT).o, $(OBJS))
$(OBJS): $(BUILD_DIR)/resources.h
RESOURCES = $(wildcard resources/*.*)
$(BUILD_DIR)/resources.c: $(RESOURCES)
	@mkdir -p $$(dirname $@)
	find $^ -exec xxd -i {} \; > $@

$(BUILD_DIR)/resources.h: $(BUILD_DIR)/resources.c
	@mkdir -p $$(dirname $@)
	./cproto.sh $< > $@

-include $(OBJS:.o=.d)

$(BUILD_DIR)/%$(OBJ_EXT).o: %.c
	@mkdir -p $$(dirname $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

$(EXECUTABLE): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@

debug: $(EXECUTABLE)
	ln -sf $(EXECUTABLE) $(EXECUTABLE_SYMLINK)

install: $(EXECUTABLE)
	install -D $(EXECUTABLE) /usr/local/bin/raylid

run: debug
	./$(EXENAME)

clean:
	rm -rf $(BUILD_DIR) $(EXENAME) .cache/ *.o *.exe stretchy_buffer.h resources.c resources.h demo.zip

# TODO: fix windows compilation
WINLDFLAGS=-lmsvcrt -lopengl32 -lgdi32 -lkernel32 -lshell32 -luser32 -lraylib -Wl,-subsystem=gui
$(EXENAME).$(TARGETS): $(wildcard *.c)
%.exe: %.c
	tcc.exe $(CFLAGS) $(WINLDFLAGS) -o $@ $^
