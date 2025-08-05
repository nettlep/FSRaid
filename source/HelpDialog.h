// ---------------------------------------------------------------------------------------------------------------------------------
//  _    _      _       _____  _       _                 _     
// | |  | |    | |     |  __ \(_)     | |               | |    
// | |__| | ___| |_ __ | |  | |_  __ _| | ___   __ _    | |__  
// |  __  |/ _ \ | '_ \| |  | | |/ _` | |/ _ \ / _` |   | '_ \ 
// | |  | |  __/ | |_) | |__| | | (_| | | (_) | (_| | _ | | | |
// |_|  |_|\___|_| .__/|_____/|_|\__,_|_|\___/ \__, |(_)|_| |_|
//               | |                            __/ |          
//               |_|                           |___/           
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

#ifndef	_H_HELPDIALOG
#define _H_HELPDIALOG

// ---------------------------------------------------------------------------------------------------------------------------------
// Module setup (required includes, macros, etc.)
// ---------------------------------------------------------------------------------------------------------------------------------

#include "WebBrowser.h"
#include "afxwin.h"

// ---------------------------------------------------------------------------------------------------------------------------------

class	HelpDialog : public CDialog
{
public:
	// Enumerations

		enum		{ IDD = IDD_HELP };

	// Construction/destruction

				HelpDialog(CWnd* pParent = NULL);
virtual				~HelpDialog();

afx_msg		BOOL		OnHelpInfo(HELPINFO* pHelpInfo);

protected:

		CButton		closeButton;
		fstl::wstring	tempLogoName;
		fstl::wstring	tempHtmlName;
		fstl::wstring	tempImagName;

	// Message map & overrides

virtual		void		DoDataExchange(CDataExchange* pDX);
virtual		BOOL		OnInitDialog();
afx_msg		void		OnSize(UINT nType, int cx, int cy);

				DECLARE_MESSAGE_MAP()
				DECLARE_DYNAMIC(HelpDialog)
public:
	afx_msg void OnClose();
protected:
	virtual void OnCancel();
	virtual void OnOK();
};

#endif // _H_HELPDIALOG
// ---------------------------------------------------------------------------------------------------------------------------------
// HelpDialog.h - End of file
// ---------------------------------------------------------------------------------------------------------------------------------
