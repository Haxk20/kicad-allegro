EESchema Schematic File Version 5
LIBS:video-cache
EELAYER 30 0
EELAYER END
$Descr A3 16535 11693
encoding utf-8
Sheet 1 8
Title "Video"
Date "Sun 22 Mar 2015"
Rev "2.0B"
Comp "Kicad EDA"
Comment1 "Main sheet"
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
Wire Wire Line
	10550 5800 11250 5800
Text Label 10600 5800 0    60   ~ 0
CSYNC-OUT
Wire Wire Line
	10550 6050 11250 6050
Text Label 10700 6050 0    60   ~ 0
CLAMP
Text Label 10700 5950 0    60   ~ 0
BLANK-
Wire Wire Line
	10550 5950 11250 5950
Wire Wire Line
	10650 5650 11250 5650
Text Label 10750 5650 0    60   ~ 0
ACQ_ON
Wire Wire Line
	3800 8300 4400 8300
Text Label 3900 8300 0    60   ~ 0
ACQ_ON
Text Label 14250 4150 0    60   ~ 0
C_OUT
Text Label 14250 4350 0    60   ~ 0
Y_OUT
Wire Wire Line
	14900 4350 14200 4350
Wire Wire Line
	14200 4150 14900 4150
Connection ~ 14800 4250
Connection ~ 14800 3950
Wire Wire Line
	14900 3950 14800 3950
Wire Wire Line
	14900 3750 14800 3750
Wire Wire Line
	14800 3750 14800 3950
Text Label 14800 2300 0    60   ~ 0
Y_OUT
Text Label 14800 1900 0    60   ~ 0
C_OUT
Wire Wire Line
	1450 8400 2000 8400
Wire Wire Line
	1450 8300 2000 8300
Wire Wire Line
	1450 8500 2000 8500
Text Label 1500 8400 0    60   ~ 0
IRQ-
Text Label 1500 8300 0    60   ~ 0
X_IRQ
Text Label 1500 8500 0    60   ~ 0
IRQ_SRL
Wire Wire Line
	3800 2150 4350 2150
Wire Wire Line
	3800 2050 4350 2050
Wire Wire Line
	3800 1950 4350 1950
Text Label 3850 2150 0    60   ~ 0
IRQ-
Text Label 3850 2050 0    60   ~ 0
X_IRQ
Text Label 3850 1950 0    60   ~ 0
IRQ_SRL
Text Label 3900 9300 0    60   ~ 0
PCA[0..2]
Wire Bus Line
	3800 9300 4600 9300
Text Label 6200 4450 0    60   ~ 0
PCA[0..1]
Wire Bus Line
	6100 4450 6900 4450
Wire Bus Line
	3800 5900 4600 5900
Text Label 3900 5900 0    60   ~ 0
TVI[0..1]
Wire Wire Line
	10450 5350 11250 5350
Wire Wire Line
	10450 5450 11250 5450
Text Label 10550 5350 0    60   ~ 0
ACCES_RAM-
Text Label 10550 5450 0    60   ~ 0
WRITE_RAM
Wire Wire Line
	3800 9100 4600 9100
Wire Wire Line
	3800 9000 4600 9000
Text Label 3900 9100 0    60   ~ 0
ACCES_RAM-
Text Label 3900 9000 0    60   ~ 0
WRITE_RAM
Text Label 10550 5200 0    60   ~ 0
DQ[0..31]
Wire Bus Line
	10450 5200 11250 5200
Wire Bus Line
	8800 6600 9750 6600
Text Label 8900 6600 0    60   ~ 0
TVRAM[0..31]
Wire Wire Line
	10750 4900 11250 4900
Wire Wire Line
	10750 5000 11250 5000
Text Label 10800 5000 0    60   ~ 0
CLKCAD
Text Label 10800 4900 0    60   ~ 0
CLKCDA
Wire Wire Line
	10650 4600 11250 4600
Wire Wire Line
	10650 4500 11250 4500
Wire Wire Line
	10650 4400 11250 4400
Wire Wire Line
	10650 4300 11250 4300
Text Label 10750 4600 0    60   ~ 0
X_PROG-
Text Label 10750 4500 0    60   ~ 0
X_DONE
Text Label 10750 4400 0    60   ~ 0
X_CLK
Text Label 10750 4300 0    60   ~ 0
X_DATA
Wire Wire Line
	3800 8800 4400 8800
Wire Wire Line
	3800 8700 4400 8700
Wire Wire Line
	3800 8600 4400 8600
Wire Wire Line
	3800 8500 4400 8500
