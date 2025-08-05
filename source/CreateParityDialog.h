// --------------------------------------------------------------------------------------------------------------------------------
//   _____                 _        _____             _ _         _____  _       _                 _     
//  / ____|               | |      |  __ \           (_) |       |  __ \(_)     | |               | |    
// | |     _ __  ___  __ _| |_  ___| |__) | __ _ _ __ _| |_ _   _| |  | |_  __ _| | ___   __ _    | |__  
// | |    | '__|/ _ \/ _` | __|/ _ \  ___/ / _` | '__| | __| | | | |  | | |/ _` | |/ _ \ / _` |   | '_ \ 
// | |____| |  |  __/ (_| | |_|  __/ |    | (_| | |  | | |_| |_| | |__| | | (_| | | (_) | (_| | _ | | | |
//  \_____|_|   \___|\__,_|\__|\___|_|     \__,_|_|  |_|\__|\__, |_____/|_|\__,_|_|\___/ \__, |(_)|_| |_|
//                                                           __/ |                        __/ |          
//                                                          |___/                        |___/           
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

#ifndef	_H_CREATEPARITYDIALOG
#define _H_CREATEPARITYDIALOG
#include "afxwin.h"
#include "afxcmn.h"

// ---------------------------------------------------------------------------------------------------------------------------------
// Module setup (required includes, macros, etc.)
// ---------------------------------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------------------------------

class	CreateParityDialog : public CDialog
{
public:
	// Enumerations

		enum			{ IDD = IDD_CREATEPAR_DIALOG };

	// Construction/Destruction

					CreateParityDialog(CWnd* pParent = NULL);
virtual					~CreateParityDialog();

	// Acceessors

inline		fstl::WStringArray &	selectedRecoverableFiles()		{return _selectedRecoverableFiles;}
inline	const	fstl::WStringArray &	selectedRecoverableFiles() const	{return _selectedRecoverableFiles;}
inline		fstl::WStringArray &	selectedNonrecoverableFiles()		{return _selectedNonrecoverableFiles;}
inline	const	fstl::WStringArray &	selectedNonrecoverableFiles() const	{return _selectedNonrecoverableFiles;}
inline		fstl::wstring &		selectedBase()				{return _selectedBase;}
inline	const	fstl::wstring &		selectedBase() const			{return _selectedBase;}
inline		fstl::wstring &		selectedDirectory()			{return _selectedDirectory;}
inline	const	fstl::wstring &		selectedDirectory() const		{return _selectedDirectory;}
inline		unsigned int &		volumeCount()				{return _volumeCount;}
inline	const	unsigned int		volumeCount() const			{return _volumeCount;}

private:

		fstl::WStringArray	_selectedRecoverableFiles;
		fstl::WStringArray	_selectedNonrecoverableFiles;
		fstl::wstring		_selectedBase;
		fstl::wstring		_selectedDirectory;
		unsigned int		_volumeCount;

protected:
		CListBox		fileList;
		CButton			removeSelectedButton;
		CButton			setRecoverableButton;
		CButton			setNonRecoverableButton;
		CButton			clearListButton;
		CEdit			baseFilenameEdit;
		CEdit			outputDirectoryEdit;
		CSliderCtrl		recoverySlider;
		CStatic			volumeCountStatic;
		CEdit			fileCountEdit;

virtual		void			enableButtons();
virtual		void			calcVolumes();
virtual		void			DoDataExchange(CDataExchange* pDX);
virtual		BOOL			OnInitDialog();
virtual		void			addFiles(const fstl::wstring & dir, const fstl::WStringArray & files);
afx_msg 	void 			OnBnClickedOutputdirbrowseButton();
afx_msg 	void 			OnBnClickedClearlistButton();
afx_msg 	void 			OnBnClickedSetnonrecoverableButton();
afx_msg 	void 			OnBnClickedSetrecoverableButton();
afx_msg 	void 			OnBnClickedAddButton();
afx_msg 	void 			OnBnClickedRemoveButton();
afx_msg 	void 			OnBnClickedOk();
afx_msg		void			OnLbnSelchangeFileList();
afx_msg		void			OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
afx_msg		void			OnDropFiles(HDROP hDropInfo);
afx_msg		BOOL			OnHelpInfo(HELPINFO* pHelpInfo);

					DECLARE_DYNAMIC(CreateParityDialog)
					DECLARE_MESSAGE_MAP()
};

#endif // _H_CREATEPARITYDIALOG
// ---------------------------------------------------------------------------------------------------------------------------------
// CreateParityDialog.h - End of file
// ---------------------------------------------------------------------------------------------------------------------------------
