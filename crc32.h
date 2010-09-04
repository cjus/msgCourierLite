#ifndef _CRC32_H
#define _CRC32_H

/*
 * MACRC32 (Mark Adler, Cyclic Redundancy Check 32-Bit)
 *
 * This hash function is based on the good old CRC-32 (Cyclic Redundancy
 * Check with 32 Bit) algorithm as invented by Mark Adler. It is one
 * of the hash functions with medium performance but with very good
 * distribution. So it can be considered as a rock solid general purpose
 * hash function. It should be used if good distribution is more
 * important than high performance.
 */

class cCRC32
{
	public:
		cCRC32();
		virtual ~cCRC32();
		virtual unsigned long crc(unsigned char *pBuf, int iBuf, unsigned long lasthash=0);
	protected:
		static unsigned long m_CRCTable[256];
};

#endif //_CRC32_H
