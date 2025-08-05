// ---------------------------------------------------------------------------------------------------------------------------------
//   ____                  _                            _ _____                 _                      
//  / __ \                | |                          | |  __ \               | |                     
// | |  | __   __ ___ _ __| | __ _ _ __  _ __   ___  __| | |__) | ___  __ _  __| |     ___ _ __  _ __  
// | |  | \ \ / // _ \ '__| |/ _` | '_ \| '_ \ / _ \/ _` |  _  / / _ \/ _` |/ _` |    / __| '_ \| '_ \ 
// | |__| |\ V /|  __/ |  | | (_| | |_) | |_) |  __/ (_| | | \ \|  __/ (_| | (_| | _ | (__| |_) | |_) |
//  \____/  \_/  \___|_|  |_|\__,_| .__/| .__/ \___|\__,_|_|  \_\\___|\__,_|\__,_|(_) \___| .__/| .__/ 
//                                | |   | |                                               | |   | |    
//                                |_|   |_|                                               |_|   |_|    
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

#include "stdafx.h"
#include "FSRaid.h"
#include "OverlappedRead.h"

// ---------------------------------------------------------------------------------------------------------------------------------

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// ---------------------------------------------------------------------------------------------------------------------------------

	OverlappedRead::OverlappedRead()
	: _buffer1(static_cast<unsigned char *>(0)), _buffer2(static_cast<unsigned char *>(0)), _alternateBuffer(false),
	_bufferCleared(false), _supportsOverlapped(false), _fileLength(0), _bytesRead(0), _startOffset(0), _handle(INVALID_HANDLE_VALUE)
{
	supportsOverlapped() = (GetVersion() & 0x80000000) == 0;

	// Forcefully disable it, if the user wants it that way...

	if (theApp.GetProfileInt(_T("Options"), _T("disableOverlappingIO"), 0)) supportsOverlapped() = false;
}

// ---------------------------------------------------------------------------------------------------------------------------------

	OverlappedRead::~OverlappedRead()
{
	close();
}

// ---------------------------------------------------------------------------------------------------------------------------------

