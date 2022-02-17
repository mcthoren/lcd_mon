#### This is a project to display server data on a 16x2 LCD display.

To run it simply pass the com port with the LCD as an arugment, for example:
./lcd_mon /dev/ttyu1


To Do:
- [x] resurrect dusty old code from RCS
- [x] get old code compiling on a modern system w.o. warnings
- [x] test with hardware
- [x] makefile
- [x] fix 'lcd_mon: sysctl: No such file or directory'
- [ ] properly walk thru temps, and only display temps that exist.
- [x] iterate thru a list of function pointers instead of explicitly calling funcs.
- [x] write readme
