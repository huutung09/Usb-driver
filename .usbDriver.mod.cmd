cmd_/home/tung/Documents/usbDriverProject/usbDriver.mod := printf '%s\n'   usbDriver.o | awk '!x[$$0]++ { print("/home/tung/Documents/usbDriverProject/"$$0) }' > /home/tung/Documents/usbDriverProject/usbDriver.mod