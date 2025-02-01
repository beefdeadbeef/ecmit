source /usr/share/orbcode/gdbtrace.init
set disassemble-next-line on
set mem inaccessible-by-default off
set print pretty
set mi-async on
set confirm off

file fw.elf
target ext /dev/ttygdb
mon swd_scan
mon swo enable
mon rtt
attach 1

enableSTM32SWO
prepareSWO 240000000 2250000

dwtSamplePC 1
dwtSyncTap 3
dwtPostTap 1
dwtPostInit 1
dwtPostReset 15
dwtCycEna 1

ITMId 1
ITMGTSFreq 3
ITMTSPrescale 3
ITMTXEna 1
ITMSYNCEna 1
ITMEna 1

ITMTER 0 0xFFFFFFFF
ITMTPR 0xFFFFFFFF
