// ---------------------------------------------------------------------------------------------------------------------------------
//  ______           _____            ______ _                                 
// |  ____|         |  __ \          |  ____(_)                                
// | |__   _ __ ___ | |  | | ___  ___| |__   ___   __ ___      ___ _ __  _ __  
// |  __| | '_ ` _ \| |  | |/ _ \/ _ \  __| | \ \ / // _ \    / __| '_ \| '_ \ 
// | |____| | | | | | |__| |  __/  __/ |    | |\ V /|  __/ _ | (__| |_) | |_) |
// |______|_| |_| |_|_____/ \___|\___|_|    |_| \_/  \___|(_) \___| .__/| .__/ 
//                                                                | |   | |    
//                                                                |_|   |_|    
//
// Description:
//
//   MD5 - Message-Digest Algorithm, compliant to rfc1321, with the following exceptions:
//	1) The data length must be <= to 2^32 bits in length
//	2) The sensitive data is not zero'd out
//	3) The code will only compile/run on little-endian machines (sorry mac users :)
//
// Notes:
//
//   Best viewed with 8-character tabs and (at least) 132 columns
//
// History:
//
//   12/18/2001 by Paul Nettle: Original creation
//
// ---------------------------------------------------------------------------------------------------------------------------------
// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep
//
// Copyright 2002, Fluid Studios, all rights reserved.
// ---------------------------------------------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "FSRaid.h"
#include "OverlappedRead.h"
#include "EmDeeFive.h"

// ---------------------------------------------------------------------------------------------------------------------------------

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// ---------------------------------------------------------------------------------------------------------------------------------

	EmDeeFive::EmDeeFive()
{
	start();
}

