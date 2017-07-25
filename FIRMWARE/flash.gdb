tar extended-remote /dev/ttyACM1
file main.elf
monitor tpwr enable
monitor swdp_scan
attach 1
load
detach
quit