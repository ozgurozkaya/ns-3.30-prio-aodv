#!/bin/sh
# This is a comment!
cd ../../

for i in $(seq 1 1 10)
do
    #run VanetRCDSR with different seed value (i)
    echo "\t------ Seed Number: "$i" -------\n"
    echo "\t------ Connections= 10 -------"
    ./waf --run "VanetRCDSR --seed="+$i+" --connections=10 --size=10"
done

for i in $(seq 101 1 110)
do
    #run VanetRCDSR with different seed value (i)
    echo "\t------ Seed Number: "$i" -------\n"
    echo "\t------ Connections= 10 -------"
    ./waf --run "VanetRCDSR --seed="+$i+" --connections=10 --size=10"
done

for i in $(seq 11 1 20)
do
    #run VanetRCDSR with different seed value (i)
    echo "\t------ Seed Number: "$i" -------\n"
    echo "\t------ Connections= 20 -------"
    ./waf --run "VanetRCDSR --seed="+$i+" --connections=20 --size=20"
done

for i in $(seq 201 1 210)
do
    #run VanetRCDSR with different seed value (i)
    echo "\t------ Seed Number: "$i" -------\n"
    echo "\t------ Connections= 20 -------"
    ./waf --run "VanetRCDSR --seed="+$i+" --connections=20 --size=20"
done

for i in $(seq 21 1 30)
do
    #run VanetRCDSR with different seed value (i)
    echo "\t------ Seed Number: "$i" -------\n"
    echo "\t------ Connections= 30 -------"
    ./waf --run "VanetRCDSR --seed="+$i+" --connections=30 --size=30"
done

for i in $(seq 301 1 310)
do
    #run VanetRCDSR with different seed value (i)
    echo "\t------ Seed Number: "$i" -------\n"
    echo "\t------ Connections= 30 -------"
    ./waf --run "VanetRCDSR --seed="+$i+" --connections=30 --size=30"
done

for i in $(seq 31 1 40)
do
    #run VanetRCDSR with different seed value (i)
    echo "\t------ Seed Number: "$i" -------\n"
    echo "\t------ Connections= 40 -------"
    ./waf --run "VanetRCDSR --seed="+$i+" --connections=40 --size=40"
done

for i in $(seq 401 1 410)
do
    #run VanetRCDSR with different seed value (i)
    echo "\t------ Seed Number: "$i" -------\n"
    echo "\t------ Connections= 40 -------"
    ./waf --run "VanetRCDSR --seed="+$i+" --connections=40 --size=40"
done

for i in $(seq 41 1 50)
do
    #run VanetRCDSR with different seed value (i)
    echo "\t------ Seed Number: "$i" -------\n"
    echo "\t------ Connections= 50 -------"
    ./waf --run "VanetRCDSR --seed="+$i+" --connections=50 --size=50"
done

for i in $(seq 501 1 510)
do
    #run VanetRCDSR with different seed value (i)
    echo "\t------ Seed Number: "$i" -------\n"
    echo "\t------ Connections= 50 -------"
    ./waf --run "VanetRCDSR --seed="+$i+" --connections=50 --size=50"
done

