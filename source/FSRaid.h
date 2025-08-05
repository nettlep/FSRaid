// ---------------------------------------------------------------------------------------------------------------------------------
//  ______  _____ _____        _     _     _     
// |  ____|/ ____|  __ \      (_)   | |   | |    
// | |__  | (___ | |__) | __ _ _  __| |   | |__  
// |  __|  \___ \|  _  / / _` | |/ _` |   | '_ \ 
// | |     ____) | | \ \| (_| | | (_| | _ | | | |
// |_|    |_____/|_|  \_\\__,_|_|\__,_|(_)|_| |_|
//
// Description:
//
//   Entry point for the program
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

#ifndef	_H_FSRAID
#define _H_FSRAID

// ---------------------------------------------------------------------------------------------------------------------------------
// Module setup (required includes, macros, etc.)
// ---------------------------------------------------------------------------------------------------------------------------------

#include "resource.h"
#include "fstl/fstl"
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "Utils.h"

// ---------------------------------------------------------------------------------------------------------------------------------
// Types
// ---------------------------------------------------------------------------------------------------------------------------------

typedef	bool	(*progressCallback)(void * userData, const fstl::wstring & displayText, const float percent);

// ---------------------------------------------------------------------------------------------------------------------------------

#define	PROGRAM_NAME_STRING	_T("FSRaid")

// ---------------------------------------------------------------------------------------------------------------------------------

class	FSRaidApp : public CWinApp
{
public:
					FSRaidApp();
virtual		BOOL			InitInstance();
virtual		void			renameKey();
virtual		fstl::WStringArray	copyKey(const fstl::wstring & source, const fstl::wstring & dest);
virtual		void			deleteSubkey(const fstl::wstring & key, const fstl::wstring & subkey);

inline	const	fstl::wstring &		programFilename() const	{return _programFilename;}
inline		fstl::wstring &		programFilename()	{return _programFilename;}

private:

		fstl::wstring		_programFilename;
					DECLARE_MESSAGE_MAP()
};

extern FSRaidApp theApp;

#endif // _H_FSRAID
// ---------------------------------------------------------------------------------------------------------------------------------
// FSRaid.h - End of file
// ---------------------------------------------------------------------------------------------------------------------------------
