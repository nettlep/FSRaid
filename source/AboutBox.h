// ---------------------------------------------------------------------------------------------------------------------------------
//           _                 _   ____                 _     
//     /\   | |               | | |  _ \               | |    
//    /  \  | |__   ___  _   _| |_| |_) | ___ __  __   | |__  
//   / /\ \ | '_ \ / _ \| | | | __|  _ < / _ \\ \/ /   | '_ \ 
//  / ____ \| |_) | (_) | |_| | |_| |_) | (_) |>  <  _ | | | |
// /_/    \_\_.__/ \___/ \__,_|\__|____/ \___//_/\_\(_)|_| |_|
//                                                            
//                                                            
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

#ifndef	_H_ABOUTBOX
#define _H_ABOUTBOX

// ---------------------------------------------------------------------------------------------------------------------------------
// Module setup (required includes, macros, etc.)
// ---------------------------------------------------------------------------------------------------------------------------------

#include "WebBrowser.h"

// ---------------------------------------------------------------------------------------------------------------------------------

class	AboutBox : public CDialog
{
public:
	// Enumerations

		enum		{ IDD = IDD_ABOUTBOX };

	// Construction/Destruction

				AboutBox(CWnd* pParent = NULL);
virtual				~AboutBox();

afx_msg		BOOL		OnHelpInfo(HELPINFO* pHelpInfo);
afx_msg 	void 		OnClose();
virtual 	void 		OnOK();
virtual 	void 		OnCancel();

protected:
		fstl::wstring	tempLogoName;
		fstl::wstring	tempHtmlName;

	// Message map and overrides

virtual		void		DoDataExchange(CDataExchange* pDX);
virtual		BOOL		OnInitDialog();

				DECLARE_DYNAMIC(AboutBox)
				DECLARE_MESSAGE_MAP()
};

#endif // _H_ABOUTBOX
// ---------------------------------------------------------------------------------------------------------------------------------
// AboutBox.h - End of file
// ---------------------------------------------------------------------------------------------------------------------------------
