// ---------------------------------------------------------------------------------------------------------------------------------
//  ______  _____ _____        _     _                      
// |  ____|/ ____|  __ \      (_)   | |                     
// | |__  | (___ | |__) | __ _ _  __| |     ___ _ __  _ __  
// |  __|  \___ \|  _  / / _` | |/ _` |    / __| '_ \| '_ \ 
// | |     ____) | | \ \| (_| | | (_| | _ | (__| |_) | |_) |
// |_|    |_____/|_|  \_\\__,_|_|\__,_|(_) \___| .__/| .__/ 
//                                             | |   | |    
//                                             |_|   |_|    
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

#include "stdafx.h"
#include "FSRaid.h"
#include "FSRaidDialog.h"

// ---------------------------------------------------------------------------------------------------------------------------------

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// ---------------------------------------------------------------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(FSRaidApp, CWinApp)
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

// ---------------------------------------------------------------------------------------------------------------------------------

FSRaidApp theApp;

// ---------------------------------------------------------------------------------------------------------------------------------

	FSRaidApp::FSRaidApp()
{
}

// ---------------------------------------------------------------------------------------------------------------------------------

BOOL	FSRaidApp::InitInstance()
{
	InitCommonControls();

	CWinApp::InitInstance();

	AfxEnableControlContainer();

	// Get the program filename

	programFilename() = GetCommandLine();

	// Strip the quotes from the filename

	if (programFilename()[0] == '"')
	{
		programFilename().erase(0, 1);
		int	idx = programFilename().find(_T("\""));
		if (idx != -1) programFilename().erase(idx);
	}

	// Strip everything past the .exe
	{
		int	idx = programFilename().ncfind(_T(".exe"));
		if (idx != -1) programFilename().erase(idx + 4);
	}

	// "This is who we are" (Millenium fans go nutz here :)

	SetRegistryKey(_T("Fluid Studios"));
	free(const_cast<void *>(static_cast<const void *>(theApp.m_pszProfileName)));
	theApp.m_pszProfileName = _tcsdup(PROGRAM_NAME_STRING);

	// Need to rename the "Fluid Studios Software RAID Toolkit" registry key to "FSRaid"

	renameKey();

	// Get the args

	fstl::wstring	args = GetCommandLine();

	// Strip the filename

	if (args[0] == '\"')
	{
		args.erase(0, 1);
		int	idx = args.find(_T("\""));
		if (idx >= 0)	args.erase(0, idx+1);
		else		args.erase();
	}
	else
	{
		int	idx = args.find(_T(" "));
		if (idx >= 0)	args.erase(0, idx+1);
		else		args.erase();
	}

	// Trim it down

	args.trim(_T(" \r\n\f\t\v"));

	// Strip off any quotes
	if (args.length() && args[0] == '\"')
	{
		args.erase(0, 1);
		int	idx = args.find(_T("\""));
		if (idx >= 0) args.erase(idx);
	}

	// Send whatever else is left to the dialog...

	FSRaidDialog dlg;
	dlg.startupFile() = args;
	m_pMainWnd = &dlg;
	dlg.DoModal();

	return FALSE;
}

// ---------------------------------------------------------------------------------------------------------------------------------
// Return-Path: <unsubscribe@fwb.com>
// Received: from www.fluidstudios.com ([216.12.218.19])
// 	by www.GraphicsPapers.com (8.11.3/8.11.3/SuSE Linux 8.11.1-0.5) with ESMTP id h5NMELo21363
// 	for <paul@graphicspapers.com>; Mon, 23 Jun 2003 17:14:22 -0500
// Received: from fwb.com (fwb.com [161.58.237.92])
// 	by www.fluidstudios.com (8.10.2/8.10.2) with ESMTP id h5NMEhx15913
// 	for <paul@FLUIDSTUDIOS.COM>; Mon, 23 Jun 2003 17:14:43 -0500
// Received: from DellWebMan ([209.172.110.70]) by fwb.com (8.12.9) id h5NM7T9j062822 for <paul@FLUIDSTUDIOS.COM>; Mon, 23 Jun 2003 16:07:35 -0600 (MDT)
// From: "FWB Software, Inc." <unsubscribe@fwb.com>
// To: <paul@FluidStudios.com>
// Subject: Raid Toolkit
// Date: Mon, 23 Jun 2003 15:07:34 -0700
// Message-ID: <034301c339d3$dce36a80$466eacd1@DellWebMan>
// MIME-Version: 1.0
// Content-Type: multipart/alternative;
// 	boundary="----=_NextPart_000_0344_01C33999.30849280"
// X-Priority: 3 (Normal)
// X-MSMail-Priority: Normal
// X-Mailer: Microsoft Outlook, Build 10.0.4024
// Importance: Normal
// X-MimeOLE: Produced By Microsoft MimeOLE V6.00.2800.1165
// X-IMAPbase: 1056484236 4
// X-UIDL: %JL"!Z<j"!:PG!!C'!"!
// Status: O
// X-Status: 
// X-Keywords:                       
// X-UID: 2
// 
// Dear Paul,
// 
// My name is Mark Strathdee, Chairman & CEO for FWB Software, Inc.  Our
// Trademark team recently came across your product name during a routine
// search of FWB's trademarks.  I just wanted to inform you that Raid
// Toolkit is currently a trademark of FWB and has been since the early
// 90's.
// 
// Under normal circumstances our counsel would send a cease and desist
// notice, but in this case on an open source project, we don't like to be
// so aggressive at first.  Please consider this a formal request to remove
// or rebrand your existing product to another name at your earliest
// convenience. 
// 
// If you have any questions, please feel free to contact me.
// 
// Thanks,
// Mark
// FWB
// ---------------------------------------------------------------------------------------------------------------------------------

