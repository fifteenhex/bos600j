CC=sdcc

bos600j.ihx: bos600j.c
	sdcc -lstm8 -mstm8 --out-fmt-ihx $<

.PHONY:clean

clean:
	rm *.ihx
