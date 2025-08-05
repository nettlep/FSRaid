// ---------------------------------------------------------------------------------------------------------------------------------
//   _____                 _        _____             _ _         _____  _       _                                  
//  / ____|               | |      |  __ \           (_) |       |  __ \(_)     | |                                 
// | |     _ __  ___  __ _| |_  ___| |__) | __ _ _ __ _| |_ _   _| |  | |_  __ _| | ___   __ _      ___ _ __  _ __  
// | |    | '__|/ _ \/ _` | __|/ _ \  ___/ / _` | '__| | __| | | | |  | | |/ _` | |/ _ \ / _` |    / __| '_ \| '_ \ 
// | |____| |  |  __/ (_| | |_|  __/ |    | (_| | |  | | |_| |_| | |__| | | (_| | | (_) | (_| | _ | (__| |_) | |_) |
//  \_____|_|   \___|\__,_|\__|\___|_|     \__,_|_|  |_|\__|\__, |_____/|_|\__,_|_|\___/ \__, |(_) \___| .__/| .__/ 
//                                                           __/ |                        __/ |        | |   | |    
//                                                          |___/                        |___/         |_|   |_|    
//
// Description:
//
//   File selection dialog for creating parity archive sets
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

#include "stdafx.h"
#include <direct.h>
#include <math.h>
#include "FSRaid.h"
#include "HelpDialog.h"
#include "CreateParityDialog.h"

// ---------------------------------------------------------------------------------------------------------------------------------

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// ---------------------------------------------------------------------------------------------------------------------------------

IMPLEMENT_DYNAMIC(CreateParityDialog, CDialog)
BEGIN_MESSAGE_MAP(CreateParityDialog, CDialog)
	ON_BN_CLICKED(IDC_OUTPUTDIRBROWSE_BUTTON, OnBnClickedOutputdirbrowseButton)
	ON_BN_CLICKED(IDC_CLEARLIST_BUTTON, OnBnClickedClearlistButton)
	ON_BN_CLICKED(IDC_SETNONRECOVERABLE_BUTTON, OnBnClickedSetnonrecoverableButton)
	ON_BN_CLICKED(IDC_SETRECOVERABLE_BUTTON, OnBnClickedSetrecoverableButton)
	ON_BN_CLICKED(IDC_ADD_BUTTON, OnBnClickedAddButton)
	ON_BN_CLICKED(IDC_REMOVE_BUTTON, OnBnClickedRemoveButton)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_LBN_SELCHANGE(IDC_FILE_LIST, OnLbnSelchangeFileList)
	ON_WM_HSCROLL()
	ON_WM_DROPFILES()
	ON_WM_HELPINFO()
END_MESSAGE_MAP()

// ---------------------------------------------------------------------------------------------------------------------------------

	CreateParityDialog::CreateParityDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CreateParityDialog::IDD, pParent)
{
}

// ---------------------------------------------------------------------------------------------------------------------------------

	CreateParityDialog::~CreateParityDialog()
{
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	CreateParityDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FILE_LIST, fileList);
	DDX_Control(pDX, IDC_REMOVE_BUTTON, removeSelectedButton);
	DDX_Control(pDX, IDC_SETRECOVERABLE_BUTTON, setRecoverableButton);
	DDX_Control(pDX, IDC_SETNONRECOVERABLE_BUTTON, setNonRecoverableButton);
	DDX_Control(pDX, IDC_CLEARLIST_BUTTON, clearListButton);
	DDX_Control(pDX, IDC_BASENAME_EDIT, baseFilenameEdit);
	DDX_Control(pDX, IDC_OUTPUTDIR_EDIT, outputDirectoryEdit);
	DDX_Control(pDX, IDC_RECOVERY_SLIDER, recoverySlider);
	DDX_Control(pDX, IDC_VOLUMECOUNT_STATIC, volumeCountStatic);
	DDX_Control(pDX, IDC_FILECOUNT_EDIT, fileCountEdit);
}

// ---------------------------------------------------------------------------------------------------------------------------------

