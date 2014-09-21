#ifndef __CC2420_PCT_H__
#define __CC2420_PCT_H__

#include "contiki.h"
# include "dev/cc2420.h"

#include <stdio.h>
#include <string.h>

void adjust_tx_power(uint8_t result, uint8_t to);
uint8_t get_adjusted_tx_power(uint8_t to);

#endif

