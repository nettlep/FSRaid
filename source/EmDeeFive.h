// ---------------------------------------------------------------------------------------------------------------------------------
//  ______           _____            ______ _                _     
// |  ____|         |  __ \          |  ____(_)              | |    
// | |__   _ __ ___ | |  | | ___  ___| |__   ___   __ ___    | |__  
// |  __| | '_ ` _ \| |  | |/ _ \/ _ \  __| | \ \ / // _ \   | '_ \ 
// | |____| | | | | | |__| |  __/  __/ |    | |\ V /|  __/ _ | | | |
// |______|_| |_| |_|_____/ \___|\___|_|    |_| \_/  \___|(_)|_| |_|
//                                                                  
//                                                                  
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

#ifndef	_H_EMDEEFIVE
#define _H_EMDEEFIVE

// ---------------------------------------------------------------------------------------------------------------------------------
// Module setup (required includes, macros, etc.)
// ---------------------------------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------------------------------

class	EmDeeFive
{
public:
	// Enumerations

		enum			{HASH_SIZE_IN_BYTES = 16};
		enum			{BLOCK_SIZE = 16};
		enum			{BLOCK_SIZE_IN_BITS = BLOCK_SIZE * 32};
		enum			{BLOCK_SIZE_IN_BYTES = BLOCK_SIZE_IN_BITS / 8};

	// Construction/Destruction

					EmDeeFive();
virtual					~EmDeeFive();

	// Operators

	// Implementation

virtual		void			reset();
virtual		bool			testSuite();
virtual		void			start();
virtual		void			finish();
virtual		bool			processBits(const unsigned char * data, const unsigned int bitCount);
virtual		bool			processString(const fstl::wstring & str);
virtual	const	unsigned char *		getHash();
virtual		bool			getHashAsString(fstl::wstring & result);
static		fstl::wstring		convertHashToString(const unsigned char fingerprint[HASH_SIZE_IN_BYTES]);
static		bool			processFile(const fstl::wstring & filename, unsigned char fingerprint[HASH_SIZE_IN_BYTES], const unsigned int totalFiles, const unsigned int curIndex, progressCallback callback = NULL, void * callbackData = NULL, const unsigned int startOffset = 0, const unsigned int maxLength = 0xffffffff);


	// Accessors

inline	const	bool			started() const				{return _started;}
inline	const	bool			finished() const			{return _finished;}
inline	const	unsigned int		dataLengthBits() const			{return _dataLengthBits;}

private:

	// Private implementation

virtual		bool			processBytes(const unsigned char * data, const unsigned int byteCount);
virtual		void			processBuffer(const unsigned char * buf);

	// Private accessors

inline		bool &			started()				{return _started;}
inline		bool &			finished()				{return _finished;}
inline		unsigned int &		dataLengthBits()			{return _dataLengthBits;}
inline		unsigned char *		workingBuffer()				{return _workingBuffer;}
inline	const	unsigned char *		workingBuffer() const			{return _workingBuffer;}
inline		unsigned int &		workingBufferLengthBits()		{return _workingBufferLengthBits;}
inline	const	unsigned int		workingBufferLengthBits() const		{return _workingBufferLengthBits;}
inline		unsigned int *		mdBuffer()				{return _mdBuffer;}
inline	const	unsigned int *		mdBuffer() const			{return _mdBuffer;}

inline	const	unsigned int		workingBufferLengthBytes() const	{return workingBufferLengthBits() / 8;}

	// Data members

		bool			_started;
		bool			_finished;
		unsigned int		_dataLengthBits;
		unsigned char		_workingBuffer[BLOCK_SIZE_IN_BYTES];
		unsigned int		_workingBufferLengthBits;
		unsigned int		_mdBuffer[4];
};

typedef	fstl::array<EmDeeFive>		EmDeeFiveArray;
typedef	fstl::array<EmDeeFive *>	EmDeeFivePointerArray;
typedef	fstl::list<EmDeeFive>		EmDeeFiveList;
typedef	fstl::list<EmDeeFive *>		EmDeeFivePointerList;

#endif // _H_EMDEEFIVE
// ---------------------------------------------------------------------------------------------------------------------------------
// EmDeeFive.h - End of file
// ---------------------------------------------------------------------------------------------------------------------------------
