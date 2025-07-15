CFLAGS = -I ../include -g
OBJS = -L ../obj -lsng

ifndef SIZE
SIZE = 500
endif


all: nxdr copy matrix

nxdr :  mtclnt1 mtwrkr1

mtclnt1 : mtclnt.c
	cc $(CFLAGS) $(INCS) -o mtclnt mtclnt.c $(OBJS)
	echo $(MAT_SIZE)

mtwrkr1 : mtwrk.c
	cc $(CFLAGS) $(INCS) -o mtwrk mtwrk.c $(OBJS)

matrix : matrix.c
	cc $(CFLAGS) $(INCS) -o matrix matrix.c $(OBJS)

copy : mtclnt mtwrk
	cp mtclnt ../bin
	cp mtwrk ../bin