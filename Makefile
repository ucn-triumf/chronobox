# Makefile for chronobox software. K.Olchanski TRIUMF August 2018

CFLAGS += -std=c++11 -Wall -Wuninitialized -g -Ialtera -Dsoc_cv_av 

EXES += main.exe
EXES += srunner_cb.exe
EXES += reboot_cb.exe

all:: $(EXES)

clean:
	rm -f $(EXES) *.o

%.o: %.cxx
	g++ -c -o $@ $(CFLAGS) $<

main.exe: main.o cb.o
	g++ -o $@ $(CFLAGS) $^

reboot_cb.exe: reboot_cb.o cb.o
	g++ -o $@ $(CFLAGS) $^

srunner_cb.exe: srunner_cb.o cb.o
	g++ -o $@ $(CFLAGS) $^

#end
