LIB=lib.a
D=$(HOME)/lib
E=$D/externals
CFLAGS=-Wall -Werror -g -I$E/include -I$D -I$E/PIGPIO

TESTS = \
	test/audio \
	test/maestro \
	test/one-off \
	test/piface \
	test/stepper \
	test/talking-skull \
	test/track \
	test/wb

UTILS = \
	utils/digital-counter \
	utils/maestro \
	utils/mcp23017 \
	utils/servo \
	utils/talking-skull-dump \
	utils/wb

AUDIO_OBJS = audio.o talker-auto-gain.o talking-skull.o track.o wav.o
GPIO_OBJS = digital-counter.o lights.o gpio.o mcp23017.o piface.o wb.o \
	    ween-board.o
MOTOR_OBJS = grove.o l298n.o stepper.o
NET_OBJS = net.o net-line-reader.o
SERVER_OBJS = server.o
SERVO_OBJS = pi-usb.o maestro.o
THREAD_OBJS = call-every.o producer-consumer.o
UTIL_OBJS = util.o file.o stop.o string-utils.o mem.o global-trace.o

OBJS = \
	$(AUDIO_OBJS) \
	$(GPIO_OBJS) \
	$(MOTOR_OBJS) \
	$(NET_OBJS) \
	$(SERVER_OBJS) \
	$(SERVO_OBJS) \
	$(THREAD_OBJS) \
	$(UTIL_OBJS)

EXTERNALS = $E/tinyalsa/pcm.o $E/tinyalsa/mixer.o \
	$E/mcp23s17.o $E/pifacedigital.o \
	$E/PIGPIO/pigpio.o $E/PIGPIO/command.o \

all: $(LIB) $(TESTS) $(UTILS)

$(LIB): $(OBJS) $(EXTERNALS)
	@echo "Linking: $(LIB)"
	@ar r $@ $(OBJS) $(EXTERNALS)

# pull in dependency info for *existing* .o files
-include $(OBJS:.o=.d)

LIBS = $(LIB) -lusb -lrt -lpthread

test/audio: test/audio.o $(LIB)
	$(CC) test/audio.o -o $@ $(LIBS)

test/maestro: test/maestro.o $(LIB)
	$(CC) test/maestro.o -o $@ $(LIBS)

test/one-off: test/one-off.o $(LIB)
	$(CXX) test/one-off.o -o $@ $(LIBS)

test/piface: test/piface.o $(LIB)
	$(CC) test/piface.o -o $@ $(LIBS)

test/stepper: test/stepper.o $(LIB)
	$(CC) test/stepper.o -o $@ $(LIBS)

test/talking-skull: test/talking-skull.o $(LIB)
	$(CC) test/talking-skull.o -o $@ $(LIBS)

test/track: test/track.o $(LIB)
	$(CC) test/track.o -o $@ $(LIBS)

test/wb: test/wb.o $(LIB)
	$(CC) test/wb.o -o $@ $(LIBS)

utils/digital-counter: utils/digital-counter.o $(LIB)
	$(CC) utils/digital-counter.o -o $@ $(LIBS)

utils/maestro: utils/maestro.o $(LIB)
	$(CC) utils/maestro.o -o $@ $(LIBS)

utils/mcp23017: utils/mcp23017.o $(LIB)
	$(CXX) utils/mcp23017.o -o $@ $(LIBS)

# compile and generate dependency info
utils/servo: utils/servo.o $(LIB)
	$(CC) utils/servo.o -o $@ $(LIBS)

utils/talking-skull-dump: utils/talking-skull-dump.o $(LIB)
	$(CC) utils/talking-skull-dump.o -o $@ $(LIBS)

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
	-rm $(LIB) $(OBJS) $(OBJS:.o=.d) test/*.o test/*.d $(TESTS) $(UTILS)
