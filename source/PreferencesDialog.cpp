// ---------------------------------------------------------------------------------------------------------------------------------
//  _____             __                                    _____  _       _                                  
// |  __ \           / _|                                  |  __ \(_)     | |                                 
// | |__) |_ __  ___| |_  ___ _ __  ___ _ __   ___  ___ ___| |  | |_  __ _| | ___   __ _      ___ _ __  _ __  
// |  ___/| '__|/ _ \  _|/ _ \ '__|/ _ \ '_ \ / __|/ _ \ __| |  | | |/ _` | |/ _ \ / _` |    / __| '_ \| '_ \ 
// | |    | |  |  __/ | |  __/ |  |  __/ | | | (__|  __/__ \ |__| | | (_| | | (_) | (_| | _ | (__| |_) | |_) |
// |_|    |_|   \___|_|  \___|_|   \___|_| |_|\___|\___|___/_____/|_|\__,_|_|\___/ \__, |(_) \___| .__/| .__/ 
//                                                                                  __/ |        | |   | |    
//                                                                                 |___/         |_|   |_|    
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

#include "stdafx.h"
#include "FSRaid.h"
#include "HelpDialog.h"
#include "PreferencesDialog.h"

// ---------------------------------------------------------------------------------------------------------------------------------

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// ---------------------------------------------------------------------------------------------------------------------------------

IMPLEMENT_DYNAMIC(PreferencesDialog, CDialog)
BEGIN_MESSAGE_MAP(PreferencesDialog, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	ON_BN_CLICKED(IDC_REPAIRAFTERLOAD, OnBnClickedRepairafterload)
	ON_BN_CLICKED(IDC_CHECKONLOAD, OnBnClickedCheckonload)
	ON_BN_CLICKED(IDC_FLUSH_SATES, OnBnClickedFlushSates)
	ON_WM_HSCROLL()
	ON_WM_HELPINFO()
END_MESSAGE_MAP()

// ---------------------------------------------------------------------------------------------------------------------------------

	PreferencesDialog::PreferencesDialog(CWnd* pParent /*=NULL*/)
	: CDialog(PreferencesDialog::IDD, pParent)
{
}

// ---------------------------------------------------------------------------------------------------------------------------------

	PreferencesDialog::~PreferencesDialog()
{
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	PreferencesDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECKONLOAD, checkOnLoad);
	DDX_Control(pDX, IDC_ASSOCIATEPAR, associatePAR);
	DDX_Control(pDX, IDC_ASSOCIATESFV, associateSFV);
	DDX_Control(pDX, IDC_FIXAFTERCHECK, fixAfterCheck);
	DDX_Control(pDX, IDC_FIXBEFOREREPAIR, fixBeforeRepair);
	DDX_Control(pDX, IDC_REMEMBERSTATE, rememberState);
	DDX_Control(pDX, IDC_REPAIRAFTERLOAD, repairAfterLoad);
	DDX_Control(pDX, IDC_LOADAFTERCREATE, loadAfterCreate);
	DDX_Control(pDX, IDC_MEMORYPERCENT, memoryPercentSlider);
	DDX_Control(pDX, IDC_MEMORYTEXT, memoryPercentText);
	DDX_Control(pDX, IDC_MEMVALUE_TEXT, memValueText);
	DDX_Control(pDX, IDC_NOOVERLAPPINGIO, disableOverlappingIO);
	DDX_Control(pDX, IDC_HIGHCONTRASTCOLORS, highContrastColors);
	DDX_Control(pDX, IDC_DOWNLOADMONITORINTERVAL, downloadMonitorSlider);
	DDX_Control(pDX, IDC_DOWNLOADMONITORINTERVALTEXT, downloadMonitorText);
	DDX_Control(pDX, IDC_SAVEWINDOWPOS, saveWindowPos);
}

// ---------------------------------------------------------------------------------------------------------------------------------

