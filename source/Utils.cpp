// ---------------------------------------------------------------------------------------------------------------------------------
//  _    _ _   _ _                          
// | |  | | | (_) |                         
// | |  | | |_ _| |___      ___ _ __  _ __  
// | |  | | __| | / __|    / __| '_ \| '_ \ 
// | |__| | |_| | \__ \ _ | (__| |_) | |_) |
//  \____/ \__|_|_|___/(_) \___| .__/| .__/ 
//                             | |   | |    
//                             |_|   |_|    
//
// Description:
//
//   General utilitarian functions
//
// Notes:
//
//   Best viewed with 8-character tabs and (at least) 132 columns
//
// History:
//
//   12/22/2001 by Paul Nettle: Original creation
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
#include "Utils.h"

// ---------------------------------------------------------------------------------------------------------------------------------

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// ---------------------------------------------------------------------------------------------------------------------------------

#define	MAX_ALLOWED 50

// ---------------------------------------------------------------------------------------------------------------------------------

unsigned int	getFileLength(const fstl::wstring & filename)
{
	CFileStatus	status;
	if (!CFile::GetStatus(filename.asArray(), status)) return 0;
	return static_cast<unsigned int>(status.m_size);

// Commented out on 6/21/2002 because wstat was failing on some strange UDF drivers...
//
//	struct _stat	st;
//	if(_wstat(filename.asArray(), &st)) return 0;
//	return st.st_size;
}

// ---------------------------------------------------------------------------------------------------------------------------------

bool	isDirectory(const fstl::wstring & filename)
{
	CFileStatus	status;
	if (!CFile::GetStatus(filename.asArray(), status)) return false;
	return (status.m_attribute & 0x10) ? true:false;

// Commented out on 6/21/2002 because wstat was failing on some strange UDF drivers...
//
//	struct _stat	st;
//	if(_wstat(filename.asArray(), &st)) return false;
//	return (st.st_mode & _S_IFDIR) ? true:false;
}

// ---------------------------------------------------------------------------------------------------------------------------------