Text Label 3900 8800 0    60   ~ 0
X_PROG-
Text Label 3900 8700 0    60   ~ 0
X_DONE
Text Label 3900 8600 0    60   ~ 0
X_CLK
Text Label 3900 8500 0    60   ~ 0
X_DATA
Wire Bus Line
	12900 4600 13850 4600
Wire Bus Line
	12900 4500 13850 4500
Wire Bus Line
	12900 4400 13850 4400
Text Label 13000 4600 0    60   ~ 0
TVB[0..7]
Text Label 13000 4500 0    60   ~ 0
TVG[0..7]
Text Label 13000 4400 0    60   ~ 0
TVR[0..7]
$Sheet
S 11250 4200 1650 1950
U 4BF0367D
F0 "muxdata" 60
F1 "muxdata.sch" 60
F2 "X_DIN" I L 11250 4300 60 
F3 "X_CLK" I L 11250 4400 60 
F4 "CLKCDA" I L 11250 4900 60 
F5 "CLKCAD" I L 11250 5000 60 
F6 "X_DONE" O L 11250 4500 60 
F7 "X_PROG-" I L 11250 4600 60 
F8 "TVB[0..7]" B R 12900 4600 60 
F9 "TVG[0..7]" B R 12900 4500 60 
F10 "TVR[0..7]" B R 12900 4400 60 
F11 "VRAM[0..31]" B R 12900 4900 60 
F12 "DPC[0..31]" B L 11250 5200 60 
F13 "ACCES_RAM-" I L 11250 5350 60 
F14 "DATA_WR" I L 11250 5450 60 
F15 "ACQ_ON" I L 11250 5650 60 
F16 "CSYNC-OUT" I L 11250 5800 60 
F17 "BLANK-" I L 11250 5950 60 
F18 "CLAMP" I L 11250 6050 60 
$EndSheet
Wire Wire Line
	12600 2450 13300 2450
Text Label 12650 2450 0    60   ~ 0
CSYNC-OUT
Text Label 12600 2100 0    60   ~ 0
BLUE_OUT
Text Label 12600 2000 0    60   ~ 0
GREEN_OUT
Text Label 12600 1900 0    60   ~ 0
RED_OUT
Wire Wire Line
	12550 2100 13300 2100
Wire Wire Line
	12550 2000 13300 2000
Wire Wire Line
	12550 1900 13300 1900
Text Label 14200 4050 0    60   ~ 0
BLUE_OUT
Text Label 14200 3850 0    60   ~ 0
GREEN_OUT
Text Label 14200 3650 0    60   ~ 0
RED_OUT
Wire Wire Line
	14150 4050 14900 4050
Wire Wire Line
	14150 3850 14900 3850
Wire Wire Line
	14150 3650 14900 3650
Text Label 8900 2950 0    60   ~ 0
BLUE_OUT
Text Label 8900 2850 0    60   ~ 0
GREEN_OUT
Text Label 8900 2750 0    60   ~ 0
RED_OUT
Wire Wire Line
	8850 2950 9500 2950
Wire Wire Line
	8850 2850 9500 2850
Wire Wire Line
	8850 2750 9500 2750
$Comp
L video_schlib:GND #GND01
U 1 1 84DFBB6C
P 14800 4650
F 0 "#GND01" H 14800 4750 60  0001 C CNN
F 1 "GND" H 14800 4550 60  0000 C CNN
F 2 "" H 14800 4650 60  0001 C CNN
F 3 "" H 14800 4650 60  0001 C CNN
	1    14800 4650
	1    0    0    -1  
$EndComp
$Comp
L video_schlib:DB9 J4
U 1 1 84DFBB8F
P 15350 4050
F 0 "J4" H 15350 4600 70  0000 C CNN
F 1 "DB9FEM" H 15350 3500 70  0000 C CNN
F 2 "Connector_Dsub:DSUB-9_Female_Horizontal_P2.77x2.84mm_EdgePinOffset14.56mm_Housed_MountingHolesOffset15.98mm" H 15350 4050 60  0001 C CNN
F 3 "" H 15350 4050 60  0001 C CNN
	1    15350 4050
	1    0    0    1   
$EndComp
Connection ~ 14800 4450
Wire Wire Line
	14800 4250 14900 4250
Wire Wire Line
	14800 4450 14900 4450
Wire Wire Line
	14650 2700 15200 2700
Wire Wire Line
	15200 2300 14650 2300
Wire Wire Line
	14650 1900 15200 1900
$Comp
L video_schlib:GND #GND02
U 1 1 30705D34
P 15350 2900
F 0 "#GND02" H 15350 3000 60  0001 C CNN
F 1 "GND" H 15350 2800 60  0000 C CNN
F 2 "" H 15350 2900 60  0001 C CNN
F 3 "" H 15350 2900 60  0001 C CNN
	1    15350 2900
	1    0    0    -1  
