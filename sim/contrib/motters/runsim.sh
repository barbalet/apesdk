#!/bin/bash
# runs the simulation for ten days in hourly increments
for ((n=0;n<240;n++)); do
    ./naweb -s realtime.txt -a apescript.txt
	echo "Step $n"
done
