#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "faux/conv.h"


int testc_faux_conv_atoull(void)
{
	int retval = 0;
	unsigned long long val = 0;

	// Negative integer
	if (faux_conv_atoull("-123", &val, 0)) {
		printf("The negative integer is converted to unsigned value\n");
		retval = -1;
	}

	// Unsigned integer
	if (!faux_conv_atoull("  234123", &val, 0)) {
		printf("Can't convert legal unsigned integer\n");
		retval = -1;
	}
	if (val != 234123) {
		printf("Illegal unsigned integer conversion\n");
		retval = -1;
	}

	// Hex integer
	if (!faux_conv_atoull(" 0x234123", &val, 0)) {
		printf("Can't convert legal hex integer\n");
		retval = -1;
	}
	if (val != 0x234123) {
		printf("Illegal hex integer conversion\n");
		retval = -1;
	}

	return retval;
}

