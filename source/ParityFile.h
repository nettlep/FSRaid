// ---------------------------------------------------------------------------------------------------------------------------------
//  _____             _ _         ______ _ _          _     
// |  __ \           (_) |       |  ____(_) |        | |    
// | |__) | __ _ _ __ _| |_ _   _| |__   _| | ___    | |__  
// |  ___/ / _` | '__| | __| | | |  __| | | |/ _ \   | '_ \ 
// | |    | (_| | |  | | |_| |_| | |    | | |  __/ _ | | | |
// |_|     \__,_|_|  |_|\__|\__, |_|    |_|_|\___|(_)|_| |_|
//                           __/ |                          
//                          |___/                           
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

#ifndef	_H_PARITYFILE
#define _H_PARITYFILE

// ---------------------------------------------------------------------------------------------------------------------------------
// Module setup (required includes, macros, etc.)
// ---------------------------------------------------------------------------------------------------------------------------------

#include "EmDeeFive.h"
#include "DataFile.h"

// ---------------------------------------------------------------------------------------------------------------------------------

class	ParityFile
{
public:
	// Enumerations

		enum			FileStatus {Unknown, Valid, Corrupt, Missing, Misnamed, Error};

	// Types

		#pragma pack(1)
		typedef	struct	tag_par_file_header
		{
			char		identifier[8];
			unsigned int	fileVersion;
			unsigned int	generator;
			unsigned char	controlHash[16];
			unsigned char	setHash[16];
			unsigned int	volumeNumberLow;
			unsigned int	volumeNumberHigh;
			unsigned int	fileCountLow;
			unsigned int	fileCountHigh;
			unsigned int	startOffsetFileListLow;
			unsigned int	startOffsetFileListHigh;
			unsigned int	fileListSizeLow;
			unsigned int	fileListSizeHigh;
			unsigned int	startOffsetDataLow;
			unsigned int	startOffsetDataHigh;
			unsigned int	dataSizeLow;
			unsigned int	dataSizeHigh;
		} ParFileHeader;
		#pragma pack()

	// Construction/Destruction

					ParityFile();
virtual					~ParityFile();

	// Operators

		bool			operator==(const ParityFile & rhs) {return volumeNumber() == rhs.volumeNumber();}
		bool			operator<(const ParityFile & rhs) {return volumeNumber() < rhs.volumeNumber();}
		bool			operator>(const ParityFile & rhs) {return volumeNumber() > rhs.volumeNumber();}

	// Implementation

virtual		bool			validate(unsigned char actualHash[EmDeeFive::HASH_SIZE_IN_BYTES], const unsigned int totalFiles, const unsigned int curIndex, progressCallback callback = NULL, void * callbackData = NULL);
virtual		bool			readPARHeader(const fstl::wstring & path, const fstl::wstring & name, fstl::wstring & createdByString, DataFileArray & dataFiles);
static		bool			isFromSet(const fstl::wstring & path, const fstl::wstring & name, const unsigned char *setHash);
virtual		fstl::ucharArray	storePARHeader(DataFileArray & dataFiles) const;
virtual		bool			writePARHeader(FILE * fp, DataFileArray & dataFiles) const;
static		fstl::wstring		getStatusString(const FileStatus status);

	// Accessors

inline		fstl::wstring &		fileName()		{return _fileName;}
inline	const	fstl::wstring &		fileName() const	{return _fileName;}
inline		fstl::wstring &		filePath()		{return _filePath;}
inline	const	fstl::wstring &		filePath() const	{return _filePath;}
inline		unsigned char *		hash()			{return _hash;}
inline	const	unsigned char *		hash() const		{return _hash;}
inline		unsigned char *		setHash()		{return _setHash;}
inline	const	unsigned char *		setHash() const		{return _setHash;}
inline		int &			volumeNumber()		{return _volumeNumber;}
inline	const	int			volumeNumber() const	{return _volumeNumber;}
inline		unsigned int &		dataOffset()		{return _dataOffset;}
inline	const	unsigned int		dataOffset() const	{return _dataOffset;}
inline		unsigned int &		dataSize()		{return _dataSize;}
inline	const	unsigned int		dataSize() const	{return _dataSize;}
inline		FileStatus &		status()		{return _status;}
inline	const	FileStatus		status() const		{return _status;}
inline		fstl::wstring &		statusString()		{return _statusString;}
inline	const	fstl::wstring &		statusString() const	{return _statusString;}

inline	const	fstl::wstring		filespec() const	{return filePath() + _T("\\") + fileName();}

private:
	// Data members

		fstl::wstring		_fileName;
		fstl::wstring		_filePath;
		unsigned char		_hash[EmDeeFive::HASH_SIZE_IN_BYTES];
		unsigned char		_setHash[EmDeeFive::HASH_SIZE_IN_BYTES];
		int			_volumeNumber;
		unsigned int		_dataOffset;
		unsigned int		_dataSize;
		FileStatus		_status;
		fstl::wstring		_statusString;
};

typedef	fstl::array<ParityFile>		ParityFileArray;
typedef	fstl::list<ParityFile>		ParityFileList;

#endif // _H_PARITYFILE
// ---------------------------------------------------------------------------------------------------------------------------------
// ParityFile.h - End of file
// ---------------------------------------------------------------------------------------------------------------------------------
