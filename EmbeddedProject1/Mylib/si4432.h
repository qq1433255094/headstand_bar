#ifndef __SI4432_H__
#define __SI4432_H__

void rx_data(void);
void tx_data(void);
void SI4432_init(void);
void SI4432_IO_init(void);
unsigned char spi_rw(unsigned char addr, unsigned char TRdata);

#endif 
