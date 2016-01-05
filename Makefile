CFLAGS=--std-sdcc99
CC=sdcc


bos600j.ihx: bos600j.c
	sdcc $(CFLAGS) -lstm8 -mstm8 --out-fmt-ihx $<

.PHONY:clean flash

clean:
	rm *.ihx

flash: bos600j.ihx
	./stm8flash/stm8flash -c stlinkv2 -p stm8s003f3 -s flash -w bos600j.ihx