BOOL	PreferencesDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	associateSFV.EnableWindow(FALSE);
	associateSFV.ShowWindow(SW_HIDE);

	memoryPercentSlider.SetRange(1, 20);
	memoryPercentSlider.SetTicFreq(1);

	downloadMonitorSlider.SetRange(1, 60);
	downloadMonitorSlider.SetTicFreq(5);

	checkOnLoad.SetCheck(theApp.GetProfileInt(_T("Options"), _T("checkOnLoad"), 1));
	fixAfterCheck.SetCheck(theApp.GetProfileInt(_T("Options"), _T("fixAfterCheck"), 0));
	fixBeforeRepair.SetCheck(theApp.GetProfileInt(_T("Options"), _T("fixBeforeRepair"), 0));
	rememberState.SetCheck(theApp.GetProfileInt(_T("Options"), _T("rememberStates"), 1));
	repairAfterLoad.SetCheck(theApp.GetProfileInt(_T("Options"), _T("repairAfterLoad"), 0));
	associatePAR.SetCheck(theApp.GetProfileInt(_T("Options"), _T("associatePAR"), 1));
	loadAfterCreate.SetCheck(theApp.GetProfileInt(_T("Options"), _T("loadAfterCreate"), 1));
	memoryPercentSlider.SetPos(theApp.GetProfileInt(_T("Options"), _T("memoryPercent"), 10) / 5);
	downloadMonitorSlider.SetPos(theApp.GetProfileInt(_T("Options"), _T("downloadMonitorInterval"), 10));
	disableOverlappingIO.SetCheck(theApp.GetProfileInt(_T("Options"), _T("disableOverlappingIO"), 0));
	highContrastColors.SetCheck(theApp.GetProfileInt(_T("Options"), _T("highContrastColors"), 0));
	saveWindowPos.SetCheck(theApp.GetProfileInt(_T("Options"), _T("saveWindowPos"), 1));

	// Force an update to our sliders

	OnHScroll(0, 0, 0);

	// Write our preferences out, just in case none exist

	writeSettings();

	return TRUE;
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	PreferencesDialog::OnBnClickedOk()
{
	writeSettings();
	OnOK();
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	PreferencesDialog::OnBnClickedCancel()
{
	OnCancel();
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	PreferencesDialog::writeSettings()
{
	theApp.WriteProfileInt(_T("Options"), _T("checkOnLoad"), checkOnLoad.GetCheck());
	theApp.WriteProfileInt(_T("Options"), _T("fixAfterCheck"), fixAfterCheck.GetCheck());
	theApp.WriteProfileInt(_T("Options"), _T("fixBeforeRepair"), fixBeforeRepair.GetCheck());
	theApp.WriteProfileInt(_T("Options"), _T("rememberStates"), rememberState.GetCheck());
	theApp.WriteProfileInt(_T("Options"), _T("repairAfterLoad"), repairAfterLoad.GetCheck());
	theApp.WriteProfileInt(_T("Options"), _T("associatePAR"), associatePAR.GetCheck());
	theApp.WriteProfileInt(_T("Options"), _T("loadAfterCreate"), loadAfterCreate.GetCheck());
	theApp.WriteProfileInt(_T("Options"), _T("memoryPercent"), memoryPercentSlider.GetPos() * 5);
	theApp.WriteProfileInt(_T("Options"), _T("downloadMonitorInterval"), downloadMonitorSlider.GetPos());
	theApp.WriteProfileInt(_T("Options"), _T("disableOverlappingIO"), disableOverlappingIO.GetCheck() ? 1:0);
	theApp.WriteProfileInt(_T("Options"), _T("highContrastColors"), highContrastColors.GetCheck() ? 1:0);
	theApp.WriteProfileInt(_T("Options"), _T("saveWindowPos"), saveWindowPos.GetCheck() ? 1:0);
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	PreferencesDialog::OnBnClickedRepairafterload()
{
	checkOnLoad.SetCheck(FALSE);
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	PreferencesDialog::OnBnClickedCheckonload()
{
	repairAfterLoad.SetCheck(FALSE);
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	PreferencesDialog::OnBnClickedFlushSates()
{
	wipeRegInfo();
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	PreferencesDialog::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// Memory slider

	unsigned int	memPercent = memoryPercentSlider.GetPos() * 5;
	TCHAR	dsp[90];
	swprintf(dsp, _T("%d%%"), memPercent);
	memoryPercentText.SetWindowText(dsp);

	MEMORYSTATUS	memStat;
	GlobalMemoryStatus(&memStat);

	unsigned int	memTotal = static_cast<unsigned int>(memStat.dwTotalPhys);
	unsigned int	memToUse = memTotal / 100 * memPercent;

	fstl::wstring	memSize = sizeString(memToUse);
	fstl::wstring	totSize = sizeString(memTotal);

	swprintf(dsp, _T("%s will be used (of %s total)"), memSize.asArray(), totSize.asArray());
	memValueText.SetWindowText(dsp);

	if (pScrollBar) CDialog::OnHScroll(nSBCode, nPos, pScrollBar);

	// Download monitor interval slider

	unsigned int	interval = downloadMonitorSlider.GetPos();
	swprintf(dsp, _T("%ds"), interval);
	downloadMonitorText.SetWindowText(dsp);

}

// ---------------------------------------------------------------------------------------------------------------------------------

BOOL PreferencesDialog::OnHelpInfo(HELPINFO* pHelpInfo)
{
	HelpDialog	dlg;
	dlg.DoModal();
	return TRUE;
}

// ---------------------------------------------------------------------------------------------------------------------------------
// PreferencesDialog.cpp - End of file
// ---------------------------------------------------------------------------------------------------------------------------------
