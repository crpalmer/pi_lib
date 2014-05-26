LIB=lib.a
D=$(HOME)/lib
E=$D/externals
CFLAGS=-Wall -Werror -g -I$E/include -I$D

TESTS = test/maestro \
	test/wav

AUDIO_OBJS = wav.o
GPIO_OBJS = gpio.o
NET_OBJS = net.o net-line-reader.o
SERVO_OBJS = pi-usb.o maestro.o
THREAD_OBJS = call-every.o
UTIL_OBJS = util.o file.o string-utils.o mem.o global-trace.o

OBJS = $(AUDIO_OBJS) \
	$(GPIO_OBJS) \
	$(NET_OBJS) \
	$(SERVO_OBJS) \
	$(THREAD_OBJS) \
	$(UTIL_OBJS)

EXTERNALS = $E/tinyalsa/pcm.o $E/tinyalsa/mixer.o \
	$E/libpifacedigital.a

all: $(LIB) $(TESTS)

$(LIB): $(OBJS)
	@echo "Linking: $(LIB)"
	@ar r $@ $(OBJS) $(EXTERNALS)

# pull in dependency info for *existing* .o files
-include $(OBJS:.o=.d)

test/maestro: test/maestro.o $(LIB)
	$(CC) test/maestro.o -o $@ $(LIB) -lusb -lrt

test/wav: test/wav.o $(LIB)
	$(CC) test/wav.o -o $@ $(LIB) -lusb -lrt

# compile and generate dependency info
%.o: %.c
	@echo "Building: $*.c"
	@gcc -c $(CFLAGS) $*.c -o $*.o
	@gcc -MM $(CFLAGS) $*.c > $*.d

clean:
	-rm $(LIB) $(OBJS) $(OBJS:.o=.d)