$EndComp
$Comp
L video_schlib:BNC P8
U 1 1 30705D02
P 15350 2700
F 0 "P8" H 15360 2820 60  0000 C CNN
F 1 "BNC" V 15460 2640 40  0000 C CNN
F 2 "Discret:SUBCLICK" H 15350 2700 60  0001 C CNN
F 3 "" H 15350 2700 60  0001 C CNN
	1    15350 2700
	1    0    0    -1  
$EndComp
$Sheet
S 13300 1750 1350 1100
U 4BF0367F
F0 "modul" 60
F1 "modul.sch" 60
F2 "BLUE" I L 13300 1900 60 
F3 "GREEN" I L 13300 2000 60 
F4 "RED" I L 13300 2100 60 
F5 "CVBSOUT" O R 14650 2700 60 
F6 "YOUT" O R 14650 2300 60 
F7 "COUT" O R 14650 1900 60 
F8 "CSYNC-OUT" I L 13300 2450 60 
$EndSheet
Wire Wire Line
	9400 5000 8800 5000
Wire Wire Line
	9400 5100 8800 5100
Wire Wire Line
	9400 5200 8800 5200
Text Label 8900 5000 0    60   ~ 0
F_PALIN
Text Label 8900 5100 0    60   ~ 0
HD_PAL-
Text Label 8900 5200 0    60   ~ 0
VD_PAL-
Wire Wire Line
	2000 8150 1400 8150
Wire Wire Line
	2000 8050 1400 8050
Wire Wire Line
	2000 7950 1400 7950
Text Label 1500 8150 0    60   ~ 0
F_PALIN
Text Label 1500 8050 0    60   ~ 0
HD_PAL-
Text Label 1500 7950 0    60   ~ 0
VD_PAL-
Wire Wire Line
	6150 5050 6900 5050
Wire Wire Line
	6150 4950 6900 4950
Text Label 6200 5050 0    60   ~ 0
BT812_WR-
Text Label 6200 4950 0    60   ~ 0
BT812_RD-
Wire Wire Line
	3800 8050 4550 8050
Wire Wire Line
	3800 7950 4550 7950
Text Label 3850 8050 0    60   ~ 0
BT812_WR-
Text Label 3850 7950 0    60   ~ 0
BT812_RD-
Wire Wire Line
	6300 5600 6900 5600
Text Label 6350 5600 0    60   ~ 0
OE_PAL-
Wire Wire Line
	8850 3200 9450 3200
Text Label 8900 3200 0    60   ~ 0
OE_PAL-
Wire Wire Line
	6850 3100 6200 3100
Text Label 6250 3100 0    60   ~ 0
BLUE_IN
Wire Wire Line
	6850 3000 6200 3000
Text Label 6250 3000 0    60   ~ 0
GREEN_IN
Wire Wire Line
	6300 2900 6850 2900
Text Label 6350 2900 0    60   ~ 0
RED_IN
Wire Wire Line
	6900 4650 6250 4650
Text Label 6300 4650 0    60   ~ 0
GREEN_IN
Wire Wire Line
	6350 4750 6900 4750
Text Label 6400 4750 0    60   ~ 0
RED_IN
Wire Wire Line
	11950 3400 11300 3400
Wire Wire Line
	11950 2600 11300 2600
Wire Wire Line
	11300 1800 11950 1800
Text Label 11350 3400 0    60   ~ 0
BLUE_IN
Text Label 11350 2600 0    60   ~ 0
GREEN_IN
Text Label 11450 1800 0    60   ~ 0
RED_IN
$Comp
L video_schlib:GND #GND03
U 1 1 32FA02CD
P 12100 2800
F 0 "#GND03" H 12100 2800 40  0001 C CNN
F 1 "GND" H 12100 2730 40  0000 C CNN
F 2 "" H 12100 2800 60  0001 C CNN
F 3 "" H 12100 2800 60  0001 C CNN
	1    12100 2800
	1    0    0    -1  
$EndComp
$Comp
L video_schlib:GND #GND04
U 1 1 32FA02C6
P 12100 3600
F 0 "#GND04" H 12100 3600 40  0001 C CNN
F 1 "GND" H 12100 3530 40  0000 C CNN
F 2 "" H 12100 3600 60  0001 C CNN
F 3 "" H 12100 3600 60  0001 C CNN
	1    12100 3600
	1    0    0    -1  