void	FSRaidApp::renameKey()
{
	fstl::WStringArray	deleteThese;

	deleteThese = copyKey(_T("Software\\Fluid Studios\\Fluid Studios Software Raid Toolkit\\Archive States"), _T("Software\\Fluid Studios\\FSRaid\\Archive States"));
	deleteSubkey(_T("Software\\Fluid Studios\\Fluid Studios Software Raid Toolkit"), _T("Archive States"));

	deleteThese = copyKey(_T("Software\\Fluid Studios\\Fluid Studios Software Raid Toolkit\\Options"), _T("Software\\Fluid Studios\\FSRaid\\Options"));
	deleteSubkey(_T("Software\\Fluid Studios\\Fluid Studios Software Raid Toolkit"), _T("Options"));

	deleteSubkey(_T("Software\\Fluid Studios"), _T("Fluid Studios Software Raid Toolkit"));
}

// ---------------------------------------------------------------------------------------------------------------------------------

fstl::WStringArray	FSRaidApp::copyKey(const fstl::wstring & source, const fstl::wstring & dest)
{
	// We'll build a list of values to delete when we're done

	fstl::WStringArray	deleteThese;

	// Begin the copy process - open the source

	HKEY	srcKey;
	DWORD	rc = RegOpenKey(HKEY_CURRENT_USER, source.asArray(), &srcKey);
	if (rc == ERROR_SUCCESS)
	{
		HKEY	dstKey;
		rc = RegCreateKey(HKEY_CURRENT_USER, dest.asArray(), &dstKey);
		if (rc == ERROR_SUCCESS)
		{
			// Start the enumeration process

			int	index = 0;
			wchar_t	valueName[1024];
			memset(valueName, 0, sizeof(valueName));
			DWORD	valueNameSize = sizeof(valueName);
			DWORD	type;
			DWORD	rc = RegEnumValue(srcKey, index, valueName, &valueNameSize, NULL, &type, NULL, NULL);

			while(rc == ERROR_SUCCESS)
			{
				// Get the data size of this value

				DWORD	type;
				DWORD	dataSize;
				RegQueryValueEx(srcKey, valueName, NULL, &type, NULL, &dataSize);

				if (dataSize)
				{
					// Allocate the memory needed

					BYTE *	buf = new BYTE[dataSize];
					memset(buf, 0, sizeof(buf));

					// Read the data

					if (RegQueryValueEx(srcKey, valueName, NULL, &type, buf, &dataSize) == ERROR_SUCCESS)
					{
						// Copy the key

						if (RegSetValueEx(dstKey, valueName, NULL, type, buf, dataSize) == ERROR_SUCCESS)
						{
							// Keep track of this value so we can delete it

							deleteThese += valueName;
						}
					}

					delete[] buf;
				}

				// Next key, please

				++index;
				memset(valueName, 0, sizeof(valueName));
				valueNameSize = sizeof(valueName);
				rc = RegEnumValue(srcKey, index, valueName, &valueNameSize, NULL, &type, NULL, NULL);
			}

			RegCloseKey(dstKey);
		}

		RegCloseKey(srcKey);
	}

	return deleteThese;
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	FSRaidApp::deleteSubkey(const fstl::wstring & key, const fstl::wstring & subkey)
{
	HKEY	hKey;
	DWORD	rc = RegOpenKey(HKEY_CURRENT_USER, key.asArray(), &hKey);
	if (rc == ERROR_SUCCESS)
	{
		RegDeleteKey(hKey, subkey.asArray());
		RegCloseKey(hKey);
	}
}

// ---------------------------------------------------------------------------------------------------------------------------------
// FSRaid.cpp - End of file
// ---------------------------------------------------------------------------------------------------------------------------------
