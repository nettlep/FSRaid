// ---------------------------------------------------------------------------------------------------------------------------------
//  _____             _ _         ______ _ _                           
// |  __ \           (_) |       |  ____(_) |                          
// | |__) | __ _ _ __ _| |_ _   _| |__   _| | ___      ___ _ __  _ __  
// |  ___/ / _` | '__| | __| | | |  __| | | |/ _ \    / __| '_ \| '_ \ 
// | |    | (_| | |  | | |_| |_| | |    | | |  __/ _ | (__| |_) | |_) |
// |_|     \__,_|_|  |_|\__|\__, |_|    |_|_|\___|(_) \___| .__/| .__/ 
//                           __/ |                        | |   | |    
//                          |___/                         |_|   |_|    
//
// Description:
//
//   Class for managing parity files
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
#include "ParityFile.h"

// ---------------------------------------------------------------------------------------------------------------------------------

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// ---------------------------------------------------------------------------------------------------------------------------------

	ParityFile::ParityFile()
	: _volumeNumber(0), _dataOffset(0), _dataSize(0), _status(Unknown)
{
	memset(_hash, 0, sizeof(_hash));
	memset(_setHash, 0, sizeof(_setHash));
	statusString() = _T("Unknown");
}

// ---------------------------------------------------------------------------------------------------------------------------------

	ParityFile::~ParityFile()
{
}

// ---------------------------------------------------------------------------------------------------------------------------------