// ---------------------------------------------------------------------------------------------------------------------------------

	EmDeeFive::~EmDeeFive()
{
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	EmDeeFive::reset()
{
	started() = false;
	finished() = false;
	dataLengthBits() = 0;
	workingBufferLengthBits() = 0;
	memset(_workingBuffer, 0, sizeof(_workingBuffer));

	// From the RFC:
	//
	// 3.3 Step 3. Initialize MD Buffer
	//
	// A four-word buffer (A,B,C,D) is used to compute the message digest.
	// Here each of A, B, C, D is a 32-bit register. These registers are
	// initialized to the following values in hexadecimal, low-order bytes
	// first):
	//
	//    word A: 01 23 45 67
	//    word B: 89 ab cd ef
	//    word C: fe dc ba 98
	//    word D: 76 54 32 10

	mdBuffer()[0] = 0x67452301;
	mdBuffer()[1] = 0xefcdab89;
	mdBuffer()[2] = 0x98badcfe;
	mdBuffer()[3] = 0x10325476;
}

// ---------------------------------------------------------------------------------------------------------------------------------
// Test suite, taken directly from the RFC
// ---------------------------------------------------------------------------------------------------------------------------------

bool	EmDeeFive::testSuite()
{
	// Helper

	#define	TEST_HASH(i,o)\
	{\
		start();\
		processString(i);\
		finish();\
		fstl::wstring result;\
		if (!getHashAsString(result)) return false;\
		if (result != fstl::wstring(o)) return false;\
	}

	// These strings taken from the RFC, as well as the required results at the end

//	TEST_HASH("", "d41d8cd98f00b204e9800998ecf8427e");
//	TEST_HASH("a", "0cc175b9c0f1b6a831c399e269772661");
//	TEST_HASH("abc", "900150983cd24fb0d6963f7d28e17f72");
//	TEST_HASH("message digest", "f96b697d7cb7938d525a2f31aaf161d0");
//	TEST_HASH("abcdefghijklmnopqrstuvwxyz", "c3fcd3d76192e4007dfb496cca67e13b");
//	TEST_HASH("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", "d174ab98d277d9f5a5611c2c9f419d9f");
//	TEST_HASH("12345678901234567890123456789012345678901234567890123456789012345678901234567890", "57edf4a22be3c955ac49da2e2107b67a");

	// Test must have passed to get here...

	return true;
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	EmDeeFive::start()
{
	reset();
	started() = true;
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	EmDeeFive::finish()
{
	// Already finished?

	if (finished()) return;

	// Save the dataLength prior to padding...

	unsigned int	dataLengthBitsPriorPadding = dataLengthBits();

	// Pad the working buffer, always add a '1' bit

	unsigned char	padOneBit = 0x80;
	processBits(&padOneBit, 1);

	// Check the working buffer length, if it's greater than (512-64 = 448) bits long, process it, then 

	if (workingBufferLengthBits() > (512 - 64))
	{
		processBuffer(workingBuffer());
		memset(_workingBuffer, 0, sizeof(_workingBuffer));
		workingBufferLengthBits() = 0;
	}

	// ...pad to (512-64 = 448)

	workingBufferLengthBits() = 512 - 64;

	// Append our length (in bits) to the buffer - here, we deviate from the RFC by not allowing buffers larger than 2^32 bits
	// in length...
	//
	// Note that we only bother to send the low-order 32-bits of the length. The upper 32-bits would be zeros, which are
	// automatically part of the working buffer.

	processBits(reinterpret_cast<unsigned char *>(&dataLengthBitsPriorPadding), 32);
	processBuffer(workingBuffer());
	memset(_workingBuffer, 0, sizeof(_workingBuffer));
	workingBufferLengthBits() = 0;

	// Restore the length

	dataLengthBits() = dataLengthBitsPriorPadding;

	// We've finished the job

	finished() = true;
}

// ---------------------------------------------------------------------------------------------------------------------------------

bool	EmDeeFive::processBits(const unsigned char * data, const unsigned int bitCount)
{
	if (!started()) return false;
	if (finished()) return false;

	// Speedier byte-aligned version of this routine

	if (!(bitCount % 8) && !(workingBufferLengthBits() % 8))
	{
		return processBytes(data, bitCount / 8);
	}

	// Our source data & bit shift

	const unsigned char *	src = data;
	unsigned char		srcShift = 0;

	// Our destination data & bit shift

	unsigned char *		dst = workingBuffer() + (workingBufferLengthBits() / 8);
	unsigned char		dstShift = workingBufferLengthBits() % 8;

	// Add these bits to the working buffer

	for (unsigned int i = 0; i < bitCount; ++i)
	{
		// Copy a bit to the destination

		(*dst) |= (((*src) << srcShift) & 0x80) >> dstShift;

		// Our working buffer just grew by a bit

		workingBufferLengthBits()++;

		// Shifter & pointer for the source

		srcShift++;
		if (srcShift >= 8)
		{
			srcShift = 0;
			src++;
		}

		dstShift++;
		if (dstShift >= 8)
		{
			dstShift = 0;
			dst++;

			// Did we just fill the working buffer?

			if (workingBufferLengthBits() == BLOCK_SIZE_IN_BITS)
			{
				// Process the working buffer

				processBuffer(workingBuffer());
				memset(_workingBuffer, 0, sizeof(_workingBuffer));
				workingBufferLengthBits() = 0;

				// Reset these so we put our bits in the right place

				dst = workingBuffer();
				dstShift = 0;
			}
		}
	}

	// Keep track of our output data length

	dataLengthBits() += bitCount;

	return true;
}

// ---------------------------------------------------------------------------------------------------------------------------------

bool	EmDeeFive::processBytes(const unsigned char * data, const unsigned int byteCount)
{
	// Process the head of the data until we fill our working buffer

	unsigned int		bytesLeftToProcess = byteCount;
	const unsigned char *	dst = data;
	unsigned int	bytesProcessedSoFar = 0;

	if (workingBufferLengthBits())
	{
		unsigned int	copyCount = fstl::min(bytesLeftToProcess, BLOCK_SIZE_IN_BYTES - workingBufferLengthBytes());
		bytesLeftToProcess -= copyCount;

		// Copy them into the workingBuffer

		memcpy(workingBuffer() + workingBufferLengthBytes(), dst, copyCount);
		workingBufferLengthBits() += copyCount * 8;
		dst += copyCount;

		// If the working buffer is full, process it

		if (workingBufferLengthBits() >= BLOCK_SIZE_IN_BITS)
		{
			processBuffer(workingBuffer());
			memset(_workingBuffer, 0, sizeof(_workingBuffer));
			workingBufferLengthBits() = 0;
		}
	}

	// Process whole blocks, not bothering to go through the working buffer

	if (bytesLeftToProcess)
	{
		while (bytesLeftToProcess >= BLOCK_SIZE_IN_BYTES)
		{
			processBuffer(dst);
			dst += BLOCK_SIZE_IN_BYTES;
			bytesLeftToProcess -= BLOCK_SIZE_IN_BYTES;
		}
	}

	// Put the leftover into the working buffer

	if (bytesLeftToProcess)
	{
		// Copy them into the workingBuffer

		memcpy(workingBuffer(), dst, bytesLeftToProcess);
		workingBufferLengthBits() = bytesLeftToProcess * 8;
	}

	// Keep track of our output data length

	dataLengthBits() += byteCount * 8;

	return true;
}

// ---------------------------------------------------------------------------------------------------------------------------------

bool	EmDeeFive::processString(const fstl::wstring & str)
{
	return processBits(reinterpret_cast<const unsigned char *>(str.asArray()), str.length() * 8);
}

// ---------------------------------------------------------------------------------------------------------------------------------

const unsigned char *	EmDeeFive::getHash()
{
	// If we never started, bail

	if (!started()) return static_cast<unsigned char *>(0);

	// If we're not finished yet, then finish

	if (!finished()) finish();

	// Return the hash

	return reinterpret_cast<unsigned char *>(mdBuffer());
}

// ---------------------------------------------------------------------------------------------------------------------------------

bool	EmDeeFive::getHashAsString(fstl::wstring & result)
{
	// Get the hash bytes

	const unsigned char *	hashBytes = getHash();
	if (!hashBytes) return false;

	result = convertHashToString(hashBytes);
	return true;
}

// ---------------------------------------------------------------------------------------------------------------------------------

fstl::wstring	EmDeeFive::convertHashToString(const unsigned char fingerprint[HASH_SIZE_IN_BYTES])
{
	fstl::wstring	result;

	// Convert the bytes to a string

	for (unsigned int i = 0; i < HASH_SIZE_IN_BYTES; ++i)
	{
		TCHAR	dsp[5];
		swprintf(dsp, _T("%02x"), fingerprint[i]);
		result += dsp;
	}

	return result;
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	EmDeeFive::processBuffer(const unsigned char * buf)
{
	// Taken from the reference implementation of the RFC

	#define	F(x, y, z) ((x & y) | ((~x) & z))
	#define	G(x, y, z) ((x & z) | (y & (~z)))
	#define	H(x, y, z) (x ^ y ^ z)
	#define	I(x, y, z) (y ^ (x | (~z)))
	#define	ROTATE_LEFT(x, n) ((x << n) | (x >> (32-n)))
	#define	FF(a, b, c, d, x, s, ac) {a += F (b, c, d) + x + (unsigned int)ac; a = ROTATE_LEFT(a, s) + b; }
	#define GG(a, b, c, d, x, s, ac) {a += G (b, c, d) + x + (unsigned int)ac; a = ROTATE_LEFT(a, s) + b; }
	#define HH(a, b, c, d, x, s, ac) {a += H (b, c, d) + x + (unsigned int)ac; a = ROTATE_LEFT(a, s) + b; }
	#define II(a, b, c, d, x, s, ac) {a += I (b, c, d) + x + (unsigned int)ac; a = ROTATE_LEFT(a, s) + b; }

	unsigned int		a = mdBuffer()[0];
	unsigned int		b = mdBuffer()[1];
	unsigned int		c = mdBuffer()[2];
	unsigned int		d = mdBuffer()[3];
	const unsigned int *	x = reinterpret_cast<const unsigned int *>(buf);

	// MD5 transformation

	FF(a, b, c, d, x[ 0],  7, 0xd76aa478); // 1
	FF(d, a, b, c, x[ 1], 12, 0xe8c7b756); // 2
	FF(c, d, a, b, x[ 2], 17, 0x242070db); // 3
	FF(b, c, d, a, x[ 3], 22, 0xc1bdceee); // 4
	FF(a, b, c, d, x[ 4],  7, 0xf57c0faf); // 5
	FF(d, a, b, c, x[ 5], 12, 0x4787c62a); // 6
	FF(c, d, a, b, x[ 6], 17, 0xa8304613); // 7
	FF(b, c, d, a, x[ 7], 22, 0xfd469501); // 8
	FF(a, b, c, d, x[ 8],  7, 0x698098d8); // 9
	FF(d, a, b, c, x[ 9], 12, 0x8b44f7af); // 10
	FF(c, d, a, b, x[10], 17, 0xffff5bb1); // 11
	FF(b, c, d, a, x[11], 22, 0x895cd7be); // 12
	FF(a, b, c, d, x[12],  7, 0x6b901122); // 13
	FF(d, a, b, c, x[13], 12, 0xfd987193); // 14
	FF(c, d, a, b, x[14], 17, 0xa679438e); // 15
	FF(b, c, d, a, x[15], 22, 0x49b40821); // 16
	GG(a, b, c, d, x[ 1],  5, 0xf61e2562); // 17
	GG(d, a, b, c, x[ 6],  9, 0xc040b340); // 18
	GG(c, d, a, b, x[11], 14, 0x265e5a51); // 19
	GG(b, c, d, a, x[ 0], 20, 0xe9b6c7aa); // 20
	GG(a, b, c, d, x[ 5],  5, 0xd62f105d); // 21
	GG(d, a, b, c, x[10],  9, 0x02441453); // 22
	GG(c, d, a, b, x[15], 14, 0xd8a1e681); // 23
	GG(b, c, d, a, x[ 4], 20, 0xe7d3fbc8); // 24
	GG(a, b, c, d, x[ 9],  5, 0x21e1cde6); // 25
	GG(d, a, b, c, x[14],  9, 0xc33707d6); // 26
	GG(c, d, a, b, x[ 3], 14, 0xf4d50d87); // 27
	GG(b, c, d, a, x[ 8], 20, 0x455a14ed); // 28
	GG(a, b, c, d, x[13],  5, 0xa9e3e905); // 29
	GG(d, a, b, c, x[ 2],  9, 0xfcefa3f8); // 30
	GG(c, d, a, b, x[ 7], 14, 0x676f02d9); // 31
	GG(b, c, d, a, x[12], 20, 0x8d2a4c8a); // 32
	HH(a, b, c, d, x[ 5],  4, 0xfffa3942); // 33
	HH(d, a, b, c, x[ 8], 11, 0x8771f681); // 34
	HH(c, d, a, b, x[11], 16, 0x6d9d6122); // 35
	HH(b, c, d, a, x[14], 23, 0xfde5380c); // 36
	HH(a, b, c, d, x[ 1],  4, 0xa4beea44); // 37
	HH(d, a, b, c, x[ 4], 11, 0x4bdecfa9); // 38
	HH(c, d, a, b, x[ 7], 16, 0xf6bb4b60); // 39
	HH(b, c, d, a, x[10], 23, 0xbebfbc70); // 40
	HH(a, b, c, d, x[13],  4, 0x289b7ec6); // 41
	HH(d, a, b, c, x[ 0], 11, 0xeaa127fa); // 42
	HH(c, d, a, b, x[ 3], 16, 0xd4ef3085); // 43
	HH(b, c, d, a, x[ 6], 23, 0x04881d05); // 44
	HH(a, b, c, d, x[ 9],  4, 0xd9d4d039); // 45
	HH(d, a, b, c, x[12], 11, 0xe6db99e5); // 46
	HH(c, d, a, b, x[15], 16, 0x1fa27cf8); // 47
	HH(b, c, d, a, x[ 2], 23, 0xc4ac5665); // 48
	II(a, b, c, d, x[ 0],  6, 0xf4292244); // 49
	II(d, a, b, c, x[ 7], 10, 0x432aff97); // 50
	II(c, d, a, b, x[14], 15, 0xab9423a7); // 51
	II(b, c, d, a, x[ 5], 21, 0xfc93a039); // 52
	II(a, b, c, d, x[12],  6, 0x655b59c3); // 53
	II(d, a, b, c, x[ 3], 10, 0x8f0ccc92); // 54
	II(c, d, a, b, x[10], 15, 0xffeff47d); // 55
	II(b, c, d, a, x[ 1], 21, 0x85845dd1); // 56
	II(a, b, c, d, x[ 8],  6, 0x6fa87e4f); // 57
	II(d, a, b, c, x[15], 10, 0xfe2ce6e0); // 58
	II(c, d, a, b, x[ 6], 15, 0xa3014314); // 59
	II(b, c, d, a, x[13], 21, 0x4e0811a1); // 60
	II(a, b, c, d, x[ 4],  6, 0xf7537e82); // 61
	II(d, a, b, c, x[11], 10, 0xbd3af235); // 62
	II(c, d, a, b, x[ 2], 15, 0x2ad7d2bb); // 63
	II(b, c, d, a, x[ 9], 21, 0xeb86d391); // 64

	mdBuffer()[0] += a;
	mdBuffer()[1] += b;
	mdBuffer()[2] += c;
	mdBuffer()[3] += d;
}

// ---------------------------------------------------------------------------------------------------------------------------------

bool	EmDeeFive::processFile(const fstl::wstring & filename, unsigned char fingerprint[HASH_SIZE_IN_BYTES], const unsigned int totalFiles, const unsigned int curIndex, progressCallback callback, void * callbackData, const unsigned int startOffset, const unsigned int maxLength)
{
	// Open an overlapped file

	OverlappedRead	overlappedRead;
	if (!overlappedRead.open(filename, 0, maxLength)) return false;

	// Prime the overlapped reader

	if (!overlappedRead.startRead()) return false;

	// Start the MD5 processing

	EmDeeFive	md5;
	md5.start();

	// Our skip count

	unsigned int	skipCount = startOffset;

	// Used to show progress...

	fstl::wstring	shortName = filename;
	int		idx = shortName.rfind(_T("\\"));
	if (idx >= 0) shortName.erase(0, idx+1);
	fstl::wstring	progressMessage = _T("Validating  --  ") + shortName;
	float		minPercent = 0, percentRange = 0;
	if (totalFiles)
	{
		minPercent = static_cast<float>(curIndex) / static_cast<float>(totalFiles) * 100.0f;
		percentRange = 1.0f / static_cast<float>(totalFiles);
	}

	// Process the file

	try
	{
		for(;;)
		{
			// Don't freeze up

			allowBackgroundProcessing();

			// Inform the user

			float	percent = static_cast<float>(overlappedRead.bytesRead()) / static_cast<float>(overlappedRead.fileLength()) * percentRange * 100.0f + minPercent;
			if (callback && !callback(callbackData, progressMessage, percent)) throw false;

			// Finish the pending read

			unsigned int	readCount;
			unsigned char *	ptr = overlappedRead.finishRead(readCount);

			// Error?

			if (!ptr) throw false;

			// Done?

			if (!readCount) break;

			// Prime the next read

			if (!overlappedRead.startRead()) throw false;

			// Process the bytes read so far

			int	offset = 0;
			if (skipCount < readCount)
			{
				offset = skipCount;
				skipCount = 0;
				if (!md5.processBits(ptr + offset, (readCount - offset) * 8)) throw false;
			}
			else
			{
				skipCount -= readCount;
			}
		}

		// If we never finished, bail

		if (!overlappedRead.finishedReadingFile()) throw false;

		// Finish the MD5 processing

		md5.finish();

		// Return the hash

		memcpy(fingerprint, md5.getHash(), HASH_SIZE_IN_BYTES);
	}
	catch (const bool)
	{
		return false;
	}

	// Done

	return true;
}

// ---------------------------------------------------------------------------------------------------------------------------------
// EmDeeFive.cpp - End of file
// ---------------------------------------------------------------------------------------------------------------------------------
