
all:
	gcc lightorgan.cpp -lasound -lwiringPi -g -Wall -o lightorgan

inst: all
	sudo cp lightorgan /usr/sbin
	sudo chmod 755 /usr/sbin/lightorgan
	sudo chown root /usr/sbin/lightorgan

