// ---------------------------------------------------------------------------------------------------------------------------------
//  ______           _   _   _   _      _ _                             
// |  ____|         | | | | | | | |    (_) |                            
// | |__    __ _ ___| |_| | | | | |_ __ _| |_  ___      ___ _ __  _ __  
// |  __|  / _` / __| __| | | | | | '__| | __|/ _ \    / __| '_ \| '_ \ 
// | |    | (_| \__ \ |_|  V _ V  | |  | | |_|  __/ _ | (__| |_) | |_) |
// |_|     \__,_|___/\__|\__/ \__/|_|  |_|\__|\___|(_) \___| .__/| .__/ 
//                                                         | |   | |    
//                                                         |_|   |_|    
//
// Description:
//
//   File writing using the FileWrite call to avoid overhead
//
// Notes:
//
//   Best viewed with 8-character tabs and (at least) 132 columns
//
// History:
//
//   12/28/2001 by Paul Nettle: Original creation
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
#include "FastWrite.h"

// ---------------------------------------------------------------------------------------------------------------------------------

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// ---------------------------------------------------------------------------------------------------------------------------------

	FastWrite::FastWrite()
	: _bytesWritten(0), _handle(INVALID_HANDLE_VALUE)
{
}

// ---------------------------------------------------------------------------------------------------------------------------------

	FastWrite::~FastWrite()
{
	close();
}

// ---------------------------------------------------------------------------------------------------------------------------------

bool	FastWrite::open(const fstl::wstring & name)
{
	// Make sure we can open a file

	if (handle() != INVALID_HANDLE_VALUE) return false;

	// Init these...

	filename() = name;
	bytesWritten() = 0;

	// Open the file

	handle() = CreateFile(filename().asArray(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_FLAG_WRITE_THROUGH|FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (handle() == INVALID_HANDLE_VALUE) return false;

	// Done

	return true;
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	FastWrite::close()
{
	bytesWritten() = 0;
	filename() = _T("");

	if (handle() != INVALID_HANDLE_VALUE)
	{
		CloseHandle(handle());
		handle() = INVALID_HANDLE_VALUE;
	}
}

// ---------------------------------------------------------------------------------------------------------------------------------

bool	FastWrite::write(void * buffer, const unsigned int count)
{
	// Make sure we have an open file

	if (handle() == INVALID_HANDLE_VALUE) return false;

	// Write some data

	DWORD	br = 0;
	if (!WriteFile(handle(), buffer, count, &br, NULL))
	{
		DWORD	er = GetLastError();

		// Okay, we got an error we don't allow, inform the caller

		fstl::wstring	err = _T("Unable to write: ") + getLastErrorString(er);
		AfxMessageBox(err.asArray());
		return false;
	}

	// Keep track of where we are...

	bytesWritten() += br;

	// Return the buffer

	return true;
}

// ---------------------------------------------------------------------------------------------------------------------------------
// FastWrite.cpp - End of file
// ---------------------------------------------------------------------------------------------------------------------------------
