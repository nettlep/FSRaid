// ---------------------------------------------------------------------------------------------------------------------------------
//  _____             _ _         _____        __                            
// |  __ \           (_) |       |_   _|      / _|                           
// | |__) | __ _ _ __ _| |_ _   _  | |  _ __ | |_  ___       ___ _ __  _ __  
// |  ___/ / _` | '__| | __| | | | | | | '_ \|  _|/ _ \     / __| '_ \| '_ \ 
// | |    | (_| | |  | | |_| |_| |_| |_| | | | | | (_) | _ | (__| |_) | |_) |
// |_|     \__,_|_|  |_|\__|\__, |_____|_| |_|_|  \___/ (_) \___| .__/| .__/ 
//                           __/ |                              | |   | |    
//                          |___/                               |_|   |_|    
//
// Description:
//
//   Parity information container
//
// Notes:
//
//   Best viewed with 8-character tabs and (at least) 132 columns
//
// History:
//
//   12/17/2001 by Paul Nettle: Original creation
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
#include <direct.h>
#include "FSRaid.h"
#include "ParityInfo.h"
#include "EmDeeFive.h"
#include "OverlappedRead.h"
#include "FastWrite.h"

// ---------------------------------------------------------------------------------------------------------------------------------

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// ---------------------------------------------------------------------------------------------------------------------------------

	ParityInfo::ParityInfo(const unsigned int rsRaidBits)
	: _rsRaidBits(rsRaidBits), _gflog(static_cast<unsigned int *>(0)), _gfexp(static_cast<unsigned int *>(0)),
	_vandMatrix(static_cast<unsigned int *>(0)), _recoveryArrays(static_cast<unsigned int *>(0))
{
}