$EndComp
$Comp
L video_schlib:GND #GND05
U 1 1 32FA02C2
P 12100 2000
F 0 "#GND05" H 12100 2000 40  0001 C CNN
F 1 "GND" H 12100 1930 40  0000 C CNN
F 2 "" H 12100 2000 60  0001 C CNN
F 3 "" H 12100 2000 60  0001 C CNN
	1    12100 2000
	1    0    0    -1  
$EndComp
$Comp
L video_schlib:BNC P3
U 1 1 32F9F1AD
P 12100 3400
F 0 "P3" H 12110 3520 60  0000 C CNN
F 1 "BNC" V 12210 3340 40  0000 C CNN
F 2 "Discret:SUBCLICK" H 12100 3400 60  0001 C CNN
F 3 "" H 12100 3400 60  0001 C CNN
	1    12100 3400
	1    0    0    -1  
$EndComp
$Comp
L video_schlib:BNC P2
U 1 1 32F9F1A3
P 12100 2600
F 0 "P2" H 12110 2720 60  0000 C CNN
F 1 "BNC" V 12210 2540 40  0000 C CNN
F 2 "Discret:SUBCLICK" H 12100 2600 60  0001 C CNN
F 3 "" H 12100 2600 60  0001 C CNN
	1    12100 2600
	1    0    0    -1  
$EndComp
$Comp
L video_schlib:BNC P1
U 1 1 32F9F198
P 12100 1800
F 0 "P1" H 12110 1920 60  0000 C CNN
F 1 "BNC" V 12210 1740 40  0000 C CNN
F 2 "Discret:SUBCLICK" H 12100 1800 60  0001 C CNN
F 3 "" H 12100 1800 60  0001 C CNN
	1    12100 1800
	1    0    0    -1  
$EndComp
Text Label 6200 4350 0    60   ~ 0
DQ[0..7]
Wire Bus Line
	6100 4350 6900 4350
Wire Wire Line
	6350 4550 6900 4550
Text Label 6400 4550 0    60   ~ 0
SYSRST-
Wire Wire Line
	3800 3600 4350 3600
Wire Bus Line
	8800 4600 9750 4600
Wire Bus Line
	8800 4500 9750 4500
Wire Bus Line
	8800 4400 9750 4400
Text Label 8900 4600 0    60   ~ 0
TVB[0..7]
Text Label 8900 4500 0    60   ~ 0
TVG[0..7]
Text Label 8900 4400 0    60   ~ 0
TVR[0..7]
$Sheet
S 6900 4200 1900 1550
U 4BF03681
F0 "pal-ntsc.sch" 60
F1 "pal-ntsc.sch" 60
F2 "F_PALIN" O R 8800 5000 60 
F3 "TVR[0..7]" B R 8800 4400 60 
F4 "PC_D[0..7]" B L 6900 4350 60 
F5 "PC_A[0..1]" I L 6900 4450 60 
F6 "RESET-" I L 6900 4550 60 
F7 "GREEN_IN" I L 6900 4650 60 
F8 "RED_IN" I L 6900 4750 60 
F9 "OE_PAL-" I L 6900 5600 60 
F10 "HD_PAL-" O R 8800 5100 60 
F11 "VD_PAL-" O R 8800 5200 60 
F12 "TVB[0..7]" B R 8800 4600 60 
F13 "TVG[0..7]" B R 8800 4500 60 
F14 "BT812_WR-" I L 6900 5050 60 
F15 "BT812_RD-" I L 6900 4950 60 
$EndSheet
Text Label 6150 1800 0    60   ~ 0
PCA[0..2]
Wire Bus Line
	6050 1800 6850 1800
Text Label 1400 6000 0    60   ~ 0
RDFIFO-
Text Label 1400 5900 0    60   ~ 0
WRFIFDO-
Wire Wire Line
	2000 6000 1400 6000
Wire Wire Line
	2000 5900 1400 5900
Text Label 1400 2750 0    60   ~ 0
RDFIFO-
Text Label 1400 2650 0    60   ~ 0
WRFIFDO-
Wire Wire Line
	1950 2750 1350 2750
Wire Wire Line
	1950 2650 1350 2650
Wire Wire Line
	3800 3450 4350 3450
Wire Wire Line
	3800 3350 4350 3350
Text Label 3900 3450 0    60   ~ 0
RDEMPTY
Text Label 3900 3350 0    60   ~ 0
WRFULL
Wire Wire Line
	1450 7750 2000 7750
Wire Wire Line
	1450 7650 2000 7650
Text Label 1500 7750 0    60   ~ 0
RDEMPTY
Text Label 1500 7650 0    60   ~ 0
WRFULL
Text Label 1400 6400 0    60   ~ 0
PTRDY-
Wire Wire Line
	1400 6400 2000 6400
