LIB=lib.a
D=$(HOME)/lib
E=$D/externals
CFLAGS=-Wall -Werror -g -I$E/include -I$D -I$E/PIGPIO

TESTS = \
	test/audio \
	test/maestro \
	test/piface \
	test/stepper \
	test/talking-skull \
	test/track \
	test/wb

UTILS = \
	utils/servo \
	utils/wb

AUDIO_OBJS = audio.o talking-skull.o track.o wav.o
GPIO_OBJS = lights.o gpio.o piface.o stepper.o wb.o
NET_OBJS = net.o net-line-reader.o
SERVER_OBJS = server.o
SERVO_OBJS = pi-usb.o maestro.o
THREAD_OBJS = call-every.o producer-consumer.o
UTIL_OBJS = util.o file.o string-utils.o mem.o global-trace.o

OBJS = \
	$(AUDIO_OBJS) \
	$(GPIO_OBJS) \
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

utils/servo: utils/servo.o $(LIB)
	$(CC) utils/servo.o -o $@ $(LIBS)

utils/wb: utils/wb.o $(LIB)
	$(CC) utils/wb.o -o $@ $(LIBS)

# compile and generate dependency info
%.o: %.c
	@echo "Building: $*.c"
	@gcc -c $(CFLAGS) $*.c -o $*.o
	@gcc -MM $(CFLAGS) $*.c > $*.d

clean:
	-rm $(LIB) $(OBJS) $(OBJS:.o=.d) test/*.o test/*.d $(TESTS) $(UTILS)
