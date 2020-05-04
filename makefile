COMP=cc
CFLAGS=-g -Wall -Wextra

all: lcd_mon

lcd_mon: lcd_mon.c
	$(COMP) $(CFLAGS) -o lcd_mon lcd_mon.c

clean:
	rm lcd_mon