// ---------------------------------------------------------------------------------------------------------------------------------

	ParityInfo::~ParityInfo()
{
	reset();
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	ParityInfo::reset()
{
	defaultPath().erase();
	defaultBaseName().erase();

	dataFiles().erase();
	parityFiles().erase();

	delete[] gflog();
	gflog() = static_cast<unsigned int *>(0);

	delete[] gfexp();
	gfexp() = static_cast<unsigned int *>(0);

	delete[] vandMatrix();
	vandMatrix() = static_cast<unsigned int *>(0);

	delete[] recoveryArrays();
	recoveryArrays() = static_cast<unsigned int *>(0);
}

// ---------------------------------------------------------------------------------------------------------------------------------

bool	ParityInfo::validateDataFile(const unsigned int index, const unsigned int totalFiles, const unsigned int curIndex, progressCallback callback, void * callbackData)
{
	// Make sure we have a valid operation

	if (index >= dataFiles().size())
	{
		AfxMessageBox(_T("Cannot validate data file (index out of range)"));
		return false;
	}

	// Validate it

	return validateDataFile(dataFiles()[index], totalFiles, curIndex, callback, callbackData);
}

// ---------------------------------------------------------------------------------------------------------------------------------

bool	ParityInfo::validateDataFile(DataFile & df, const unsigned int totalFiles, const unsigned int curIndex, progressCallback callback, void * callbackData) const
{
	unsigned char	actualHash[EmDeeFive::HASH_SIZE_IN_BYTES];
	if (!df.validate(actualHash, totalFiles, curIndex, callback, callbackData))
	{
		// Scan the list of files to see if this checksum matches another file in the set

		for (unsigned int i = 0; i < dataFiles().size(); ++i)
		{
			if (!memcmp(dataFiles()[i].hash(), actualHash, EmDeeFive::HASH_SIZE_IN_BYTES))
			{
				df.status() = DataFile::Misnamed;
				df.statusString() = _T("File is misnamed, should be ") + dataFiles()[i].fileName();
				return false;
			}
		}
	}

	return true;
}

// ---------------------------------------------------------------------------------------------------------------------------------

bool	ParityInfo::loadParFile(fstl::wstring & filename)
{
	try
	{
		// Reset the parity info

		reset();
		
		// Set the default path

		defaultPath() = filename;
		defaultBaseName() = filename;
		int	idx = defaultPath().rfind(_T("\\"));
		if (idx >= 0)
		{
			defaultPath().erase(idx);
			defaultBaseName().erase(0, idx+1);
		}
		else
		{
			TCHAR	buf[2048];
			memset(buf, 0, sizeof(buf));
			_wgetcwd(buf, sizeof(buf)-1);
			defaultPath() = buf;
		}

		// Read the header

		ParityFile	parityFile;
		{
			DataFileArray	dfa;
			static	fstl::wstring	headerErr;
			if (!parityFile.readPARHeader(defaultPath(), defaultBaseName(), createdByString(), dfa)) throw _T("Unable to read file header");
			if (!validateParFile(parityFile, 1, 0))
			{
				static	fstl::wstring	err = _T("Cannot validate file.\n\nThe file status is reported to be:\n\n") + parityFile.statusString();
				throw err.asArray();
			}
			dataFiles() = dfa;
		}

		// Save the set hash

		memcpy(setHash(), parityFile.setHash(), EmDeeFive::HASH_SIZE_IN_BYTES);

		// Strip the extension off of the base name, so we have JUST the base name

		idx = defaultBaseName().rfind(_T("."));
		if (idx >= 0)
		{
			defaultBaseName().erase(idx);
		}

		// Find the par files

		if (!findParFiles(parityFiles())) throw _T("Unable to scan the directory for PAR files");

		// Avoid checking the par file that we've just loaded...

		for (unsigned int i = 0; i < parityFiles().size(); ++i)
		{
			ParityFile &	pf = parityFiles()[i];
			if (!memcmp(pf.hash(), parityFile.hash(), EmDeeFive::HASH_SIZE_IN_BYTES))
			{
				pf.status() = ParityFile::FileStatus::Valid;
				pf.statusString() = _T("Valid");
				break;
			}
		}
	}
	catch (const TCHAR * err)
	{
		// Make sure we're reset

		reset();

		fstl::wstring	msg = fstl::wstring(_T("An error has occurred while trying to read the PAR file:\n\n")) + err;
		AfxMessageBox(msg.asArray());
		return false;
	}

	// Bits are always 8 for PAR files

	rsRaidBits() = 8;

	return true;
}

// ---------------------------------------------------------------------------------------------------------------------------------

bool	ParityInfo::validateParFile(const unsigned int index, const unsigned int totalFiles, const unsigned int curIndex, progressCallback callback, void * callbackData)
{
	// Make sure we have a valid operation

	if (index >= parityFiles().size())
	{
		AfxMessageBox(_T("Cannot validate parity file (index out of range)"));
		return false;
	}

	// Validate it

	return validateParFile(parityFiles()[index], totalFiles, curIndex, callback, callbackData);
}

// ---------------------------------------------------------------------------------------------------------------------------------

bool	ParityInfo::validateParFile(ParityFile & pf, const unsigned int totalFiles, const unsigned int curIndex, progressCallback callback, void * callbackData) const
{
	unsigned char	actualHash[EmDeeFive::HASH_SIZE_IN_BYTES];
	return pf.validate(actualHash, totalFiles, curIndex, callback, callbackData);
}

// ---------------------------------------------------------------------------------------------------------------------------------

bool	ParityInfo::findParFiles(ParityFileArray & pfa) const
{
	// Start clean

	pfa.erase();

	// Filespec

	_wfinddata_t	fd;
	fstl::wstring	filespec = defaultPath() + _T("\\") + defaultBaseName() + _T(".*");
	intptr_t	handle = 0;

	// Start scanning files

	try
	{
		handle = _wfindfirst((TCHAR *) filespec.asArray(), &fd);
		if (handle <= 0) throw _T("Unable to get directory listing for Pxx file scan");

		do
		{
			// Get the extension...

			fstl::wstring	ext = fd.name;
			int	idx = ext.rfind(_T("."));

			if (idx == -1) continue;
			ext.erase(0, idx+1);

			if (ext.length() != 3) continue;
			if (towlower(ext[0]) != _T('p') && towlower(ext[0]) != _T('q')) continue;
			if (towlower(ext[1]) != _T('a') && !iswdigit(ext[1])) continue;
			if (towlower(ext[2]) != _T('r') && !iswdigit(ext[2])) continue;

			// Does it match the set hash?

			if (!ParityFile::isFromSet(defaultPath(), fd.name, setHash())) continue;

			// We have a match, try to load it

			ParityFile	thisParityFile;
			DataFileArray	dfa;
			fstl::wstring	creatorString;
			if (!thisParityFile.readPARHeader(defaultPath(), fd.name, creatorString, dfa)) continue;

			// Add this parity file to the set

			pfa += thisParityFile;
		} while (_wfindnext(handle, &fd) == 0);

		// Sort the parity files

		pfa.sort();
		pfa.unique();

		// Cleanup

		_findclose(handle);
	}
	catch (const TCHAR *)
	{
		if (handle > 0) _findclose(handle);
		return false;
	}

	return true;
}

// ---------------------------------------------------------------------------------------------------------------------------------

bool	ParityInfo::genParFiles(unsigned char parSetHash[EmDeeFive::HASH_SIZE_IN_BYTES], ParityFileArray & parityVolumes, DataFileArray & dataVolumes, progressCallback callback, void * callbackData)
{
	EmDeeFiveArray			inputHashes;
	EmDeeFiveArray			inputHashes16k;

	fstl::array<unsigned char *>	outputBuffers;
	FastWriteArray			outputFiles;

	FILE *				fp = NULL;

	// Clear this out

	memset(parSetHash, 0, sizeof(parSetHash));

	// Count the recoverable files

	unsigned int	recoverableCount = 0;
	unsigned int	largestInputFile = 0;
	__int64		totalInputData = 0;
	for (unsigned int i = 0; i < dataVolumes.size(); ++i)
	{
		totalInputData += dataVolumes[i].fileSize();

		if (dataVolumes[i].recoverable())
		{
			recoverableCount++;
			largestInputFile = fstl::max(largestInputFile, dataVolumes[i].fileSize());
		}
	}

	// Total output data (used by the progress bar)

	__int64		totalOutputData = largestInputFile * (parityVolumes.size()-1);

	try
	{
		// Determine how much memory to use for each output buffer

		MEMORYSTATUS	memStat;
		GlobalMemoryStatus(&memStat);

		double	memPercentage = static_cast<double>(theApp.GetProfileInt(_T("Options"), _T("memoryPercent"), 10)) / 100;
		if (memPercentage < 0) memPercentage = 0;
		if (memPercentage > 1) memPercentage = 1;
		double	memToUse = static_cast<double>(memStat.dwTotalPhys) * memPercentage;

		// Our memToUse contains the total memory (for all buffers), we now need to know how much per buffer
		// (remember, we don't allocate RAM for the PAR file.)

		unsigned int	memToUsePerBuffer = 1;
		if (parityVolumes.size() > 1) memToUsePerBuffer = static_cast<unsigned int>(memToUse / (parityVolumes.size() - 1));
		if (largestInputFile && memToUsePerBuffer > largestInputFile) memToUsePerBuffer = largestInputFile;

		if (memToUsePerBuffer % OverlappedRead::BUFFER_SIZE)
		{
			memToUsePerBuffer /= OverlappedRead::BUFFER_SIZE;
			memToUsePerBuffer += 1;
			memToUsePerBuffer *= OverlappedRead::BUFFER_SIZE;
		}

		// Make sure we have a valid operation

		if (!parityVolumes.size()) throw _T("No parity files to be generated");
		if (parityVolumes.size() > recoverableCount + 1) throw _T("Cannot create more parity files than recoverable data files");
		if (recoverableCount + parityVolumes.size() >= static_cast<unsigned int>(1 << rsRaidBits())) throw _T("Parity and data files may not total a value greater than 2^bit_depth");
		if (recoverableCount && !largestInputFile) throw _T("No file size to any of the data files?!");

		// Generate the GF tables

		if (!genGaloisFieldTables()) throw _T("unable to generate Galois Field tables");

		// Setup the MxN Vandermonde matrix where M is the number of parity devices, and N is the number of data devices

		if (!genVandermondeMatrix(recoverableCount, parityVolumes.size()-1)) throw _T("unable to generate Vandermonde matrix");

		// This is necessary, since the FastWrites can't be copied around

		outputFiles.reserve(parityVolumes.size());

		// Setup the output buffers, files & hashes

		for (unsigned int i = 0; i < parityVolumes.size(); ++i)
		{
			// Setup the fastwrite file

			FastWrite	fw;
			outputFiles += fw;
			if (!outputFiles[i].open(parityVolumes[i].filespec())) throw _T("Unable to open/create output file");

			// Generate a header

			fstl::ucharArray	fileHeader = parityVolumes[i].storePARHeader(dataVolumes);
			if (!fileHeader.size()) throw _T("Unable to generate PAR file header");

			// Write the header's placeholder...

			outputFiles[i].write(&fileHeader[0], fileHeader.size());

			// Allocate our output buffers (only for the pxx files, not the actual PAR file, as it has no data)

			if (i)
			{
				unsigned char *	ptr = new unsigned char[memToUsePerBuffer];
				if (!ptr) throw _T("Cannot allocate output buffer");
				outputBuffers += ptr;
			}
			else
			{
				// Insert a NULL placeholder for the PAR file

				outputBuffers += static_cast<unsigned char *>(0);
			}
		}

		// Setup the input hashes

		for (unsigned int i = 0; i < dataVolumes.size(); ++i)
		{
			// Create an input file hash

			EmDeeFive	md5;
			md5.start();

			inputHashes += md5;
			inputHashes16k += md5;
		}

		// Read in a block (group of chunks)

		__int64		totalInputDataRead = 0;
		__int64		totalOutputDataWritten = 0;
		unsigned int	groupOffset = 0;
		while(totalInputDataRead < totalInputData)
		{
			// Clear out the output buffer for this group

			for (unsigned int i = 0; i < outputBuffers.size(); ++i)
			{
				if (outputBuffers[i]) memset(outputBuffers[i], 0, memToUsePerBuffer);
			}

			// Visit each D (data device)

			unsigned int	currentRecoverableFile = 0;
			for (unsigned int j = 0; j < dataVolumes.size(); ++j)
			{
				// Prime the buffer

				if (groupOffset < dataVolumes[j].fileSize())
				{
					OverlappedRead	or;
					if (!or.open(dataVolumes[j].filespec(), groupOffset)) throw _T("Unable to open data file");
					if (!or.startRead()) throw _T("Unable to read data file");

					// We don't process the entire input file, we only process so many blocks of data...

					unsigned int	blocksPerChunk = memToUsePerBuffer / OverlappedRead::BUFFER_SIZE;

					// Process a chunk of this input file

					while(blocksPerChunk--)
					{
						double	percent = static_cast<double>(totalInputDataRead+totalOutputDataWritten) / static_cast<double>(totalInputData+totalOutputData) * 100.0f;
						if (callback && !callback(callbackData, _T("Generating parity data..."), static_cast<float>(percent))) throw _T("Operation cancelled");

						// Get some data

						unsigned int	oldBytesRead = or.bytesRead();
						unsigned int	readCount;
						unsigned char *	readBuffer = or.finishRead(readCount);
						if (!readBuffer) throw _T("Unable to read");
						if (!readCount) break;

						// Track the data read

						totalInputDataRead += readCount;

						// Only prime the next read if we've got another block to read...

						if (blocksPerChunk && !or.startRead()) throw _T("Unable to prime the reader for data file");

						// Hash is block

						if (!inputHashes[j].processBits(readBuffer, readCount * 8)) throw false;

						// Hash the first 16K of the input file

						if (!groupOffset && oldBytesRead < 16*1024)
						{
							unsigned int	hashCount = readCount;
							if (hashCount > 16*1024 - oldBytesRead) hashCount = 16*1024 - oldBytesRead;
							if (!inputHashes16k[j].processBits(readBuffer, hashCount * 8)) throw false;
						}

						// Generate parity data for recoverable files

						if (dataVolumes[j].recoverable())
						{
							// Munge it with the PAR data (this contains an optimized gfADD and gfMUL merged right into this loop for speed)

							for (unsigned int i = 1; i < outputBuffers.size(); ++i)
							{
								unsigned int	matrixValue = vandMatrix()[currentRecoverableFile + ((i-1)*recoverableCount)];
								if (!matrixValue) continue;

								unsigned char tab[0x100];
								make_lut(tab, matrixValue);

								unsigned char *	dst = outputBuffers[i] + oldBytesRead;
								unsigned char *	src = readBuffer;

								for (unsigned int n = 0; n < readCount; ++n, ++src, ++dst)
								{
									(*dst) ^= tab[*src];
								}
							}
						}
					}
				}

				// Track the recoverable files processed

				if (dataVolumes[j].recoverable())
				{
					++currentRecoverableFile;
				}
			}

			// Write the output buffers

			if (totalOutputDataWritten < totalOutputData)
			{
				for (unsigned int i = 1; i < outputBuffers.size(); ++i)
				{
					// How many bytes to process?

					unsigned int	bytes = memToUsePerBuffer;
					if (groupOffset + bytes > largestInputFile) bytes = largestInputFile - groupOffset;

					// Write the data out in chunks, so we have a smooth progress bar

					for (unsigned int k = 0; k < bytes; k += OverlappedRead::BUFFER_SIZE)
					{
						double	percent = static_cast<double>(totalInputDataRead+totalOutputDataWritten) / static_cast<double>(totalInputData+totalOutputData) * 100.0f;
						if (callback && !callback(callbackData, _T("Writing parity data..."), static_cast<float>(percent))) throw _T("Operation cancelled");

						__int64		b = OverlappedRead::BUFFER_SIZE;
						if (k + b > bytes) b = bytes - k;
						if (!outputFiles[i].write(outputBuffers[i] + k, static_cast<unsigned int>(b))) throw _T("Unable to write PAR file header");

						totalOutputDataWritten += b;
					}
				}
			}

			// Next group

			groupOffset += memToUsePerBuffer;
		}

		// Finish the input data hashes and calculate the set hash

		EmDeeFive	setHash;
		{
			setHash.start();
			for (unsigned int i = 0; i < dataVolumes.size(); ++i)
			{
				// Finish the input file hash

				inputHashes[i].finish();
				const unsigned char *	inputHash = inputHashes[i].getHash();
				if (!inputHash) throw _T("Unable to retrive data file hash");
				memcpy(dataVolumes[i].hash(), inputHash, EmDeeFive::HASH_SIZE_IN_BYTES);

				// Finish the input file's 16K hash

				inputHashes16k[i].finish();
				const unsigned char *	inputHash16k = inputHashes16k[i].getHash();
				if (!inputHash16k) throw _T("Unable to retrive data file hash (16k)");
				memcpy(dataVolumes[i].hashFirst16K(), inputHash16k, EmDeeFive::HASH_SIZE_IN_BYTES);

				// Add recoverable files to the set hash

				if (dataVolumes[i].recoverable())
				{
					if (!setHash.processBits(inputHash, EmDeeFive::HASH_SIZE_IN_BYTES * 8)) throw _T("Unable to calculate set hash");
				}

			}
			setHash.finish();
		}

		const unsigned char *	setHashPointer = setHash.getHash();
		if (!setHashPointer) throw _T("Unable to retrieve set hash pointer");

		// Save the set hash

		memcpy(parSetHash, setHashPointer, EmDeeFive::HASH_SIZE_IN_BYTES);

		// Set the percent bar to zero

		if (callback && !callback(callbackData, _T("Fingerprinting PAR file headers..."), 0)) throw _T("Operation cancelled");

		// Dump the output buffers

		unsigned int	totalParData = (parityVolumes.size() - 1) * largestInputFile;
		unsigned int	totalParDataWritten = 0;

		for (unsigned int i = 0; i < parityVolumes.size(); ++i)
		{
			// Close the output file

			outputFiles[i].close();

			// Cleanup the output buffer

			delete[] outputBuffers[i];
			outputBuffers[i] = 0;

			// Copy the set hash into the volume

			memcpy(parityVolumes[i].setHash(), setHashPointer, EmDeeFive::HASH_SIZE_IN_BYTES);

			// Generate a new header that contains all the proper data file hashes
			{
				fstl::ucharArray	fileHeader = parityVolumes[i].storePARHeader(dataVolumes);
				if (!fileHeader.size()) throw _T("Unable to generate PAR file header");

				// Write the new header

				fp = _wfopen(parityVolumes[i].filespec().asArray(), _T("r+b"));
				if (!fp) throw _T("Unable to open output file for header write");
				if (fwrite(&fileHeader[0], fileHeader.size(), 1, fp) != 1) throw _T("Unable to write PAR file header");
				fclose(fp);
				fp = NULL;
			}

			// Checksum the par file

			if (!EmDeeFive::processFile(parityVolumes[i].filespec(), parityVolumes[i].hash(), parityVolumes.size(), i, callback, callbackData, 0x20)) throw _T("Unable to fingerprint PAR file");

			// Generate a final header, with the entire file's fingerprint
			{
				fstl::ucharArray	fileHeader = parityVolumes[i].storePARHeader(dataVolumes);
				if (!fileHeader.size()) throw _T("Unable to generate PAR file header");

				// Write the new header

				fp = _wfopen(parityVolumes[i].filespec().asArray(), _T("r+b"));
				if (!fp) throw _T("Unable to open output file for header write");
				if (fwrite(&fileHeader[0], fileHeader.size(), 1, fp) != 1) throw _T("Unable to write PAR file header");
				fclose(fp);
				fp = NULL;
			}
		}
	}
	catch (const TCHAR * err)
	{
		// Close any open files

		if (fp) fclose(fp);

		// Cleanup the output buffers

		for (unsigned int i = 0; i < outputBuffers.size(); ++i)
		{
			delete[] outputBuffers[i];
		}

		// Error exit

		fstl::wstring	msg = fstl::wstring(_T("Unable to generate parity archive: \n\n")) + err;
		AfxMessageBox(msg.asArray());
		return false;
	}

	return true;
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	ParityInfo::make_lut(unsigned char lut[0x100], const int m) const
{
        for (int j = 0x100; --j; )
	{
                lut[j] = gfexp()[gflog()[m] + gflog()[j]];
	}
        lut[0] = 0;
}

// ---------------------------------------------------------------------------------------------------------------------------------

bool	ParityInfo::recoverFiles(ParityFileArray & inParityVolumes, DataFileArray & dataVolumes, progressCallback callback, void * callbackData, const int repairSingleIndex)
{
	ParityFileArray			parityVolumes = inParityVolumes;

	fstl::array<unsigned char *>	outputBuffers;
	FastWriteArray			outputFiles;

	try
	{
		// Count the recoverable files and the files needed to be recovered

		unsigned int	recoverableCount = 0;
		unsigned int	validCount = 0;
		unsigned int	corruptCount = 0;
		unsigned int	largestInputFile = 0;
		__int64		totalInputData = 0;
		__int64		totalOutputData = 0;
		unsigned int	parityCount = 0;
		int		adjustedRepairSingleIndex = -1;
		fstl::boolArray	dataFileValidityFlags;

		for (unsigned int i = 0; i < dataVolumes.size(); ++i)
		{
			DataFile &	df = dataVolumes[i];

			if (!df.recoverable()) continue;

			recoverableCount++;

			if (df.status() == DataFile::Unknown)	throw _T("All files need to be checked before recovery");
			if (df.status() == DataFile::Valid)
			{
				dataFileValidityFlags += true;
				validCount++;
				totalInputData += df.fileSize();
			}
			else
			{
				dataFileValidityFlags += false;
				corruptCount++;
				if (df.recoverable() && (repairSingleIndex == i || repairSingleIndex == -1)) totalOutputData += df.fileSize();
			}

			// Adjust the repair single index to skip over the non-recoverable files

			if (repairSingleIndex == i) adjustedRepairSingleIndex = recoverableCount - 1;

			largestInputFile = fstl::max(largestInputFile, df.fileSize());
		}

		// Generate a list of valid parity volume IDs

		fstl::intArray	parityIDs;
		for (unsigned int i = 0; i < parityVolumes.size(); ++i)
		{
			if (parityVolumes[i].volumeNumber() == 0) continue;
			if (parityVolumes[i].status() != ParityFile::Valid) continue;
			if (parityIDs.size() < corruptCount) totalInputData += largestInputFile;
			parityIDs += parityVolumes[i].volumeNumber();
		}

		// Make sure we have a valid operation

		if (!corruptCount) throw _T("There are no files to recover");
		if (parityIDs.size() < corruptCount) throw _T("You do not have enough parity files to recover the set");
		if (parityIDs.size() > recoverableCount) throw _T("Cannot have more parity files than recoverable data files");
		if (recoverableCount + parityIDs.size() >= static_cast<unsigned int>(1 << rsRaidBits())) throw _T("Parity and data files may not total a value greater than 2^bit_depth");

		// Generate the GF tables

		if (!genGaloisFieldTables()) throw _T("unable to generate Galois Field tables");

		// Generate the recovery array

		bool	setUnrecoverable;
		bool	rc = analyzeRecoverable(dataFileValidityFlags, parityIDs, parityVolumes, corruptCount, setUnrecoverable);
		if (!rc && !setUnrecoverable) throw _T("Unable to generate recovery matrix");
		if (!rc && setUnrecoverable) throw _T("");

//		if (!genRecoveryMultipliers(dataFileValidityFlags, parityIDs)) throw _T("Unable to generate recovery matrix");

		// Determine how much memory to use for each output buffer

		MEMORYSTATUS	memStat;
		GlobalMemoryStatus(&memStat);

		double	memPercentage = static_cast<double>(theApp.GetProfileInt(_T("Options"), _T("memoryPercent"), 10)) / 100;
		if (memPercentage < 0) memPercentage = 0;
		if (memPercentage > 1) memPercentage = 1;
		double	memToUse = static_cast<double>(memStat.dwTotalPhys) * memPercentage;

		// Our memToUse contains the total memory (for all buffers), we now need to know how much per buffer
		// (remember, we don't allocate RAM for the PAR file.)

		unsigned int	memToUsePerBuffer = static_cast<unsigned int>(memToUse / corruptCount);
		if (memToUsePerBuffer > largestInputFile) memToUsePerBuffer = largestInputFile;

		if (memToUsePerBuffer % OverlappedRead::BUFFER_SIZE)
		{
			memToUsePerBuffer /= OverlappedRead::BUFFER_SIZE;
			memToUsePerBuffer += 1;
			memToUsePerBuffer *= OverlappedRead::BUFFER_SIZE;
		}

		// Setup the output buffers

		for (unsigned int i = 0; i < dataFileValidityFlags.size(); ++i)
		{
			if (dataFileValidityFlags[i] == true) continue;

			// Allocate our output buffers

			unsigned char *	ptr = NULL;
			
			if (adjustedRepairSingleIndex == -1 || adjustedRepairSingleIndex == i)
			{
				ptr = new unsigned char[memToUsePerBuffer];
				if (!ptr) throw _T("Cannot allocate output buffer");

			}
			outputBuffers += ptr;
		}

		// Output files
		{
			// We need to do this, because the FastWrite doesn't like being moved around

			outputFiles.reserve(dataVolumes.size());

			unsigned int	outputIndex = 0;
			for (unsigned int i = 0; i < dataVolumes.size(); ++i)
			{
				DataFile &	df = dataVolumes[i];
				if (df.status() == DataFile::Valid) continue;
				if (!df.recoverable()) continue;

				// Only process those files we're supposed to bother repairing

				FastWrite	fw;
				outputFiles += fw;

				if (outputBuffers[outputIndex])
				{
					if (!outputFiles[outputIndex].open(df.filespec())) throw _T("Unable to open output file for write");
				}

				outputIndex++;
			}
		}

		// Visit the valid files first

		__int64		totalInputDataRead = 0;
		__int64		totalOutputDataWritten = 0;
		unsigned int	groupOffset = 0;
		while(totalInputDataRead < totalInputData)
		{
			// Clear out the output buffer for this group

			for (unsigned int i = 0; i < outputBuffers.size(); ++i)
			{
				if (outputBuffers[i]) memset(outputBuffers[i], 0, memToUsePerBuffer);
			}

			unsigned int	totalVolumesUsed = 0;
			for (unsigned int j = 0; j < dataVolumes.size(); ++j)
			{
				DataFile &	df = dataVolumes[j];
				if (!df.recoverable()) continue;
				if (df.status() != DataFile::Valid) continue;

				if (groupOffset < df.fileSize())
				{
					// Prime the buffer

					OverlappedRead	or;
	                                if (!or.open(df.filespec(), groupOffset)) throw _T("Unable to open data file");
					if (!or.startRead()) throw _T("Unable to read data file");

					// We don't process the entire input file, we only process so many blocks of data...

					unsigned int	blocksPerChunk = memToUsePerBuffer / OverlappedRead::BUFFER_SIZE;

					// Process a chunk of this input file

					while(blocksPerChunk--)
					{
						// Keep the user informed

						double	percent = static_cast<double>(totalInputDataRead+totalOutputDataWritten) / static_cast<double>(totalInputData+totalOutputData) * 100.0f;
						if (callback && !callback(callbackData, _T("Recovering data files..."), static_cast<float>(percent))) throw _T("Operation cancelled");

						// Get some data

						unsigned int	oldBytesRead = or.bytesRead();
						unsigned int	readCount;
						unsigned char *	readBuffer = or.finishRead(readCount);
						if (!readBuffer) throw _T("Unable to read");
						if (!readCount) break;

						// Track our progress

						totalInputDataRead += readCount;

						// Prime the next read

						if (!or.startRead()) throw _T("Unable to prime the reader for data file");

						// Generate the data for recoverable files

						for (unsigned int i = 0; i < outputBuffers.size(); ++i)
						{
							// Skip those files we're not supposed to bother repairing

							if (!outputBuffers[i]) continue;

							unsigned int	mplier = recoveryArrays()[totalVolumesUsed + (i*recoverableCount)];
							if (!mplier) continue;

							unsigned char tab[0x100];
							make_lut(tab, mplier);

							unsigned char *	dst = outputBuffers[i] + oldBytesRead;
							unsigned char *	src = readBuffer;

							for (unsigned int n = 0; n < readCount; ++n, ++src, ++dst)
							{
								(*dst) ^= tab[*src];
							}
						}
					}
				}

				++totalVolumesUsed;
			}

			// Visit the parity files next

			unsigned int	parityVolumesUsed = 0;
			for (unsigned int j = 0; parityVolumesUsed < corruptCount && j < parityVolumes.size(); ++j)
			{
				ParityFile &	pf = parityVolumes[j];
				if (!pf.volumeNumber()) continue;
				if (pf.status() != ParityFile::Valid) continue;

				// We skip the PAR header when restoring files, so how big is that header?

				unsigned int	headerSize = 0;
				FILE *	fp = _wfopen(pf.filespec().asArray(), _T("rb"));
				if (!fp) throw _T("Unable to get header size for parity file");
				fseek(fp, 0x50, SEEK_SET);
				fread(&headerSize, 4, 1, fp);
				fclose(fp);

				// Prime the buffer

				OverlappedRead	or;
				if (!or.open(pf.filespec(), groupOffset)) throw _T("Unable to open parity file");

				if (groupOffset < largestInputFile)
				{
					if (!or.startRead()) throw _T("Unable to read parity file");

					// We don't process the entire input file, we only process so many blocks of data...

					unsigned int	bytesProcessed = 0;

					// Process a chunk of this input file

					while(bytesProcessed < memToUsePerBuffer)
					{
						// Keep the user informed

						float	percent = static_cast<float>(totalInputDataRead+totalOutputDataWritten) / static_cast<float>(totalInputData+totalOutputData) * 100.0f;
						if (callback && !callback(callbackData, _T("Recovering data files..."), percent)) throw _T("Operation cancelled");

						// Get some data

						unsigned int	oldBytesRead = or.bytesRead();
						unsigned int	readCount;
						unsigned char *	readBuffer = or.finishRead(readCount);
						if (!readBuffer) throw _T("Unable to read");
						if (!readCount) break;

						// Skip the header?

						if (!oldBytesRead)
						{
							readCount -= headerSize;
							readBuffer += headerSize;
						}

						// Make sure we don't overflow our buffer

						if (bytesProcessed + readCount > memToUsePerBuffer)
						{
							readCount = memToUsePerBuffer - bytesProcessed;
						}

						// Track our progress

						totalInputDataRead += readCount;

						// Prime the next read

						if (!or.startRead()) throw _T("Unable to prime the reader for parity file");

						// Generate the data for recoverable files

						for (unsigned int i = 0; i < outputBuffers.size(); ++i)
						{
							// Skip those files we're not supposed to bother repairing

							if (!outputBuffers[i]) continue;

							unsigned int	mplier = recoveryArrays()[totalVolumesUsed + (i*recoverableCount)];
							if (!mplier) continue;

							unsigned char tab[0x100];
							make_lut(tab, mplier);

							unsigned char *	dst = outputBuffers[i] + bytesProcessed;
							unsigned char *	src = readBuffer;

							for (unsigned int n = 0; n < readCount; ++n, ++src, ++dst)
							{
								(*dst) ^= tab[*src];
							}
						}

						bytesProcessed += readCount;
					}
				}

				++totalVolumesUsed;
				++parityVolumesUsed;
			}

			// Write the output files

			unsigned int	outputIndex = 0;
			for (unsigned int i = 0; i < dataVolumes.size(); ++i)
			{
				DataFile &	df = dataVolumes[i];
				if (df.status() == DataFile::Valid) continue;
				if (!df.recoverable()) continue;

				// Only process those files we're supposed to bother repairing

				unsigned char *	buffer = outputBuffers[outputIndex];

				if (buffer && groupOffset < df.fileSize())
				{
					// Get the file

					FastWrite &	fw = outputFiles[outputIndex];

					// How many bytes to process?

					unsigned int	bytes = memToUsePerBuffer;
					if (groupOffset + bytes > df.fileSize()) bytes = df.fileSize() - groupOffset;

					for (unsigned int k = 0; k < bytes; k += OverlappedRead::BUFFER_SIZE)
					{
						// Keep the user informed

						float	percent = static_cast<float>(totalInputDataRead+totalOutputDataWritten) / static_cast<float>(totalInputData+totalOutputData) * 100.0f;
						if (callback && !callback(callbackData, _T("Writing recovered data files..."), percent)) throw _T("Operation cancelled");

						// How many bytes to write?

						unsigned int	b = OverlappedRead::BUFFER_SIZE;
						if (k + b > bytes) b = bytes - k;
						if (!fw.write(buffer + k, b)) throw _T("Unable to write recovered data");

						totalOutputDataWritten += b;
					}
				}

				outputIndex++;
			}

			// Next group

			groupOffset += memToUsePerBuffer;
		}

		// Update the datafiles

		unsigned int	outputIndex = 0;
		for (unsigned int i = 0; i < dataVolumes.size(); ++i)
		{
			DataFile &	df = dataVolumes[i];
			if (df.status() == DataFile::Valid) continue;
			if (!df.recoverable()) continue;

			// Only process those files we're supposed to bother repairing

			unsigned char *	buffer = outputBuffers[outputIndex];

			if (buffer)
			{
				// Mark the file as unknown

				df.status() = DataFile::Unknown;

				// Free its buffer

				delete[] buffer;
				outputBuffers[outputIndex] = 0;
			}

			outputIndex++;
		}
	}
	catch (const TCHAR * err)
	{
		// Cleanup the output buffers

		for (unsigned int i = 0; i < outputBuffers.size(); ++i)
		{
			delete[] outputBuffers[i];
		}

		// Error exit

		if (err && wcslen(err))
		{
			fstl::wstring	msg = fstl::wstring(_T("Unable to restore data files: \n\n")) + err;
			AfxMessageBox(msg.asArray());
		}
		return false;
	}

	return true;
}

// ---------------------------------------------------------------------------------------------------------------------------------

unsigned int	ParityInfo::gfADD(const unsigned int a, const unsigned int b) const
{
	return a ^ b;
}

// ---------------------------------------------------------------------------------------------------------------------------------

unsigned int	ParityInfo::gfSUB(const unsigned int a, const unsigned int b) const
{
	return gfADD(a,b);
}

// ---------------------------------------------------------------------------------------------------------------------------------

unsigned int	ParityInfo::gfMUL(const unsigned int a, const unsigned int b) const
{
	if (!a || !b) return 0;

	unsigned int	largestWordValue = (1 << rsRaidBits()) - 1;
	unsigned int	val = gflog()[a] + gflog()[b];
	if (val > largestWordValue) val -= largestWordValue;
	return gfexp()[val];
}

// ---------------------------------------------------------------------------------------------------------------------------------

unsigned int	ParityInfo::gfDIV(const unsigned int a, const unsigned int b) const
{
	if (!a || !b) return 0;

	unsigned int	largestWordValue = (1 << rsRaidBits()) - 1;
	int val = gflog()[a] - gflog()[b];
	if (val < 0) val += largestWordValue;
	return  gfexp()[val];
}

// ---------------------------------------------------------------------------------------------------------------------------------

unsigned int	ParityInfo::gfPOW(const unsigned int a, const unsigned int b) const
{
	if (!a) return 0;

	unsigned int	largestWordValue = (1 << rsRaidBits()) - 1;
	int val = (gflog()[a] * b) % 255;
	return  gfexp()[val];
}

// ---------------------------------------------------------------------------------------------------------------------------------

bool	ParityInfo::genGaloisFieldTables()
{
	// Compute the Galois Fields log & inverse log tables

	unsigned int	x_to_w = 1 << rsRaidBits();
	unsigned int	wordLimit = x_to_w - 1;
	unsigned int	q_x;
	switch	(rsRaidBits())
	{
		case 4:
			q_x = (1<< 4) + (2) + 1;
			break;
		case 8:
			q_x = (1<< 8) + (1<<4) + (1<<3) + (1<<2) + 1;
			break;
		case 16:
			q_x = (1<<16) + (1<<12) + (1<<3) + (2) + 1;
			break;
		case 32:
			q_x = (1<<32) + (1<<22) + (1<<2) + (2) + 1;
			break;
		case 64:
			q_x = (1<<64) + (1<<4) + (1<<3) + (2) + 1;
			break;
		default:
			AfxMessageBox(_T("Invalid bit depth, must be 8, 16 or 32"));
			return false;
	}

	// Allocate the tables

	delete[] gflog();
	gflog() = new unsigned int[x_to_w];
	memset(gflog(), 0, x_to_w * sizeof(unsigned int));
	if (!gflog()) return false;

	delete[] gfexp();
	gfexp() = new unsigned int[x_to_w*2];
	memset(gfexp(), 0, x_to_w * sizeof(unsigned int));
	if (!gfexp()) return false;

	for (unsigned int log = 0, bin = 1; log < wordLimit; ++log)
	{
		gflog()[bin] = log;
		gfexp()[log] = bin;
		gfexp()[log+wordLimit] = bin;

		bin <<= 1;
		if (bin > wordLimit) bin ^= q_x;
	}
	gfexp()[wordLimit] = gfexp()[0];

	return true;
}

// ---------------------------------------------------------------------------------------------------------------------------------

bool	ParityInfo::genVandermondeMatrix(const unsigned int dataFileCount, const unsigned int parityFileCount)
{
	// For convenience...

	unsigned int	nData = dataFileCount;
	unsigned int	mParity = parityFileCount;

	// Allocate the matrix

	delete[] vandMatrix();
	vandMatrix() = new unsigned int[mParity * nData];
	if (!vandMatrix()) return false;

	// Build the matrix (n across, m down)

//	TRACE("Dump of Vandermonde matrix:\n");
	unsigned int *	ptr = vandMatrix();

	for (unsigned int m = 0; m < mParity; ++m)
	{
//		TRACE("   [ ");

		for (unsigned int n = 0; n < nData; ++n, ++ptr)
		{
			*ptr = gfPOW(n+1, m);
//			TRACE("%03u ", *ptr);
		}

//		TRACE("]\n");
	}
//	TRACE("\n");

	return true;
}

// ---------------------------------------------------------------------------------------------------------------------------------

bool	ParityInfo::genRecoveryMultipliers(const fstl::boolArray & dataFileValidityFlags, const fstl::intArray & parityIDs, bool & setUnrecoverable)
{
	// Until we formally determine otherwise, this set is recoverable

	setUnrecoverable = false;

	// Count the valid files and corrupt files

	unsigned int	corruptCount = 0;
	unsigned int	validCount = 0;
	for (unsigned int i = 0; i < dataFileValidityFlags.size(); ++i)
	{
		if (dataFileValidityFlags[i] == false) corruptCount++;
		else if (dataFileValidityFlags[i] == true) validCount++;
		else return false;
	}

	// Total files to work with

	unsigned int	totalCount = corruptCount + validCount;

	// Allocate the working arrays (our initial working array set is twice as wide as our final result will be)

	unsigned int *	workingArrays = new unsigned int[totalCount * 2 * corruptCount];
	if (!workingArrays) return false;

	// Populate the working arrays

	unsigned int *	dst = workingArrays;
	for (unsigned int y = 0; y < corruptCount; ++y, dst += totalCount * 2)
	{
		unsigned int	validOccupied = 0;
		unsigned int	corruptOccupied = 0;

		for (unsigned int x = 0; x < totalCount; ++x)
		{
			unsigned int	pid = parityIDs[y] - 1;
			unsigned int	val = gfPOW(x+1, pid);

			// Valid or corrupt?

			if (dataFileValidityFlags[x] == true)
			{
				dst[x] = 0;
				dst[totalCount + validOccupied] = val;
				++validOccupied;
			}
			else
			{
				dst[x] = val;
				if (corruptOccupied == y)	dst[totalCount + validCount + corruptOccupied] = 1;
				else				dst[totalCount + validCount + corruptOccupied] = 0;
				++corruptOccupied;
			}
		}
	}

	// Dump it
	#if 0
	{
		TRACE("Dump of working multiplier arrays:\n");
		unsigned int *	dst = workingArrays;
		for (unsigned int y = 0; y < corruptCount; ++y)
		{
			TRACE("[ ");
			for (unsigned int x = 0; x < totalCount*2; ++x, dst++)
			{
				TRACE("%02x ", *dst);
				if (x == totalCount-1) TRACE("| ");
			}
			TRACE("]\n");
		}
		TRACE("\n");
	}
	#endif

	// For each valid device

	int	orderList[256];
	for (unsigned int i = 0; i < corruptCount; ++i)
	{
		// We'll be multiplying and dividing rows of multipliers by a column, we need to find that column

		int	column = 0;
		{
			unsigned int *	src = workingArrays + i * totalCount * 2;
			for (unsigned int j = 0; j < totalCount; ++j)
			{
				if (src[j])
				{
					column = j;
					break;
				}
			}
		}

		// Remember the order of the rows...

		orderList[i] = 0;
		for (int j = 0; j < column; ++j)
		{
			if (!dataFileValidityFlags[j]) orderList[i]++;
		}

#if 0
		TRACE("Column: %d (reordered to: %d)\n", column, orderList[i]);
#endif

		// Current row gets divided, the rest are multiplied

		unsigned int *	src = workingArrays + i * totalCount * 2;
		unsigned int	scalar = src[column];
		for (unsigned int x = 0; x < totalCount*2; ++x)
		{
			src[x] = gfDIV(src[x], scalar);
		}

		// Scale the other rows and then subtract the current row from them

		for (unsigned int y = 0; y < corruptCount; ++y)
		{
			// Skip the current row

			if (y == i) continue;

			// Current row's multiplier

			unsigned int *	dst = workingArrays + y * totalCount * 2;
			unsigned int	scalar = workingArrays[y * totalCount * 2 + column];

			for (unsigned int x = 0; x < totalCount*2; ++x)
			{
				dst[x] = gfSUB(dst[x], gfMUL(src[x], scalar));
			}
		}

		// Dump it
		#if 0
		{
			TRACE("Dump of working multiplier arrays (pass %d):\n", i+1);
			unsigned int *	dst = workingArrays;
			for (unsigned int y = 0; y < corruptCount; ++y)
			{
				TRACE("[ ");
				for (unsigned int x = 0; x < totalCount*2; ++x, dst++)
				{
					TRACE("%02x ", *dst);
					if (x == totalCount-1) TRACE("| ");
				}
				TRACE("]\n");
			}
			TRACE("\n");
		}
		#endif
	}

	// Allocate our resulting array

	delete[] recoveryArrays();
	recoveryArrays() = new unsigned int[totalCount * corruptCount];
	if (!recoveryArrays()) return false;

	// Simplify the double-sized array into the result array
	{
		unsigned int *	src = workingArrays;
		unsigned int *	dst = recoveryArrays();
		for (unsigned int y = 0; y < corruptCount; ++y, src += totalCount * 2)
		{
			// Copy this row to the multiplier array

			for (unsigned int x = 0; x < totalCount; ++x)
			{
				dst[orderList[y]*totalCount+x] = src[x+totalCount];
			}
		}

		// Dump the final result
		#if 0
		{
			TRACE("Dump of resulting multiplier arrays:\n");
			unsigned int *	ptr = recoveryArrays();
			for (unsigned int y = 0; y < corruptCount; ++y)
			{
				TRACE("[ ");
				for (unsigned int x = 0; x < totalCount; ++x, ptr++)
				{
					TRACE("%02x ", *ptr);
				}
				TRACE("]\n");
			}
			TRACE("\n");
		}
		#endif

		// Scan the array for any rows of zeros (i.e. an unrecoverable situation)
		{
			for (unsigned int y = 0; y < corruptCount; ++y)
			{
				unsigned int *	ptr = recoveryArrays() + y*totalCount;
				bool		recoverable = false;
				for (unsigned int x = 0; x < totalCount && !recoverable; ++x, ptr++)
				{
					if (*ptr != 0) recoverable = true;
				}

				if (!recoverable)
				{
					setUnrecoverable = true;
					break;
				}
			}
		}
	}

	delete[] workingArrays;

	// Fail if we've determined that the set will be unrecoverable

	if (setUnrecoverable) return false;

	// All's well!

	return true;
}

// ---------------------------------------------------------------------------------------------------------------------------------

bool	ParityInfo::analyzeRecoverable(const fstl::boolArray & dataFileValidityFlags, fstl::intArray & parityIDs, ParityFileArray & parityVolumes, const unsigned int corruptCount, bool & setUnrecoverable)
{
	// There are some [rare] situations where data recovery is not possible. However, if there are extra PARs available, we can
	// determine a different combination of PARs that might yield a recoverable situation.

	// We use an exhaustive search of par-file combinations. We do this by defining a pattern and manipulating it...

	fstl::intArray	pattern;
	pattern.reserve(corruptCount);
	for (unsigned int i = 0; i < corruptCount; ++i)
	{
		pattern += i;
	}

	setUnrecoverable = true;

	while(setUnrecoverable == true)
	{
		// Dump the pattern
		#if 0
		{
			fstl::wstring	temp;
			for (unsigned int i = 0; i < parityIDs.size(); ++i)
			{
				temp +=_T(" ");
			}

			for (unsigned int i = 0; i < corruptCount; ++i)
			{
				temp[pattern[i]] = pattern[i] + _T('0');
			}

			TRACE(_T("%s\n"), temp.asArray());
		}
		#endif

		// Build the parityIDs from the pattern

		fstl::intArray	newParityIDs;
		newParityIDs.reserve(corruptCount);
		for (unsigned int i = 0; i < corruptCount; ++i)
		{
			newParityIDs += parityIDs[pattern[i]];
		}

		// Build a new list of parity volumes

		ParityFileArray	newParityVolumes;
		newParityVolumes.reserve(corruptCount);
		for (unsigned int i = 0; i < corruptCount; ++i)
		{
			// Find the matching parity volume

			bool	found = false;
			for (unsigned int j = 0; j < parityVolumes.size(); ++j)
			{
				if (parityVolumes[j].volumeNumber() == newParityIDs[i])
				{
					newParityVolumes += parityVolumes[j];
					found = true;
					break;
				}
			}

			if (!found) break;
		}

		// Make sure we found all the volumes we needed

		if (newParityVolumes.size() == corruptCount)
		{
			bool	rc = genRecoveryMultipliers(dataFileValidityFlags, newParityIDs, setUnrecoverable);

			// If we have an error other than an unrecoverable scenario, bail

			if (rc == false && setUnrecoverable == false) return false;

			// If everything is peachy, return success

			if (rc == true)
			{
				parityIDs = newParityIDs;
				parityVolumes = newParityVolumes;
				return true;
			}
		}

		// Not peachy, try another combination (i.e. advance through our exhaustive-search pattern)

		bool	advanced = false;
		for (int i = corruptCount - 1; !advanced && i >= 0; --i)
		{
			// Try to advance this digit

			// Final digit advances up to the limit of parityIDs.size()...

			if (i == corruptCount - 1 && pattern[i] < static_cast<int>(parityIDs.size()) - 1)
			{
				++pattern[i];
				advanced = true;
			}

			// Other digits advance up to the limit set by the digit following them...

			else if (i < static_cast<int>(corruptCount) - 1)
			{
				if (pattern[i] < pattern[i+1] - 1)
				{
					++pattern[i];

					// Every time we advance a secondary digit, we need to make the following digits sequential

					for (unsigned int j = i+1; j < corruptCount; ++j)
					{
						pattern[j] = pattern[j-1] + 1;
					}

					advanced = true;
				}
			}
		}

		// Done searching for a workable pattern?

		if (!advanced) break;
	}

	// ********************************** NEW MESSGE *****************************

	if (setUnrecoverable)
	{
		AfxMessageBox(	_T("You need at least one more PAR file or data file to recover this set. Please\n")
				_T("continue reading, for instructions on how you might still be able to recover\n")
				_T("your data files...\n")
				_T("\n")
				_T("There is a rare situation when a specific combination of corrupt data files\n")
				_T("along with a specific combination of valid PAR files cannot be recovered. The\n")
				_T("actual combination of files changes from dataset to dataset, and most datasets\n")
				_T("aren't affected. You happen to have found a dataset that is affected, and you\n")
				_T("also happened to have found the specific combination of data files and PAR\n")
				_T("files to run into this problem. Lucky you! You should play the lottery! :)\n")
				_T("\n")
				_T("Given the chance, FSRaid will do an exhaustive search (i.e. it will try all\n")
				_T("possible combinations) to find a working combination of PAR files from the ones\n")
				_T("that are available. However, if you only have as many valid PAR files as you have\n")
				_T("corrupt data files, then there's only one possible combination.\n")
				_T("\n")
				_T("So, by adding another valid PAR file (or a valid data file) to the set, you give\n")
				_T("FSRaid the chance to try more combinations. One extra file should do the trick,\n")
				_T("but if you end up back at this message again, try adding yet another valid PAR\n")
				_T("or data file.\n")
				_T("\n")
				_T("Unfortunately, if you are unable to add another valid datafile or PAR file,\n")
				_T("then the dataset cannot, mathematically, be recovered.\n")
				_T("\n")
				_T("If you are using datafiles with error correction in them (for example, RAR\n")
				_T("files which include recovery records), try recovering as many of the data\n")
				_T("files as you can, and then try to use the PAR files to recover the rest.\n"));
		return false;
	}

	return false;
}

// ---------------------------------------------------------------------------------------------------------------------------------
// ParityInfo.cpp - End of file
// ---------------------------------------------------------------------------------------------------------------------------------
