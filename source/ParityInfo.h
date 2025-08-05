// ---------------------------------------------------------------------------------------------------------------------------------
//  _____             _ _         _____        __           _     
// |  __ \           (_) |       |_   _|      / _|         | |    
// | |__) | __ _ _ __ _| |_ _   _  | |  _ __ | |_  ___     | |__  
// |  ___/ / _` | '__| | __| | | | | | | '_ \|  _|/ _ \    | '_ \ 
// | |    | (_| | |  | | |_| |_| |_| |_| | | | | | (_) | _ | | | |
// |_|     \__,_|_|  |_|\__|\__, |_____|_| |_|_|  \___/ (_)|_| |_|
//                           __/ |                                
//                          |___/                                 
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

#ifndef	_H_PARITYINFO
#define _H_PARITYINFO

// ---------------------------------------------------------------------------------------------------------------------------------
// Module setup (required includes, macros, etc.)
// ---------------------------------------------------------------------------------------------------------------------------------

#include "DataFile.h"
#include "ParityFile.h"

// ---------------------------------------------------------------------------------------------------------------------------------

class	ParityInfo
{
public:
	// Construction/Destruction

					ParityInfo(const unsigned int rsRaidBits = 8);
virtual					~ParityInfo();

	// Operators

	// Implementation

virtual		void			reset();
virtual		bool			validateDataFile(const unsigned int index, const unsigned int totalFiles, const unsigned int curIndex, progressCallback callback = NULL, void * callbackData = NULL);
virtual		bool			validateDataFile(DataFile & df, const unsigned int totalFiles, const unsigned int curIndex, progressCallback callback = NULL, void * callbackData = NULL) const;
virtual		bool			loadParFile(fstl::wstring & filename);
virtual		bool			validateParFile(const unsigned int index, const unsigned int totalFiles, const unsigned int curIndex, progressCallback callback = NULL, void * callbackData = NULL);
virtual		bool			validateParFile(ParityFile & pf, const unsigned int totalFiles, const unsigned int curIndex, progressCallback callback = NULL, void * callbackData = NULL) const;
virtual		bool			findParFiles(ParityFileArray & pfa) const;
virtual		bool			genParFiles(unsigned char parSetHash[EmDeeFive::HASH_SIZE_IN_BYTES], ParityFileArray & parityVolumes, DataFileArray & dataVolumes, progressCallback callback = NULL, void * callbackData = NULL);
virtual		void			make_lut(unsigned char lut[0x100], const int m) const;
virtual		bool			recoverFiles(ParityFileArray & parityVolumes, DataFileArray & dataVolumes, progressCallback callback, void * callbackData, const int repairSingleIndex = -1);

	// Accessors

inline		fstl::wstring &		defaultPath()		{return _defaultPath;}
inline	const	fstl::wstring &		defaultPath() const	{return _defaultPath;}
inline		fstl::wstring &		defaultBaseName()	{return _defaultBaseName;}
inline	const	fstl::wstring &		defaultBaseName() const	{return _defaultBaseName;}
inline		fstl::wstring &		createdByString()	{return _createdByString;}
inline	const	fstl::wstring &		createdByString() const	{return _createdByString;}
inline		DataFileArray &		dataFiles()		{return _dataFiles;}
inline	const	DataFileArray &		dataFiles() const	{return _dataFiles;}
inline		ParityFileArray &	parityFiles()		{return _parityFiles;}
inline	const	ParityFileArray &	parityFiles() const	{return _parityFiles;}
inline		unsigned int &		rsRaidBits()		{return _rsRaidBits;}
inline	const	unsigned int		rsRaidBits() const	{return _rsRaidBits;}
inline		unsigned int *&		gflog()			{return _gflog;}
inline	const	unsigned int *		gflog() const		{return _gflog;}
inline		unsigned int *&		gfexp()			{return _gfexp;}
inline	const	unsigned int *		gfexp() const		{return _gfexp;}
inline		unsigned int *&		vandMatrix()		{return _vandMatrix;}
inline	const	unsigned int *		vandMatrix() const	{return _vandMatrix;}
inline		unsigned int *&		recoveryArrays()	{return _recoveryArrays;}
inline	const	unsigned int *		recoveryArrays() const	{return _recoveryArrays;}
inline		unsigned char *		setHash()		{return _setHash;}
inline	const	unsigned char *		setHash() const		{return _setHash;}

private:
	// Explicitly disallowed calls (they appear here, because if we don't do this, the compiler will generate them for us)
		
					ParityInfo(const ParityInfo & rhs);
inline		ParityInfo &		operator =(const ParityInfo & rhs);

	// Utilitarian

virtual		unsigned int		gfADD(const unsigned int a, const unsigned int b) const;
virtual		unsigned int		gfSUB(const unsigned int a, const unsigned int b) const;
virtual		unsigned int		gfMUL(const unsigned int a, const unsigned int b) const;
virtual		unsigned int		gfDIV(const unsigned int a, const unsigned int b) const;
virtual		unsigned int		gfPOW(const unsigned int a, const unsigned int b) const;
virtual		bool			genGaloisFieldTables();
virtual		bool			genVandermondeMatrix(const unsigned int dataFileCount, const unsigned int parityFileCount);
virtual		bool			genRecoveryMultipliers(const fstl::boolArray & dataFileValidityFlags, const fstl::intArray & parityIDs, bool & setUnrecoverable);
virtual		bool			analyzeRecoverable(const fstl::boolArray & dataFileValidityFlags, fstl::intArray & parityIDs, ParityFileArray & parityVolumes, const unsigned int corruptCount, bool & setUnrecoverable);

	// Data members

		fstl::wstring		_defaultPath;
		fstl::wstring		_defaultBaseName;
		fstl::wstring		_createdByString;
		unsigned int		_version;
		DataFileArray		_dataFiles;
		ParityFileArray		_parityFiles;
		unsigned int		_rsRaidBits;
		unsigned int *		_gflog;
		unsigned int *		_gfexp;
		unsigned int *		_vandMatrix;
		unsigned int *		_recoveryArrays;
		unsigned char		_setHash[16];
};

typedef	fstl::array<ParityInfo *>	ParityInfoPointerArray;
typedef	fstl::list<ParityInfo *>	ParityInfoPointerList;


#endif // _H_PARITYINFO
// ---------------------------------------------------------------------------------------------------------------------------------
// ParityInfo.h - End of file
// ---------------------------------------------------------------------------------------------------------------------------------
