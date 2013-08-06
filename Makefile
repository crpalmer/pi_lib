LIB=lib.a
CFLAGS=-Wall -Werror --std=c99

SERVO_OBJS = pi-usb.o maestro.o
UTIL_OBJS = util.o file.o string-utils.o mem.o global-trace.o
GPIO_OBJS = gpio.o

OBJS = $(UTIL_OBJS) $(SERVO_OBJS) $(GPIO_OBJS)

test: test.o $(LIB)
	$(CC) test.o -o $@ servo.a -lusb

$(LIB): $(OBJS)
	ar r $@ $(OBJS)

# pull in dependency info for *existing* .o files
-include $(OBJS:.o=.d)

# compile and generate dependency info
%.o: %.c
	gcc -c $(CFLAGS) $*.c -o $*.o
	gcc -MM $(CFLAGS) $*.c > $*.d

clean:
	-rm $(LIB) $(OBJS) $(OBJS:.o=.d)
