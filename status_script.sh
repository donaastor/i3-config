#!/bin/sh
# shell script to prepend i3status with more stuff

if [[ ! -p "/tmp/i3statusP" ]]; then
	mkfifo "/tmp/i3statusP"
fi
i3status --config /home/korsic/.config/i3/i3status > /tmp/i3statusP &
cpref="[{\"name\":\"keyboard layout\",\"markup\":\"none\",\"full_text\":\""

getlo(){
	lo=$(xkblayout-state print %s/%v)
	lol=${#lo}-1
	if [ ${lo:$lol} = "/" ]; then
		lo=${lo:0:$lol}
	fi
#	  soviet boost:
	if [ "$lo" == "ru" ]; then
		lo="CCCP"
	fi
#	  boosted!
}

(while :
do
	read line
	if [ "$line" = "" ]; then
		break
	elif [ "$line" == "U" ]; then
		getlo
		echo $pref$cpref$lo$"\"},"$postf
	else
		poc=${line:0:2}
		if [ "$poc" == "[{" ]; then
			pref=""
			postf="${line:1}"
			getlo
			echo $pref$cpref$lo$"\"},"$postf
		elif [ "$poc" == ",[" ]; then
			pref=","
			postf="${line:2}"
			getlo
			echo $pref$cpref$lo$"\"},"$postf
		else
			echo $line
		fi
	fi
done) < /tmp/i3statusP