bool	OverlappedRead::open(const fstl::wstring & name, const unsigned int offset, const unsigned int maxLength)
{
	// Allocate the I/O buffers

	if (!buffer1()) buffer1() = (unsigned char *) VirtualAlloc(NULL, BUFFER_SIZE, MEM_COMMIT, PAGE_READWRITE);
	if (!buffer1())
	{
		AfxMessageBox(_T("Unable to allocate virtual RAM (1)"));
		return false;
	}

	if (supportsOverlapped())
	{
		if (!buffer2()) buffer2() = (unsigned char *) VirtualAlloc(NULL, BUFFER_SIZE, MEM_COMMIT, PAGE_READWRITE);
		if (!buffer2())
		{
			AfxMessageBox(_T("Unable to allocate virtual RAM (2)"));
			return false;
		}

		// Reset this to true, so that our first read will toggle it back to zero before the read

		alternateBuffer() = true;
	}
	else
	{
		alternateBuffer() = false;
	}

	// We haven't started padding yet

	bufferCleared() = false;

	// Make sure we can open a file

	if (handle() != INVALID_HANDLE_VALUE) return false;

	// Init these...

	filename() = name;
	bytesRead() = 0;
	startOffset() = offset;

	// Get the file length

	fileLength() = getFileLength(filename());
	if (!fileLength()) return false;

	// Limit file length

	if (fileLength() > maxLength) fileLength() = maxLength;

	// Open the file

	int	ovl = supportsOverlapped() ? FILE_FLAG_OVERLAPPED:0;
	handle() = CreateFile(filename().asArray(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING|ovl|FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (handle() == INVALID_HANDLE_VALUE) return false;

	// Clear out the overlapped struct

	memset(&overlapped(), 0, sizeof(OVERLAPPED));

	// Done

	return true;
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	OverlappedRead::close()
{
	fileLength() = 0;
	bytesRead() = 0;
	startOffset() = 0;
	filename() = _T("");

	if (handle() != INVALID_HANDLE_VALUE)
	{
		CloseHandle(handle());
		handle() = INVALID_HANDLE_VALUE;
	}

	if (buffer1())
	{
		VirtualFree(buffer1(), 0, MEM_RELEASE);
		buffer1() = static_cast<unsigned char *>(0);
	}

	if (buffer2())
	{
		VirtualFree(buffer2(), 0, MEM_RELEASE);
		buffer2() = static_cast<unsigned char *>(0);
	}
}

// ---------------------------------------------------------------------------------------------------------------------------------

bool	OverlappedRead::startRead()
{
	// "Different strokes for different folks..."

	if (!supportsOverlapped()) return nonOverlappedStartRead();

	// Make sure we have an open file

	if (handle() == INVALID_HANDLE_VALUE) return false;

	// Done?

	if (finishedReadingFile()) return true;

	// Setup an overlapped structure

	overlapped().Offset = bytesRead() + startOffset();

	// Toggle the buffer

	alternateBuffer() = !alternateBuffer();

	// Read some data

	DWORD	br;
	if (ReadFile(handle(), alternateBuffer() ? buffer2():buffer1(), BUFFER_SIZE, &br, &overlapped())) return true;

	// We do allow certain errors...

	DWORD	er = GetLastError();
	if (er == ERROR_IO_PENDING || er == ERROR_HANDLE_EOF) return true;

	// Okay, we got an error we don't allow, inform the caller

	return false;
}

// ---------------------------------------------------------------------------------------------------------------------------------

unsigned char *	OverlappedRead::finishRead(unsigned int & readCount)
{
	// "Different strokes for different folks..."

	if (!supportsOverlapped()) return nonOverlappedFinishRead(readCount);

	// Make sure we have an open file

	if (handle() == INVALID_HANDLE_VALUE) return static_cast<unsigned char *>(0);

	// Init the read count

	readCount = 0;

	// If we're completely done, just give them an empty buffer

	if (finishedReadingFile())
	{
		// If we haven't already done so, clear a buffer

		if (!bufferCleared())
		{
			memset(buffer1(), 0, BUFFER_SIZE);
			bufferCleared() = true;
		}

		return buffer1();
	}

	// Check on the results of the asynchronous read 

	DWORD	br;
	if (!GetOverlappedResult(handle(), &overlapped(), &br, TRUE)) return static_cast<unsigned char *>(0);

	// Adjust br based on a possibly limited file length

	if (br + bytesRead() + startOffset() > fileLength())
	{
		br = fileLength() - bytesRead() - startOffset();
	}

	// Keep track of where we are...

	bytesRead() += br;

	// Which buffer are we working with?

	unsigned char *	pBuf = alternateBuffer() ? buffer2():buffer1();

	// Do we need to clear out any of the leftover buffer (for padding)?

	if (finishedReadingFile())
	{
		memset(pBuf + br, 0, BUFFER_SIZE - br);
	}

	// Return the buffer

	readCount = br;
	return pBuf;
}

// ---------------------------------------------------------------------------------------------------------------------------------

bool	OverlappedRead::nonOverlappedStartRead()
{
	// Make sure we have an open file

	if (handle() == INVALID_HANDLE_VALUE) return false;

	// Move to the starting offset?

	if (!bytesRead() && startOffset())
	{
		SetFilePointer(handle(), startOffset(), NULL, FILE_BEGIN);
	}

	// That's all we do here.. there's nothing to start when it's not overlapped

	return true;
}

// ---------------------------------------------------------------------------------------------------------------------------------

unsigned char *	OverlappedRead::nonOverlappedFinishRead(unsigned int & readCount)
{
	// Make sure we have an open file

	if (handle() == INVALID_HANDLE_VALUE) return static_cast<unsigned char *>(0);

	// Init the read count

	readCount = 0;

	// If we're completely done, just give them an empty buffer

	if (finishedReadingFile())
	{
		// If we haven't already done so, clear a buffer

		if (!bufferCleared())
		{
			memset(buffer1(), 0, BUFFER_SIZE);
			bufferCleared() = true;
		}

		return buffer1();
	}

	// Read some data

	DWORD	br = 0;
	if (!ReadFile(handle(), buffer1(), BUFFER_SIZE, &br, NULL))
	{
		// We do allow certain errors...

		DWORD	er = GetLastError();
		if (er != ERROR_HANDLE_EOF)
		{
			// Okay, we got an error we don't allow, inform the caller

			return static_cast<unsigned char *>(0);
		}
	}

	// Adjust br based on a possibly limited file length

	if (br + bytesRead() + startOffset() > fileLength())
	{
		br = fileLength() - bytesRead() - startOffset();
	}

	// Keep track of where we are...

	bytesRead() += br;

	// Do we need to clear out any of the leftover buffer (for padding)?

	if (finishedReadingFile())
	{
		memset(buffer1() + br, 0, BUFFER_SIZE - br);
	}

	// Return the buffer

	readCount = br;
	return buffer1();
}

// ---------------------------------------------------------------------------------------------------------------------------------
// OverlappedRead.cpp - End of file
// ---------------------------------------------------------------------------------------------------------------------------------
