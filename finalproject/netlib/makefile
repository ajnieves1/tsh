INCL = -I../include
FLAGS = -c -O3 
EXTRA = 
cc = gcc

all :	netlib.o api.o wall_clock.o archive ranl move

wall_clock.o : wall_clock.c
	$(cc) $(EXTRA) $(INCL) $(FLAGS) wall_clock.c $(TAIL)
netlib.o : netlib.c $(TAIL)
	$(cc) $(EXTRA)  $(INCL) $(FLAGS)  netlib.c $(TAIL)

api.o : api.c $(TAIL)
	$(cc) $(EXTRA)  $(INCL) $(FLAGS)  api.c $(TAIL)
archive : 
	ar rcv libb.a *.o
ranl:
move :
	mv libb.a ../obj/libsng.a
	/bin/rm -f *.o
