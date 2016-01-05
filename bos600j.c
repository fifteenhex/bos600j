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
 * channel and a stm8s000f3 microcontroller. The
 * microcontroller communicates with the VTX module via
 * what is called SPI in the data for the main VTX chip
 * but uses a combined MOSI/MISO line. There is also
 * a latch line that is used to push the new register
 * values written via SPI into the actual register.
 * Although the microcontroller has hardware SPI the pins
 * used aren't the SPI controller's pins except for clock
 * so it would seem that the SPI is bitbanged in the
 * original firmware.
 *
 * microcontroller connnections:
 *
 * Button - Pin 12 (PB4)
 *
 * Band leds:
 * A	Pin 1 (PD4)
 * B	Pin 2 (PD5)
 * E	pin 3 (PD6)
 * F	pin 5 (PA1)
 *
 * Channel leds:
 * 1	Pin 6	(PA2)
 * 2	Pin 20	(PD3)
 * 3	Pin 19	(PD2)
 * 4	Pin 18	(PD1)
 * 5	Pin 17	(PC7)
 * 6	Pin 16	(PC6)
 * 7	pin 11	(PB5) note: pulled up, pin is open drain
 * 8	Pin 10	(PA3)
 *
 * SPI:
 * clk		Pin 15 (PC5)
 * latch	Pin 14 (PC4)
 * data		Pin 13 (PC3)
 */

#include <stdint.h>
#include <stdbool.h>

#define VTX_ADDRWIDTH	4
#define VTX_REGWIDTH	20
#define VTX_CLKBIT		5
#define VTX_LATCHBIT	4
#define VTX_DATABIT		3
#define VTX_DATABITMASK (1 << VTX_DATABIT)
#define VTX_CLOCKBITMASK (1 << VTX_CLKBIT)

#define ARRAYELEMENTS(a) (sizeof(a)/sizeof(a[0]))

#define ODRREG(base) (base)
#define IDRREG(base) (base + 1)
#define DDRREG(base) (base + 2)
#define CR1REG(base) (base + 3)
#define CR2REG(base) (base + 4)

#define PA_BASE 0x5000
#define PB_BASE 0x5005
#define PC_BASE 0x500A
#define PD_BASE 0x500F

#define PA_ODR (volatile uint8_t*)(ODRREG(PA_BASE))
#define PA_DDR (volatile uint8_t*)(DDRREG(PA_BASE))
#define PA_CR1 (volatile uint8_t*)(CR1REG(PA_BASE))
#define PA_CR2 (volatile uint8_t*)(CR2REG(PA_BASE))

#define PB_ODR (volatile uint8_t*)(ODRREG(PB_BASE))
#define PB_IDR (volatile uint8_t*)(IDRREG(PB_BASE))
#define PB_DDR (volatile uint8_t*)(DDRREG(PB_BASE))
#define PB_CR1 (volatile uint8_t*)(CR1REG(PB_BASE))
#define PB_CR2 (volatile uint8_t*)(CR2REG(PB_BASE))

#define PC_ODR (volatile uint8_t*)(ODRREG(PC_BASE))
#define PC_DDR (volatile uint8_t*)(DDRREG(PC_BASE))
#define PC_CR1 (volatile uint8_t*)(CR1REG(PC_BASE))
#define PC_CR2 (volatile uint8_t*)(CR2REG(PC_BASE))

#define PD_ODR (volatile uint8_t*)(ODRREG(PD_BASE))
#define PD_DDR (volatile uint8_t*)(DDRREG(PD_BASE))
#define PD_CR1 (volatile uint8_t*)(CR1REG(PD_BASE))
#define PD_CR2 (volatile uint8_t*)(CR2REG(PD_BASE))

#define VTX_DDR (*PC_DDR)
#define VTX_ODR (*PC_ODR)

#define DATAEEPROMSZ 128
int channelslot = 0;
volatile uint8_t* dataeeprom = (volatile uint8_t*) 0x4000;

typedef struct {
	volatile uint8_t* const odr;
	volatile uint8_t* const cr1;
	volatile uint8_t* const cr2;
	volatile uint8_t* const ddr;
	const int bit;
} led;

static const led leds[] = { //
				{ .odr = PA_ODR, .ddr = PA_DDR, .cr1 = PA_CR1, .cr2 = PA_CR2,
						.bit = 2 }, // Channel 1, PA2
				{ .odr = PD_ODR, .ddr = PD_DDR, .cr1 = PD_CR1, .cr2 = PD_CR2,
						.bit = 3 }, // Channel 2, PD3
				{ .odr = PD_ODR, .ddr = PD_DDR, .cr1 = PD_CR1, .cr2 = PD_CR2,
						.bit = 2 }, // Channel 3, PD2
				{ .odr = PD_ODR, .ddr = PD_DDR, .cr1 = PD_CR1, .cr2 = PD_CR2,
						.bit = 1 }, // Channel 4, PD1
				{ .odr = PC_ODR, .ddr = PC_DDR, .cr1 = PC_CR1, .cr2 = PC_CR2,
						.bit = 7 }, // Channel 5, PC7
				{ .odr = PC_ODR, .ddr = PC_DDR, .cr1 = PC_CR1, .cr2 = PC_CR2,
						.bit = 6 }, // Channel 6, PC6
				{ .odr = PB_ODR, .ddr = PB_DDR, .cr1 = PB_CR1, .cr2 = PB_CR2,
						.bit = 5 }, // Channel 7, PB5
				{ .odr = PA_ODR, .ddr = PA_DDR, .cr1 = PA_CR1, .cr2 = PA_CR2,
						.bit = 3 }, // Channel 8, PA3
				{ .odr = PD_ODR, .ddr = PD_DDR, .cr1 = PD_CR1, .cr2 = PD_CR2,
						.bit = 4 }, // Band A, PD4
				{ .odr = PD_ODR, .ddr = PD_DDR, .cr1 = PD_CR1, .cr2 = PD_CR2,
                                                .bit = 5 }, // Band B, PD5
				{ .odr = PD_ODR, .ddr = PD_DDR, .cr1 = PD_CR1, .cr2 = PD_CR2,
                                                .bit = 6 }, // Band E, PD6
				{ .odr = PA_ODR, .ddr = PA_DDR, .cr1 = PA_CR1, .cr2 = PA_CR2,
                                                .bit = 1 }, // Band F, PA1
};

