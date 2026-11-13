source /usr/share/orbcode/gdbtrace.init
set mem inaccessible-by-default off
set disassemble-next-line on

file fw.elf

target ext |openocd -f .openocd

enableSTM32SWO
prepareSWO 288000000

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
