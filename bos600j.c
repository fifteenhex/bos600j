/* Firmware to replace the original on the bos600 600mW
 * 5.8ghz video transmitter to make it legal to operate
 * on the ATV part of the 5.6ghz band in Japan with a
 * ham license by removing the ability to use channels
 * that aren't within the ATV part of the band.
 * You will need to get a guarantee for the transmitter
 * before you can actually operate it.
 *
 * Hardware info:
 * The transmitter consists of a standard 5.8ghz solder
 * down VTX module and a carrier PCB that has a button,
 * 4 leds for the current band, 8 leds for the current
 * channel and a stm8 microcontroller. The microcontroller
 * communicates with the VTX module via what is called SPI
 * in the data for the main VTX chip but uses a combined
 * MOSI/MISO line. There is also a latch line that is used
 * to push the new register values written via SPI into
 * the actual register.
 *
 * Button - Pin 12
 *
 * Band leds:
 * A - Pin 1
 * B - Pin 2
 * E - pin 3
 * F - pin 5
 *
 * Channel leds:
 * 1 - Pin 6
 * 2 - Pin 20
 * 3 - Pin 19
 * 4 - Pin 18
 * 5 - Pin 17
 * 6 - Pin 16
 * 7 - pin 11
 * 8 - Pin 10
 *
 * SPI:
 * clk - 15
 * latch - 14
 * data - 13
 */

int main(){
	while(1){
	}
}
