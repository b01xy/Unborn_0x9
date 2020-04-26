#!/bin/bash

IP=192.168.128.3
EXE=server
DATA=srcbin/data/image_hand.txt

sshpass -p root scp ./$EXE root@$IP:/root/
sshpass -p root scp ./$DATA root@$IP:/root/
sshpass -p root ssh root@$IP
