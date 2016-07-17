#!/bin/bash

. ./Vars.sh

#IPs donde se hacen los deploy
declare -a arr=("192.168.3.46" "192.168.3.49" "192.168.3.48")

git clone https://github.com/sisoputnfrba/tp-2016-1c-Lajew
git clone https://$token@github.com/sisoputnfrba/so-commons-library
git clone https://$token@github.com/sisoputnfrba/scripts-ansisop
git clone https://$token@github.com/sisoputnfrba/ansisop-parser

for i in "${arr[@]}" 
do
	scp -r so-commons-library scripts-ansisop ansisop-parser tp-2016-1c-Lajew utnso@$i:$REMOTE_PATH
done


