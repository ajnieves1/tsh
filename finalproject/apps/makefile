SYS_LINK=-L/usr/lib
INCS = -I../include
OBJS = ../obj 
FLAGS = -g 
cc = gcc

all : tshPingPong

tshPingPong: tshPingPong.c
	$(cc) $(EXTRA) $(INCS) $(FLAGS) -o tshPingPong tshPingPong.c -L$(OBJS) -lsng