bool	ParityFile::validate(unsigned char actualHash[EmDeeFive::HASH_SIZE_IN_BYTES], const unsigned int totalFiles, const unsigned int curIndex, progressCallback callback, void * callbackData)
{
	// Clear out the actual hash until we calculate a valid one

	memset(actualHash, 0, sizeof(actualHash));

	// Does the file specifically exist?

	if (!doesFileExist(filespec()))
	{
		status() = Missing;
		statusString() = _T("File is missing");
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

	// Get the MD5 checksum of the file (starting at offset 0x20)

	if (!EmDeeFive::processFile(filespec(), actualHash, totalFiles, curIndex, callback, callbackData, 0x20))
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

	status() = Valid;
	statusString() = _T("Valid");
	return true;
}

// ---------------------------------------------------------------------------------------------------------------------------------

bool	ParityFile::readPARHeader(const fstl::wstring & path, const fstl::wstring & name, fstl::wstring & createdByString, DataFileArray & dataFiles)
{
	// Pointers (so they're visible to the catch construct)

	FILE *		fp = NULL;

	// Init the stuff we won't be reading from the file

	filePath() = path;
	fileName() = name;
	status() = Unknown;
	statusString() = _T("Unknown");

	try
	{
		// Open the file

		fp = _wfopen(filespec().asArray(), _T("rb"));
		if (!fp) throw _T("unable to open file");

		ParFileHeader	header;
		if (fread(&header, sizeof(header), 1, fp) != 1) throw _T("read failed");

		// Validate the identifier

		if (memcmp(header.identifier, "PAR\0\0\0\0\0", 8)) throw _T("identifier mismatch, possibly invalid file?");

		// Make sure all the 64-bit values have a "high" dword of zero... that is kinda pointless in the header, since the
		// header would have to contain a HUGE number of files to outgrow a friggin' 32-bit value! Sheesh, talk about overkill.

		if (header.volumeNumberHigh || header.dataSizeHigh || header.fileCountHigh || header.fileListSizeHigh || header.startOffsetDataHigh || header.startOffsetFileListHigh || header.volumeNumberHigh)
		{
			throw	_T("Either this is the year 3000 and hard drives are\n")
				_T("amazingly huge now, (and I'm dead) or you're trying\n")
				_T("to load an invalid PAR file because one of the values\n")
				_T("in the file header has exceeded a 32-bit value");
		}

		// Seek to the file entries

		if (fseek(fp, header.startOffsetFileListLow, SEEK_SET)) throw _T("seek failed");
		
		// Read the file entries

		dataFiles.erase();
		dataFiles.reserve(header.fileCountLow);
		for (unsigned int i = 0; i < header.fileCountLow; ++i)
		{
			DataFile	df;
			if (!df.readParHeader(fp, path)) throw _T("unable to read file entry");

			// Add it to the list

			dataFiles += df;
		}

		// Setup the parityFile structure

		dataOffset() = header.startOffsetDataLow;
		dataSize() = header.dataSizeLow;
		memcpy(hash(), header.controlHash, EmDeeFive::HASH_SIZE_IN_BYTES);
		memcpy(setHash(), header.setHash, EmDeeFive::HASH_SIZE_IN_BYTES);
		volumeNumber() = header.volumeNumberLow;

		// Finally, set the created-by string

		TCHAR	dsp[1024];
		swprintf(dsp, _T("v%d.%d.%d"), (header.generator >> 16) & 0xff, (header.generator >> 8) & 0xff, header.generator & 0xff);
		switch(header.generator >> 24)
		{
			case 0:
				createdByString = fstl::wstring(_T("Undefined ")) + dsp;
				break;
			case 1:
				createdByString = fstl::wstring(_T("Mirror ")) + dsp;
				break;
			case 2:
				createdByString = fstl::wstring(_T("PAR ")) + dsp;
				break;
			case 3:
				createdByString = fstl::wstring(_T("SmartPar ")) + dsp;
				break;
			case 0xff:
				createdByString = fstl::wstring(_T("FSRaid ")) + dsp;
				break;
			case 0xfe:
				createdByString = fstl::wstring(_T("Newspost ")) + dsp;
				break;
			default:
				createdByString = fstl::wstring(_T("Unknown ")) + dsp;
				break;
		}

		// Cleanup

		fclose(fp);
	}
	catch (const TCHAR * err)
	{
		status() = Error;
		statusString() = err;

		if (fp) fclose(fp);

		return false;
	}

	return true;
}

// ---------------------------------------------------------------------------------------------------------------------------------

bool	ParityFile::isFromSet(const fstl::wstring & path, const fstl::wstring & name, const unsigned char *setHash)
{
	// Pointers (so they're visible to the catch construct)

	FILE *		fp = NULL;

	try
	{
		// Build the filename

		fstl::wstring	filespec = path + _T("\\") + name;

		// Open the file

		fp = _wfopen(filespec.asArray(), _T("rb"));
		if (!fp) throw false;

		ParFileHeader	header;
		if (fread(&header, sizeof(header), 1, fp) != 1) throw false;

		// Validate the identifier

		if (memcmp(header.identifier, "PAR\0\0\0\0\0", 8)) throw false;

		// Does the hash match?

		if (memcmp(header.setHash, setHash, EmDeeFive::HASH_SIZE_IN_BYTES)) throw false;

		// Cleanup

		fclose(fp);
		fp = NULL;
	}
	catch (const bool rc)
	{
		if (fp) fclose(fp);
		return rc;
	}

	return true;
}

// ---------------------------------------------------------------------------------------------------------------------------------

fstl::ucharArray	ParityFile::storePARHeader(DataFileArray & dataFiles) const
{
	// Size of the stored data

	fstl::ucharArray	result;

	try
	{
		// Make sure we have something to work with

		if (!dataFiles.size()) throw _T("no datafiles in archive");

		// Calculate data size (size of largest recoverable data file)
		//
		// While we're at it, we can calculate the size of the file list...

		unsigned int	largestFile = 0;
		unsigned int	recoverableCount = 0;
		unsigned int	fileListSize = 0x38 * dataFiles.size(); // size of the struct minus the filename
		for (unsigned int i = 0; i < dataFiles.size(); ++i)
		{
			if (dataFiles[i].recoverable())
			{
				largestFile = fstl::max(largestFile, dataFiles[i].fileSize());
				recoverableCount++;
			}
			fileListSize += dataFiles[i].fileName().length() * 2;
		}

		// Build the header

		ParFileHeader	header;
		memset(&header, 0, sizeof(header));

		strcpy(header.identifier, "PAR");
		header.fileVersion = 0x00010000;
		header.generator = 0xff020900;  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!! VERSION NUMBER !!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		memcpy(header.controlHash, hash(), EmDeeFive::HASH_SIZE_IN_BYTES);
		memcpy(header.setHash, setHash(), EmDeeFive::HASH_SIZE_IN_BYTES);
		header.volumeNumberLow = volumeNumber();
		header.fileCountLow = dataFiles.size();
		header.startOffsetFileListLow = 0x60;
		header.fileListSizeLow = fileListSize;
		header.startOffsetDataLow = header.startOffsetFileListLow + header.fileListSizeLow;
		header.dataSizeLow = (volumeNumber()) ? largestFile:0;

		// Store the header

		result.populate(0, sizeof(header));
		memcpy(&result[0], &header, sizeof(header));

		// Store the file entries

		for (unsigned int i = 0; i < dataFiles.size(); ++i)
		{
			fstl::ucharArray	ar = dataFiles[i].storeParHeader();
			if (!ar.size()) throw _T("Unable to write file entry to PAR header");

			// Add this to the mem buffer

			result += ar;
		}
	}
	catch (const TCHAR * err)
	{
		AfxMessageBox(err);
		result.erase();
		return result;
	}

	return result;
}

// ---------------------------------------------------------------------------------------------------------------------------------

bool	ParityFile::writePARHeader(FILE * fp, DataFileArray & dataFiles) const
{
	try
	{
		// Make sure we have something to work with

		if (!fp) throw _T("invalid file pointer (internal error)");

		// Generate a header

		fstl::ucharArray	headerBuffer = storePARHeader(dataFiles);
		if (!headerBuffer.size()) throw "";

		// Write it

		if (fwrite(&headerBuffer[0], headerBuffer.size(), 1, fp) != 1) throw _T("write failed");
	}
	catch (const TCHAR * err)
	{
		if (err && *err) AfxMessageBox(err);
		return false;
	}

	return true;
}

// ---------------------------------------------------------------------------------------------------------------------------------

fstl::wstring	ParityFile::getStatusString(const FileStatus status)
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
// ParityFile.cpp - End of file
// ---------------------------------------------------------------------------------------------------------------------------------
