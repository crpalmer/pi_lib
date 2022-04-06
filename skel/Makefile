CFLAGS=-Wall -Werror -g -I/home/crpalmer/lib -I.
CPPFLAGS=$(CFLAGS)

# pull in dependency info for *existing* .o files
DEP_LIBS = ~/lib/lib.a
LIBS = $(DEP_LIBS) -lusb -lrt -lpthread

XXX: XXX.o $(DEP_LIBS)
	$(CXX) -o $@ XXX.o $(LIBS)

# compile and generate dependency info
%.o: %.c %.d
	@echo "Building: $*.c"
	@gcc -c $(CFLAGS) $*.c -o $*.o

%.o: %.cpp %.d
	@echo "Building: $*.cpp"
	@(CXX) -c $(CPPFLAGS) $*.cpp -o $*.o

%.d: %.c
	@echo "Depends:  $*.c"
	@gcc -MM $(CFLAGS) $*.c > $*.d

clean:
	-rm *.o *.d */*.o */*.d $(PROPS) $(DATA) 

include ${glob *.d}
