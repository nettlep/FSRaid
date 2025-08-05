// ---------------------------------------------------------------------------------------------------------------------------------
//   ____                  _                            _ _____                 _     _     
//  / __ \                | |                          | |  __ \               | |   | |    
// | |  | __   __ ___ _ __| | __ _ _ __  _ __   ___  __| | |__) | ___  __ _  __| |   | |__  
// | |  | \ \ / // _ \ '__| |/ _` | '_ \| '_ \ / _ \/ _` |  _  / / _ \/ _` |/ _` |   | '_ \ 
// | |__| |\ V /|  __/ |  | | (_| | |_) | |_) |  __/ (_| | | \ \|  __/ (_| | (_| | _ | | | |
//  \____/  \_/  \___|_|  |_|\__,_| .__/| .__/ \___|\__,_|_|  \_\\___|\__,_|\__,_|(_)|_| |_|
//                                | |   | |                                                 
//                                |_|   |_|                                                 
//
// Description:
//
//   Overlapped IO (the input portion)
//
// Notes:
//
//   Best viewed with 8-character tabs and (at least) 132 columns
//
// History:
//
//   12/25/2001 by Paul Nettle: Original creation
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

#ifndef	_H_OVERLAPPEDREAD
#define _H_OVERLAPPEDREAD

// ---------------------------------------------------------------------------------------------------------------------------------
// Module setup (required includes, macros, etc.)
// ---------------------------------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------------------------------

class	OverlappedRead
{
public:
	// Enumerations

		enum			{BUFFER_SIZE = 64*1024};

	// Construction/Destruction

					OverlappedRead();
virtual					~OverlappedRead();

	// Operators

	// Implementation

virtual		bool			open(const fstl::wstring & name, const unsigned int offset = 0, const unsigned int maxLength = 0xffffffff);
virtual		void			close();
virtual		bool			startRead();
virtual		unsigned char *		finishRead(unsigned int & readCount);
virtual		bool			nonOverlappedStartRead();
virtual		unsigned char *		nonOverlappedFinishRead(unsigned int & readCount);

	// Accessors

inline		fstl::wstring &		filename()			{return _filename;}
inline	const	fstl::wstring &		filename() const		{return _filename;}
inline		unsigned char *&	buffer1()			{return _buffer1;}
inline	const	unsigned char *		buffer1() const			{return _buffer1;}
inline		unsigned char *&	buffer2()			{return _buffer2;}
inline	const	unsigned char *		buffer2() const			{return _buffer2;}
inline		bool &			alternateBuffer()		{return _alternateBuffer;}
inline	const	bool			alternateBuffer() const		{return _alternateBuffer;}
inline		bool &			bufferCleared()			{return _bufferCleared;}
inline	const	bool			bufferCleared() const		{return _bufferCleared;}
inline		bool &			supportsOverlapped()		{return _supportsOverlapped;}
inline	const	bool			supportsOverlapped() const	{return _supportsOverlapped;}
inline		unsigned int &		fileLength()			{return _fileLength;}
inline	const	unsigned int		fileLength() const		{return _fileLength;}
inline		unsigned int &		bytesRead()			{return _bytesRead;}
inline	const	unsigned int		bytesRead() const		{return _bytesRead;}
inline		unsigned int &		startOffset()			{return _startOffset;}
inline	const	unsigned int		startOffset() const		{return _startOffset;}
inline		HANDLE &		handle()			{return _handle;}
inline	const	HANDLE			handle() const			{return _handle;}
inline		OVERLAPPED &		overlapped()			{return _overlapped;}
inline	const	OVERLAPPED &		overlapped() const		{return _overlapped;}

inline		bool			finishedReadingFile() const	{return bytesRead()+startOffset() >= fileLength();}

private:
	// Explicitly disallowed calls (they appear here, because if we don't do this, the compiler will generate them for us)
		
					OverlappedRead(const OverlappedRead & rhs);
inline		OverlappedRead &	operator =(const OverlappedRead & rhs);

	// Data members

		fstl::wstring		_filename;
		unsigned char *		_buffer1;
		unsigned char *		_buffer2;
		bool			_alternateBuffer;
		bool			_bufferCleared;
		bool			_supportsOverlapped;
		unsigned int		_fileLength;
		unsigned int		_bytesRead;
		unsigned int		_startOffset;
		HANDLE			_handle;
		OVERLAPPED		_overlapped;
};

typedef	fstl::array<OverlappedRead *>	OverlappedReadPointerArray;
typedef	fstl::list<OverlappedRead *>	OverlappedReadPointerList;

#endif // _H_OVERLAPPEDREAD
// ---------------------------------------------------------------------------------------------------------------------------------
// OverlappedRead.h - End of file
// ---------------------------------------------------------------------------------------------------------------------------------