Wire Wire Line
	1350 6950 2000 6950
Text Label 1450 6950 0    60   ~ 0
PTBURST
Wire Wire Line
	3800 2950 4450 2950
Text Label 3900 2950 0    60   ~ 0
PTBURST
Text Label 1400 2400 0    60   ~ 0
PTRDY-
Wire Wire Line
	1350 2400 1950 2400
Wire Wire Line
	2000 7050 1400 7050
Text Label 1450 7050 0    60   ~ 0
SELECT-
Wire Wire Line
	1500 6850 2000 6850
Text Label 1600 6850 0    60   ~ 0
PTWR
Wire Wire Line
	3800 3050 4300 3050
Wire Wire Line
	1950 2500 1350 2500
Text Label 3900 3050 0    60   ~ 0
PTWR
Text Label 1400 2500 0    60   ~ 0
SELECT-
Wire Wire Line
	1500 6750 2000 6750
Wire Wire Line
	1400 6300 2000 6300
Text Label 1400 6300 0    60   ~ 0
PTATN-
Text Label 1550 6750 0    60   ~ 0
PTADR-
Wire Wire Line
	2000 7150 1500 7150
Text Label 1500 7150 0    60   ~ 0
BPCLK
Wire Wire Line
	3800 3150 4300 3150
Wire Wire Line
	3800 2650 4300 2650
Wire Wire Line
	3800 2550 4300 2550
Text Label 3900 3150 0    60   ~ 0
BPCLK
Text Label 3850 2550 0    60   ~ 0
PTATN-
Text Label 3850 2650 0    60   ~ 0
PTADR-
Wire Bus Line
	2000 7450 1300 7450
Text Label 1350 7450 0    60   ~ 0
BE-[0..3]
Wire Bus Line
	3800 2750 4600 2750
Text Label 3850 2750 0    60   ~ 0
PTBE-[0..3]
Wire Bus Line
	1200 6650 2000 6650
Wire Bus Line
	1950 1800 1250 1800
Text Label 1300 1800 0    60   ~ 0
BE-[0..3]
Text Label 1250 6650 0    60   ~ 0
PTBE-[0..3]
Wire Wire Line
	1400 6550 2000 6550
Wire Wire Line
	1400 7350 2000 7350
Text Label 1500 7350 0    60   ~ 0
PTNUM0
Text Label 1500 6550 0    60   ~ 0
PTNUM1
Wire Wire Line
	1450 7250 2000 7250
Text Label 1500 7250 0    60   ~ 0
SYSRST-
Text Label 3900 3600 0    60   ~ 0
SYSRST-
Text Label 3900 2450 0    60   ~ 0
PTNUM1
Text Label 3900 2350 0    60   ~ 0
PTNUM0
Wire Wire Line
	3800 2450 4300 2450
Wire Wire Line
	3800 2350 4300 2350
Text Label 1300 5400 0    60   ~ 0
ADR[2..6]
Text Label 3950 5700 0    60   ~ 0
BLANK-
Wire Wire Line
	3800 5700 4350 5700
Text Label 3850 5400 0    60   ~ 0
CSYNC-OUT
Text Label 1400 6150 0    60   ~ 0
CSYNCIN-
Wire Wire Line
	1400 6150 2000 6150
Text Label 8950 2400 0    60   ~ 0
CSYNCIN-
Wire Wire Line
	8850 2400 9700 2400
Wire Wire Line
	6150 3450 6850 3450
Wire Wire Line
	3800 5400 4500 5400
Text Label 6200 3450 0    60   ~ 0
CSYNC-OUT
Text Label 6450 6550 0    60   ~ 0
WRAM-
Wire Wire Line
	6450 6550 6950 6550
Text Label 3900 6950 0    60   ~ 0
WRAM-
Wire Wire Line
	3800 6950 4300 6950
Wire Wire Line
	6350 2000 6850 2000
Wire Wire Line
	6350 2100 6850 2100
Wire Wire Line
	6350 2200 6850 2200
Wire Wire Line
	6350 2300 6850 2300
Text Label 6400 2000 0    60   ~ 0
WRCAD-
Text Label 6400 2100 0    60   ~ 0
RDCAD-
Text Label 6400 2200 0    60   ~ 0
WRCDA-
Text Label 6400 2300 0    60   ~ 0
RDCDA-
Wire Wire Line
	3800 7850 4300 7850
Wire Wire Line
	3800 7750 4300 7750
Wire Wire Line
	3800 7650 4300 7650
Wire Wire Line
	3800 7550 4300 7550