BOOL	CreateParityDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Setup the listbox with tab stops at 20 chars (for the [N] and [R])

	int	ts = 20;
	fileList.SetTabStops(ts);

	// Setup the slider

	recoverySlider.SetRange(0, 100);
	recoverySlider.SetTicFreq(1);
	recoverySlider.SetPos(theApp.GetProfileInt(_T("Options"), _T("recoveryRatio"), 10));

	// Default the volume count

	calcVolumes();

	// Set proper button states

	enableButtons();

	// Enable drag/drop

	DragAcceptFiles();

	return TRUE;
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	CreateParityDialog::OnDropFiles(HDROP hDropInfo)
{
	unsigned int	count = DragQueryFile(hDropInfo, 0xffffffff, NULL, 0);

	// Build a list of files...

	if (count)
	{
		TCHAR	defdir[MAX_PATH];
		memset(defdir, 0, sizeof(defdir));
		_wgetcwd(defdir, sizeof(defdir)-1);

		for (unsigned int i = 0; i < count; ++i)
		{
			TCHAR	fname[MAX_PATH];
			memset(fname, 0, sizeof(fname));
			if (DragQueryFile(hDropInfo, i, fname, sizeof(fname)-1) > 0)
			{
				fstl::wstring	dir = fname;
				fstl::wstring	name;

				// Strip the directory

				int	idx = dir.rfind(_T("\\"));
				if (idx >= 0 && idx != dir.length() - 1)
				{
					name = dir.substring(idx+1);
					dir.erase(idx);
				}
				else
				{
					name = dir;
					dir = defdir;
				}

				fstl::WStringArray	names;
				names += name;
				addFiles(dir, names);
			}
		}

	}

	CDialog::OnDropFiles(hDropInfo);
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	CreateParityDialog::enableButtons()
{
	clearListButton.EnableWindow(fileList.GetCount() ? TRUE:FALSE);

	BOOL	selection = (fileList.GetSelCount() > 0) ? TRUE:FALSE;

	removeSelectedButton.EnableWindow(selection);
	setRecoverableButton.EnableWindow(selection);
	setNonRecoverableButton.EnableWindow(selection);
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	CreateParityDialog::OnLbnSelchangeFileList()
{
	enableButtons();
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	CreateParityDialog::OnBnClickedOutputdirbrowseButton()
{
	BROWSEINFO	bi;
	memset(&bi, 0, sizeof(bi));

	bi.hwndOwner = GetSafeHwnd();
	bi.lpszTitle = _T("Select output folder");
	bi.ulFlags = BIF_RETURNONLYFSDIRS;

	LPITEMIDLIST	idl = SHBrowseForFolder(&bi);
	if (idl)
	{
		TCHAR	path[MAX_PATH];
		SHGetPathFromIDList(idl, path);
		size_t	plen = wcslen(path);

		// Make sure the path does NOT have a trailing bacsklash

		if (path[plen-1] == _T('\\')) path[plen-1] = 0;

		theApp.WriteProfileString(_T("Options"), _T("lastDirectory"), path);

		// Always show the user a path with the trailing backslash

		fstl::wstring	temp = path;
		temp += _T("\\");
		outputDirectoryEdit.SetWindowText(temp.asArray());
	}
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	CreateParityDialog::OnBnClickedClearlistButton()
{
	while(fileList.GetCount())
	{
		fileList.DeleteString(0);
	}

	// Clear the output dir, base filename and volume counts

	outputDirectoryEdit.SetWindowText(_T(""));
	baseFilenameEdit.SetWindowText(_T(""));

	// Calc our volumes

	calcVolumes();
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	CreateParityDialog::OnBnClickedSetnonrecoverableButton()
{
	if (fileList.GetCount() == 0 || fileList.GetCount() == LB_ERR) return;

	for (unsigned int i = 0; i < (unsigned int) fileList.GetCount(); ++i)
	{
		int	sel = fileList.GetSel(i);
		if (sel == 0 || sel == LB_ERR) continue;

		CString	txt;
		fileList.GetText(i, txt);
		txt.SetAt(1, 'N');
		fileList.DeleteString(i);
		fileList.InsertString(i, txt);
		fileList.SetSel(i);
	}

	// Calc our volumes

	calcVolumes();
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	CreateParityDialog::OnBnClickedSetrecoverableButton()
{
	if (fileList.GetCount() == 0 || fileList.GetCount() == LB_ERR) return;

	for (unsigned int i = 0; i < (unsigned int) fileList.GetCount(); ++i)
	{
		int	sel = fileList.GetSel(i);
		if (sel == 0 || sel == LB_ERR) continue;

		CString	txt;
		fileList.GetText(i, txt);
		txt.SetAt(1, 'R');
		fileList.DeleteString(i);
		fileList.InsertString(i, txt);
		fileList.SetSel(i);
	}

	// Calc our volumes

	calcVolumes();
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	CreateParityDialog::OnBnClickedAddButton()
{
	const int	fnameSize = 8192;
	TCHAR		fname[fnameSize];
	memset(fname, 0, sizeof(fname));

	TCHAR		filters[] =	_T("All files (*.*)\0*.*\0RAR files (*.r*)\0*.r*\0\0");

	OPENFILENAME	of;
	memset(&of, 0, sizeof(OPENFILENAME));

	of.lStructSize  = sizeof(OPENFILENAME);
	of.hwndOwner    = GetSafeHwnd();
	of.lpstrFilter  = filters;
	of.nFilterIndex = 1;
	of.lpstrFile    = fname;
	of.nMaxFile     = fnameSize;
	of.lpstrTitle   = _T("Select files for archive");
	of.Flags        = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_ALLOWMULTISELECT;
	of.lpstrDefExt  = _T("*");

	// Default directory

	fstl::wstring	dir = theApp.GetProfileString(_T("Options"), _T("lastDirectory"), _T(""));
	of.lpstrInitialDir = dir.asArray();

	if (!GetOpenFileName(&of)) return;

	// Get the files

	dir = of.lpstrFile;
	fstl::WStringArray	files;
	TCHAR *		ptr = of.lpstrFile + dir.length() + 1;

	while(ptr && *ptr)
	{
		files += ptr;
		ptr += wcslen(ptr) + 1;
	}

	// One file?

	if (!files.size())
	{
		// Strip dir to just be the directory

		int	idx = dir.rfind(_T("\\"));
		if (idx >= 0 && idx != dir.length() - 1)
		{
			files += dir.substring(idx+1);
			dir.erase(idx);
		}
		else
		{
			TCHAR	defdir[MAX_PATH];
			memset(defdir, 0, sizeof(defdir));
			_wgetcwd(defdir, sizeof(defdir)-1);

			files += dir;
			dir = defdir;
		}
	}

	theApp.WriteProfileString(_T("Options"), _T("lastDirectory"), dir.asArray());

	// Add the files

	addFiles(dir, files);

	// Enable buttons

	enableButtons();
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	CreateParityDialog::OnBnClickedRemoveButton()
{
	if (fileList.GetCount() == 0 || fileList.GetCount() == LB_ERR) return;

	for (unsigned int i = 0; i < (unsigned int) fileList.GetCount(); ++i)
	{
		int	sel = fileList.GetSel(i);
		if (sel == 0 || sel == LB_ERR) continue;
		fileList.DeleteString(i);
		i--;
	}

	// Calc our volumes

	calcVolumes();

	// Wiped the list clean?

	if (!fileList.GetCount()) OnBnClickedClearlistButton();
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	CreateParityDialog::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	calcVolumes();

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	CreateParityDialog::OnBnClickedOk()
{
	CString	tmp;

	baseFilenameEdit.GetWindowText(tmp);
	selectedBase() = (LPCTSTR) tmp;

	outputDirectoryEdit.GetWindowText(tmp);
	selectedDirectory() = (LPCTSTR) tmp;

	// Strip all trailing backslashes from the output directory...

	int	idx = selectedDirectory().findLastNotOf(_T("\\"));
	if (idx != -1 && idx != selectedDirectory().length()-1) selectedDirectory().erase(idx+1);

	selectedRecoverableFiles().erase();
	selectedNonrecoverableFiles().erase();

	unsigned int	largest = 0;
	unsigned int	smallest = 0xffffffff;

	if (fileList.GetCount() > 0 && fileList.GetCount() != LB_ERR)
	{
		for (int i = 0; i < fileList.GetCount(); ++i)
		{
			fileList.GetText(i, tmp);

			fstl::wstring	stmp = tmp;

			// File length... if there is no length, skip it

			unsigned int	len = getFileLength(stmp.substring(4));
			if (!len)
			{
				fstl::wstring	err;
				err += _T("An error has occurred with the following file\n\n     \"");
				err += stmp.substring(4);
				err += _T("\"\n\n");
				err += _T("This file has a reported length of zero-bytes, which\n");
				err += _T("may be caused by an error while trying to access the file\n");
				err += _T("or that the file may actually have no length.\n");
				err += _T("\n");
				err += _T("Either way, this file cannot be added to the PAR set and\n");
				err += _T("must be skipped.\n");
				err += _T("\n");
				err += _T("Would you like to continue anyway?\n");

				if (AfxMessageBox(err.asArray(), MB_YESNO) != IDYES) return;
				continue;
			}

			if (tmp[1] == 'R')
			{
				// Largest && Smallest files

				largest = fstl::max(largest, len);
				smallest = fstl::min(smallest, len);

				selectedRecoverableFiles() += stmp.substring(4);
			}
			else
			{
				selectedNonrecoverableFiles() += stmp.substring(4);
			}
		}
	}

	if (!selectedRecoverableFiles().size() && !selectedNonrecoverableFiles().size())
	{
		AfxMessageBox(_T("No files have been selected. Please select at least one file, or cancel the operation."));
		return;
	}

	// If the smallest file is less than 10K _and_ it's considerably smaller than the largest, then warn the user

	if (smallest < 10*1024 && largest / smallest > 10)
	{
		if (AfxMessageBox(
		_T("***    IMPORTANT    ***    READ ALL OF THIS    ***\n")
		_T("\n")
		_T("The smallest recoverable file is smaller than 10% of the\n")
		_T("largest recoverable file. This can often lead to problems.\n")
		_T("\n")
		_T("If your data set includes informational files that are not\n")
		_T("absolutely necessary for data recovery, they should be marked\n")
		_T("as non-recoverable. These files include files with the\n")
		_T("extensions NFO, DIZ, SFV and TXT (as well as others.)\n")
		_T("Including these files as part of the recoverable set is\n")
		_T("STRONGLY DISCOURAGED because it reduces the effectiveness of\n")
		_T("the parity files.\n")
		_T("\n")
		_T("This is especially true for SFV files that are marked as\n")
		_T("recoverable, as these files are often modified when used,\n")
		_T("which causes them to appear corrupt. This would then require\n")
		_T("the user to download an extra parity file in order to recover\n")
		_T("a corrupt data file.\n")
		_T("\n")
		_T("Would you like to continue anyway?\n"),
		MB_YESNO) != IDYES)
			return;

	}

	// Save this setting

	theApp.WriteProfileInt(_T("Options"), _T("recoveryRatio"), recoverySlider.GetPos());

	OnOK();
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	CreateParityDialog::calcVolumes()
{
	volumeCount() = 0;
	float	percent = static_cast<float>(recoverySlider.GetPos());

	int	recoverableCount = 0;
	if (fileList.GetCount())
	{
		for (int i = 0; i < fileList.GetCount(); ++i)
		{
			CString	str;
			fileList.GetText(i, str);
			if (str[1] == 'R') recoverableCount++;
		}

		float	fVolumeCount = static_cast<float>(recoverableCount) * (percent/100.0f);

		// Round up

		volumeCount() = static_cast<int>(ceil(fVolumeCount));
	}

	TCHAR	disp[1024];
	if (volumeCount())
	{
		swprintf(disp, _T("Ratio set to %d%% - %d parity volume%s will be created"), static_cast<int>(percent), volumeCount(), volumeCount()>1 ? "s":"");
	}
	else
	{
		swprintf(disp, _T("Ratio set to %d%% - Only a single PAR file will be created (no volumes)"), static_cast<int>(percent));
	}
	volumeCountStatic.SetWindowText(disp);

	// Update the count, while we're at it...

	swprintf(disp, _T("%d [R] + %d [N] = %d"), recoverableCount, fileList.GetCount()-recoverableCount, fileList.GetCount());
	fileCountEdit.SetWindowText(disp);
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	CreateParityDialog::addFiles(const fstl::wstring & dir, const fstl::WStringArray & files)
{
	// Populate the list

	for (unsigned int i = 0; i < files.size(); ++i)
	{
		// The filespec

		fstl::wstring	filespec = dir + _T("\\") + files[i];

		// Skip directories

		if (isDirectory(filespec)) continue;

		// The listbox string

		fstl::wstring	str = _T("[R]\t") + filespec;

		// Scan the listbox to see if this string already exists...

		bool	found = false;
		if (fileList.GetCount() > 0 && fileList.GetCount() != LB_ERR)
		{
			for (int j = 0; !found && j < fileList.GetCount(); ++j)
			{
				CString	tmp;
				fileList.GetText(j, tmp);
				fstl::wstring	stmp = tmp;
				if (!stmp.substring(4).ncCompare(str.substring(4))) found = true;
			}
		}

		if (!found) fileList.AddString(str.asArray());
	}

	// If the target directory is empty, populate it

	CString	outdirText;
	outputDirectoryEdit.GetWindowText(outdirText);
	if (!wcslen(outdirText))
	{
		fstl::wstring	tmp = dir + _T("\\");
		outputDirectoryEdit.SetWindowText(tmp.asArray());
	}

	// If the base filename is empty, populate it

	CString	baseText;
	baseFilenameEdit.GetWindowText(baseText);
	if (!wcslen(baseText))
	{
		fstl::wstring	temp = files[0];
		int	idx = temp.rfind(_T("."));
		if (idx >= 0) temp.erase(idx);
		baseFilenameEdit.SetWindowText(temp.asArray());
	}

	// Calc our volumes

	calcVolumes();

	// Deal with button states

        enableButtons();
}

// ---------------------------------------------------------------------------------------------------------------------------------

BOOL	CreateParityDialog::OnHelpInfo(HELPINFO* pHelpInfo)
{
	HelpDialog	dlg;
	dlg.DoModal();
	return TRUE;
}

// ---------------------------------------------------------------------------------------------------------------------------------
// CreateParityDialog.cpp - End of file
// ---------------------------------------------------------------------------------------------------------------------------------
