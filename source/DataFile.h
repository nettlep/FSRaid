// ---------------------------------------------------------------------------------------------------------------------------------
//  _____        _         ______ _ _          _     
// |  __ \      | |       |  ____(_) |        | |    
// | |  | | __ _| |_  __ _| |__   _| | ___    | |__  
// | |  | |/ _` | __|/ _` |  __| | | |/ _ \   | '_ \ 
// | |__| | (_| | |_| (_| | |    | | |  __/ _ | | | |
// |_____/ \__,_|\__|\__,_|_|    |_|_|\___|(_)|_| |_|
//                                                   
//                                                   
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

#ifndef	_H_DATAFILE
#define _H_DATAFILE

// ---------------------------------------------------------------------------------------------------------------------------------
// Module setup (required includes, macros, etc.)
// ---------------------------------------------------------------------------------------------------------------------------------

#include "EmDeeFive.h"

// ---------------------------------------------------------------------------------------------------------------------------------

class	DataFile
{
public:
	// Enumerations

		enum			FileStatus {Unknown, Valid, Corrupt, Missing, Misnamed, Error};

	// Types

		#pragma pack(1)
		typedef	struct	tag_par_file_entry
		{
			unsigned int	entrySizeLow;
			unsigned int	entrySizeHigh;
			unsigned int	statusFieldLow;
			unsigned int	statusFieldHigh;
			unsigned int	fileSizeLow;
			unsigned int	fileSizeHigh;
			unsigned char	md5Hash[16];
			unsigned char	md5Hash16K[16];
		} ParFileEntry;
		#pragma pack()

	// Construction/Destruction

					DataFile();
virtual					~DataFile();

	// Operators

	// Implementation

virtual		bool			validate(unsigned char actualHash[EmDeeFive::HASH_SIZE_IN_BYTES], const unsigned int totalFiles, const unsigned int curIndex, progressCallback callback = NULL, void * callbackData = NULL);
virtual		bool			readParHeader(FILE * fp, const fstl::wstring & path = _T(""));
virtual		bool			writeParHeader(FILE * fp) const;
virtual		fstl::ucharArray	storeParHeader() const;
static		fstl::wstring		getStatusString(const FileStatus status);

	// Accessors

inline		fstl::wstring &		fileName()		{return _fileName;}
inline	const	fstl::wstring &		fileName() const	{return _fileName;}
inline		fstl::wstring &		filePath()		{return _filePath;}
inline	const	fstl::wstring &		filePath() const	{return _filePath;}
inline		unsigned int &		fileSize()		{return _fileSize;}
inline	const	unsigned int		fileSize() const	{return _fileSize;}
inline		unsigned char *		hash()			{return _hash;}
inline	const	unsigned char *		hash() const		{return _hash;}
inline		unsigned char *		hashFirst16K()		{return _hashFirst16K;}
inline	const	unsigned char *		hashFirst16K() const	{return _hashFirst16K;}
inline		bool &			recoverable()		{return _recoverable;}
inline	const	bool			recoverable() const	{return _recoverable;}
inline		FileStatus &		status()		{return _status;}
inline	const	FileStatus		status() const		{return _status;}
inline		fstl::wstring &		statusString()		{return _statusString;}
inline	const	fstl::wstring &		statusString() const	{return _statusString;}

inline	const	fstl::wstring		filespec() const	{return filePath() + _T("\\") + fileName();}

private:
	// Data members

		fstl::wstring		_fileName;
		fstl::wstring		_filePath;
		unsigned int		_fileSize;
		unsigned char		_hash[EmDeeFive::HASH_SIZE_IN_BYTES];
		unsigned char		_hashFirst16K[EmDeeFive::HASH_SIZE_IN_BYTES];
		bool			_recoverable;
		FileStatus		_status;
		fstl::wstring		_statusString;
};

typedef	fstl::array<DataFile>		DataFileArray;
typedef	fstl::list<DataFile>		DataFileList;

#endif // _H_DATAFILE
// ---------------------------------------------------------------------------------------------------------------------------------
// DataFile.h - End of file
// ---------------------------------------------------------------------------------------------------------------------------------
