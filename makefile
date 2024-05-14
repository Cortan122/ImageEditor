# this is a weird makefile that doesnt use .o files

LDFLAGS=-lraylib -lm -lglfw
WINLDFLAGS=-lmsvcrt -lopengl32 -lgdi32 -lkernel32 -lshell32 -luser32 -lraylib -Wl,-subsystem=gui
CFLAGS=-fdollars-in-identifiers -funsigned-char -Wall -Wextra -Wno-parentheses -Wno-unused-value -Werror=vla -g -Og
EXENAME=main

DRAWABLES = $(filter-out resources.c, $(wildcard *.c))
RESOURCES = $(wildcard resources/*.*)

all: $(EXENAME)
allexe: $(EXENAME).exe

resource_loader.c: stretchy_buffer.h resources.c resources.h
$(DRAWABLES): $(wildcard *.h)
$(EXENAME) $(EXENAME).exe: $(DRAWABLES)

resources.c: $(RESOURCES)
	find $^ -exec xxd -i {} \; > $@

resources.h: resources.c
	./cproto.sh $< > $@

resource_loader.c:
	touch $@

%.exe: %.c
	tcc.exe $(CFLAGS) $(WINLDFLAGS) -o $@ $^

stretchy_buffer.h:
	curl --silent -O https://raw.githubusercontent.com/nothings/stb/master/deprecated/stretchy_buffer.h

run: all
	./$(EXENAME)

runexe: allexe
	./$(EXENAME).exe

clean:
	rm -f *.o *.exe $(EXENAME) stretchy_buffer.h resources.c resources.h demo.zip

install: $(EXENAME)
	install -D $(EXENAME) /usr/local/bin/raylid

demo.zip: allexe
	# strip.exe $(EXENAME).exe
	zip -j demo.zip $(EXENAME).exe $$(which raylib.dll)
	cp demo.zip ~/Downloads/demo.zip.txt