Text Label 3850 7850 0    60   ~ 0
WRCAD-
Text Label 3850 7750 0    60   ~ 0
RDCAD-
Text Label 3850 7650 0    60   ~ 0
WRCDA-
Text Label 3850 7550 0    60   ~ 0
RDCDA-
Text Label 6450 7900 0    60   ~ 0
RAS7-
Text Label 6450 7800 0    60   ~ 0
RAS6-
Text Label 6450 7700 0    60   ~ 0
RAS5-
Text Label 6450 7600 0    60   ~ 0
RAS4-
Text Label 6450 7500 0    60   ~ 0
RAS3-
Text Label 6450 7400 0    60   ~ 0
RAS2-
Text Label 6450 7300 0    60   ~ 0
RAS1-
Text Label 6450 7200 0    60   ~ 0
RAS0-
Wire Wire Line
	6450 7900 6950 7900
Wire Wire Line
	6450 7800 6950 7800
Wire Wire Line
	6450 7700 6950 7700
Wire Wire Line
	6450 7600 6950 7600
Wire Wire Line
	6450 7500 6950 7500
Wire Wire Line
	6450 7400 6950 7400
Wire Wire Line
	6450 7300 6950 7300
Wire Wire Line
	6450 7200 6950 7200
Text Label 6450 7000 0    60   ~ 0
CAS3-
Text Label 6450 6900 0    60   ~ 0
CAS2-
Text Label 6450 6800 0    60   ~ 0
CAS1-
Text Label 6450 6700 0    60   ~ 0
CAS0-
Wire Wire Line
	6450 7000 6950 7000
Wire Wire Line
	6450 6900 6950 6900
Wire Wire Line
	6450 6800 6950 6800
Wire Wire Line
	6450 6700 6950 6700
Text Label 3900 7350 0    60   ~ 0
CAS3-
Text Label 3900 7250 0    60   ~ 0
CAS2-
Text Label 3900 7150 0    60   ~ 0
CAS1-
Text Label 3900 7050 0    60   ~ 0
CAS0-
Text Label 3900 6800 0    60   ~ 0
RAS7-
Text Label 3900 6700 0    60   ~ 0
RAS6-
Text Label 3900 6600 0    60   ~ 0
RAS5-
Text Label 3900 6500 0    60   ~ 0
RAS4-
Text Label 3900 6400 0    60   ~ 0
RAS3-
Text Label 3900 6300 0    60   ~ 0
RAS2-
Text Label 3900 6200 0    60   ~ 0
RAS1-
Text Label 3900 6100 0    60   ~ 0
RAS0-
Wire Wire Line
	3800 7350 4300 7350
Wire Wire Line
	3800 7250 4300 7250
Wire Wire Line
	3800 7150 4300 7150
Wire Wire Line
	3800 7050 4300 7050
Wire Wire Line
	3800 6800 4300 6800
Wire Wire Line
	3800 6700 4300 6700
Wire Wire Line
	3800 6600 4300 6600
Wire Wire Line
	3800 6500 4300 6500
Wire Wire Line
	3800 6400 4300 6400
Wire Wire Line
	3800 6300 4300 6300
Wire Wire Line
	3800 6200 4300 6200
Wire Wire Line
	3800 6100 4300 6100
Text Label 1400 5600 0    60   ~ 0
/PCWR
Text Label 1400 5700 0    60   ~ 0
/PCRD
Wire Wire Line
	1400 5600 2000 5600
Wire Wire Line
	1400 5700 2000 5700
Wire Bus Line
	1200 5400 2000 5400
Text Label 1300 5300 0    60   ~ 0
DQ[0..15]
Wire Bus Line
	1200 5300 2000 5300
Wire Wire Line
	6150 3350 6850 3350
Text Label 6300 3350 0    60   ~ 0
CLAMP
Wire Wire Line
	3800 5600 4300 5600
Text Label 3950 5600 0    60   ~ 0
CLAMP
Wire Bus Line
	6100 8050 6950 8050
Text Label 6250 8050 0    60   ~ 0
MXA[0..10]
Wire Bus Line
	3800 6000 4600 6000
Text Label 3900 6000 0    60   ~ 0
MXA[0..10]
Wire Wire Line
	6350 2700 6850 2700
Wire Wire Line
	6350 2600 6850 2600
Text Label 6400 2600 0    60   ~ 0
CLKCAD
Text Label 6400 2700 0    60   ~ 0
CLKCDA
Wire Wire Line
	3800 5300 4300 5300
Wire Wire Line
	3800 5200 4300 5200
