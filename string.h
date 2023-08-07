#ifndef __STRING_H_INCLUDED__
#define __STRING_H_INCLUDED__

#include <stdint.h>

void strcatnum(uint8_t *str, uint8_t num, uint8_t numwidth, uint8_t padchar);
void strcpynum(uint8_t *str, uint8_t num, uint8_t numwidth, uint8_t padchar);

#endif //__STRING_H_INCLUDED__
