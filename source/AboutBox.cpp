// ---------------------------------------------------------------------------------------------------------------------------------
//           _                 _   ____                                  
//     /\   | |               | | |  _ \                                 
//    /  \  | |__   ___  _   _| |_| |_) | ___ __  __     ___ _ __  _ __  
//   / /\ \ | '_ \ / _ \| | | | __|  _ < / _ \\ \/ /    / __| '_ \| '_ \ 
//  / ____ \| |_) | (_) | |_| | |_| |_) | (_) |>  <  _ | (__| |_) | |_) |
// /_/    \_\_.__/ \___/ \__,_|\__|____/ \___//_/\_\(_) \___| .__/| .__/ 
//                                                          | |   | |    
//                                                          |_|   |_|
//
// Description:
//
//   About box
//
// Notes:
//
//   Best viewed with 8-character tabs and (at least) 132 columns
//
// History:
//
//   12/21/2001 by Paul Nettle: Original creation
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
#include "AboutBox.h"
#include "HelpDialog.h"
#include "FSRaidDialog.h"

// ---------------------------------------------------------------------------------------------------------------------------------

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// ---------------------------------------------------------------------------------------------------------------------------------

IMPLEMENT_DYNAMIC(AboutBox, CDialog)
BEGIN_MESSAGE_MAP(AboutBox, CDialog)
	ON_WM_HELPINFO()
	ON_WM_CLOSE()
END_MESSAGE_MAP()

// ---------------------------------------------------------------------------------------------------------------------------------

AboutBox::AboutBox(CWnd* pParent /*=NULL*/)
	: CDialog(AboutBox::IDD, pParent)
{
	Create(IDD, pParent);
}

// ---------------------------------------------------------------------------------------------------------------------------------

	AboutBox::~AboutBox()
{
	// Delete the temp files...

	if (tempLogoName.length()) _wunlink(tempLogoName.asArray());
	if (tempHtmlName.length()) _wunlink(tempHtmlName.asArray());

	// Destroy it

	DestroyWindow();
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	AboutBox::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

// ---------------------------------------------------------------------------------------------------------------------------------

BOOL	AboutBox::OnInitDialog()
{
	CDialog::OnInitDialog();

	TCHAR	buf[MAX_PATH];
	GetTempPath(MAX_PATH, buf);

	FILE *	fpJPG = NULL;
	FILE *	fpHTM = NULL;

	try
	{
		{
			tempLogoName = fstl::wstring(buf) + _T("logo.jpg");

			HRSRC	hrsc = FindResource(NULL, MAKEINTRESOURCE(IDR_RT_COMPANYLOGO), RT_RCDATA);
			if (!hrsc) throw _T("Unable to locate image resource");

			DWORD	fSize = SizeofResource(NULL, hrsc);
			if (!fSize) throw _T("Unable to locate image resource length");

			HGLOBAL	fGlob = LoadResource(NULL, hrsc);
			if (!fGlob) throw _T("Unable to load image resource");

			void *	fMem = LockResource(fGlob);
			if (!fMem) throw _T("Unable to lock image resource");

			fpJPG = _wfopen(tempLogoName.asArray(), _T("wb"));
			if (!fpJPG)
			{
				if (_waccess(tempLogoName.asArray(), 0)) throw _T("Unable to create temporary image resource file");
			}
			else
			{
				fwrite(fMem, fSize, 1, fpJPG);
				fclose(fpJPG);
				fpJPG = NULL;
			}
		}
		{
			tempHtmlName = fstl::wstring(buf) + _T("FSRaidAboutBox.html");

			HRSRC	hrsc = FindResource(NULL, MAKEINTRESOURCE(IDR_ABOUT_PAGE), RT_HTML);
			if (!hrsc) throw _T("Unable to locate HTML resource");

			DWORD	fSize = SizeofResource(NULL, hrsc);
			if (!fSize) throw _T("Unable to locate HTML resource length");

			HGLOBAL	fGlob = LoadResource(NULL, hrsc);
			if (!fGlob) throw _T("Unable to load HTML resource");

			void *	fMem = LockResource(fGlob);
			if (!fMem) throw _T("Unable to lock HTML resource");

			fpHTM = _wfopen(tempHtmlName.asArray(), _T("wb"));
			if (!fpHTM)
			{
				if (_waccess(tempHtmlName.asArray(), 0)) throw _T("Unable to create temporary HTML resource file");
			}
			else
			{
				fwrite(fMem, fSize, 1, fpHTM);
				fclose(fpHTM);
				fpHTM = NULL;
			}
		}

		// View the page...

		COleVariant varEmpty;
		fstl::wstring	url(_T("file://"));
		url += tempHtmlName;
		((WebBrowser *)GetDlgItem(IDC_ABOUT_BROWSER))->Navigate(url.asArray(), &varEmpty, &varEmpty, &varEmpty, &varEmpty);
	}
	catch(const TCHAR * err)
	{
		// Close any open files...

		if (fpJPG) fclose(fpJPG);
		if (fpHTM) fclose(fpHTM);

		AfxMessageBox(err, MB_ICONEXCLAMATION);
	}

	return TRUE;
}

// ---------------------------------------------------------------------------------------------------------------------------------

BOOL	AboutBox::OnHelpInfo(HELPINFO* pHelpInfo)
{
	FSRaidDialog *	fsrd = static_cast<FSRaidDialog *>(GetParent());
	fsrd->OnBnClickedHelpButton();
	return TRUE;
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	AboutBox::OnClose()
{
	FSRaidDialog *	ptr = (FSRaidDialog *) GetParent();
	ptr->aboutDialog() = NULL;
	delete this;
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	AboutBox::OnOK()
{
	FSRaidDialog *	ptr = (FSRaidDialog *) GetParent();
	ptr->aboutDialog() = NULL;
	delete this;
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	AboutBox::OnCancel()
{
	FSRaidDialog *	ptr = (FSRaidDialog *) GetParent();
	ptr->aboutDialog() = NULL;
	delete this;
}

// ---------------------------------------------------------------------------------------------------------------------------------
// AboutBox.cpp - End of file
// ---------------------------------------------------------------------------------------------------------------------------------