Text Label 3850 5200 0    60   ~ 0
CLKCAD
Text Label 3850 5300 0    60   ~ 0
CLKCDA
$Sheet
S 2000 5000 1800 4650
U 4BF03683
F0 "graphic" 60
F1 "graphic.sch" 60
F2 "CSYNC-OUT" O R 3800 5400 60 
F3 "DQ[0..15]" B L 2000 5300 60 
F4 "ADR[2..6]" O L 2000 5400 60 
F5 "CADCLK" O R 3800 5200 60 
F6 "CDACLK" O R 3800 5300 60 
F7 "CLAMP" O R 3800 5600 60 
F8 "CSYNCIN-" I L 2000 6150 60 
F9 "MXA[0..10]" O R 3800 6000 60 
F10 "RAS7-" O R 3800 6800 60 
F11 "RAS6-" O R 3800 6700 60 
F12 "RAS5-" O R 3800 6600 60 
F13 "RAS4-" O R 3800 6500 60 
F14 "RAS3-" O R 3800 6400 60 
F15 "RAS2-" O R 3800 6300 60 
F16 "RAS1-" O R 3800 6200 60 
F17 "RAS0-" O R 3800 6100 60 
F18 "CAS2-" O R 3800 7250 60 
F19 "CAS1-" O R 3800 7150 60 
F20 "CAS0-" O R 3800 7050 60 
F21 "RD-" I L 2000 5700 60 
F22 "WR-" I L 2000 5600 60 
F23 "CAS3-" O R 3800 7350 60 
F24 "RDCDA-" O R 3800 7550 60 
F25 "WRCDA-" O R 3800 7650 60 
F26 "RDCAD-" O R 3800 7750 60 
F27 "WRCAD-" O R 3800 7850 60 
F28 "WRAM-" O R 3800 6950 60 
F29 "BLANK-" O R 3800 5700 60 
F30 "PTATN-" I L 2000 6300 60 
F31 "PTRDY-" O L 2000 6400 60 
F32 "PTNUM1" I L 2000 6550 60 
F33 "PTBE-[0..3]" I L 2000 6650 60 
F34 "PTADR-" O L 2000 6750 60 
F35 "PTWR" I L 2000 6850 60 
F36 "PTBURST-" I L 2000 6950 60 
F37 "SELECT-" O L 2000 7050 60 
F38 "BPCLK" I L 2000 7150 60 
F39 "SYSRST-" I L 2000 7250 60 
F40 "PTNUM0" I L 2000 7350 60 
F41 "BE-[0..3]" O L 2000 7450 60 
F42 "WRFULL" O L 2000 7650 60 
F43 "RDEMPTY" O L 2000 7750 60 
F44 "RDFIFO-" O L 2000 6000 60 
F45 "WRFIFO-" O L 2000 5900 60 
F46 "BT812_WR-" O R 3800 8050 60 
F47 "BT812_RD-" O R 3800 7950 60 
F48 "VD_PAL-" I L 2000 7950 60 
F49 "HD_PAL-" I L 2000 8050 60 
F50 "F_PALIN" I L 2000 8150 60 
F51 "X_DOUT" O R 3800 8500 60 
F52 "X_CLK" O R 3800 8600 60 
F53 "X_DONE" O R 3800 8700 60 
F54 "X_PROG-" O R 3800 8800 60 
F55 "WRITE_RAM" O R 3800 9000 60 
F56 "ACCES_RAM-" O R 3800 9100 60 
F57 "TVI[0..1]" O R 3800 5900 60 
F58 "PCA[0..2]" O R 3800 9300 60 
F59 "X_IRQ" B L 2000 8300 60 
F60 "IRQ-" I L 2000 8400 60 
F61 "IRQ_SLR" B L 2000 8500 60 
F62 "ACQ_ON" O R 3800 8300 60 
$EndSheet
Wire Bus Line
	8850 2200 9800 2200
Text Label 8950 2200 0    60   ~ 0
TVI[0..1]
Wire Bus Line
	12900 4900 13850 4900
Text Label 13000 4900 0    60   ~ 0
TVRAM[0..31]
$Sheet
S 6950 6450 1850 1700
U 4BF03685
F0 "RAMS" 60
F1 "rams.sch" 60
F2 "TVRAM[0..31]" T R 8800 6600 60 
F3 "WRAM-" I L 6950 6550 60 
F4 "CAS3-" I L 6950 7000 60 
F5 "RAS7-" I L 6950 7900 60 
F6 "RAS6-" I L 6950 7800 60 
F7 "RAS5-" I L 6950 7700 60 
F8 "RAS4-" I L 6950 7600 60 
F9 "RAS3-" I L 6950 7500 60 
F10 "RAS2-" I L 6950 7400 60 
F11 "RAS1-" I L 6950 7300 60 
F12 "RAS0-" I L 6950 7200 60 
F13 "CAS2-" I L 6950 6900 60 
F14 "CAS1-" I L 6950 6800 60 
F15 "CAS0-" I L 6950 6700 60 
F16 "MXA[0..10]" I L 6950 8050 60 
$EndSheet
Wire Bus Line
	6850 1900 6000 1900
