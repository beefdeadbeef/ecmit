source /usr/share/orbcode/gdbtrace.init
set disassemble-next-line on
set mem inaccessible-by-default off

file fw.elf
target ext /dev/ttygdb
mon swd_scan
att 1

mon swo enable
mon rtt ram 0x20000000 0x20001000
mon rtt poll 512 16 16
mon rtt channel 0 1
mon rtt enable

enableSTM32SWO 4
prepareSWO 96000000

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
