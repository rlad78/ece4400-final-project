#!/bin/bash
echo "which interface do you want to restore?"
read eth

sudo tc qdisc delete dev $eth root

echo "$eth restored"