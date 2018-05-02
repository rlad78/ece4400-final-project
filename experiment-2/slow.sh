#!/bin/bash
echo "which interface do you want to slow?"
read eth
found=$(ifconfig -s | grep $eth -c)
case $found in
	0) echo "interface not found"; exit;;
	1) echo "$eth found";;
	*) echo "interface not found"; exit;;
esac

qdiscs=$(tc -s qdisc ls dev $eth | grep -c -e 'netem' -e 'tbf')
if [ $qdiscs -lt 2 ]
then
	echo "initializing $eth..."
	sudo tc qdisc delete dev $eth root
	sudo tc qdisc add dev $eth root handle 1:0 netem delay 0ms
	sudo tc qdisc add dev $eth parent 1:1 handle 10: tbf rate 1mbps burst 32kbit limit 1512
fi

echo "qdisc found"
echo "set [loss|rate]:"
read action
case $action in
	loss) 	echo "what percent of packets should be lost?"
			read loss
			sudo tc qdisc change dev $eth root handle 1:0 netem loss $loss
	;;
	rate)	echo "rate?"
			read rate
			echo "burst?"
			read burst
			echo "limit?"
			read limit
			sudo tc qdisc change dev $eth parent 1:1 handle 10: tbf rate $rate burst $burst limit $limit
	;;
esac