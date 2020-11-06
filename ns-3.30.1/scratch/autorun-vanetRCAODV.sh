#!/bin/sh
# This is a comment!
cd ../

for i in $(seq 12501 1 12520)
do
    #run VanetRCDSDV with different seed value (i)
    echo "\t------ Seed Number: "$i" -------\n"
    echo "\t------ Connections= 10 | Size=125 DSR -------"
    ./waf --run "VanetRCDSRRWPM --seed="+$i+" --connections=10 --size=125"
done
