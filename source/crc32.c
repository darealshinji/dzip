#include <inttypes.h>

uint32_t crcval;
uint32_t crctable[256];

uint32_t crc_reflect(uint32_t x, int bits)
{
	int i;
	uint32_t v = 0, b = 1 << (bits - 1);

	for (i = 0; i < bits; ++i)
	{
		if (x & 1) v += b;
		x >>= 1; b >>= 1;
	}
	return v;
}

void crc_init(void)
{
	uint32_t crcpol = 0x04C11DB7; /* CRC-32 bit mask */
	uint32_t i, j, k;

	for (i = 0; i < 256; ++i)
	{
		k = crc_reflect(i,8) << 24;
		for (j = 0; j < 8; ++j)
			k = (k << 1) ^ ((k & 0x80000000)? crcpol : 0);
		crctable[i] = crc_reflect(k,32);
	}
}

void make_crc (unsigned char *ptr, int len)
{
	while (len--)
		crcval = (crcval >> 8) ^ crctable[(crcval & 0x000000FF) ^ *ptr++];
}
