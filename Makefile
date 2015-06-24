LIB=lib.a
D=$(HOME)/lib
E=$D/externals
CFLAGS=-Wall -Werror -g -I$E/include -I$D

TESTS = \
	test/audio \
	test/maestro \
	test/piface \
	test/talking-skull \
	test/track

AUDIO_OBJS = audio.o talking-skull.o track.o wav.o
GPIO_OBJS = lights.o gpio.o piface.o
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

all: $(LIB) $(TESTS)

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

test/talking-skull: test/talking-skull.o $(LIB)
	$(CC) test/talking-skull.o -o $@ $(LIBS)

test/track: test/track.o $(LIB)
	$(CC) test/track.o -o $@ $(LIBS)

# compile and generate dependency info
%.o: %.c
	@echo "Building: $*.c"
	@gcc -c $(CFLAGS) $*.c -o $*.o
	@gcc -MM $(CFLAGS) $*.c > $*.d

clean:
	-rm $(LIB) $(OBJS) $(OBJS:.o=.d) test/*.o test/*.d $(TESTS)