typedef struct {
	const int number;
} vtxchannel;

#define BANDBOFFSET 8
#define BANDEOFFSET 16
#define BANDFOFFSET 24

static const vtxchannel channels[] = {
	{ .number = 4 }, // 5785mhz
	{ .number = 6 }, // 5745mhz
	{ .number = BANDBOFFSET + 3 }, // 5790mhz
	{ .number = BANDEOFFSET + 0 }, // 5705mhz
	{ .number = BANDFOFFSET + 0 }, // 5740mhz
	{ .number = BANDFOFFSET + 2 }, // 5780mhz
	{ .number = BANDFOFFSET + 3 } // 5800mhz
};

static void initleds() {
	int i;
	for (i = 0; i < ARRAYELEMENTS(leds); i++) {
		// configure the led pin as a push-pull output
		uint8_t mask = (1 << leds[i].bit);
		*leds[i].ddr |= mask;
		*leds[i].cr1 |= mask;
		*leds[i].cr2 &= ~mask;
		*leds[i].odr &= mask;
	}
}

static void setled(int which, bool on) {
	int number = channels[which].number;
	int chanoffset = number & 0x7;
	int bandoffset = ((number & 0x18) >> 3) + 8;
	if (on){
		*leds[chanoffset].odr |= (1 << leds[chanoffset].bit);
		*leds[bandoffset].odr |= (1 << leds[bandoffset].bit);
	}
	else {
		*leds[chanoffset].odr &= ~(1 << leds[chanoffset].bit);
		*leds[bandoffset].odr &= ~(1 << leds[bandoffset].bit);
	}
}

static void initbutton(){
	uint8_t mask = (1 << 4);
	*PB_DDR &= ~mask;
	*PB_CR1 |= mask;
	*PB_CR2 &= ~mask;
}

static bool checkbutton() {
	static int downcount = 0;
	bool ret = false;
	if((*PB_IDR & (1 << 4)) == 0){
		downcount++;
		if(downcount == 1024){
			ret = true;
		}
	}
	else 
		downcount = 0;
	return ret;
}

static void changespidirection(bool in) {
	if (in)
		VTX_DDR &= ~(1 << VTX_DATABIT);
	else
		VTX_DDR |= (1 << VTX_DATABIT);
}

static void initspi() {
	changespidirection(false);
	VTX_DDR |= (1 << VTX_CLKBIT) | (1 << VTX_LATCHBIT);
	VTX_ODR &= ~((1 << VTX_CLKBIT) | (1 << VTX_LATCHBIT));
}

static void setclock(bool high) {
	if (high)
		VTX_ODR |= VTX_CLOCKBITMASK;
	else
		VTX_ODR &= ~VTX_CLOCKBITMASK;
}

static void writebit(bool high) {
	// set data pin
	if (high)
		VTX_ODR |= VTX_DATABITMASK;
	else
		VTX_ODR &= ~VTX_DATABITMASK;
	// wait low time
	// set clock high
	setclock(true);
	// wait high time
	// set clock low
	setclock(false);
}

static bool readbit() {
	return false;
}

static void writeregisteraddress(uint8_t which) {
	int i;
	for (i = 0; i < VTX_ADDRWIDTH; i++) {
		writebit((which >> i) & 1);
	}
}

static void setvtxregister(uint8_t which, uint32_t value) {
	int i;

// set latch low
	writeregisteraddress(which);
	writebit(true);
	for (i = 0; i < VTX_REGWIDTH; i++) {
		writebit((value >> i) & 1);
	}
// set latch high
}

static uint32_t getvtxregister(uint8_t which) {
	int i;
	uint32_t result = 0;

	writeregisteraddress(which);
	writebit(false);
	changespidirection(true);
	for (i = 0; i < VTX_REGWIDTH; i++) {
		result |= ((uint32_t) readbit) << i;
	}
	changespidirection(false);
	return result;
}

static void setchannel(int newchannel) {
	static int lastchannel = -1;
	// unset last channel
	if (lastchannel != -1)
		setled(lastchannel, false);
	lastchannel = newchannel;
	// update channel
	setled(newchannel, true);
}

static int loadchannel() {
	int i;
	for (i = 0; i < DATAEEPROMSZ; i++) {
		if (dataeeprom[i] != 0) {
			channelslot = i;
			return (dataeeprom[i] - 1) % ARRAYELEMENTS(channels);
		}
	}
	return 0;
}

static void savechannel(int channel) {
	dataeeprom[channelslot] = 0;
	channelslot = (channelslot + 1) % DATAEEPROMSZ;
	dataeeprom[channelslot] = (channel + 1);
}

int main() {
	int channel;

	initleds();
	initspi();
	initbutton();
	channel = loadchannel();
	setchannel(channel);

	while (1) {
		if (checkbutton()) {
			channel = (channel + 1) % ARRAYELEMENTS(channels);
			setchannel(channel);
			savechannel(channel);
		}
	}
}
