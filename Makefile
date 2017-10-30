SOURCE = cantest.c
CC = arm-linux-gnueabihf-gcc
TARGET = cantest

all:
	$(CC) $(SOURCE) -o $(TARGET)
	sudo cp $(TARGET)  /media/sf_VirtualBox/  -rf
	sync

clean:
	rm -rf $(TARGET)
	sync
