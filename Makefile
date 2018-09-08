# Makefile for chronobox software. K.Olchanski TRIUMF August 2018

INC_DIR   = $(MIDASSYS)/include
LIB_DIR   = $(MIDASSYS)/linux-arm/lib
SRC_DIR   = $(MIDASSYS)/src

CFLAGS += -std=c++11 -Wall -Wuninitialized -g -Ialtera -Dsoc_cv_av -I$(INC_DIR)
LIBS = -lm -lz -lutil -lnsl -lpthread
LIB = $(LIB_DIR)/libmidas.a -lrt

EXES += srunner_cb.exe
EXES += reboot_cb.exe
EXES += test_cb.exe
EXES += test_cb_ac.exe
EXES += fechrono.exe

all:: $(EXES)

clean:
	rm -f $(EXES) *.o

%.o: %.cxx
	g++ -c -o $@ $(CFLAGS) $<

reboot_cb.exe: reboot_cb.o cb.o
	g++ -o $@ $(CFLAGS) $^

test_cb.exe: test_cb.o cb.o
	g++ -o $@ $(CFLAGS) $^

test_cb_ac.exe: test_cb_ac.o cb.o
	g++ -o $@ $(CFLAGS) $^

srunner_cb.exe: srunner_cb.o cb.o
	g++ -o $@ $(CFLAGS) $^

fechrono.exe: $(LIB_DIR)/mfe.o cb.o fechrono.o 
	g++ -o $@ $(CFLAGS) $^ $(LIB) $(LIBS)
#end
