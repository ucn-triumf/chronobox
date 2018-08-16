# Makefile for chronobox software. K.Olchanski TRIUMF August 2018

CFLAGS += -std=c++11 -Wall -Wuninitialized -g -Ialtera -Dsoc_cv_av 

EXES += srunner_cb.exe
EXES += reboot_cb.exe
EXES += test_cb.exe

all:: $(EXES)

clean:
	rm -f $(EXES) *.o

%.o: %.cxx
	g++ -c -o $@ $(CFLAGS) $<

reboot_cb.exe: reboot_cb.o cb.o
	g++ -o $@ $(CFLAGS) $^

test_cb.exe: test_cb.o cb.o
	g++ -o $@ $(CFLAGS) $^

srunner_cb.exe: srunner_cb.o cb.o
	g++ -o $@ $(CFLAGS) $^

#end
