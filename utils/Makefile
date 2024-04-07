LIB=../lib.a
D=$(HOME)/lib
E=$D/externals
CFLAGS=-Wall -Werror -g -I$E/include -I$D -I$E/PIGPIO

UTILS = \
	adc \
	digital-counter \
	life \
	maestro \
	mcp23017 \
	neopixel-pi \
	nes-dump \
	ssd1306 \
	st7735s \
	talking-skull-dump \
	talking-skull-prepare \
	wb
#	servo \

all: $(UTILS)

LIBS = $(LIB) -lusb -lrt -lpthread -lpng16 ../../tinyalsa/build/libtinyalsa.a -lgpiod

adc: adc.o $(LIB)
	$(CXX) adc.o -o $@ $(LIBS)

digital-counter: digital-counter.o $(LIB)
	$(CXX) digital-counter.o -o $@ $(LIBS)

life: life.o $(LIB)
	$(CXX) life.o -o $@ $(LIBS)

maestro: maestro.o $(LIB)
	$(CC) maestro.o -o $@ $(LIBS)

mcp23017: mcp23017.o $(LIB)
	$(CXX) mcp23017.o -o $@ $(LIBS)

neopixel-pi: neopixel-pi.o $(LIB)
	$(CXX) neopixel-pi.o -o $@ $(LIBS)

nes-dump: nes-dump.o $(LIB)
	$(CXX) nes-dump.o -o $@ $(LIBS)

ssd1306: ssd1306.o $(LIB)
	$(CXX) ssd1306.o -o $@ $(LIBS)

st7735s: st7735s.o $(LIB)
	$(CXX) st7735s.o -o $@ $(LIBS) -lpigpio

servo: servo.o $(LIB)
	$(CXX) servo.o -o $@ $(LIBS)

talking-skull-dump: talking-skull-dump.o $(LIB)
	$(CC) talking-skull-dump.o -o $@ $(LIBS)

talking-skull-prepare: talking-skull-prepare.o $(LIB)
	$(CC) talking-skull-prepare.o -o $@ $(LIBS)

wb: wb.o $(LIB)
	$(CXX) wb.o -o $@ $(LIBS)

# compile and generate dependency info
%.o: %.c
	@echo "Building: $*.c"
	@gcc -c $(CFLAGS) $*.c -o $*.o
	@gcc -MM $(CFLAGS) $*.c > $*.d

# compile and generate dependency info
%.o: %.cpp
	@echo "Building: $*.cpp"
	@g++ -c $(CFLAGS) $*.cpp -o $*.o
	@g++ -MM $(CFLAGS) $*.cpp > $*.d

clean:
	-rm *.o *.d $(UTILS)

-include *.d 
