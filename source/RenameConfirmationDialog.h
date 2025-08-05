// ---------------------------------------------------------------------------------------------------------------------------------
//  _____                                   _____              __ _                      _   _             _____  _       _                 _     
// |  __ \                                 / ____|            / _(_)                    | | (_)           |  __ \(_)     | |               | |    
// | |__) | ___ _ __   __ _ _ __ ___   ___| |      ___  _ __ | |_ _ _ __ _ __ ___   __ _| |_ _  ___  _ __ | |  | |_  __ _| | ___   __ _    | |__  
// |  _  / / _ \ '_ \ / _` | '_ ` _ \ / _ \ |     / _ \| '_ \|  _| | '__| '_ ` _ \ / _` | __| |/ _ \| '_ \| |  | | |/ _` | |/ _ \ / _` |   | '_ \ 
// | | \ \|  __/ | | | (_| | | | | | |  __/ |____| (_) | | | | | | | |  | | | | | | (_| | |_| | (_) | | | | |__| | | (_| | | (_) | (_| | _ | | | |
// |_|  \_\\___|_| |_|\__,_|_| |_| |_|\___|\_____|\___/|_| |_|_| |_|_|  |_| |_| |_|\__,_|\__|_|\___/|_| |_|_____/|_|\__,_|_|\___/ \__, |(_)|_| |_|
//                                                                                                                                 __/ |          
//                                                                                                                                |___/           
//
// Description:
//
//   Rename confirmation dialog (part of the fix-names process)
//
// Notes:
//
//   Best viewed with 8-character tabs and (at least) 132 columns
//
// History:
//
//   01/07/2002 by Paul Nettle: Original creation
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

#ifndef	_H_RENAMECONFIRMATIONDIALOG
#define _H_RENAMECONFIRMATIONDIALOG

// ---------------------------------------------------------------------------------------------------------------------------------
// Module setup (required includes, macros, etc.)
// ---------------------------------------------------------------------------------------------------------------------------------

//#include "afxcmn.h"

// ---------------------------------------------------------------------------------------------------------------------------------

class	RenameConfirmationDialog : public CDialog
{
public:
	// Types

		enum			{IDD = IDD_RENAME_DIALOG};

	// Construction/Destruction

					RenameConfirmationDialog(CWnd* pParent = NULL);
virtual					~RenameConfirmationDialog();


	// Accessors

inline		fstl::WStringArray &	fromList()		{return _fromList;}
inline	const	fstl::WStringArray &	fromList() const	{return _fromList;}
inline		fstl::WStringArray &	toList()		{return _toList;}
inline	const	fstl::WStringArray &	toList() const		{return _toList;}

afx_msg		BOOL			OnHelpInfo(HELPINFO* pHelpInfo);

private:
	// Data members

		fstl::WStringArray	_fromList;
		fstl::WStringArray	_toList;
		CListCtrl		renameList;

protected:
	// Message map and overrides
					DECLARE_DYNAMIC(RenameConfirmationDialog)
					DECLARE_MESSAGE_MAP()
virtual		void			DoDataExchange(CDataExchange* pDX);
virtual		BOOL			OnInitDialog();
};

#endif // _H_RENAMECONFIRMATIONDIALOG
// ---------------------------------------------------------------------------------------------------------------------------------
// RenameConfirmationDialog.h - End of file
// ---------------------------------------------------------------------------------------------------------------------------------
