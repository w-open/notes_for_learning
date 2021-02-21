SOURCE = $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(SOURCE))
INCLUDE = -I../inc

all:$(OBJS)
	$(CC) -static  $(OUTPATH)/*.o -o $(BINPATH)/$(EXEC_NAME) -lpthread

%.o:%.c
	$(CC) $(INCLUDE) -Wall  -c $< -o $(OUTPATH)/$@ -lpthread

