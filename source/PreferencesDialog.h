// ---------------------------------------------------------------------------------------------------------------------------------
//  _____             __                                    _____  _       _                 _     
// |  __ \           / _|                                  |  __ \(_)     | |               | |    
// | |__) |_ __  ___| |_  ___ _ __  ___ _ __   ___  ___ ___| |  | |_  __ _| | ___   __ _    | |__  
// |  ___/| '__|/ _ \  _|/ _ \ '__|/ _ \ '_ \ / __|/ _ \ __| |  | | |/ _` | |/ _ \ / _` |   | '_ \ 
// | |    | |  |  __/ | |  __/ |  |  __/ | | | (__|  __/__ \ |__| | | (_| | | (_) | (_| | _ | | | |
// |_|    |_|   \___|_|  \___|_|   \___|_| |_|\___|\___|___/_____/|_|\__,_|_|\___/ \__, |(_)|_| |_|
//                                                                                  __/ |          
//                                                                                 |___/           
//
// Description:
//
//   Application preferences
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

#ifndef	_H_PREFERENCESDIALOG
#define _H_PREFERENCESDIALOG
#include "afxwin.h"
#include "afxcmn.h"

// ---------------------------------------------------------------------------------------------------------------------------------
// Module setup (required includes, macros, etc.)
// ---------------------------------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------------------------------

class	PreferencesDialog : public CDialog
{
public:
	// Enumerations

		enum		{ IDD = IDD_PREFERENCES_DIALOG };

	// Construction/Destruction

				PreferencesDialog(CWnd* pParent = NULL);
virtual				~PreferencesDialog();

protected:
	// Controls

		CButton 	checkOnLoad;
		CButton 	associatePAR;
		CButton 	associateSFV;
		CButton 	fixAfterCheck;
		CButton 	fixBeforeRepair;
		CButton 	rememberState;
		CButton		repairAfterLoad;
		CButton		loadAfterCreate;
		CButton		highContrastColors;
		CSliderCtrl	memoryPercentSlider;
		CStatic		memoryPercentText;
		CStatic		memValueText;
		CButton		disableOverlappingIO;
		CSliderCtrl	downloadMonitorSlider;
		CStatic		downloadMonitorText;
		CButton		saveWindowPos;

	// Message map & Overrides

virtual		void		DoDataExchange(CDataExchange* pDX);
virtual		BOOL		OnInitDialog();
afx_msg		void 		OnBnClickedOk();
afx_msg		void 		OnBnClickedCancel();
afx_msg		void		OnBnClickedRepairafterload();
afx_msg		void		OnBnClickedCheckonload();
afx_msg		void		OnBnClickedFlushSates();
afx_msg		void		OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
afx_msg		BOOL		OnHelpInfo(HELPINFO* pHelpInfo);
virtual		void		writeSettings();

				DECLARE_DYNAMIC(PreferencesDialog)
				DECLARE_MESSAGE_MAP()
};

#endif // _H_PREFERENCESDIALOG
// ---------------------------------------------------------------------------------------------------------------------------------
// PreferencesDialog.h - End of file
// ---------------------------------------------------------------------------------------------------------------------------------
