// ---------------------------------------------------------------------------------------------------------------------------------
//  _____                                   _____              __ _                      _   _             _____  _       _                                  
// |  __ \                                 / ____|            / _(_)                    | | (_)           |  __ \(_)     | |                                 
// | |__) | ___ _ __   __ _ _ __ ___   ___| |      ___  _ __ | |_ _ _ __ _ __ ___   __ _| |_ _  ___  _ __ | |  | |_  __ _| | ___   __ _      ___ _ __  _ __  
// |  _  / / _ \ '_ \ / _` | '_ ` _ \ / _ \ |     / _ \| '_ \|  _| | '__| '_ ` _ \ / _` | __| |/ _ \| '_ \| |  | | |/ _` | |/ _ \ / _` |    / __| '_ \| '_ \ 
// | | \ \|  __/ | | | (_| | | | | | |  __/ |____| (_) | | | | | | | |  | | | | | | (_| | |_| | (_) | | | | |__| | | (_| | | (_) | (_| | _ | (__| |_) | |_) |
// |_|  \_\\___|_| |_|\__,_|_| |_| |_|\___|\_____|\___/|_| |_|_| |_|_|  |_| |_| |_|\__,_|\__|_|\___/|_| |_|_____/|_|\__,_|_|\___/ \__, |(_) \___| .__/| .__/ 
//                                                                                                                                 __/ |        | |   | |    
//                                                                                                                                |___/         |_|   |_|    
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

#include "stdafx.h"
#include "FSRaid.h"
#include "RenameConfirmationDialog.h"
#include "HelpDialog.h"

// ---------------------------------------------------------------------------------------------------------------------------------

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// ---------------------------------------------------------------------------------------------------------------------------------

IMPLEMENT_DYNAMIC(RenameConfirmationDialog, CDialog)
BEGIN_MESSAGE_MAP(RenameConfirmationDialog, CDialog)
//	ON_WM_CHAR()
ON_WM_HELPINFO()
END_MESSAGE_MAP()

// ---------------------------------------------------------------------------------------------------------------------------------

	RenameConfirmationDialog::RenameConfirmationDialog(CWnd* pParent /*=NULL*/)
	: CDialog(RenameConfirmationDialog::IDD, pParent)
{
}

// ---------------------------------------------------------------------------------------------------------------------------------

	RenameConfirmationDialog::~RenameConfirmationDialog()
{
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	RenameConfirmationDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RENAME_LIST, renameList);
}

// ---------------------------------------------------------------------------------------------------------------------------------

BOOL	RenameConfirmationDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Setup the list control

	renameList.InsertColumn(0, _T("Current filename"));
	renameList.InsertColumn(1, _T("Rename to"));

	renameList.DeleteAllItems();

	CRect	r;
	renameList.GetClientRect(r);
	renameList.SetColumnWidth(0, r.Width()/2-10);
	renameList.SetColumnWidth(1, r.Width()/2-10);

	if (fromList().size() == toList().size())
	{
		for (unsigned int i = 0; i < fromList().size(); ++i)
		{
			renameList.InsertItem(i, fromList()[i].asArray(), 0);

			LVITEM	li;
			memset(&li, 0, sizeof(li));
			li.mask = LVIF_TEXT;
			li.iItem = i;
			li.iSubItem = 1;
			li.pszText = const_cast<TCHAR *>(toList()[i].asArray());
			renameList.SetItem(&li);
		}
	}

	return TRUE;
}

// ---------------------------------------------------------------------------------------------------------------------------------

BOOL RenameConfirmationDialog::OnHelpInfo(HELPINFO* pHelpInfo)
{
	HelpDialog	dlg;
	dlg.DoModal();
	return TRUE;
}

// ---------------------------------------------------------------------------------------------------------------------------------
// RenameConfirmationDialog.cpp - End of file
// ---------------------------------------------------------------------------------------------------------------------------------

