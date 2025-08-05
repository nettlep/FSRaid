// ---------------------------------------------------------------------------------------------------------------------------------
//  _____        _         ______ _ _                           
// |  __ \      | |       |  ____(_) |                          
// | |  | | __ _| |_  __ _| |__   _| | ___      ___ _ __  _ __  
// | |  | |/ _` | __|/ _` |  __| | | |/ _ \    / __| '_ \| '_ \ 
// | |__| | (_| | |_| (_| | |    | | |  __/ _ | (__| |_) | |_) |
// |_____/ \__,_|\__|\__,_|_|    |_|_|\___|(_) \___| .__/| .__/ 
//                                                 | |   | |    
//                                                 |_|   |_|    
//
// Description:
//
//   Class for managing data files
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
#include "DataFile.h"

// ---------------------------------------------------------------------------------------------------------------------------------

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// ---------------------------------------------------------------------------------------------------------------------------------

	DataFile::DataFile()
	: _fileSize(0), _recoverable(true), _status(Unknown)
{
	memset(_hash,  0, sizeof(_hash));
	memset(_hashFirst16K,  0, sizeof(_hashFirst16K));
	statusString() = _T("Unknown");
}

// ---------------------------------------------------------------------------------------------------------------------------------

	DataFile::~DataFile()
{
}

// ---------------------------------------------------------------------------------------------------------------------------------

bool	DataFile::validate(unsigned char actualHash[EmDeeFive::HASH_SIZE_IN_BYTES], const unsigned int totalFiles, const unsigned int curIndex, progressCallback callback, void * callbackData)
{
	// Clear out the actual hash until we calculate a valid one

	memset(actualHash, 0, sizeof(actualHash));

	// Does the file specifically exist?

	if (!doesFileExist(filespec()))
	{
		status() = Missing;
		statusString() = _T("Missing");
		return false;
	}

	// Get the file length

	unsigned int	size = getFileLength(filespec());
	if (!size)
	{
		status() = Error;
		statusString() = _T("Cannot get file length");
		return false;
	}

	if (size < fileSize())
	{
		status() = Error;
		statusString() = _T("File is incomplete by ") + sizeString(fileSize() - size);
		return false;
	}

	if (size > fileSize())
	{
		status() = Corrupt;
		statusString() = _T("File is large by ") + sizeString(size - fileSize());
		return false;
	}

	// Get the MD5 checksum of the file

	if (!EmDeeFive::processFile(filespec(), actualHash, totalFiles, curIndex, callback, callbackData))
	{
		status() = Error;
		statusString() = _T("Unable to read the file");
		return false;
	}

	// Compare hashes

	if (memcmp(hash(), actualHash, EmDeeFive::HASH_SIZE_IN_BYTES))
	{
		status() = Corrupt;
		statusString() = _T("Corrupt");
		return false;
	}

	// File is fine, cleanup

	status() = Valid;
	statusString() = _T("Valid");
	return true;
}

// ---------------------------------------------------------------------------------------------------------------------------------

bool	DataFile::readParHeader(FILE * fp, const fstl::wstring & path)
{
	// Init the stuff we won't be reading from the file

	filePath() = path;
	status() = Unknown;
	statusString() = _T("Unknown");

	try
	{
		// Make sure we have something to read from

		if (!fp) throw _T("Invalid file pointer (internal error)");

		ParFileEntry	fileEntry;
		if (fread(&fileEntry, sizeof(fileEntry), 1, fp) != 1) throw "read failed";

		// See comment above (near similar code)

		if (fileEntry.entrySizeHigh || fileEntry.fileSizeHigh)
		{
			throw   _T("Either this is the year 3000 and hard drives are\n")
				_T("amazingly huge now, (and I'm dead) or you're trying\n")
				_T("to load an invalid PAR file because one of the values\n")
				_T("in the file header has exceeded a 32-bit value");
		}

		// Read in the filename

		TCHAR	dataFilename[MAX_PATH];
		memset(dataFilename, 0, sizeof(dataFilename));

		unsigned int	nameLength = fileEntry.entrySizeLow - 0x38;
		if (nameLength > sizeof(dataFilename)) throw _T("filename of data file too long");

		// Read the filename

		if (fread(dataFilename, nameLength, 1, fp) != 1) throw _T("read failed");

		// Setup the rest of my stuff

		fileName() = dataFilename;
		fileName() = oemToAnsi(fileName());

		memcpy(hash(), fileEntry.md5Hash, EmDeeFive::HASH_SIZE_IN_BYTES);
		memcpy(hashFirst16K(), fileEntry.md5Hash16K, EmDeeFive::HASH_SIZE_IN_BYTES);
		fileSize() = fileEntry.fileSizeLow;
		recoverable() = (fileEntry.statusFieldLow & 1) ? true:false;
	}
	catch (const TCHAR * err)
	{
		status() = Error;
		statusString() = err;
		return false;
	}

	return true;
}

// ---------------------------------------------------------------------------------------------------------------------------------

fstl::ucharArray	DataFile::storeParHeader() const
{
	fstl::wstring		oemName = ansiToOem(fileName());
	fstl::ucharArray	result;

	ParFileEntry	fileEntry;
	memset(&fileEntry, 0, sizeof(fileEntry));

	fileEntry.entrySizeLow = oemName.length() * 2 + sizeof(fileEntry);
	fileEntry.statusFieldLow = recoverable() ? 1:0;
	fileEntry.fileSizeLow = fileSize();
	memcpy(fileEntry.md5Hash, hash(), EmDeeFive::HASH_SIZE_IN_BYTES);
	memcpy(fileEntry.md5Hash16K, hashFirst16K(), EmDeeFive::HASH_SIZE_IN_BYTES);

	// Store the datafile header

	result.populate(0, sizeof(fileEntry));
	memcpy(&result[0], &fileEntry, sizeof(fileEntry));

	// Store the filename (Unicode)

	int	s = result.size();
	for (unsigned int i = 0; i < oemName.length(); ++i)
	{
		result += 0;
		result += 0;
	}
	memcpy(&result[s], &oemName[0], oemName.length() * 2);

	return result;
}

// ---------------------------------------------------------------------------------------------------------------------------------

bool	DataFile::writeParHeader(FILE * fp) const
{
	// Make sure we have something to work with

	if (!fp) return false;

	// Generate a header

	fstl::ucharArray	headerBuffer = storeParHeader();
	if (!headerBuffer.size()) throw "";

	// Write it

	if (fwrite(&headerBuffer[0], headerBuffer.size(), 1, fp) != 1) return false;

	return true;
}

// ---------------------------------------------------------------------------------------------------------------------------------

fstl::wstring	DataFile::getStatusString(const FileStatus status)
{
	switch(status)
	{
		case Valid: return _T("Valid");
		case Corrupt: return _T("Corrupt");
		case Missing: return _T("Missing");
		case Misnamed: return _T("Misnamed");
		case Error: return _T("Error");
	}
	return _T("Unknown");
}

// ---------------------------------------------------------------------------------------------------------------------------------
// DataFile.cpp - End of file
// ---------------------------------------------------------------------------------------------------------------------------------
