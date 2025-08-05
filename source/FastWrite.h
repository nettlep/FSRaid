// ---------------------------------------------------------------------------------------------------------------------------------
//  ______           _   _   _   _      _ _            _     
// |  ____|         | | | | | | | |    (_) |          | |    
// | |__    __ _ ___| |_| | | | | |_ __ _| |_  ___    | |__  
// |  __|  / _` / __| __| | | | | | '__| | __|/ _ \   | '_ \ 
// | |    | (_| \__ \ |_|  V _ V  | |  | | |_|  __/ _ | | | |
// |_|     \__,_|___/\__|\__/ \__/|_|  |_|\__|\___|(_)|_| |_|
//                                                           
//                                                           
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

#ifndef	_H_FASTWRITE
#define _H_FASTWRITE

// ---------------------------------------------------------------------------------------------------------------------------------
// Module setup (required includes, macros, etc.)
// ---------------------------------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------------------------------

class	FastWrite
{
public:
	// Construction/Destruction

				FastWrite();
virtual				~FastWrite();

	// Operators

virtual		bool		open(const fstl::wstring & name);
virtual		void		close();
virtual		bool		write(void * buffer, const unsigned int count);

	// Implementation

	// Accessors

inline		fstl::wstring &	filename()			{return _filename;}
inline	const	fstl::wstring &	filename() const		{return _filename;}
inline		unsigned int &	bytesWritten()			{return _bytesWritten;}
inline	const	unsigned int	bytesWritten() const		{return _bytesWritten;}
inline		HANDLE &	handle()			{return _handle;}
inline	const	HANDLE		handle() const			{return _handle;}

private:
	// Data members

		fstl::wstring	_filename;
		unsigned int	_bytesWritten;
		HANDLE		_handle;
};

typedef	fstl::array<FastWrite>		FastWriteArray;
typedef	fstl::array<FastWrite *>	FastWritePointerArray;
typedef	fstl::list<FastWrite>		FastWriteList;
typedef	fstl::list<FastWrite *>		FastWritePointerList;

#endif // _H_FASTWRITE
// ---------------------------------------------------------------------------------------------------------------------------------
// FastWrite.h - End of file
// ---------------------------------------------------------------------------------------------------------------------------------