Wire Bus Line
	8850 2100 9800 2100
Wire Bus Line
	8850 2000 9800 2000
Wire Bus Line
	8850 1900 9800 1900
Text Label 8950 2100 0    60   ~ 0
TVB[0..7]
Text Label 8950 2000 0    60   ~ 0
TVG[0..7]
$Sheet
S 1950 1600 1850 2400
U 4BF03687
F0 "buspci.sch" 60
F1 "bus_pci.sch" 60
F2 "WR-" I L 1950 1950 60 
F3 "RD-" I L 1950 2050 60 
F4 "DQ[0..31]" B R 3800 1800 60 
F5 "ADR[2..6]" I L 1950 2200 60 
F6 "BE-[0..3]" I L 1950 1800 60 
F7 "IRQ_SRL" B R 3800 1950 60 
F8 "X_IRQ" B R 3800 2050 60 
F9 "PTNUM0" O R 3800 2350 60 
F10 "PTNUM1" O R 3800 2450 60 
F11 "PTATN-" O R 3800 2550 60 
F12 "PTRDY-" I L 1950 2400 60 
F13 "PTBE-[0..3]" O R 3800 2750 60 
F14 "PTWR" O R 3800 3050 60 
F15 "PTBURST-" O R 3800 2950 60 
F16 "SELECT-" I L 1950 2500 60 
F17 "BPCLK" O R 3800 3150 60 
F18 "SYSRST-" O R 3800 3600 60 
F19 "WRFULL" O R 3800 3350 60 
F20 "RDEMPTY" O R 3800 3450 60 
F21 "WRFIFO-" I L 1950 2650 60 
F22 "RDFIFO-" I L 1950 2750 60 
F23 "IRQ-" O R 3800 2150 60 
F24 "PTADR-" O R 3800 2650 60 
$EndSheet
Text Label 1250 2200 0    60   ~ 0
ADR[2..6]
Text Label 1400 1950 0    60   ~ 0
/PCWR
Text Label 1400 2050 0    60   ~ 0
/PCRD
Text Label 6150 1900 0    60   ~ 0
DQ[0..7]
Text Label 8950 1900 0    60   ~ 0
TVR[0..7]
Text Label 3900 1800 0    60   ~ 0
DQ[0..31]
Text Label 6300 3250 0    60   ~ 0
BLANK-
Wire Wire Line
	1250 1950 1950 1950
Wire Wire Line
	1250 2050 1950 2050
Wire Wire Line
	6150 3250 6850 3250
Wire Bus Line
	3800 1800 4600 1800
Wire Bus Line
	1150 2200 1950 2200
$Sheet
S 6850 1650 2000 2150
U 4BF03689
F0 "ESVIDEO-RVB" 60
F1 "esvideo.sch" 60
F2 "DPC[0..7]" B L 6850 1900 60 
F3 "TVR[0..7]" O R 8850 1900 60 
F4 "BLANK-" I L 6850 3250 60 
F5 "TVG[0..7]" O R 8850 2000 60 
F6 "TVB[0..7]" O R 8850 2100 60 
F7 "WRCAD-" I L 6850 2000 60 
F8 "RDCAD-" I L 6850 2100 60 
F9 "WRCDA-" I L 6850 2200 60 
F10 "RDCDA-" I L 6850 2300 60 
F11 "CSYNCIN-" O R 8850 2400 60 
F12 "CSYNCOUT-" I L 6850 3450 60 
F13 "CLKCAD" I L 6850 2600 60 
F14 "CLKCDA" I L 6850 2700 60 
F15 "TVI[0..1]" O R 8850 2200 60 
F16 "CLAMP" I L 6850 3350 60 
F17 "BLUE_IN" I L 6850 3100 60 
F18 "GREEN_IN" I L 6850 3000 60 
F19 "RED_IN" I L 6850 2900 60 
F20 "RED" O R 8850 2750 60 
F21 "GREEN" O R 8850 2850 60 
F22 "BLUE" O R 8850 2950 60 
F23 "PCA[0..2]" I L 6850 1800 60 
F24 "OE_PAL-" O R 8850 3200 60 
$EndSheet
Wire Wire Line
	14800 4250 14800 4450
Wire Wire Line
	14800 3950 14800 4250
Wire Wire Line
	14800 4450 14800 4650
$EndSCHEMATC
