/* CRC-32 (PNG / zlib polynomial) of a buffer, bit by bit. JC patch 0002: upstream's common/crc32.c fills a 1 KB table into a static array
 * (internal RAM on the target) on first use; this is called once per file (the SV8 stream header), so no table. */
unsigned long mpc_crc32(unsigned char *buf, int len)
{
	unsigned long c = 0xffffffffUL;
	int n, k;

	for (n = 0; n < len; n++) {
		c ^= buf[n];
		for (k = 0; k < 8; k++)
			c = (c & 1) ? 0xedb88320UL ^ (c >> 1) : c >> 1;
	}
	return (c ^ 0xffffffffUL) & 0xffffffffUL;
}
