LIB=lib.a
CFLAGS=-Wall -Werror -g -I/home/crpalmer/tinyalsa/include -I/home/crpalmer/lib

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

$(LIB): $(OBJS)
	ar r $@ $(OBJS)

# pull in dependency info for *existing* .o files
-include $(OBJS:.o=.d)

test/maestro: test/maestro.o $(LIB)
	$(CC) test/maestro.o -o $@ $(LIB) -lusb -lrt

test/wav: test/wav.o $(LIB)
	$(CC) test/wav.o -o $@ $(LIB) -lusb -lrt /home/crpalmer/tinyalsa/pcm.o

# compile and generate dependency info
%.o: %.c
	gcc -c $(CFLAGS) $*.c -o $*.o
	gcc -MM $(CFLAGS) $*.c > $*.d

clean:
	-rm $(LIB) $(OBJS) $(OBJS:.o=.d)
