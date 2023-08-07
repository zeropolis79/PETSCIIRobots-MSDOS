#include <stdint.h>
#include "string.h"

void strcatnum(uint8_t *str, uint8_t num, uint8_t numwidth, uint8_t padchar){
	uint8_t len;
	
	len = 0;
	while (str[len] != 0){
		len++;
	}
	// Hundereds
	if (num >= 200){
		str[len] = '2';
		len++;
		num -= 200;
		padchar = '0';
	} else if (num >= 100){
		str[len] = '1';
		len++;
		num -= 100;
		padchar = '0';
	} else if (numwidth > 2){
		str[len] = padchar;
		len++;
	}
	// Tens
	if (num >= 50){
		if (num >= 90){
			str[len] = '9';
			len++;
			num -= 90;
		} else if (num >= 80){
			str[len] = '8';
			len++;
			num -= 80;
		} else if (num >= 70){
			str[len] = '7';
			len++;
			num -= 70;
		} else if (num >= 60){
			str[len] = '6';
			len++;
			num -= 60;
		} else{
			str[len] = '5';
			len++;
			num -= 50;
		}
	} else{
		if (num >= 40){
			str[len] = '4';
			len++;
			num -= 40;
		} else if (num >= 30){
			str[len] = '3';
			len++;
			num -= 30;
		} else if (num >= 20){
			str[len] = '2';
			len++;
			num -= 20;
		} else if (num >= 10){
			str[len] = '1';
			len++;
			num -= 10;
		} else if (numwidth > 1){
			str[len] = padchar;
			len++;
		}
	}
	// Ones
	str[len] = '0' + num;
	str[len + 1] = 0;
}

void strcpynum(uint8_t *str, uint8_t num, uint8_t numwidth, uint8_t padchar){
	str[0] = 0;
	strcatnum(str, num, numwidth, padchar);
}

