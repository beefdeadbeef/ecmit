source /usr/share/orbcode/gdbtrace.init
set disassemble-next-line on
set mem inaccessible-by-default off
set print pretty

file fw.elf
target extended-remote /dev/ttygdb
monitor swdp_scan
attach 1

enableSTM32SWO
monitor traceswo 2250000
prepareSWO 244000000 2250000

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
