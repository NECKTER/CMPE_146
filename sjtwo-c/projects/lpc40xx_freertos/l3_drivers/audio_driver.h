// #include <stdio.h>
#include "gpio.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include "ssp2.h"
#include <stdio.h>
// #define VS1053_SCI_READ 0x03  //!< Serial read address
// #define VS1053_SCI_WRITE 0x02 //!< Serial write address

// #define VS1053_REG_MODE 0x00       //!< Mode control
// #define VS1053_REG_STATUS 0x01     //!< Status of VS1053b
// #define VS1053_REG_BASS 0x02       //!< Built-in bass/treble control
// #define VS1053_REG_CLOCKF 0x03     //!< Clock frequency + multiplier
// #define VS1053_REG_DECODETIME 0x04 //!< Decode time in seconds
// #define VS1053_REG_AUDATA 0x05     //!< Misc. audio data
// #define VS1053_REG_WRAM 0x06       //!< RAM write/read
// #define VS1053_REG_WRAMADDR 0x07   //!< Base address for RAM write/read
// #define VS1053_REG_HDAT0 0x08      //!< Stream header data 0
// #define VS1053_REG_HDAT1 0x09      //!< Stream header data 1
// #define VS1053_REG_VOLUME 0x0B     //!< Volume control

// #define VS1053_GPIO_DDR 0xC017   //!< Direction
// #define VS1053_GPIO_IDATA 0xC018 //!< Values read from pins
// #define VS1053_GPIO_ODATA 0xC019 //!< Values set to the pins

// #define VS1053_INT_ENABLE 0xC01A //!< Interrupt enable

// #define VS1053_MODE_SM_DIFF
// 0x0001                                 //!< Differential, 0: normal in-phase audio, 1: left channel inverted
// #define VS1053_MODE_SM_LAYER12 0x0002  //!< Allow MPEG layers I & II
// #define VS1053_MODE_SM_RESET 0x0004    //!< Soft reset
// #define VS1053_MODE_SM_CANCEL 0x0008   //!< Cancel decoding current file
// #define VS1053_MODE_SM_EARSPKLO 0x0010 //!< EarSpeaker low setting
// #define VS1053_MODE_SM_TESTS 0x0020    //!< Allow SDI tests
// #define VS1053_MODE_SM_STREAM 0x0040   //!< Stream mode
// #define VS1053_MODE_SM_SDINEW 0x0800   //!< VS1002 native SPI modes
// #define VS1053_MODE_SM_ADPCM 0x1000    //!< PCM/ADPCM recording active
// #define VS1053_MODE_SM_LINE1 0x4000    //!< MIC/LINE1 selector, 0: MICP, 1: LINE1
// #define VS1053_MODE_SM_CLKRANGE
//     0x8000 //!< Input clock range, 0: 12..13 MHz, 1: 24..26 MHz

// #define VS1053_SCI_AIADDR
//     0x0A //!< Indicates the start address of the application code written earlier
//          //!< with SCI_WRAMADDR and SCI_WRAM registers.
// #define VS1053_SCI_AICTRL0
//     0x0C //!< SCI_AICTRL register 0. Used to access the user's application program
// #define VS1053_SCI_AICTRL1
//     0x0D //!< SCI_AICTRL register 1. Used to access the user's application program
// #define VS1053_SCI_AICTRL2
//     0x0E //!< SCI_AICTRL register 2. Used to access the user's application program
// #define VS1053_SCI_AICTRL3
//     0x0F //!< SCI_AICTRL register 3. Used to access the user's application program

// #define VS1053_DATABUFFERLEN 32 //!< Length of the data buffer

//     void
//     pin_init();
// void spi_software_init(uint8_t rst, uint8_t cs, uint8_t deselect_cs, uint8_t dreq);

// void spi_hardware_init(uint8_t rst, uint8_t cs, uint8_t deselect_cs, uint8_t dreq);

// uint8_t begin();

bool pause_music(bool pause);

void reset();

void soft_reset();

uint16_t sci_read(uint8_t address_to_read_from);

void pin_config();

void sci_write(uint8_t address_to_write_to, uint16_t data_to_write);

void sine_test(uint8_t which_sine_test, uint16_t ms_delay);

void set_volume(uint8_t left, uint8_t right);

// void set_xdcs(bool check);

// // maybe
// void spi_write_data(uint8_t data_to_write);

// void spi_write_buffer(uint8_t *char_pointer_to_buffer, uint16_t number_of_elements_in_buffer);

// uint8_t spi_read();

// void print_registers();

void play_data(uint8_t *buffer, uint16_t buffer_size);

bool dreq_level();

// bool ready_for_data();
