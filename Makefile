# Makefile for chronobox software. K.Olchanski TRIUMF August 2018

CFLAGS += -std=c++11 -Wall -Wuninitialized -g -Ialtera -Dsoc_cv_av 

EXES += main.exe
EXES += srunner_cb.exe

all:: $(EXES)

clean:
	rm -f $(EXES) *.o

%.o: %.cxx
	g++ -c -o $@ $(CFLAGS) $<

main.exe: main.o cb.o
	g++ -o $@ $(CFLAGS) $^

srunner_cb.exe: srunner_cb.o cb.o
	g++ -o $@ $(CFLAGS) $^


#	gcc -o main.exe main.c
#	g++ -o srunner_cb.exe -Wall -g -Ialtera -Dsoc_cv_av srunner_cb.cxx cb.cxx

#end
