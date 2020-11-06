#!/bin/sh
# This is a comment!
cd ../

for i in $(seq 1001 1 1100)
do
    #run VanetRCDSDV with different seed value (i)
    echo "\t------ Seed Number: "$i" -------\n"
    echo "\t------ Connections= 20 | Size=50 AODV -------"
    ./waf --run "VanetRCAODVRWPM --seed="+$i+" --connections=20 --size=50 --meters=1000"
done

for i in $(seq 1101 1 1200)
do
    #run VanetRCDSDV with different seed value (i)
    echo "\t------ Seed Number: "$i" -------\n"
    echo "\t------ Connections= 20 | Size=75 AODV -------"
    ./waf --run "VanetRCAODVRWPM --seed="+$i+" --connections=20 --size=75 --meters=1200"
done

for i in $(seq 1201 1 1300)
do
    #run VanetRCDSDV with different seed value (i)
    echo "\t------ Seed Number: "$i" -------\n"
    echo "\t------ Connections= 20 | Size=100 AODV -------"
    ./waf --run "VanetRCAODVRWPM --seed="+$i+" --connections=20 --size=100 --meters=1400"
done

for i in $(seq 1301 1 1400)
do
    #run VanetRCDSDV with different seed value (i)
    echo "\t------ Seed Number: "$i" -------\n"
    echo "\t------ Connections= 20 | Size=125 AODV -------"
    ./waf --run "VanetRCAODVRWPM --seed="+$i+" --connections=20 --size=125 --meters=1600"
done
