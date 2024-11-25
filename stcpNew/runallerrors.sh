#!/bin/bash
file=smallfile
rm -f OutputFile
pkill sender
pkill receiver
./waitForPorts
./receiver probpointtwonocorrupt.script & sleep 1
./sender   $file
sleep 1
pkill receiver
diff $file OutputFile
