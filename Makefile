VERSION = 1.7.0

O      = o

CC      = /usr/local/bin/arm-apple-darwin8-gcc
CXX     = /usr/local/bin/arm-apple-darwin8-g++
AS      = /usr/local/bin/arm-apple-darwin8-g++
AR      = /usr/local/bin/arm-apple-darwin8-ar
STRIP   = /usr/local/bin/arm-apple-darwin8-strip
INC     = 
INCS    = ${INC}
LD      = /usr/local/bin/arm-apple-darwin8-g++
LDFLAGS = \
          -lobjc \
          -framework CoreFoundation \
          -framework Foundation \
          -framework UIKit \
          -framework LayerKit \
          -framework CoreGraphics \
          -framework GraphicsServices \
          -framework CoreSurface \
          -framework CoreAudio \
          -framework Celestial \
          -framework AudioToolbox \
          -lz

PROG   = gameboy4iphone

DEFAULT_CFLAGS =
MORE_CFLAGS = 
MORE_CFLAGS += -msoft-float -march=armv6 -maspen-version-min=1.0 -O3 -ftemplate-depth-36
MORE_CFLAGS += -fomit-frame-pointer
MORE_CFLAGS += -mstructure-size-boundary=32 -falign-functions=32 -falign-loops -falign-labels -falign-jumps -fno-builtin -fno-common
MORE_CFLAGS += -frename-registers -finline -finline-functions
MORE_CFLAGS += -fstrict-aliasing -fexpensive-optimizations -fweb -funroll-loops -fstrength-reduce
MORE_CFLAGS += -fsigned-char
MORE_CFLAGS += -DVERSION='"$(VERSION)"'

CFLAGS  = $(DEFAULT_CFLAGS) $(MORE_CFLAGS)
CPPFLAGS  = $(DEFAULT_CFLAGS) $(MORE_CFLAGS)
ASFLAGS  = -c $(DEFAULT_CFLAGS) $(MORE_CFLAGS)

#all: $(PROG)

OBJS =	\
	cpu.o \
	debug.o \
	emu.o \
	events.o \
	exports.o \
	fastmem.o \
	hw.o \
	keytable.o \
	lcd.o \
	lcdc.o \
	lj_gb.o \
	loader.o \
	mem.o \
	palette.o \
	path.o \
	rccmds.o \
	rcfile.o \
	rckeys.o \
	rcvars.o \
	refresh.o \
	rtc.o \
	save.o \
	sound.o \
	split.o \
	sys/iphone/main.o \
	sys/iphone/app.o \
	sys/iphone/ControllerView.o \
	sys/iphone/MainView.o \
	sys/iphone/FileTable.o \
	sys/iphone/FileBrowser.o \
	sys/iphone/EmulationView.o \
	sys/iphone/ScreenView.o \
	sys/iphone/app_iPhone.o \
	sys/iphone/JoyPad.o 


%.o: %.cpp
	${CXX} -fpeel-loops ${CFLAGS} -c -o $@ $<

%.o: %.m
	${CXX} -fpeel-loops ${CFLAGS} -c -o $@ $<

%.o: %.s
	${CXX} -fpeel-loops ${CFLAGS} -c -o $@ $<

%.o: %.c
	${CC} -fpeel-loops ${CFLAGS} -c -o $@ $<

all:	${OBJS}
	${LD} -O3 ${CFLAGS} ${OBJS} -o ${PROG} ${LDFLAGS}  
#	${STRIP} ${PROG}

clean:
	$(RM) $(PROG) $(OBJS)

test:
	$(CXX) $(CFLAGS) -S -o src/main.cpp.S src/main.cpp
