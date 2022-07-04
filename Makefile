LIB=lib.a
D=$(HOME)/lib
E=$D/externals
CFLAGS=-Wall -Werror -g -I$E/include -I$D -I$E/PIGPIO

UTILS = \
	utils/digital-counter \
	utils/maestro \
	utils/mcp23017 \
	utils/nes-dump \
	utils/st7735s \
	utils/talking-skull-dump \
	utils/talking-skull-prepare \
	utils/wb
#	utils/servo \

AUDIO_OBJS = audio.o talker-auto-gain.o talking-skull.o track.o wav.o
GPIO_OBJS = digital-counter.o lights.o gpio.o mcp23017.o piface.o piface_lights.o pca9685.o wb.o \
	    ween-board.o
MOTOR_OBJS = grove.o l298n.o stepper.o
NET_OBJS = net.o net-line-reader.o
SERVER_OBJS = server.o
SERVO_OBJS = pi-usb.o maestro.o
THREAD_OBJS = call-every.o producer-consumer.o
UTIL_OBJS = util.o file.o stop.o string-utils.o mem.o global-trace.o pi.o

OBJS = \
	$(AUDIO_OBJS) \
	$(GPIO_OBJS) \
	$(MOTOR_OBJS) \
	$(NET_OBJS) \
	$(SERVER_OBJS) \
	$(SERVO_OBJS) \
	$(THREAD_OBJS) \
	$(UTIL_OBJS) \
	nes.o \
	st7735s.o \

EXTERNALS = $E/tinyalsa/pcm.o $E/tinyalsa/mixer.o \
	$E/mcp23s17.o $E/pifacedigital.o \
	$E/PIGPIO/pigpio.o $E/PIGPIO/command.o \

all: $(LIB) $(UTILS)

$(LIB): $(OBJS) $(EXTERNALS)
	@echo "Linking: $(LIB)"
	@ar r $@ $(OBJS) $(EXTERNALS)

# pull in dependency info for *existing* .o files
-include $(OBJS:.o=.d)

LIBS = $(LIB) -lusb -lrt -lpthread

utils/digital-counter: utils/digital-counter.o $(LIB)
	$(CXX) utils/digital-counter.o -o $@ $(LIBS)

utils/maestro: utils/maestro.o $(LIB)
	$(CC) utils/maestro.o -o $@ $(LIBS)

utils/mcp23017: utils/mcp23017.o $(LIB)
	$(CXX) utils/mcp23017.o -o $@ $(LIBS)

utils/nes-dump: utils/nes-dump.o $(LIB)
	$(CXX) utils/nes-dump.o -o $@ $(LIBS)

utils/st7735s: utils/st7735s.o $(LIB)
	$(CXX) utils/st7735s.o -o $@ $(LIBS)

utils/servo: utils/servo.o $(LIB)
	$(CXX) utils/servo.o -o $@ $(LIBS)

utils/talking-skull-dump: utils/talking-skull-dump.o $(LIB)
	$(CC) utils/talking-skull-dump.o -o $@ $(LIBS)

utils/talking-skull-prepare: utils/talking-skull-prepare.o $(LIB)
	$(CC) utils/talking-skull-prepare.o -o $@ $(LIBS)

utils/wb: utils/wb.o $(LIB)
	$(CXX) utils/wb.o -o $@ $(LIBS)

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
	-rm $(LIB) $(OBJS) $(OBJS:.o=.d) test/*.o test/*.d $(UTILS)
