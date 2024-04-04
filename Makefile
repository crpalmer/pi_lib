LIB=lib.a
D=$(HOME)/lib
E=$D/externals
CFLAGS=-Wall -Werror -g -I../tinyalsa/include -I$D -I$E/PIGPIO

AUDIO_OBJS = audio.o talker-auto-gain.o talking-skull.o track.o wav.o
DISPLAY_OBJS = st7735s.o canvas.o canvas_png.o
GPIO_OBJS = digital-counter.o lights.o gpio.o mcp23017.o pca9685.o wb.o \
	    ween-board.o
MOTOR_OBJS = grove.o l298n.o stepper.o
NET_OBJS = net.o net-line-reader.o
SERVER_OBJS = server.o
SERVO_OBJS = pi-usb.o maestro.o
THREAD_OBJS = call-every.o producer-consumer.o
UTIL_OBJS = util.o file.o stop.o string-utils.o mem.o global-trace.o pi.o

OBJS = \
	$(AUDIO_OBJS) \
	$(DISPLAY_OBJS) \
	$(GPIO_OBJS) \
	$(MOTOR_OBJS) \
	neopixel-pi.o \
	pico-slave.o \
	$(NET_OBJS) \
	$(SERVER_OBJS) \
	$(SERVO_OBJS) \
	$(THREAD_OBJS) \
	$(UTIL_OBJS) \
	nes.o \

EXTERNALS = $E/PIGPIO/pigpio.o $E/PIGPIO/command.o

all: $(LIB) $(UTILS)
	cd utils && make

$(LIB): $(OBJS) $(EXTERNALS)
	@echo "Linking: $(LIB)"
	@ar r $@ $(OBJS) $(EXTERNALS)

# pull in dependency info for *existing* .o files
-include $(OBJS:.o=.d)

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
