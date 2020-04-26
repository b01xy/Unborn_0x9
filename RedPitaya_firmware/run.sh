#!/bin/bash

# This is a simple run and compile script. It copies the file
# argv[0] to a specific Red Pitaya with IP argv[1]. It uses putty-tools,
# installed with install.sh.

#Global variables definition
IP=192.168.128.3
EXE=server

usage(){
  echo    "Just run this run.sh file :)"
  exit 1
}


echo "EXECUTING RED PITAYA RUN SCRIPT..."
echo "COPYING RED PITAYA INCLUDES IF NECESSARY..."
if [ -f "./srcbin/librp.so" ];then
	echo "./srcbin/librp.so does exists. Not copying."
else
	echo "./srcbin/librp.so does not exists. Copying..."
	sshpass -p root scp -r root@$IP:/opt/redpitaya/lib/librp.so srcbin
fi
if [ -f "./srcbin/rp.h" ];then
	echo "./srcbin/rp.h does exists. Not copying."
else
	echo "./srcbin/rp.h does not exists. Copying..."
	sshpass -p root scp -r root@$IP:/opt/redpitaya/include/redpitaya/rp.h srcbin
fi

echo "COMPILING SOURCE FILE..."

if [ -e $EXE ]; then # if the file exist
	rm $EXE
fi

arm-linux-gnueabi-gcc -c -O2 -std=gnu99 -Wall -Werror srcbin/TCP_API.c -o srcbin/TCP_API.o
arm-linux-gnueabi-gcc -c -O2 -std=gnu99 -Wall -Werror srcbin/UDP_API.c -o srcbin/UDP_API.o
arm-linux-gnueabi-gcc -c -O2 -std=gnu99 -Wall -Werror srcbin/settings.c -o srcbin/settings.o
arm-linux-gnueabi-gcc -c -O2 -std=gnu99 -Wall -Werror srcbin/stepper.c -o srcbin/stepper.o
arm-linux-gnueabi-gcc -c -O2 -std=gnu99 -Wall -Werror srcbin/echopenRP2.c -o srcbin/echopenRP2.o
arm-linux-gnueabi-gcc -c -O2 -std=gnu99 -Wall -Werror srcbin/communication.c -o srcbin/communication.o

arm-linux-gnueabi-gcc -g -O2 -std=gnu99 -Wall -Werror srcbin/communication.o srcbin/echopenRP2.o srcbin/settings.o srcbin/stepper.o srcbin/TCP_API.o srcbin/UDP_API.o srcbin/$EXE.c -o $EXE -L./srcbin -lpthread -lrp #the dynamic library (.so) is located in srcbin so we need -L./srcbin 

if [ "$?" -ne "0" ]; then
	echo 'Failling to compile the code'
	exit 1
fi

sshpass -p root scp ./$EXE root@$IP:/root/

if [ "$?" -eq "0" ]; then
	sshpass -p root ssh root@192.168.128.3
else
	echo 'Error, you must connect to wifi network redpitaya,'
	echo 'pass: redpitaya'
	echo 'if successfull compilation, you can just make a scp to transfer the code'
fi
