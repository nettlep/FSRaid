// ---------------------------------------------------------------------------------------------------------------------------------
//  _    _ _   _ _         _     
// | |  | | | (_) |       | |    
// | |  | | |_ _| |___    | |__  
// | |  | | __| | / __|   | '_ \ 
// | |__| | |_| | \__ \ _ | | | |
//  \____/ \__|_|_|___/(_)|_| |_|
//                               
//                               
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

#ifndef	_H_UTILS
#define _H_UTILS

// ---------------------------------------------------------------------------------------------------------------------------------
// Module setup (required includes, macros, etc.)
// ---------------------------------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------------------------------
// Types
// ---------------------------------------------------------------------------------------------------------------------------------

class	RegInfo
{
public:

inline	bool		operator <(RegInfo & rhs) {return lastAccessed < rhs.lastAccessed;}
inline	bool		operator >(RegInfo & rhs) {return lastAccessed > rhs.lastAccessed;}
	unsigned int	lastAccessed;
	unsigned int	hashCount;
	unsigned int	dataCount;
	unsigned int	parityCount;
	fstl::charArray	hash;
	fstl::charArray	data;
	fstl::charArray	parity;
};
typedef	fstl::array<RegInfo>	RegInfoArray;

// ---------------------------------------------------------------------------------------------------------------------------------

unsigned int	getFileLength(const fstl::wstring & filename);
bool		isDirectory(const fstl::wstring & filename);
bool		doesFileExist(const fstl::wstring & filename);
void		allowBackgroundProcessing();
fstl::wstring	sizeString(const unsigned int s);
fstl::wstring	getLastErrorString(const DWORD err);
RegInfoArray	getRegInfo(unsigned int & maxAllowed);
bool		putRegInfo(const RegInfoArray & ria, const unsigned int maxAllowed);
void		wipeRegInfo();
void		stripUnicodeToAscii(fstl::wstring & str);
bool		shouldStrip(const fstl::wstring & str);
bool		doesContainNonASCII(const fstl::wstring & str);
fstl::wstring	oemToAnsi(const fstl::wstring & oemString);
fstl::wstring	ansiToOem(const fstl::wstring & ansiString);

#endif // _H_UTILS
// ---------------------------------------------------------------------------------------------------------------------------------
// Utils.h - End of file
// ---------------------------------------------------------------------------------------------------------------------------------
