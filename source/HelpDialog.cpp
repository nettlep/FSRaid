// ---------------------------------------------------------------------------------------------------------------------------------
//  _    _      _       _____  _       _                                  
// | |  | |    | |     |  __ \(_)     | |                                 
// | |__| | ___| |_ __ | |  | |_  __ _| | ___   __ _      ___ _ __  _ __  
// |  __  |/ _ \ | '_ \| |  | | |/ _` | |/ _ \ / _` |    / __| '_ \| '_ \ 
// | |  | |  __/ | |_) | |__| | | (_| | | (_) | (_| | _ | (__| |_) | |_) |
// |_|  |_|\___|_| .__/|_____/|_|\__,_|_|\___/ \__, |(_) \___| .__/| .__/ 
//               | |                            __/ |        | |   | |    
//               |_|                           |___/         |_|   |_|    
//
// Description:
//
//   Help dialog
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
#include "HelpDialog.h"
#include "FSRaidDialog.h"

// ---------------------------------------------------------------------------------------------------------------------------------

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// ---------------------------------------------------------------------------------------------------------------------------------

IMPLEMENT_DYNAMIC(HelpDialog, CDialog)
BEGIN_MESSAGE_MAP(HelpDialog, CDialog)
	ON_WM_SIZE()
	ON_WM_HELPINFO()
END_MESSAGE_MAP()

// ---------------------------------------------------------------------------------------------------------------------------------

	HelpDialog::HelpDialog(CWnd* pParent /*=NULL*/)
	: CDialog(HelpDialog::IDD, pParent)
{
	Create(IDD, pParent);
}

// ---------------------------------------------------------------------------------------------------------------------------------

	HelpDialog::~HelpDialog()
{
	// Delete the temp files...

	if (tempLogoName.length()) _wunlink(tempLogoName.asArray());
	if (tempHtmlName.length()) _wunlink(tempHtmlName.asArray());
	if (tempImagName.length()) _wunlink(tempImagName.asArray());

	// Destroy it

	DestroyWindow();
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	HelpDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, closeButton);
}

// ---------------------------------------------------------------------------------------------------------------------------------

BOOL	HelpDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	TCHAR	buf[MAX_PATH];
	GetTempPath(MAX_PATH, buf);

	FILE *	fpJPG = NULL;
	FILE *	fpHTM = NULL;
	FILE *	fpIMG = NULL;

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
			tempImagName = fstl::wstring(buf) + _T("help1.gif");

			HRSRC	hrsc = FindResource(NULL, MAKEINTRESOURCE(IDR_RT_HELP1), RT_RCDATA);
			if (!hrsc) throw _T("Unable to locate image(2) resource");

			DWORD	fSize = SizeofResource(NULL, hrsc);
			if (!fSize) throw _T("Unable to locate image(2) resource length");

			HGLOBAL	fGlob = LoadResource(NULL, hrsc);
			if (!fGlob) throw _T("Unable to load image(2) resource");

			void *	fMem = LockResource(fGlob);
			if (!fMem) throw _T("Unable to lock image(2) resource");

			fpIMG = _wfopen(tempImagName.asArray(), _T("wb"));
			if (!fpIMG)
			{
				if (_waccess(tempImagName.asArray(), 0)) throw _T("Unable to create temporary image(2) resource file");
			}
			else
			{
				fwrite(fMem, fSize, 1, fpIMG);
				fclose(fpIMG);
				fpIMG = NULL;
			}
		}
		{
			tempHtmlName = fstl::wstring(buf) + _T("FSRaidHelp.html");

			HRSRC	hrsc = FindResource(NULL, MAKEINTRESOURCE(IDR_HELP_PAGE), RT_HTML);
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
		((WebBrowser *)GetDlgItem(IDC_HELP_BROWSER))->Navigate(url.asArray(), &varEmpty, &varEmpty, &varEmpty, &varEmpty);
	}
	catch(const TCHAR * err)
	{
		// Close any open files...

		if (fpJPG) fclose(fpJPG);
		if (fpHTM) fclose(fpHTM);
		if (fpIMG) fclose(fpIMG);

		AfxMessageBox(err, MB_ICONEXCLAMATION);
	}

	return TRUE;
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	HelpDialog::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	if (!closeButton.GetSafeHwnd()) return;

	CRect	clientRect;
	GetClientRect(clientRect);

	// Border...

	clientRect.left += 11;
	clientRect.top += 11;
	clientRect.right -= 11;
	clientRect.bottom -= 11;

	CRect	buttonRect;
	closeButton.GetWindowRect(buttonRect);

        ((WebBrowser *)GetDlgItem(IDC_HELP_BROWSER))->MoveWindow(clientRect.left, clientRect.top, clientRect.Width(), clientRect.Height() - buttonRect.Height() - 8);

	closeButton.MoveWindow(clientRect.right - buttonRect.Width(), clientRect.bottom - buttonRect.Height(), buttonRect.Width(), buttonRect.Height());
}

// ---------------------------------------------------------------------------------------------------------------------------------

BOOL HelpDialog::OnHelpInfo(HELPINFO* pHelpInfo)
{
	return TRUE;
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	HelpDialog::OnClose()
{
	FSRaidDialog *	ptr = (FSRaidDialog *) GetParent();
	ptr->helpDialog() = NULL;
	delete this;
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	HelpDialog::OnCancel()
{
	FSRaidDialog *	ptr = (FSRaidDialog *) GetParent();
	ptr->helpDialog() = NULL;
	delete this;
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	HelpDialog::OnOK()
{
	FSRaidDialog *	ptr = (FSRaidDialog *) GetParent();
	ptr->helpDialog() = NULL;
	delete this;
}

// ---------------------------------------------------------------------------------------------------------------------------------
// HelpDialog.cpp - End of file
// ---------------------------------------------------------------------------------------------------------------------------------