bool	doesFileExist(const fstl::wstring & filename)
{
	return _waccess(filename.asArray(), 0) == 0;
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	allowBackgroundProcessing()
{
	for (int i = 0; i < 10; i++)
	{
		MSG	msg;

		if (!PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) break;

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

// ---------------------------------------------------------------------------------------------------------------------------------

fstl::wstring	sizeString(const unsigned int s)
{
	float	fs = static_cast<float>(s);

	TCHAR	dsp[90];
	if (s >= 1024*1024*1024)
	{
		swprintf(dsp, _T("%.2fGB"), fs / (1024*1024*1024));
	}
	else if (s >= 1024*1024)
	{
		swprintf(dsp, _T("%.2fMB"), fs / (1024*1024));
	}
	else if (s >= 1024)
	{
		swprintf(dsp, _T("%.2fKB"), fs / (1024));
	}
	else
	{
		swprintf(dsp, _T("%d bytes"), s);
	}

	return fstl::wstring(dsp);
}

// ---------------------------------------------------------------------------------------------------------------------------------

fstl::wstring	getLastErrorString(const DWORD err)
{
	LPVOID	lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL);
	fstl::wstring	result(reinterpret_cast<TCHAR *>(lpMsgBuf));
	LocalFree( lpMsgBuf );
	return result;
}

// ---------------------------------------------------------------------------------------------------------------------------------

RegInfoArray	getRegInfo(unsigned int & maxAllowed)
{
	// Max size of the data

	maxAllowed = theApp.GetProfileInt(_T("Archive States"), _T("maxAllowed"), MAX_ALLOWED);
	unsigned int	totalUsed = theApp.GetProfileInt(_T("Archive States"), _T("totalused"), 0);
	if (totalUsed > maxAllowed) totalUsed = maxAllowed;

	// The information will go here...

	RegInfoArray	ria;
	ria.reserve(maxAllowed);

	// Get the block of data from the registry

	LPBYTE		data;
	unsigned int	bytes;
	if (theApp.GetProfileBinary(_T("Archive States"), _T("data"), &data, &bytes))
	{
		// Parse the data into the RegInfo array

		unsigned int *	ptr = reinterpret_cast<unsigned int *>(data);

		for(unsigned int i = 0; i < totalUsed; ++i)
		{
			RegInfo	ri;

			ri.lastAccessed = *(ptr++);

			ri.hashCount = *(ptr++);
			ri.hash.reserve(ri.hashCount);

			ri.dataCount = *(ptr++);
			ri.data.reserve(ri.dataCount);

			ri.parityCount = *(ptr++);
			ri.parity.reserve(ri.parityCount);

			unsigned char *	cptr = reinterpret_cast<unsigned char *>(ptr);

			for (unsigned int i = 0; i < ri.hashCount; ++i)
			{
				ri.hash += *(cptr++);
			}
			for (unsigned int i = 0; i < ri.dataCount; ++i)
			{
				ri.data += *(cptr++);
			}
			for (unsigned int i = 0; i < ri.parityCount; ++i)
			{
				ri.parity += *(cptr++);
			}

			// Add it to the list

			ria += ri;

			ptr = reinterpret_cast<unsigned int *>(cptr);
		}

		// Free it

		delete[] data;
	}

	return ria;
}

// ---------------------------------------------------------------------------------------------------------------------------------

bool	putRegInfo(const RegInfoArray & ria, const unsigned int maxAllowed)
{
	// Build the output data buffer (we need to update it, so we always know the last accessed time)

	fstl::charArray	outputData;

	// Calc reserved size...

	unsigned int	reserveSize = 0;
	for (unsigned int i = 0; i < ria.size(); ++i)
	{
		reserveSize += 16;
		reserveSize += 16;

		const RegInfo &	ri = ria[i];
		reserveSize += ri.hash.size();
		reserveSize += ri.data.size();
		reserveSize += ri.parity.size();
	}

	outputData.reserve(reserveSize);

	for (unsigned int i = 0; i < ria.size(); ++i)
	{
		const RegInfo &	ri = ria[i];
		outputData += (ri.lastAccessed >>  0) & 0xff;
		outputData += (ri.lastAccessed >>  8) & 0xff;
		outputData += (ri.lastAccessed >> 16) & 0xff;
		outputData += (ri.lastAccessed >> 24) & 0xff;

		outputData += (ri.hashCount >>  0) & 0xff;
		outputData += (ri.hashCount >>  8) & 0xff;
		outputData += (ri.hashCount >> 16) & 0xff;
		outputData += (ri.hashCount >> 24) & 0xff;

		outputData += (ri.dataCount >>  0) & 0xff;
		outputData += (ri.dataCount >>  8) & 0xff;
		outputData += (ri.dataCount >> 16) & 0xff;
		outputData += (ri.dataCount >> 24) & 0xff;

		outputData += (ri.parityCount >>  0) & 0xff;
		outputData += (ri.parityCount >>  8) & 0xff;
		outputData += (ri.parityCount >> 16) & 0xff;
		outputData += (ri.parityCount >> 24) & 0xff;

		outputData += ri.hash;
		outputData += ri.data;
		outputData += ri.parity;
	}

	// Write the suckers out

	theApp.WriteProfileInt(_T("Archive States"), _T("maxAllowed"), maxAllowed);
	theApp.WriteProfileInt(_T("Archive States"), _T("totalUsed"), ria.size());
	theApp.WriteProfileBinary(_T("Archive States"), _T("data"), reinterpret_cast<LPBYTE>(&outputData[0]), outputData.size());

	return true;
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	wipeRegInfo()
{
	// Write the suckers out

	theApp.WriteProfileInt(_T("Archive States"), _T("maxAllowed"), MAX_ALLOWED);
	theApp.WriteProfileInt(_T("Archive States"), _T("totalUsed"), 0);
	theApp.WriteProfileBinary(_T("Archive States"), _T("data"), reinterpret_cast<LPBYTE>("\0"), 1);
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	stripUnicodeToAscii(fstl::wstring & str)
{
	for (unsigned int i = 0; i < str.length(); ++i)
	{
		str[i] &= 0xff;
	}
}

// ---------------------------------------------------------------------------------------------------------------------------------

fstl::wstring oemToAnsi(const fstl::wstring & oemString)
{
	// Convert the filename to multibyte

	char	mbBuffer[MAX_PATH * 2];
	WideCharToMultiByte(CP_OEMCP, 0, oemString.asArray(), -1, mbBuffer, sizeof(mbBuffer), NULL, NULL);

	// Convert from multibyte to Wide (ansi)

	TCHAR	ansiBuffer[MAX_PATH];
	MultiByteToWideChar(CP_ACP, 0, mbBuffer, -1, ansiBuffer, sizeof(ansiBuffer));

	fstl::wstring	ansiString = ansiBuffer;
	return ansiString;
}

// ---------------------------------------------------------------------------------------------------------------------------------

fstl::wstring ansiToOem(const fstl::wstring & ansiString)
{
	// Convert the filename to multibyte

	char	mbBuffer[MAX_PATH * 2];
	WideCharToMultiByte(CP_ACP, 0, ansiString.asArray(), -1, mbBuffer, sizeof(mbBuffer), NULL, NULL);

	// Convert from multibyte to Wide (oem)

	TCHAR	oemBuffer[MAX_PATH];
	MultiByteToWideChar(CP_OEMCP, 0, mbBuffer, -1, oemBuffer, sizeof(oemBuffer));

	fstl::wstring	oemString = oemBuffer;
	return oemString;
}

// ---------------------------------------------------------------------------------------------------------------------------------
// Utils.cpp - End of file
// ---------------------------------------------------------------------------------------------------------------------------------
