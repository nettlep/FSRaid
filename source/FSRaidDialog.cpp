// ---------------------------------------------------------------------------------------------------------------------------------
//  ______  _____ _____        _     _ _____  _       _                                  
// |  ____|/ ____|  __ \      (_)   | |  __ \(_)     | |                                 
// | |__  | (___ | |__) | __ _ _  __| | |  | |_  __ _| | ___   __ _      ___ _ __  _ __  
// |  __|  \___ \|  _  / / _` | |/ _` | |  | | |/ _` | |/ _ \ / _` |    / __| '_ \| '_ \ 
// | |     ____) | | \ \| (_| | | (_| | |__| | | (_| | | (_) | (_| | _ | (__| |_) | |_) |
// |_|    |_____/|_|  \_\\__,_|_|\__,_|_____/|_|\__,_|_|\___/ \__, |(_) \___| .__/| .__/ 
//                                                             __/ |        | |   | |    
//                                                            |___/         |_|   |_|    
//
// Description:
//
//   Main dialog for the program
//
// Notes:
//
//   Best viewed with 8-character tabs and (at least) 132 columns
//
// History:
//
//   12/17/2001 by Paul Nettle: Original creation
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
#include "FSRaidDialog.h"
#include "ParityInfo.h"
#include "EmDeeFive.h"
#include "AboutBox.h"
#include "HelpDialog.h"
#include "PreferencesDialog.h"
#include "CreateParityDialog.h"
#include "RenameConfirmationDialog.h"

// ---------------------------------------------------------------------------------------------------------------------------------

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// ---------------------------------------------------------------------------------------------------------------------------------

#define	XLOG(a)\
{\
	FILE *fp = _wfopen(_T("C:\\fsraid.log.txt"), _T("a"));\
	if (fp)\
	{\
		TCHAR	dsp[4096];\
		swprintf(dsp, _T("%s\n"), a);\
		fprintf(fp, dsp);\
		fclose(fp);\
	}\
}

// ---------------------------------------------------------------------------------------------------------------------------------

static	const unsigned int	startupTimer = 12345;
static	const unsigned int	monitorTimer = startupTimer + 1;

DECLARE_HANDLE(HMONITOR);

typedef struct tagMONITORINFO
{
    DWORD   cbSize;
    RECT    rcMonitor;
    RECT    rcWork;
    DWORD   dwFlags;
} MONITORINFO, *LPMONITORINFO;

static	HMONITOR		(WINAPI* pfnMonitorFromWindow)(HWND, BOOL);
static	BOOL			(WINAPI* pfnGetMonitorInfo)(HMONITOR, LPMONITORINFO);
static	bool			mmInitialized;

// ---------------------------------------------------------------------------------------------------------------------------------

#define	CHECK_CANCEL()\
{\
	allowBackgroundProcessing();\
	if (cancelFlag())\
	{\
		if (AfxMessageBox(_T("Are you sure you wish to cancel?"), MB_YESNO) == IDYES) throw _T("Operation cancelled");\
		stopButton().EnableWindow(TRUE);\
		cancelFlag() = false;\
	}\
}

// ---------------------------------------------------------------------------------------------------------------------------------

static	fstl::wstring	uniqueFilename(const fstl::wstring & dir, const fstl::wstring & postfix)
{
	// Try 100 random numbers

	srand(static_cast<unsigned int>(time(NULL)));

	for (unsigned int i = 0; i < 100; ++i)
	{
		TCHAR	randstr[1024];
		swprintf(randstr, _T("%08x"), (rand() | rand() << 16));
		fstl::wstring	tester = dir + _T("\\") + randstr + _T("_") + postfix;

		// Does this file exist?

		if (doesFileExist(tester)) continue;

		// Nope, return it

		return fstl::wstring(randstr) + _T("_") + postfix;
	}

	return _T("");
}

// ---------------------------------------------------------------------------------------------------------------------------------

static	bool	progCallback(void * userData, const fstl::wstring & displayText, const float percent)
{
static	fstl::wstring	lastDisplayText;
static	float		lastPercent;
static	bool		rangeSet;
static	int		lastIntegerPercent;
static	unsigned int	periodicAllowance;

	// Our dialog object

	if (!userData) return false;
	FSRaidDialog &	object = *reinterpret_cast<FSRaidDialog *>(userData);

	// Pause?

	bool	forceTextUpdate = false;

	if (object.pausedFlag())
	{
		object.progressText().SetWindowText(_T("Paused..."));

		// Wait for them to stop or unpause

		while(object.pausedFlag() && !object.cancelFlag())
		{
			allowBackgroundProcessing();
			Sleep(100);
		}

		// Remove the paused text

		forceTextUpdate = true;
	}

	if (!rangeSet)
	{
		object.progress().SetRange(0, 100);
		rangeSet = true;
	}

	if (fstl::abs(lastPercent - percent) > 0.3f)
	{
		lastPercent = percent;
		int	integerPercent = static_cast<int>(percent + 0.5f);
		if (integerPercent < 0)		integerPercent = 0;
		if (integerPercent > 100)	integerPercent = 100;

		object.progress().SetPos(static_cast<int>(percent));

		if (integerPercent != lastIntegerPercent)
		{
			lastIntegerPercent = integerPercent;
			forceTextUpdate = true;
		}
	}

	if (forceTextUpdate || displayText != lastDisplayText)
	{
		lastDisplayText = displayText;

		TCHAR	dsp[1024];
		if (lastIntegerPercent)
		{
			swprintf(dsp, _T("%d%% - %s"), lastIntegerPercent, displayText.asArray());
		}
		else
		{
			swprintf(dsp, _T("%s"), displayText.asArray());
		}
		object.progressText().SetWindowText(dsp);
	}

	++periodicAllowance;

	if (periodicAllowance >= 16)
	{
		allowBackgroundProcessing();
		periodicAllowance = 0;
	}

	if (object.cancelFlag())
	{
		if (AfxMessageBox(_T("Are you sure you wish to cancel?"), MB_YESNO) != IDYES)
		{
			object.cancelFlag() = false;
			object.stopButton().EnableWindow(TRUE);
		}
		else
		{
			object.pausedFlag() = false;
			object.pauseButton().SetCheck(FALSE);
		}
	}

	return !object.cancelFlag();
}

// ---------------------------------------------------------------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(FSRaidDialog, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONUP()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_LOAD_BUTTON, OnBnClickedLoadButton)
	ON_BN_CLICKED(IDC_CREATE_BUTTON, OnBnClickedCreateButton)
	ON_BN_CLICKED(IDC_CHECK_BUTTON, OnBnClickedCheckButton)
	ON_BN_CLICKED(IDC_CHECKALL_BUTTON, OnBnClickedCheckallButton)
	ON_BN_CLICKED(IDC_REPAIR_BUTTON, OnBnClickedRepairButton)
	ON_BN_CLICKED(IDC_HELP_BUTTON, OnBnClickedHelpButton)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDC_STOP_BUTTON, OnBnClickedStopButton)
	ON_BN_CLICKED(IDC_ABOUT_BUTTON, OnBnClickedAboutButton)
	ON_BN_CLICKED(IDC_PREFS_BUTTON, OnBnClickedPrefsButton)
	ON_BN_CLICKED(IDC_FIXNAMES_BUTTON, OnBnClickedFixnamesButton)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_PAUSE_BUTTON, OnBnClickedPauseButton)
	ON_WM_DROPFILES()
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_BN_CLICKED(IDC_MONITORDOWNLOAD_BUTTON, OnBnClickedMonitordownloadButton)
	ON_WM_CLOSE()
	ON_WM_MOVE()
	ON_WM_MOVING()
	ON_WM_NCLBUTTONDOWN()
	ON_WM_NCLBUTTONUP()
//	ON_WM_CHAR()
ON_WM_HELPINFO()
END_MESSAGE_MAP()

// ---------------------------------------------------------------------------------------------------------------------------------

	FSRaidDialog::FSRaidDialog(CWnd* pParent)
	: CDialog(FSRaidDialog::IDD, pParent), _aboutDialog(NULL), _helpDialog(NULL), _monitorFlag(false), _cancelFlag(false), _pausedFlag(false), _silent(false), _busy(false), _toolTip(NULL), resizeDistance(0)
{
	potentialPercent = 0;
	validPercent = 0;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

// ---------------------------------------------------------------------------------------------------------------------------------

	FSRaidDialog::~FSRaidDialog()
{
	delete aboutDialog();
	delete helpDialog();
	delete toolTip();
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	FSRaidDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BASE_EDIT, baseEdit);
	DDX_Control(pDX, IDC_PROGRESS_SEPARATOR, progressSeparator);
	DDX_Control(pDX, IDC_PROGRESS, _progress);
	DDX_Control(pDX, IDC_PROGRESS_TEXT, _progressText);
	DDX_Control(pDX, IDC_LOAD_BUTTON, loadButton);
	DDX_Control(pDX, IDC_CREATE_BUTTON, createButton);
	DDX_Control(pDX, IDC_CHECK_BUTTON, checkButton);
	DDX_Control(pDX, IDC_CHECKALL_BUTTON, checkAllButton);
	DDX_Control(pDX, IDC_REPAIR_BUTTON, repairButton);
	DDX_Control(pDX, IDC_HELP_BUTTON, helpButton);
	DDX_Control(pDX, IDOK, closeButton);
	DDX_Control(pDX, IDC_STOP_BUTTON, _stopButton);
	DDX_Control(pDX, IDC_ABOUT_BUTTON, aboutButton);
	DDX_Control(pDX, IDC_PREFS_BUTTON, prefsButton);
	DDX_Control(pDX, IDC_DATAFILE_MAP_TEXT, dataFileMapText);
	DDX_Control(pDX, IDC_DATAFILE_MAP, dataFileMapFrame);
	DDX_Control(pDX, IDC_PARITYFILE_MAP_TEXT, parityFileMapText);
	DDX_Control(pDX, IDC_PARITYFILE_MAP, parityFileMapFrame);
	DDX_Control(pDX, IDC_FIXNAMES_BUTTON, fixNamesButton);
	DDX_Control(pDX, IDC_STATUS_EDIT, archiveStatusEdit);
	DDX_Control(pDX, IDC_PAUSE_BUTTON, _pauseButton);
	DDX_Control(pDX, IDC_LEGEND_UNKNOWN, unknownLegend);
	DDX_Control(pDX, IDC_LEGEND_VALID, validLegend);
	DDX_Control(pDX, IDC_LEGEND_CORRUPT, corruptLegend);
	DDX_Control(pDX, IDC_LEGEND_MISSING, missingLegend);
	DDX_Control(pDX, IDC_LEGEND_MISNAMED, misnamedLegend);
	DDX_Control(pDX, IDC_LEGEND_ERROR, errorLegend);
	DDX_Control(pDX, IDC_INFO_BORDER, infoBorderFrame);
	DDX_Control(pDX, IDC_VALID_LEGEND_TEXT, validLegendText);
	DDX_Control(pDX, IDC_CORRUPT_LEGEND_TEXT, corruptLegendText);
	DDX_Control(pDX, IDC_ERROR_LEGEND_TEXT, errorLegendText);
	DDX_Control(pDX, IDC_MISSING_LEGEND_TEXT, missingLegendText);
	DDX_Control(pDX, IDC_MISNAMED_LEGEND_TEXT, misnamedLegendText);
	DDX_Control(pDX, IDC_UNKNOWN_LEGEND_TEXT, unknownLegendText);
	DDX_Control(pDX, IDC_MONITORDOWNLOAD_BUTTON, downloadMonitorButton);
}

// ---------------------------------------------------------------------------------------------------------------------------------

BOOL	FSRaidDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// Initial dialog rect (for resizing it to expose the progress bar)

	GetWindowRect(initialDialogRect);
	initialDialogRect.OffsetRect(-initialDialogRect.left, initialDialogRect.top);

	// Remember our window position

	if (theApp.GetProfileInt(_T("Options"), _T("saveWindowPos"), 1))
	{
		windowX = theApp.GetProfileInt(_T("Options"), _T("windowX"), 12345);
		windowY = theApp.GetProfileInt(_T("Options"), _T("windowY"), 12345);
		windowW = theApp.GetProfileInt(_T("Options"), _T("windowW"), 12345);

		if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
		{
			AfxMessageBox(_T("You were holding down the SHIFT key, so FSRaid\nhas reset your window position for you."));
			windowX = 12345;
			windowY = 12345;
			windowW = 12345;
		}

		if (windowX != 12345 && windowY != 12345)
		{
			CRect	r = initialDialogRect;
			r.left = windowX;
			r.top = windowY;
			r.right = r.left + windowW;
			r.bottom = r.top + initialDialogRect.Height();
			MoveWindow(r);
			initialDialogRect = r;
		}
	}

	// Accepting files...

	DragAcceptFiles();

	// Default display

	baseEdit.SetWindowText(_T("No file loaded"));

	// Determine the resize distance for displaying the progress bar...
	{
		CRect	r;
		progressSeparator.GetWindowRect(r);
		resizeDistance = initialDialogRect.bottom - (r.top - 1);
	}

	resizeWindow(false);

	// Tooltips

	toolTip() = new CToolTipCtrl;

	if (toolTip() && toolTip()->Create(this))
	{
		toolTip()->AddTool(&baseEdit,_T("Shows the currently loaded parity archive"));
		toolTip()->AddTool(&loadButton,_T("Load a new parity archive"));
		toolTip()->AddTool(&createButton,_T("Create a new parity archive from a set of data files"));
		toolTip()->AddTool(&checkButton,_T("Validate all files (does not re-validate valid files -- use \"Check all\")"));
		toolTip()->AddTool(&checkAllButton,_T("Resets the status of all files to unknown and re-validates everything"));
		toolTip()->AddTool(&repairButton,_T("Repairs invalid data files"));
		toolTip()->AddTool(&fixNamesButton,_T("Scans the entire directory for any data or parity files with the wrong names and corrects them"));
		toolTip()->AddTool(&helpButton,_T("Got questions?"));
		toolTip()->AddTool(&closeButton,_T("Exit the application"));
		toolTip()->AddTool(&aboutButton,_T("Information about FSRaid"));
		toolTip()->AddTool(&prefsButton,_T("Set defaults and other options for the software"));
		toolTip()->AddTool(&dataFileMapText,_T("Hover your mouse over the data file map and you'll see information displayed here"));
		toolTip()->AddTool(&parityFileMapText,_T("Hover your mouse over the parity file map and you'll see information displayed here"));
		toolTip()->AddTool(&archiveStatusEdit,_T("Shows the current status of the archive"));
		toolTip()->AddTool(&parityFileMapFrame,_T("This is a map of your parity files, right-click for file-specific actions"));
		toolTip()->AddTool(&dataFileMapFrame,_T("This is a map of your parity files, right-click for file-specific actions"));
		toolTip()->AddTool(&downloadMonitorButton,_T("Track the progress of a download on this dataset"));
		toolTip()->Activate(TRUE);
	}

	// Automatically take them to the options?

	SetTimer(startupTimer, 0, NULL);

	return TRUE;
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	FSRaidDialog::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	if (resizeDistance)
	{
		lpMMI->ptMinTrackSize.x = 550;

		if (busy())
		{
			lpMMI->ptMinTrackSize.y = initialDialogRect.Height();
			lpMMI->ptMaxTrackSize.y = initialDialogRect.Height();
		}
		else
		{
			lpMMI->ptMinTrackSize.y = initialDialogRect.Height() - resizeDistance;
			lpMMI->ptMaxTrackSize.y = initialDialogRect.Height() - resizeDistance;
		}
	}

	CDialog::OnGetMinMaxInfo(lpMMI);
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	FSRaidDialog::OnClose()
{
	if (busy())
	{
		AfxMessageBox(_T("Please cancel your current operation before attempting to close"));
		return;
	}

	OnBnClickedOk();
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	FSRaidDialog::OnMove(int x, int y)
{
	CDialog::OnMove(x, y);

	// If not minimized, track position

	if (!IsIconic())
	{
		CRect	r;
		GetWindowRect(r);
		windowX = r.left;
		windowY = r.top;
	}
}

// ---------------------------------------------------------------------------------------------------------------------------------

void FSRaidDialog::OnNcLButtonDown(UINT nHitTest, CPoint point)
{
	dragPos = point;
	CRect	wr;
	GetWindowRect(wr);
	dragCorner = wr.TopLeft();

	CDialog::OnNcLButtonDown(nHitTest, point);
}

// ---------------------------------------------------------------------------------------------------------------------------------

void FSRaidDialog::OnNcLButtonUp(UINT nHitTest, CPoint point)
{
	CDialog::OnNcLButtonUp(nHitTest, point);
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	FSRaidDialog::OnMoving(UINT fwSide, LPRECT pRect)
{
	CDialog::OnMoving(fwSide, pRect);

	if (!mmInitialized)
	{
		mmInitialized = true;

		HMODULE hUser32 = GetModuleHandle(TEXT("USER32"));
		if (hUser32)
		{
			*(FARPROC*)&pfnMonitorFromWindow = GetProcAddress(hUser32,"MonitorFromWindow");
			*(FARPROC*)&pfnGetMonitorInfo = GetProcAddress(hUser32,"GetMonitorInfoA");
		}
	}

	// Do the dragging

	if (theApp.GetProfileInt(_T("Options"), _T("snapToDesktopEdges"), 1))
	{
		int	w = pRect->right - pRect->left;
		int	h = initialDialogRect.Height();

		CRect	desktopRect;
		if (pfnMonitorFromWindow && pfnGetMonitorInfo)
		{
			HMONITOR	hm = pfnMonitorFromWindow(GetSafeHwnd(), 2 /*MONITOR_DEFAULTTONEAREST*/);
			MONITORINFO mi;
			mi.cbSize = sizeof(mi);
			pfnGetMonitorInfo(hm, &mi);
			desktopRect = mi.rcWork;
		}
		else
		{
			CWnd	*dw = GetDesktopWindow();
			dw->GetWindowRect(desktopRect);
		}

		POINT	p;
		GetCursorPos(&p);
		pRect->left = dragCorner.x - dragPos.x + p.x;
		pRect->top = dragCorner.y - dragPos.y + p.y;
		pRect->right = pRect->left + w;
		pRect->bottom = pRect->top + h;

		// Before we move it, let's snap it to the desktop rect

		const	unsigned int	snapDistance = 10;
		if (fstl::abs(pRect->top - desktopRect.top) < snapDistance)
		{
			pRect->top = desktopRect.top;
			pRect->bottom = pRect->top + h;
		}

		if (fstl::abs(pRect->left - desktopRect.left) < snapDistance)
		{
			pRect->left = desktopRect.left;
			pRect->right = pRect->left + w;
		}

		if (fstl::abs(pRect->bottom - desktopRect.bottom) < snapDistance)
		{
			pRect->bottom = desktopRect.bottom;
			pRect->top = pRect->bottom - h;
		}

		if (fstl::abs(pRect->right - desktopRect.right) < snapDistance)
		{
			pRect->right = desktopRect.right;
			pRect->left = pRect->right - w;
		}
	}
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	FSRaidDialog::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	// Save the width

	if (cx)
	{
		CRect	wr;
		GetWindowRect(wr);
		windowW = wr.Width();
	}

	// Don't try to resize when there's nothing there...

	if (!cx || !cy || !resizeDistance) return;

	// Client rect

	CRect	cr;
	GetClientRect(cr);

	// Border...

	cr.left += 11;
	cr.top += 11;
	cr.right -= 11;
	cr.bottom -= 11;

	CRect	r;

	// Edit boxes and maps

	baseEdit.GetWindowRect(r);
	ScreenToClient(r);
	r.right = cr.right;
	baseEdit.MoveWindow(r);

	archiveStatusEdit.GetWindowRect(r);
	ScreenToClient(r);
	r.right = cr.right;
	archiveStatusEdit.MoveWindow(r, FALSE);

	dataFileMapText.GetWindowRect(r);
	ScreenToClient(r);
	r.right = cr.right;
	dataFileMapText.MoveWindow(r, FALSE);

	parityFileMapText.GetWindowRect(r);
	ScreenToClient(r);
	r.right = cr.right;
	parityFileMapText.MoveWindow(r, FALSE);

	dataFileMapFrame.GetWindowRect(r);
	ScreenToClient(r);
	r.right = cr.right;
	dataFileMapFrame.MoveWindow(r, FALSE);

	parityFileMapFrame.GetWindowRect(r);
	ScreenToClient(r);
	r.right = cr.right;
	parityFileMapFrame.MoveWindow(r, FALSE);

	infoBorderFrame.GetWindowRect(r);
	ScreenToClient(r);
	r.right = cr.right;
	infoBorderFrame.MoveWindow(r, FALSE);

	// Legend text labels

	validLegendText.GetWindowRect(r);
	ScreenToClient(r);
	r.left = cr.right - 151;
	r.right = r.left + 58;
	validLegendText.MoveWindow(r, FALSE);

	unknownLegendText.GetWindowRect(r);
	ScreenToClient(r);
	r.left = cr.right - 151;
	r.right = r.left + 58;
	unknownLegendText.MoveWindow(r, FALSE);

	missingLegendText.GetWindowRect(r);
	ScreenToClient(r);
	r.left = cr.right - 151;
	r.right = r.left + 58;
	missingLegendText.MoveWindow(r, FALSE);

	corruptLegendText.GetWindowRect(r);
	ScreenToClient(r);
	r.left = cr.right - 66;
	r.right = r.left + 58;
	corruptLegendText.MoveWindow(r, FALSE);

	errorLegendText.GetWindowRect(r);
	ScreenToClient(r);
	r.left = cr.right - 66;
	r.right = r.left + 58;
	errorLegendText.MoveWindow(r, FALSE);

	misnamedLegendText.GetWindowRect(r);
	ScreenToClient(r);
	r.left = cr.right - 66;
	r.right = r.left + 58;
	misnamedLegendText.MoveWindow(r, FALSE);

	// Legend color boxes

	validLegend.GetWindowRect(r);
	ScreenToClient(r);
	r.left = cr.right - 172;
	r.right = r.left + 15;
	validLegend.MoveWindow(r, FALSE);

	unknownLegend.GetWindowRect(r);
	ScreenToClient(r);
	r.left = cr.right - 172;
	r.right = r.left + 15;
	unknownLegend.MoveWindow(r, FALSE);

	missingLegend.GetWindowRect(r);
	ScreenToClient(r);
	r.left = cr.right - 172;
	r.right = r.left + 15;
	missingLegend.MoveWindow(r, FALSE);

	corruptLegend.GetWindowRect(r);
	ScreenToClient(r);
	r.left = cr.right - 87;
	r.right = r.left + 15;
	corruptLegend.MoveWindow(r, FALSE);

	errorLegend.GetWindowRect(r);
	ScreenToClient(r);
	r.left = cr.right - 87;
	r.right = r.left + 15;
	errorLegend.MoveWindow(r, FALSE);

	misnamedLegend.GetWindowRect(r);
	ScreenToClient(r);
	r.left = cr.right - 87;
	r.right = r.left + 15;
	misnamedLegend.MoveWindow(r, FALSE);

	// Two far-right buttons (Prefs & Close)

	prefsButton.GetWindowRect(r);
	ScreenToClient(r);
	r.left = cr.right - 77;
	r.right = r.left + 75;
	prefsButton.MoveWindow(r, FALSE);

	closeButton.GetWindowRect(r);
	ScreenToClient(r);
	r.left = cr.right - 77;
	r.right = r.left + 75;
	closeButton.MoveWindow(r, FALSE);

	// Progress area stuff (pause, stop, progress bar & separator bar)

	progressSeparator.GetWindowRect(r);
	ScreenToClient(r);
	r.right = cr.right;
	progressSeparator.MoveWindow(r, FALSE);

	pauseButton().GetWindowRect(r);
	ScreenToClient(r);
	r.left = cr.right - 94;
	r.right = r.left + 45;
	pauseButton().MoveWindow(r, FALSE);

	stopButton().GetWindowRect(r);
	ScreenToClient(r);
	r.left = cr.right - 45;
	r.right = r.left + 45;
	stopButton().MoveWindow(r, FALSE);

	progress().GetWindowRect(r);
	ScreenToClient(r);
	r.right = cr.right - 101;
	progress().MoveWindow(r, FALSE);

	// Update the window

	RedrawWindow();
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	FSRaidDialog::OnTimer(UINT nIDEvent)
{
	if (nIDEvent == startupTimer)
	{
		KillTimer(nIDEvent);

		// Supposed to offer options?

		if (!theApp.GetProfileInt(_T("Options"), _T("optionsSet"), 0))
		{
			AfxMessageBox(	_T("Since you haven't set your preferences yet, you will\n")
					_T("be taken to the preferences dialog. This will only\n")
					_T("happen the first time you execute the software.\n")
					_T("\n")
					_T("You may also visit the preferences dialog by clicking\n")
					_T("the [Prefs] button."));

			PreferencesDialog	dlg;
			dlg.DoModal();
			theApp.WriteProfileInt(_T("Options"), _T("optionsSet"), 1);
		}

		// Set file associations?

		if (theApp.GetProfileInt(_T("Options"), _T("associatePAR"), 1) && !checkFileAssociations())
		{
			if (AfxMessageBox(	_T("Your preferences have been set to associate PAR files with FSRaid,\n")
						_T("yet this is currently not the case.\n")
						_T("\n")
						_T("Would you like to associate PAR files with FSRaid now?\n"), MB_YESNO) != IDYES)
			{
				return;
			}

			setFileAssociations();
		}

		// Autoload?

		if (startupFile().length())
		{
			OnBnClickedLoadButton();
		}
	}

	if (nIDEvent == monitorTimer)
	{
		// Monitor that download...

		monitorDownload();
	}

	CDialog::OnTimer(nIDEvent);
}

// ---------------------------------------------------------------------------------------------------------------------------------

BOOL	FSRaidDialog::PreTranslateMessage(MSG* pMsg)
{
	if (NULL != toolTip()) toolTip()->RelayEvent(pMsg);

	return CDialog::PreTranslateMessage(pMsg);
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	FSRaidDialog::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CPaintDC dc(this);
		drawMaps(dc);
		if (monitorFlag()) drawMonitorProgress();
	}
}

// ---------------------------------------------------------------------------------------------------------------------------------

HCURSOR	FSRaidDialog::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	FSRaidDialog::OnMouseMove(UINT nFlags, CPoint point)
{
	int	index;
	if (dataFileIndexFromMousePoint(point, index))
	{
		fstl::wstring	text = parityInfo().dataFiles()[index].fileName();
		text += _T(" -- [") + sizeString(parityInfo().dataFiles()[index].fileSize()) + _T("]");
		text += _T(" -- (") + parityInfo().dataFiles()[index].statusString() + _T(")");
		if (!parityInfo().dataFiles()[index].recoverable()) text += _T(" -- not recoverable");

		dataFileMapText.SetWindowText(text.asArray());
	}
	else if (parityFileIndexFromMousePoint(point, index))
	{
		fstl::wstring	text = parityInfo().parityFiles()[index].fileName();
		text += _T(" -- (") + parityInfo().parityFiles()[index].statusString() + _T(") -- volume #") + fstl::wstring(parityInfo().parityFiles()[index].volumeNumber());

		parityFileMapText.SetWindowText(text.asArray());
	}

	CDialog::OnMouseMove(nFlags, point);
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	FSRaidDialog::OnRButtonUp(UINT nFlags, CPoint point)
{
	int	disabledFlags = MF_GRAYED|MF_DISABLED;
	int	busyDisabledFlags = busy() ? disabledFlags:0;

	int	index;
	if (dataFileIndexFromMousePoint(point, index))
	{
		DataFile &	df = parityInfo().dataFiles()[index];

		CMenu	menu;
		menu.CreatePopupMenu();

		contextIndex() = index;
		menu.AppendMenu(MF_STRING|disabledFlags, -1, df.fileName().asArray());
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING|busyDisabledFlags, MENU_CHECK_DATA_FILE, _T("Check this file"));

		unsigned int	valid, corrupt, misnamed, missing, unknown, error, needed, recoverable, validRecoverable;
		calcStats(valid, corrupt, misnamed, missing, unknown, error, needed, recoverable, validRecoverable);
		int	disabled = 0;
		if (needed || df.status() == DataFile::Valid || df.recoverable() == false) disabled = disabledFlags;
		menu.AppendMenu(MF_STRING|busyDisabledFlags|disabled, MENU_REPAIR_DATA_FILE, _T("Repair this file"));

		menu.AppendMenu(MF_STRING, MENU_COPY_DATA_FILE, _T("Copy filename to clipboard"));
		menu.AppendMenu(MF_SEPARATOR);

		disabled = 0;
		if (df.status() == DataFile::Missing) disabled = disabledFlags;
		menu.AppendMenu(MF_STRING|busyDisabledFlags|disabled, MENU_DELETE_DATA_FILE, _T("Delete this file"));

		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING, MENU_CANCEL, _T("Cancel"));

		POINT	loc;
		loc.x = point.x;
		loc.y = point.y;
		MapWindowPoints(NULL, &loc, 1);
		SetForegroundWindow();
		menu.TrackPopupMenu(TPM_CENTERALIGN, loc.x, loc.y, this, NULL);
		PostMessage(WM_NULL, 0, 0);

		menu.DestroyMenu();
	}

	if (parityFileIndexFromMousePoint(point, index))
	{
		ParityFile &	pf = parityInfo().parityFiles()[index];

		CMenu	menu;
		menu.CreatePopupMenu();

		contextIndex() = index;
		menu.AppendMenu(MF_STRING|MF_GRAYED|MF_DISABLED, -1, pf.fileName().asArray());

		menu.AppendMenu(MF_SEPARATOR);

		menu.AppendMenu(MF_STRING|busyDisabledFlags, MENU_CHECK_PARITY_FILE, _T("Check this file"));
//		menu.AppendMenu(MF_STRING, MENU_REPAIR_PARITY_FILE, _T("Repair this file"));
		menu.AppendMenu(MF_STRING, MENU_COPY_PARITY_FILE, _T("Copy filename to clipboard"));

		menu.AppendMenu(MF_SEPARATOR);

		int	disabled = 0;
		if (pf.status() == ParityFile::Missing) disabled = disabledFlags;
		menu.AppendMenu(MF_STRING|busyDisabledFlags|disabled, MENU_DELETE_PARITY_FILE, _T("Delete this file"));

		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING, MENU_CANCEL, _T("Cancel"));

		POINT	loc;
		loc.x = point.x;
		loc.y = point.y;
		MapWindowPoints(NULL, &loc, 1);
		SetForegroundWindow();
		menu.TrackPopupMenu(TPM_CENTERALIGN, loc.x, loc.y, this, NULL);
		PostMessage(WM_NULL, 0, 0);

		menu.DestroyMenu();
	}


	CDialog::OnRButtonUp(nFlags, point);
}

// ---------------------------------------------------------------------------------------------------------------------------------

BOOL	FSRaidDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (!lParam)
	{
		// DATA FILES

		if (wParam == MENU_CHECK_DATA_FILE)
		{
			resizeWindow(true);
			checkDataFile(contextIndex(), 1, 0);

			// If they cancelled on this file, set it to unknown

			if (cancelFlag())
			{
				parityInfo().dataFiles()[contextIndex()].status() = DataFile::Unknown;
				parityInfo().dataFiles()[contextIndex()].statusString() = _T("Unknown - user cancelled during validation");
			}

			resizeWindow(false);
			saveStates(parityInfo().setHash(), parityInfo().parityFiles(), parityInfo().dataFiles());
		}
		else if (wParam == MENU_REPAIR_DATA_FILE)
		{
			resizeWindow(true);
			progressText().SetWindowText(_T("Repairing data file..."));
			if (repairDataFile(contextIndex()))
			{
				progressText().SetWindowText(_T("Checking data file..."));
				checkDataFile(contextIndex(), 1, 0);
			}

			resizeWindow(false);

			MessageBeep(MB_ICONASTERISK);
			saveStates(parityInfo().setHash(), parityInfo().parityFiles(), parityInfo().dataFiles());
		}
		else if (wParam == MENU_COPY_DATA_FILE)
		{
			copyStringToClipboard(parityInfo().dataFiles()[contextIndex()].fileName());
		}
		else if (wParam == MENU_DELETE_DATA_FILE)
		{
			fstl::wstring	messageString = _T("Are you sure you wish to delete this file?\n\n") + parityInfo().dataFiles()[contextIndex()].fileName();
			if (AfxMessageBox(messageString.asArray(), MB_YESNO) == IDYES)
			{
				fstl::wstring	filespec = parityInfo().dataFiles()[contextIndex()].filespec();
				if (_wunlink(filespec.asArray()) == -1)
				{
					fstl::wstring	err = fstl::wstring(_T("An error occurred while trying to delete the file...\n\n")) + _wcserror(errno);
					AfxMessageBox(err.asArray());
				}
				else
				{
					parityInfo().dataFiles()[contextIndex()].status() = DataFile::FileStatus::Missing;
					parityInfo().dataFiles()[contextIndex()].statusString() = _T("Deleted");
					drawMaps();
				}
				saveStates(parityInfo().setHash(), parityInfo().parityFiles(), parityInfo().dataFiles());
			}
		}

		// PARITY FILES

		if (wParam == MENU_CHECK_PARITY_FILE)
		{
			resizeWindow(true);
			checkParityFile(contextIndex(), 1, 0);

			// If they cancelled on this file, set it to unknown

			if (cancelFlag())
			{
				parityInfo().parityFiles()[contextIndex()].status() = ParityFile::Unknown;
				parityInfo().parityFiles()[contextIndex()].statusString() = _T("Unknown - user cancelled during validation");
			}

			resizeWindow(false);
			saveStates(parityInfo().setHash(), parityInfo().parityFiles(), parityInfo().dataFiles());
		}
		else if (wParam == MENU_REPAIR_PARITY_FILE)
		{
		}
		else if (wParam == MENU_COPY_PARITY_FILE)
		{
			copyStringToClipboard(parityInfo().parityFiles()[contextIndex()].fileName());
		}
		else if (wParam == MENU_DELETE_PARITY_FILE)
		{
			fstl::wstring	messageString = _T("Are you sure you wish to delete this file?\n\n") + parityInfo().parityFiles()[contextIndex()].fileName();
			if (AfxMessageBox(messageString.asArray(), MB_YESNO) == IDYES)
			{
				fstl::wstring	filespec = parityInfo().parityFiles()[contextIndex()].filespec();
				if (_wunlink(filespec.asArray()) == -1)
				{
					fstl::wstring	err = fstl::wstring(_T("An error occurred while trying to delete the file...\n\n")) + _wcserror(errno);
					AfxMessageBox(err.asArray());
				}
				else
				{
					parityInfo().parityFiles()[contextIndex()].status() = ParityFile::Missing;
					parityInfo().parityFiles()[contextIndex()].statusString() = _T("Deleted");
					drawMaps();
				}
				saveStates(parityInfo().setHash(), parityInfo().parityFiles(), parityInfo().dataFiles());
			}
		}
	}

	return CDialog::OnCommand(wParam, lParam);
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	FSRaidDialog::OnBnClickedMonitordownloadButton()
{
	// Disable the pause button and show them the progress bar

	resizeWindow();
	progress().ShowWindow(SW_HIDE);

	// Start the monitor...

	monitorFlag() = true;
	SetTimer(monitorTimer, 0, NULL);
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	FSRaidDialog::OnBnClickedLoadButton()
{
	// If this isn't an auto-start, then ask them for a file

	fstl::wstring	filename;
	if (!startupFile().length())
	{
		const int	fnameSize = 8192;
		TCHAR		fname[fnameSize];
		memset(fname, 0, sizeof(fname));

		TCHAR		filters[] =	_T("PAR files (par, p01, p02...)\0")
						_T("*.par;")
						_T("*.p01;*.p02;*.p03;*.p04;*.p05;*.p06;*.p07;*.p08;*.p09;*.p10;")
						_T("*.p11;*.p12;*.p13;*.p14;*.p15;*.p16;*.p17;*.p18;*.p19;*.p20;")
						_T("*.p21;*.p22;*.p23;*.p24;*.p25;*.p26;*.p27;*.p28;*.p29;*.p30\0")
						_T("All files (*.*)\0")
						_T("*.*\0")
						_T("\0\0");

		OPENFILENAME	of;
		memset(&of, 0, sizeof(OPENFILENAME));

		of.lStructSize  = sizeof(OPENFILENAME);
		of.hwndOwner    = GetSafeHwnd();
		of.lpstrFilter  = filters;
		of.nFilterIndex = 1;
		of.lpstrFile    = fname;
		of.nMaxFile     = fnameSize;
		of.lpstrTitle   = _T("Choose a PAR file");
		of.Flags        = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_EXPLORER;
		of.lpstrDefExt  = _T("par");

		// Default directory

		fstl::wstring	dir = theApp.GetProfileString(_T("Options"), _T("lastDirectory"), _T(""));
		of.lpstrInitialDir = dir.asArray();

		if (!GetOpenFileName(&of)) return;

		filename = of.lpstrFile;

		// Strip dir to just be the directory

		dir = filename;
		int	idx = dir.rfind(_T("\\"));
		if (idx >= 0)	dir.erase(idx);
		else		dir.erase();

		theApp.WriteProfileString(_T("Options"), _T("lastDirectory"), dir.asArray());
	}
	else
	{
		filename = startupFile();
		startupFile() = _T("");

		// Strip dir to just be the directory

		fstl::wstring	dir = filename;
		int	idx = dir.rfind(_T("\\"));
		if (idx >= 0)	dir.erase(idx);
		else		dir.erase();

		theApp.WriteProfileString(_T("Options"), _T("lastDirectory"), dir.asArray());
	}

	// Tell the user we're busy...

	resizeWindow(true);
	stopButton().EnableWindow(FALSE);
	pauseButton().EnableWindow(FALSE);
	progressText().SetWindowText(_T("Loading..."));

	// Load the PAR file

	bool	loadOK = parityInfo().loadParFile(filename);
	drawMaps();

	if (!loadOK)
	{
		AfxMessageBox(_T("Unable to load file"));
		resizeWindow(false);
		return;
	}

	// Put the par filename in the dialog

	fstl::wstring	parFilename = filename;
	int	pathEnd = parFilename.rfind(_T("\\"));
	if (pathEnd >= 0) parFilename.erase(0, pathEnd+1);
	parFilename += _T(" (created by ") + parityInfo().createdByString() + _T(")");
	baseEdit.SetWindowText(parFilename.asArray());

	// Load the existing states, if there are any...

	loadStates(parityInfo().setHash(), parityInfo().parityFiles(), parityInfo().dataFiles());

	// Scan for missing

	scanForMissingFiles();

	// Automatically repair?

	if (theApp.GetProfileInt(_T("Options"), _T("repairAfterLoad"), 0))
	{
		// If the repair won't automatically check, then we'll do that here, since the files need to be checked before
		// a repair and we just loaded them, so they haven't been checked yet.

		if (!theApp.GetProfileInt(_T("Options"), _T("fixBeforeRepair"), 0))
		{
			silent() = true;
			OnBnClickedCheckButton();
			silent() = false;
		}

		// Automatically repair

		if (shouldRepair())
		{
			OnBnClickedRepairButton();
		}
	}

	// Not repairing, automatically checking?

	else if (theApp.GetProfileInt(_T("Options"), _T("checkOnLoad"), 1))
	{
		OnBnClickedCheckButton();
	}

	resizeWindow(false);
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	FSRaidDialog::OnBnClickedCreateButton()
{
	// Get the creation info

	CreateParityDialog	dlg;
	if (dlg.DoModal() != IDOK) return;

	// Show them the progress bar

	resizeWindow();

	try
	{
		// Create a series of parity files

		ParityFileArray	parityVolumes;
		for (unsigned int i = 0; i <= dlg.volumeCount(); ++i)
		{
			ParityFile	pf;
			pf.volumeNumber() = i;
			pf.filePath() = dlg.selectedDirectory();
			pf.fileName() = dlg.selectedBase() + _T(".");
			pf.status() = ParityFile::Valid;
			pf.statusString() = _T("Just created");
			
			// What's this sucker's extension? (p01-p99, q00-q99, etc.)

			if (i == 0)
			{
				pf.fileName() += _T("par");
			}
			else
			{
				TCHAR	dsp[10];
				swprintf(dsp, _T("%c%02d"), (i / 100) + _T('p'), i % 100);
				pf.fileName() += dsp;
			}

			// Add the volume to the list

			parityVolumes += pf;
		}

		// Create a series of data files

		DataFileArray	dataVolumes;

		// Non-recoverable files first...

		for (unsigned int i = 0; i < dlg.selectedNonrecoverableFiles().size(); ++i)
		{
			DataFile	df;

			// Strip out the path & name

			fstl::wstring	filespec = dlg.selectedNonrecoverableFiles()[i];
			int		idx = filespec.rfind(_T("\\"));
			if (idx >= 0)
			{
				df.fileName() = filespec.substring(idx+1);
				df.filePath() = filespec.substring(0, idx);
			}
			else
			{
				df.fileName() = filespec;
			}

			df.fileSize() = getFileLength(df.filespec());
			df.recoverable() = false;
			df.status() = DataFile::Valid;
			df.statusString() = _T("Just created");

			// Add the data volume

			dataVolumes += df;
		}

		// Now for the recoverable files...

		for (unsigned int i = 0; i < dlg.selectedRecoverableFiles().size(); ++i)
		{
			DataFile	df;

			// Strip out the path & name

			fstl::wstring	filespec = dlg.selectedRecoverableFiles()[i];
			int		idx = filespec.rfind(_T("\\"));
			if (idx >= 0)
			{
				df.fileName() = filespec.substring(idx+1);
				df.filePath() = filespec.substring(0, idx);
			}
			else
			{
				df.fileName() = filespec;
			}

			df.fileSize() = getFileLength(df.filespec());
			df.recoverable() = true;
			df.status() = DataFile::Valid;
			df.statusString() = _T("Just created");

			// Add the data volume

			dataVolumes += df;
		}

		unsigned char	setHash[EmDeeFive::HASH_SIZE_IN_BYTES];
		if (!parityInfo().genParFiles(setHash, parityVolumes, dataVolumes, progCallback, this)) throw _T("");

		// Normal

		resizeWindow(false);

		// Load the par set?

		bool	shouldLoad = theApp.GetProfileInt(_T("Options"), _T("loadAfterCreate"), 1) ? true:false;

		if (shouldLoad)
		{
			// If the set won't load properly after creation, let them know...

			bool	allSameDir = parityVolumes.size() && dataVolumes.size();

			for (unsigned int i = 0; allSameDir && i < dataVolumes.size(); ++i)
			{
				if (dataVolumes[i].filePath().ncCompare(parityVolumes[0].filePath()))
				{
					allSameDir = false;
				}
			}

			if (!allSameDir)
			{
				AfxMessageBox(	_T("In the preferences, you've requested that parity archives should be loaded\n")
						_T("after creation.\n")
						_T("\n")
						_T("However, not all of the data files are in the same directory as your new\n")
						_T("parity archive. Because of this, your archive may appear incomplete.\n")
						_T("\n")
						_T("The archive should be properly tested (by moving all data and parity files\n")
						_T("into the same directory) before it is considered trustworthy."));
			}

			// Save the status of this set

			saveStates(setHash, parityVolumes, dataVolumes);

			// Now load it up

			startupFile() = parityVolumes[0].filespec();
			OnBnClickedLoadButton();
		}
		else
		{
			// Done sound

			if (!silent()) MessageBeep(MB_ICONASTERISK);
		}

	}
	catch (const TCHAR * err)
	{
		if (err && *err) AfxMessageBox(err);
		resizeWindow(false);
	}
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	FSRaidDialog::OnBnClickedCheckButton()
{
	// If we're set to automatically fix names, then just do that now, since it performs a check first

	if (theApp.GetProfileInt(_T("Options"), _T("fixAfterCheck"), 0))
	{
		OnBnClickedFixnamesButton();
	}
	else
	{
		// Anything to do?

		if (!parityInfo().dataFiles().size())
		{
			AfxMessageBox(_T("No files to check"));
			return;
		}

		// Show the progress meter

		resizeWindow(true);

		// Check the files

		bool	allOK;
		checkFiles(allOK);

		// Hide the progress meter

		resizeWindow(false);

		// Reset the cancel flag

		if (cancelFlag())
		{
			cancelFlag() = false;
			AfxMessageBox(_T("Operation cancelled"));
			return;
		}

		// Make a sound

		if (!silent())
		{
			if (allOK)	MessageBeep(MB_ICONASTERISK);
			else		MessageBeep(MB_ICONEXCLAMATION);
		}
	}
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	FSRaidDialog::OnBnClickedCheckallButton()
{
	// Go through and mark all files as unknown

	for (unsigned int i = 0; i < parityInfo().dataFiles().size(); ++i)
	{
		parityInfo().dataFiles()[i].status() = DataFile::Unknown;
		parityInfo().dataFiles()[i].statusString() = _T("Unknown");
	}

	for (unsigned int i = 0; i < parityInfo().parityFiles().size(); ++i)
	{
		parityInfo().parityFiles()[i].status() = ParityFile::Unknown;
		parityInfo().parityFiles()[i].statusString() = _T("Unknown");
	}

	// Update the datafile map

	drawMaps();

	// Check the files

	OnBnClickedCheckButton();
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	FSRaidDialog::OnBnClickedFixnamesButton()
{
	// Show the progress meter

	resizeWindow(true);

	try
	{
		// First, check files

		bool	allOK;
		checkFiles(allOK);

		// If they cancelled, bail

		if (cancelFlag()) throw _T("Operation cancelled");

		// Everything okay?

		if (allOK) throw _T("");

		// Keep the user informed

		progressText().SetWindowText(_T("Scanning files..."));

		// Count the files first...

		float	totalFiles = 0, currentFile = 0;
		{
			_wfinddata_t	fd;
			fstl::wstring	filespec = parityInfo().defaultPath() + _T("\\*.*");
			intptr_t	handle = 0;
			handle = _wfindfirst((TCHAR *) filespec.asArray(), &fd);
			if (handle <= 0) throw _T("Unable to get directory listing");
			do{totalFiles = totalFiles + 1;} while (_wfindnext(handle, &fd) == 0);
			_findclose(handle);
		}

		// Get a list of files in the directory

		fstl::WStringArray	directoryFiles;
		{
			// Reserve (for speed)

			directoryFiles.reserve(static_cast<unsigned int>(totalFiles));

			// Filespec

			_wfinddata_t	fd;
			fstl::wstring	filespec = parityInfo().defaultPath() + _T("\\*.*");
			intptr_t	handle = 0;

			// Start scanning files

			try
			{
				handle = _wfindfirst((TCHAR *) filespec.asArray(), &fd);
				if (handle <= 0) throw _T("Unable to get directory listing");

				do
				{
					currentFile = currentFile + 1;
					if (!progCallback(this, _T("Scanning files..."), currentFile / totalFiles * 100)) throw _T("Operation cancelled");

					// Only add filenames that are not already marked as valid...

					bool	found = false;
					for (unsigned int i = 0; !found && i < parityInfo().dataFiles().size(); ++i)
					{
						if (!parityInfo().dataFiles()[i].fileName().ncCompare(fd.name) && parityInfo().dataFiles()[i].status() == DataFile::Valid) found = true;
					}

					if (!found) directoryFiles += fd.name;
				} while (_wfindnext(handle, &fd) == 0);

				_findclose(handle);
			}
			catch (const TCHAR * err)
			{
				if (handle > 0) _findclose(handle);

				static	fstl::wstring	msg = fstl::wstring(_T("An error has ocurred:\n\n")) + err;
				throw msg.asArray();
			}
		}

		// Create a list of file indices that need to be found (i.e. not valid)

		fstl::intArray		fileIndices;
		{
			for (unsigned int i = 0; i < parityInfo().dataFiles().size(); ++i)
			{
				if (parityInfo().dataFiles()[i].status() == DataFile::Valid) continue;
				fileIndices += i;
			}
		}

		// Process of elimination

		fstl::WStringArray	whatItIs;
		fstl::WStringArray	whatItShouldBe;

		for (unsigned int i = 0; i < directoryFiles.size(); ++i)
		{
			// Inform the user

			if (!progCallback(this, _T("Finding relevant data files..."), static_cast<float>(i)/static_cast<float>(directoryFiles.size())*100))
			{
				throw _T("Operation cancelled");
			}

			// Is it a par file from this set?

			if (ParityFile::isFromSet(parityInfo().defaultPath(), directoryFiles[i], parityInfo().setHash()))
			{
				DataFileArray	dfa;
				fstl::wstring	createdBy;
				ParityFile	pf;
				pf.readPARHeader(parityInfo().defaultPath(), directoryFiles[i], createdBy, dfa);

				// Is this a known, valid par file?

				bool	found = false;
				for (unsigned int j = 0; j < parityInfo().parityFiles().size(); ++j)
				{
					ParityFile &	parFile = parityInfo().parityFiles()[j];
					if (parFile.volumeNumber() == pf.volumeNumber() && parFile.status() == ParityFile::FileStatus::Valid)
					{
						found = true;
						break;
					}
				}

				if (found) continue;

				// What's this sucker's extension? (p01-p99, q00-q99, etc.)

				fstl::wstring	actualName = parityInfo().defaultBaseName();
				if (pf.volumeNumber() == 0)
				{
					actualName += _T(".par");
				}
				else
				{
					TCHAR	ext[10];
					swprintf(ext, _T(".%c%02d"), (pf.volumeNumber() / 100) + _T('p'), pf.volumeNumber() % 100);
					actualName += ext;
				}

				// If the names don't match, add them

				if (directoryFiles[i].ncCompare(actualName))
				{
					whatItIs += directoryFiles[i];
					whatItShouldBe += actualName;
				}
			}
			else
			{
				// Get the file info...

				fstl::wstring	filespec = parityInfo().defaultPath() + _T("\\") + directoryFiles[i];
				unsigned int	fileLength = getFileLength(filespec);

				// Check the file length against the files we're looking for

				bool	found = false;
				for (unsigned int j = 0; j < fileIndices.size() && !found; ++j)
				{
					if (parityInfo().dataFiles()[fileIndices[j]].fileSize() == fileLength) found = true;
				}

				// If it doesn't match a valid size, skip it

				if (!found) continue;

				// Okay, we have a valid candidate here.. check the 16K checksum

				unsigned char	hash[EmDeeFive::HASH_SIZE_IN_BYTES];
				try{ if (!EmDeeFive::processFile(filespec, hash, 0, 0, NULL, NULL, 0, 1024*16))	continue;}
				catch(...){continue;}

				// See if this checksum matches one of the 16K checkums of the missing files

				int	foundIndex = -1;
				for (unsigned int j = 0; foundIndex == -1 && j < fileIndices.size(); ++j)
				{
					CHECK_CANCEL();

					if (!memcmp(parityInfo().dataFiles()[fileIndices[j]].hashFirst16K(), hash, EmDeeFive::HASH_SIZE_IN_BYTES))
					{
						// Passed the 16K test, try the whole thing

						unsigned char		hash[EmDeeFive::HASH_SIZE_IN_BYTES];
						try{ if (!EmDeeFive::processFile(filespec, hash, 0, 0))	continue;}
						catch(...){						continue;}

						// Make sure the full hash matches

						if (memcmp(parityInfo().dataFiles()[fileIndices[j]].hash(), hash, EmDeeFive::HASH_SIZE_IN_BYTES)) continue;

						// Got a match

						foundIndex = fileIndices[j];
					}
				}

				if (foundIndex == -1) continue;

				// Make sure we didn't match a file with itself...

				if (!filespec.ncCompare(parityInfo().dataFiles()[foundIndex].filespec())) continue;

				// Found one, track it

				whatItIs += directoryFiles[i];
				whatItShouldBe += parityInfo().dataFiles()[foundIndex].fileName();
			}
		}

		// Remove duplicate destination files from the list

		fstl::WStringArray	srcFiles;
		fstl::WStringArray	dstFiles;

		for (unsigned int i = 0; i < whatItShouldBe.size(); ++i)
		{
			bool	found = false;
			for (unsigned int j = 0; !found && j < dstFiles.size(); ++j)
			{
				if (whatItShouldBe[i] == dstFiles[j]) found = true;
			}

			if (!found)
			{
				srcFiles += whatItIs[i];
				dstFiles += whatItShouldBe[i];
			}
		}

		// Empty dataset?

		if (!srcFiles.size()) throw _T("");

		// Make sure there aren't duplicates in the source & dest lists...
		{
			fstl::WStringArray	tempList = srcFiles;
			tempList.sort();
			tempList.unique();
			if (tempList.size() != srcFiles.size())
			{
				throw	"This PAR archive is possibly corrupt...\n"
					"\n"
					"A file was found to match more than\n"
					"one data file. This is bad, because\n"
					"the data files are declared unique by\n"
					"a very rigorous fingerprinting scheme.\n"
					"\n"
					"The chances of a duplicate fingerprint\n"
					"(as was just found) are astronomical.";
			}

			tempList = dstFiles;
			tempList.sort();
			tempList.unique();
			if (tempList.size() != srcFiles.size())
			{
				throw	"An internal error was detected within this software.\n"
					"\n"
					"Fortunately, the error was detected before any files\n"
					"were modified, so you can relax. :)";
			}
		}

		// Build a list of files for the user so they can verify before we go further
		{
			RenameConfirmationDialog	dlg;
			for (unsigned int i = 0; i < srcFiles.size(); ++i)
			{
				dlg.fromList() += srcFiles[i];
				dlg.toList() += dstFiles[i];
			}
			if (dlg.DoModal() != IDOK) throw _T("Operation cancelled");
		}

		// Rename existing destination files to temporary names
		//
		// Note that if two files are swapped, and you rename the destination of one name pair, then the source of the other
		// pair is no longer valid (it's file was renamed), so as we rename the dest files to temporary files, we need to scan
		// the source filename list and change the names to the temporary names.

		// During the rename process, don't let then cancel...

		stopButton().EnableWindow(FALSE);
		pauseButton().EnableWindow(FALSE);

		progCallback(this, _T("Renaming files (1 of 2)..."), 0);

		for (unsigned int i = 0; i < dstFiles.size(); ++i)
		{
			// Does the file exist?

			if (!doesFileExist(parityInfo().defaultPath() + _T("\\") + dstFiles[i])) continue;

			// Generate a temp name

			fstl::wstring	oldDst = dstFiles[i];
			fstl::wstring	newDst = uniqueFilename(parityInfo().defaultPath(), oldDst);

			if (!newDst.length())
			{
				throw	_T("Unable to get a temporary name, cannot finish rename process.\n")
					_T("\n")
					_T("Don't worry, your files are all still there, some of them may\n")
					_T("just have strange names.");
			}

			// Do the rename...

			fstl::wstring	a = parityInfo().defaultPath() + _T("\\") + oldDst;
			fstl::wstring	b = parityInfo().defaultPath() + _T("\\") + newDst;

			if (_wrename(a.asArray(), b.asArray()))
			{
				fstl::wstring	msg;
				msg += _T("Unable to rename a file, cannot finish rename process.\n");
				msg += _T("\n");
				msg += fstl::wstring(_T("The error returned was: ")) + _wcserror(errno) + _T("\n");
				msg += _T("\n");
				msg += _T("Don't worry, your files are all still there, some of them may\n");
				msg += _T("just have strange names.");
				throw msg.asArray();
			}

			// Replace any occurance of 'dstFiles[i]' found within 'srcFiles' with 'newDst'

			for (unsigned int j = 0; j < srcFiles.size(); ++j)
			{
				if (!srcFiles[j].ncCompare(dstFiles[i]))
				{
					srcFiles[j] = newDst;
					break;
				}
			}
		}

		// Rename source -> dest files

		progCallback(this, _T("Renaming files (1 of 2)..."), 0);

		for (unsigned int i = 0; i < srcFiles.size(); ++i)
		{
			fstl::wstring	a = parityInfo().defaultPath() + _T("\\") + srcFiles[i];
			fstl::wstring	b = parityInfo().defaultPath() + _T("\\") + dstFiles[i];

			if (_wrename(a.asArray(), b.asArray()))
			{
				static	fstl::wstring	msg;
				msg += _T("Unable to rename a file, cannot finish rename process.\n");
				msg += _T("\n");
				msg += fstl::wstring(_T("The error returned was: ")) + _wcserror(errno) + _T("\n");
				msg += _T("\n");
				msg += _T("Don't worry, your files are all still there, some of them may\n");
				msg += _T("just have strange names.");
				throw msg.asArray();
			}

			// Mark the file as valid, so we don't have to re-check it...

			for (unsigned int j = 0; j < parityInfo().dataFiles().size(); ++j)
			{
				if (parityInfo().dataFiles()[j].fileName() == dstFiles[i])
				{
					parityInfo().dataFiles()[j].status() = DataFile::Valid;
					parityInfo().dataFiles()[j].statusString() = _T("Valid");
				}
			}
		}

		// Okay, re-enable this sucker

		stopButton().EnableWindow(TRUE);
		pauseButton().EnableWindow(TRUE);

		// Check files again, to update status

		checkFiles(allOK);

		// If they cancelled, bail

		if (cancelFlag()) throw _T("Operation cancelled");

		// Make a sound

		if (!silent())
		{
			if (allOK)	MessageBeep(MB_ICONASTERISK);
			else		MessageBeep(MB_ICONEXCLAMATION);
		}
	}
	catch (const TCHAR * err)
	{
		// Reset stuff

		resizeWindow(false);

		// Show them an error

		if (err && *err)
		{
			AfxMessageBox(err);
			return;
		}
		else
		{
			if (!silent()) MessageBeep(MB_ICONASTERISK);
		}
	}

	// Put stuff back to normal

	resizeWindow(false);
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	FSRaidDialog::OnBnClickedRepairButton()
{
	// Fix names first?

	if (theApp.GetProfileInt(_T("Options"), _T("fixBeforeRepair"), 0))
	{
		silent() = true;
		OnBnClickedFixnamesButton();
		silent() = false;
	}

	resizeWindow();

	try
	{
		if (!parityInfo().recoverFiles(parityInfo().parityFiles(), parityInfo().dataFiles(), progCallback, this)) throw _T("");

		// Force a check

		OnBnClickedCheckButton();
	}
	catch (const TCHAR * err)
	{
		if (err && *err) AfxMessageBox(err);
	}

	resizeWindow(false);
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	FSRaidDialog::OnBnClickedHelpButton()
{
	if (helpDialog())
	{
		delete helpDialog();
		helpDialog() = NULL;
	}
	else
	{
		helpDialog() = new HelpDialog(this);
	}
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	FSRaidDialog::OnBnClickedAboutButton()
{
	if (aboutDialog())
	{
		delete aboutDialog();
		aboutDialog() = NULL;
	}
	else
	{
		aboutDialog() = new AboutBox(this);
	}
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	FSRaidDialog::OnBnClickedPrefsButton()
{
	PreferencesDialog	dlg;
	dlg.DoModal();

	// Update the file associations?

	if (theApp.GetProfileInt(_T("Options"), _T("associatePAR"), 1))
	{
		setFileAssociations();
	}

	// Redraw, because they may have changed the color scheme

	RedrawWindow();
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	FSRaidDialog::OnBnClickedOk()
{
	// Cancel any pending operations... should never happen, but you never know!

	cancelFlag() = true;

	// Save our window pos

	if (theApp.GetProfileInt(_T("Options"), _T("saveWindowPos"), 1))
	{
		theApp.WriteProfileInt(_T("Options"), _T("windowX"), windowX);
		theApp.WriteProfileInt(_T("Options"), _T("windowY"), windowY);
		theApp.WriteProfileInt(_T("Options"), _T("windowW"), windowW);
	}

	OnOK();
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	FSRaidDialog::OnBnClickedStopButton()
{
	cancelFlag() = true;

	// Specifically cancel the download monitor

	if (monitorFlag())
	{
		// Kill the timer if it exists... if it DID exist, set a new timer, shorter

		if (KillTimer(monitorTimer)) SetTimer(monitorTimer, 0, NULL);
	}

	// All done

	stopButton().EnableWindow(FALSE);
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	FSRaidDialog::OnBnClickedPauseButton()
{
	pausedFlag() = pauseButton().GetCheck() ? true:false;
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	FSRaidDialog::OnDropFiles(HDROP hDropInfo)
{
	if (busy())
	{
		AfxMessageBox(_T("Please stop the current process before trying to load a new file"));
	}
	else
	{
		unsigned int	count = DragQueryFile(hDropInfo, 0xffffffff, NULL, 0);

		// Ignore all but the first file

		if (count)
		{
			TCHAR	fname[MAX_PATH];
			memset(fname, 0, sizeof(fname));
			if (DragQueryFile(hDropInfo, 0, fname, sizeof(fname)-1) > 0)
			{
				startupFile() = fname;
				OnBnClickedLoadButton();
			}
		}
	}

	CDialog::OnDropFiles(hDropInfo);
}

// ---------------------------------------------------------------------------------------------------------------------------------

BOOL FSRaidDialog::OnHelpInfo(HELPINFO* pHelpInfo)
{
	OnBnClickedHelpButton();
	return TRUE;
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	FSRaidDialog::resizeWindow(const bool large)
{
	if (large)
	{
		progCallback(this, _T(""), 0);

		stopButton().EnableWindow(TRUE);
		pauseButton().EnableWindow(TRUE);
		progressText().EnableWindow(TRUE);
		progress().EnableWindow(TRUE);
		enableButtons(false);
		busy() = true;
	}
	else
	{
		stopButton().EnableWindow(FALSE);
		pauseButton().EnableWindow(FALSE);
		progressText().EnableWindow(FALSE);
		progress().EnableWindow(FALSE);
		enableButtons();
		busy() = false;
	}

	// Make sure we're not cancelled or paused

	cancelFlag() = false;
	pausedFlag() = false;
	pauseButton().SetCheck(FALSE);

	// This may look strange, but the resize limitation is actually limited in the OnGetMinMaxInfo() member function

	CRect	wr;
	GetWindowRect(wr);
	MoveWindow(wr);

	enableButtons();

	// We often resize the window after potentially "serious calculations" so it may be a while before we get a chance
	// to redraw if we're low on memory, so force one here.

	RedrawWindow();
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	FSRaidDialog::enableButtons(const bool enabled)
{
	BOOL	en = enabled ? TRUE:FALSE;

	// If we're showing the status bar, then force a disable state

	if (busy()) en = FALSE;

	unsigned int	valid, corrupt, misnamed, missing, unknown, error, needed, recoverable, validRecoverable;
	calcStats(valid, corrupt, misnamed, missing, unknown, error, needed, recoverable, validRecoverable);

	loadButton.EnableWindow(en);
	prefsButton.EnableWindow(en);
	closeButton.EnableWindow(en);
	createButton.EnableWindow(en);

	if (parityInfo().dataFiles().size())	checkButton.EnableWindow(en);
	else					checkButton.EnableWindow(FALSE);

	if (parityInfo().dataFiles().size())	checkAllButton.EnableWindow(en);
	else					checkAllButton.EnableWindow(FALSE);

	if (parityInfo().dataFiles().size())	fixNamesButton.EnableWindow(en);
	else					fixNamesButton.EnableWindow(FALSE);

	if (needed)				downloadMonitorButton.EnableWindow(en);
	else					downloadMonitorButton.EnableWindow(FALSE);

	bool	allOK = validRecoverable >= recoverable;
	if (!needed && !allOK && parityInfo().dataFiles().size())	repairButton.EnableWindow(en);
	else								repairButton.EnableWindow(FALSE);
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	FSRaidDialog::drawMaps()
{
	CDC *	dc = GetDC();
	drawMaps(*dc);
	ReleaseDC(dc);
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	FSRaidDialog::drawMaps(CDC & dc)
{
	// Colors...

	COLORREF	corruptColor  = RGB(0x80, 0x00, 0x00);
	COLORREF	errorColor    = RGB(0xff, 0x00, 0x00);
	COLORREF	missingColor  = RGB(0xe4, 0xe4, 0x00);
	COLORREF	misnamedColor = RGB(0x2d, 0xcb, 0xb8);
	COLORREF	validColor    = RGB(0x00, 0x80, 0x00);
	COLORREF	unknownColor  = RGB(0xe0, 0xdf, 0xe3);

	if (theApp.GetProfileInt(_T("Options"), _T("highContrastColors"), 0))
	{
		corruptColor  = RGB(0x80, 0x00, 0x00); // Colors chosen specifically to allow color-blind people to see
		errorColor    = RGB(0xBD, 0x00, 0x00); // differences between them clearly. If you screenshot the program
		missingColor  = RGB(0xE3, 0xE3, 0x33); // and desaturate the image, you'll see that they're all distinguishable
		misnamedColor = RGB(0x8F, 0xB8, 0xCA); // shades of gray. The order they are displayed here (from the top down)
		validColor    = RGB(0xC3, 0xE9, 0xAF); // are those colors, which produce desaturated shades of gray from
		unknownColor  = RGB(0xe0, 0xdf, 0xe3); // darkest (top) to brightest (bottom).
	}

	// Data files

	if (!parityInfo().dataFiles().size())
	{
		CRect	r;
		dataFileMapFrame.GetClientRect(r);
		dataFileMapFrame.MapWindowPoints(this, r);
		dc.FillSolidRect(r, GetSysColor(COLOR_BTNFACE));
		dc.Draw3dRect(r, GetSysColor(COLOR_3DHILIGHT), GetSysColor(COLOR_3DDKSHADOW));
		r.DeflateRect(1,1,1,1);
		dc.Draw3dRect(r, GetSysColor(COLOR_3DLIGHT), GetSysColor(COLOR_3DSHADOW));
	}
	else
	{
		CRect	r;
		dataFileMapFrame.GetClientRect(r);
		dataFileMapFrame.MapWindowPoints(this, r);

		// How wide is each file?

		float	fileWidth = static_cast<float>(r.Width()) / static_cast<float>(parityInfo().dataFiles().size());
		float	x0 = static_cast<float>(r.left);

		for (unsigned int i = 0; i < parityInfo().dataFiles().size(); ++i)
		{
			CRect	cr = r;
			cr.left = static_cast<int>(x0);
			x0 += fileWidth;
			cr.right = static_cast<int>(x0);

			dc.Draw3dRect(cr, GetSysColor(COLOR_3DHILIGHT), GetSysColor(COLOR_3DDKSHADOW));
			cr.DeflateRect(1,1,1,1);
			dc.Draw3dRect(cr, GetSysColor(COLOR_3DLIGHT), GetSysColor(COLOR_3DSHADOW));
			cr.DeflateRect(1,1,1,1);

			// What color to make it?

			COLORREF	fileColor;

			switch(parityInfo().dataFiles()[i].status())
			{
				case DataFile::Unknown:
					fileColor = unknownColor;
					break;

				case DataFile::Valid:
					fileColor = validColor;
					break;

				case DataFile::Corrupt:
					fileColor = corruptColor;
					break;

				case DataFile::Missing:
					fileColor = missingColor;
					break;

				case DataFile::Misnamed:
					fileColor = misnamedColor;
					break;

				case DataFile::Error:
				default:
					fileColor = errorColor;
					break;
			}

			dc.FillSolidRect(cr, fileColor);
		}
	}

	// Parity files

	if (!parityInfo().parityFiles().size())
	{
		CRect	r;
		parityFileMapFrame.GetClientRect(r);
		parityFileMapFrame.MapWindowPoints(this, r);
		dc.FillSolidRect(r, GetSysColor(COLOR_BTNFACE));
		dc.Draw3dRect(r, GetSysColor(COLOR_3DHILIGHT), GetSysColor(COLOR_3DDKSHADOW));
		r.DeflateRect(1,1,1,1);
		dc.Draw3dRect(r, GetSysColor(COLOR_3DLIGHT), GetSysColor(COLOR_3DSHADOW));
	}
	else
	{
		CRect	r;
		parityFileMapFrame.GetClientRect(r);
		parityFileMapFrame.MapWindowPoints(this, r);

		// How wide is each file?

		float	fileWidth = static_cast<float>(r.Width()) / static_cast<float>(parityInfo().parityFiles().size());
		float	x0 = static_cast<float>(r.left);

		for (unsigned int i = 0; i < parityInfo().parityFiles().size(); ++i)
		{
			CRect	cr = r;
			cr.left = static_cast<int>(x0);
			x0 += fileWidth;
			cr.right = static_cast<int>(x0);

			dc.Draw3dRect(cr, GetSysColor(COLOR_3DHILIGHT), GetSysColor(COLOR_3DDKSHADOW));
			cr.DeflateRect(1,1,1,1);
			dc.Draw3dRect(cr, GetSysColor(COLOR_3DLIGHT), GetSysColor(COLOR_3DSHADOW));
			cr.DeflateRect(1,1,1,1);

			// What color to make it?

			COLORREF	fileColor;

			switch(parityInfo().parityFiles()[i].status())
			{
				case ParityFile::Unknown:
					fileColor = unknownColor;
					break;

				case ParityFile::Valid:
					fileColor = validColor;
					break;

				case ParityFile::Corrupt:
					fileColor = corruptColor;
					break;

				case ParityFile::Missing:
					fileColor = missingColor;
					break;

				case ParityFile::Misnamed:
					fileColor = misnamedColor;
					break;

				case ParityFile::Error:
				default:
					fileColor = errorColor;
					break;
			}

			if (!parityInfo().parityFiles()[i].volumeNumber() && parityInfo().parityFiles()[i].status() != ParityFile::Unknown)
			{
				unsigned int	r = static_cast<int>(static_cast<float>((fileColor >> 16) & 0xff) * 0.5f); if (r > 255) r = 255;
				unsigned int	g = static_cast<int>(static_cast<float>((fileColor >>  8) & 0xff) * 0.5f); if (g > 255) g = 255;
				unsigned int	b = static_cast<int>(static_cast<float>((fileColor >>  0) & 0xff) * 0.5f); if (b > 255) b = 255;
				unsigned int	tempColor = (r << 16) | (g << 8) | b;
				dc.FillSolidRect(cr, tempColor);

				cr.DeflateRect(4, 4, 4, 4);
			}

			dc.FillSolidRect(cr, fileColor);
		}
	}

	// Draw the legend colors
	{
		CRect	r;

		unknownLegend.GetClientRect(r);
		unknownLegend.MapWindowPoints(this, r);
		r.DeflateRect(1, 1, 1, 1);
		dc.FillSolidRect(r, unknownColor);

		validLegend.GetClientRect(r);
		validLegend.MapWindowPoints(this, r);
		r.DeflateRect(1, 1, 1, 1);
		dc.FillSolidRect(r, validColor);

		corruptLegend.GetClientRect(r);
		corruptLegend.MapWindowPoints(this, r);
		r.DeflateRect(1, 1, 1, 1);
		dc.FillSolidRect(r, corruptColor);

		missingLegend.GetClientRect(r);
		missingLegend.MapWindowPoints(this, r);
		r.DeflateRect(1, 1, 1, 1);
		dc.FillSolidRect(r, missingColor);

		misnamedLegend.GetClientRect(r);
		misnamedLegend.MapWindowPoints(this, r);
		r.DeflateRect(1, 1, 1, 1);
		dc.FillSolidRect(r, misnamedColor);

		errorLegend.GetClientRect(r);
		errorLegend.MapWindowPoints(this, r);
		r.DeflateRect(1, 1, 1, 1);
		dc.FillSolidRect(r, errorColor);
	}

	// If we have no archive loaded, wipe the archive status

	if (!parityInfo().dataFiles().size() || !parityInfo().parityFiles().size())
	{
		archiveStatusEdit.SetWindowText(_T(""));
	}
	else
	{
		unsigned int	valid, corrupt, misnamed, missing, unknown, error, needed, recoverable, validRecoverable;
		calcStats(valid, corrupt, misnamed, missing, unknown, error, needed, recoverable, validRecoverable);

		TCHAR	disp[256];
		swprintf(disp, _T("Total: %d (%d NR)    Valid: %d    Corrupt: %d    Misnamed: %d    Missing: %d    ???: %d    Err: %d    Need: %d"), parityInfo().dataFiles().size(), parityInfo().dataFiles().size() - recoverable, valid, corrupt, misnamed, missing, unknown, error, needed);
		archiveStatusEdit.SetWindowText(disp);
	}

	// Always wipe these...

	parityFileMapText.SetWindowText(_T(""));
	dataFileMapText.SetWindowText(_T(""));

	// Update button states

	enableButtons();

	// Force a mousemove, too

	POINT	p;
	GetCursorPos(&p);
	::MapWindowPoints(GetDesktopWindow()->GetSafeHwnd(), GetSafeHwnd(), &p, 1);
	OnMouseMove(0, p);
}

// ---------------------------------------------------------------------------------------------------------------------------------

bool	FSRaidDialog::dataFileIndexFromMousePoint(const CPoint & p, int & index)
{
	// Where is the datafile map rect?

	CRect	r;
	dataFileMapFrame.GetClientRect(r);
	dataFileMapFrame.MapWindowPoints(this, r);

	// If the point isn't in the rect, bail

	if (!r.PtInRect(p) || !parityInfo().dataFiles().size()) return false;;

	// How wide is each file?

	float	fileWidth = static_cast<float>(r.Width()) / static_cast<float>(parityInfo().dataFiles().size());
	float	x0 = static_cast<float>(p.x) - static_cast<float>(r.left);
	index = static_cast<unsigned int>(x0 / fileWidth);
	if (index >= static_cast<int>(parityInfo().dataFiles().size())) index = parityInfo().dataFiles().size() - 1;

	return true;
}

// ---------------------------------------------------------------------------------------------------------------------------------

bool	FSRaidDialog::parityFileIndexFromMousePoint(const CPoint & p, int & index)
{
	// Where is the datafile map rect?

	CRect	r;
	parityFileMapFrame.GetClientRect(r);
	parityFileMapFrame.MapWindowPoints(this, r);

	// If the point isn't in the rect, bail

	if (!r.PtInRect(p) || !parityInfo().parityFiles().size()) return false;;

	// How wide is each file?

	float	fileWidth = static_cast<float>(r.Width()) / static_cast<float>(parityInfo().parityFiles().size());
	float	x0 = static_cast<float>(p.x) - static_cast<float>(r.left);
	index = static_cast<unsigned int>(x0 / fileWidth);
	if (index >= static_cast<int>(parityInfo().parityFiles().size())) index = parityInfo().parityFiles().size() - 1;

	return true;
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	FSRaidDialog::checkFiles(bool & allOK)
{
	// Scan for missing files

	scanForMissingFiles();

	unsigned int	dataFileCount = 0;
	unsigned int	parityFileCount = 0;

	try
	{
		stopButton().EnableWindow(FALSE);
		pauseButton().EnableWindow(FALSE);
		progCallback(this, _T("Scanning the directory for parity files..."), 0);

		// How many data files to check?

		for (unsigned int i = 0; i < parityInfo().dataFiles().size(); ++i)
		{
			if (parityInfo().dataFiles()[i].status() != DataFile::Valid) dataFileCount++;
		}

		// Scan for new parity files

		if (!scanForParityFiles())
		{
			AfxMessageBox(_T("Unable to scan for new parity files"));
			allOK = false;
			throw false;
		}

		// How many parity files to check?

		for (unsigned int i = 0; i < parityInfo().parityFiles().size(); ++i)
		{
			if (parityInfo().parityFiles()[i].status() != ParityFile::Valid) parityFileCount++;
		}

		// Re-enable these suckers (they were disabled for parity file checking)

		stopButton().EnableWindow(TRUE);
		pauseButton().EnableWindow(TRUE);
	}
	catch (const bool)
	{
		// Re-enable these suckers (they were disabled for parity file checking)

		stopButton().EnableWindow(TRUE);
		pauseButton().EnableWindow(TRUE);
		return;
	}

	allOK = true;
	unsigned int	checkedCount = 0;
	for (unsigned int i = 0; i < parityInfo().dataFiles().size(); ++i)
	{

		// Make sure the file still exists...

		if (!doesFileExist(parityInfo().dataFiles()[i].filespec()))
		{
			// File no longer exists.. update it's status

			parityInfo().dataFiles()[i].status() = DataFile::Missing;
			parityInfo().dataFiles()[i].statusString() = _T("Missing");
		}

		// Skip files that are A-OK

		if (parityInfo().dataFiles()[i].status() == DataFile::Valid)
		{
			continue;
		}

		// Validate it

		if (!checkDataFile(i, dataFileCount + parityFileCount, checkedCount))
		{
			allOK = false;
		}

		// Keep track of actual files checked

		checkedCount++;

		if (cancelFlag())
		{
			// If they cancelled on this file, set it to unknown

			parityInfo().dataFiles()[i].status() = DataFile::Unknown;
			parityInfo().dataFiles()[i].statusString() = _T("Unknown - user cancelled during validation");

			allOK = false;
			saveStates(parityInfo().setHash(), parityInfo().parityFiles(), parityInfo().dataFiles());
			return;
		}
	}

	for (unsigned int i = 0; i < parityInfo().parityFiles().size(); ++i)
	{
		// Skip files that are A-OK

		if (parityInfo().parityFiles()[i].status() == ParityFile::Valid) continue;

		// Validate it

		if (!checkParityFile(i, dataFileCount + parityFileCount, checkedCount)) allOK = false;

		// Keep track of actual files checked

		checkedCount++;

		if (cancelFlag())
		{
			// If they cancelled on this file, set it to unknown

			parityInfo().parityFiles()[i].status() = ParityFile::Unknown;
			parityInfo().parityFiles()[i].statusString() = _T("Unknown - user cancelled during validation");

			allOK = false;
			saveStates(parityInfo().setHash(), parityInfo().parityFiles(), parityInfo().dataFiles());
			return;
		}
	}

	// Make sure these are updated

	drawMaps();
	saveStates(parityInfo().setHash(), parityInfo().parityFiles(), parityInfo().dataFiles());
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	FSRaidDialog::scanForMissingFiles()
{
	// Don't let them stop the progress...

	stopButton().EnableWindow(FALSE);
	pauseButton().EnableWindow(FALSE);
	progCallback(this, _T("Scanning for missing files..."), 0);

	// Scan the files for existence only

	for (unsigned int i = 0; i < parityInfo().dataFiles().size(); ++i)
	{
		// Commented out on 3/6/2002 - should be a more robust search for missing files...
		//if (parityInfo().dataFiles()[i].status() != DataFile::Unknown) continue;

		// Exists?

		if (!doesFileExist(parityInfo().dataFiles()[i].filespec()))
		{
			// File no longer exists.. update it's status

			parityInfo().dataFiles()[i].status() = DataFile::Missing;
			parityInfo().dataFiles()[i].statusString() = _T("Missing");
		}
		else
		{
			if (parityInfo().dataFiles()[i].status() == DataFile::Missing)
			{
				parityInfo().dataFiles()[i].status() = DataFile::Unknown;
				parityInfo().dataFiles()[i].statusString() = _T("Unknown");
			}
		}
	}

	// Re-enable these suckers...

	stopButton().EnableWindow(TRUE);
	pauseButton().EnableWindow(TRUE);

	// Make sure these are updated

	drawMaps();
	saveStates(parityInfo().setHash(), parityInfo().parityFiles(), parityInfo().dataFiles());
}

// ---------------------------------------------------------------------------------------------------------------------------------

bool	FSRaidDialog::checkDataFile(const int index, const unsigned int totalFiles, const unsigned int curIndex)
{
	// Validate the file

	parityInfo().validateDataFile(index, totalFiles, curIndex, progCallback, reinterpret_cast<void *>(this));

	// Is it invalid?

	bool	ok = parityInfo().dataFiles()[index].status() == DataFile::Valid;

	// Update the datafile map

	drawMaps();

	// Return status

	return ok;
}

// ---------------------------------------------------------------------------------------------------------------------------------

bool	FSRaidDialog::repairDataFile(const int index)
{
	// Repair the file

	if (!parityInfo().recoverFiles(parityInfo().parityFiles(), parityInfo().dataFiles(), progCallback, this, index)) return false;

	// Update the datafile map

	drawMaps();

	// Done

	return true;
}

// ---------------------------------------------------------------------------------------------------------------------------------

bool	FSRaidDialog::checkParityFile(const int index, const unsigned int totalFiles, const unsigned int curIndex)
{
	// Validate the file

	parityInfo().validateParFile(index, totalFiles, curIndex, progCallback, reinterpret_cast<void *>(this));

	// Is it invalid?

	bool	ok = parityInfo().parityFiles()[index].status() == ParityFile::Valid;

	// Update the datafile map

	drawMaps();

	// Return status

	return ok;
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	FSRaidDialog::copyStringToClipboard(const fstl::wstring & str)
{
	bool	useUnicode = (GetVersion() & 0x80000000) != 0x80000000;

	// Allocate RAM for the data

	HGLOBAL	hand;

	if (useUnicode)	hand = GlobalAlloc(GMEM_MOVEABLE, str.length() * sizeof(TCHAR) + sizeof(TCHAR));
	else		hand = GlobalAlloc(GMEM_MOVEABLE, str.length() * sizeof(char) + sizeof(char));

	if (hand)
	{
		LPVOID	ptr = GlobalLock(hand);
		if (ptr)
		{
			// Copy the data over

			if (useUnicode)	fstl::memcpy((TCHAR *) ptr, (TCHAR *) str.asArray(), str.length() + 1);
			else
			{
				fstl::string	asciiString = str.asArray();
				fstl::memcpy((char *) ptr, asciiString.asArray(), asciiString.length() + 1);
			}

			GlobalUnlock(hand);

			if (OpenClipboard())
			{
				// Remove the current Clipboard contents

				if (EmptyClipboard())
				{
					// Get the currently selected data, hData handle to global memory of data

					if (::SetClipboardData(useUnicode ? CF_UNICODETEXT : CF_TEXT, hand) == NULL)
					{
						AfxMessageBox(_T("Unable to set Clipboard data"));
					}
				}
				else
				{
					AfxMessageBox(_T("Cannot empty the Clipboard"));
				}

				CloseClipboard();
			}
			else
			{
				AfxMessageBox(_T("Cannot open the Clipboard"));
			}
		}
		else
		{
			AfxMessageBox(_T("Unable to lock global memory for clipboard data"));
		}

		// GlobalFree(hand);
	}
	else
	{
		AfxMessageBox(_T("Unable to allocate global memory for clipboard data"));
	}
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	FSRaidDialog::calcStats(unsigned int & valid, unsigned int & corrupt, unsigned int & misnamed, unsigned int & missing, unsigned int & unknown, unsigned int & error, unsigned int & needed, unsigned int & recoverable, unsigned int & validRecoverable)
{
	validRecoverable = 0;
	recoverable = 0;
	valid = 0, corrupt = 0, misnamed = 0, missing = 0, unknown = 0, error = 0, needed = 0;

	for (unsigned int i = 0; i < parityInfo().dataFiles().size(); ++i)
	{
		if (parityInfo().dataFiles()[i].recoverable()) recoverable++;

		switch(parityInfo().dataFiles()[i].status())
		{
			case DataFile::Valid:		++valid; if (parityInfo().dataFiles()[i].recoverable()) ++validRecoverable;	break;
			case DataFile::Corrupt:		++corrupt;	break;
			case DataFile::Misnamed:	++misnamed;	break;
			case DataFile::Missing:		++missing;	break;
			case DataFile::Error:		++error;	break;
			default:			++unknown;	break;
		}
	}

	unsigned int	parityCount = 0;
	for (unsigned int i = 0; i < parityInfo().parityFiles().size(); ++i)
	{
		if (parityInfo().parityFiles()[i].volumeNumber() && parityInfo().parityFiles()[i].status() == ParityFile::Valid) ++parityCount;
	}

	int	n = recoverable - validRecoverable;
	if (n) n -= parityCount;
	if (n < 0) n = 0;
	needed = n;
}

// ---------------------------------------------------------------------------------------------------------------------------------

bool	FSRaidDialog::shouldRepair() const
{
	// Should we try to repair?

	for (unsigned int i = 0; i < parityInfo().dataFiles().size(); ++i)
	{
		const DataFile &	df = parityInfo().dataFiles()[i];

		if (!df.recoverable()) continue;
		if (df.status() != DataFile::Valid) return true;
	}

	return false;
}

// --------------------------------------------------------------------------------------------------------------------------------

void	FSRaidDialog::setFileAssociations()
{
	fstl::wstring	prog = theApp.programFilename();

	// Setup the file associations
	{
		fstl::wstring	command = _T("\"") + prog + _T("\" \"%1\"");
		HKEY		hProgKey;
		HKEY		hIconKey;
		HKEY		hShellKey;
		HKEY		hOpenKey;
		HKEY		hOCmdKey;
		try
		{
			// Get the dotted name

			fstl::wstring	dottedString = PROGRAM_NAME_STRING;
			for(;;)
			{
				int	idx = dottedString.find(_T(" "));
				if (idx == -1) break;
				dottedString[idx] = _T('.');
			}

			DWORD	dw;
			HKEY	hKey;

			// .PAR

			if (RegCreateKeyEx(HKEY_CLASSES_ROOT, _T(".par"), 0, REG_NONE, REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL, &hKey, &dw) != ERROR_SUCCESS) return;
			RegSetValueEx(hKey, NULL, NULL, REG_SZ, (const BYTE *) dottedString.asArray(), dottedString.length() * 2 + 2);
			RegCloseKey(hKey);

			// .P00 - P30

			for (unsigned int i = 0; i <= 30; ++i)
			{
				TCHAR	ext[90];
				swprintf(ext, _T(".p%02d"), i);
				if (RegCreateKeyEx(HKEY_CLASSES_ROOT, ext, 0, REG_NONE, REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL, &hKey, &dw) != ERROR_SUCCESS) return;
				RegSetValueEx(hKey, NULL, NULL, REG_SZ, (const BYTE *) dottedString.asArray(), dottedString.length() * 2 + 2);
				RegCloseKey(hKey);
			}

			// Association registry

			if (RegCreateKeyEx(HKEY_CLASSES_ROOT, dottedString.asArray(), 0, REG_NONE, REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL, &hProgKey, &dw) != ERROR_SUCCESS) throw false;
			RegSetValueEx(hProgKey, NULL, NULL, REG_SZ, (const BYTE *) _T("Parity archive"), (DWORD) wcslen(_T("Parity archive")) * 2 + 2);

				fstl::wstring	iconString = prog + _T(",0");
				if (RegCreateKeyEx(hProgKey, _T("DefaultIcon"), 0, REG_NONE, REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL, &hIconKey, &dw) != ERROR_SUCCESS) throw false;
					RegSetValueEx(hIconKey, NULL, NULL, REG_SZ, (const BYTE *) iconString.asArray(), iconString.length() * 2 + 2);

				if (RegCreateKeyEx(hProgKey, _T("shell"), 0, REG_NONE, REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL, &hShellKey, &dw) != ERROR_SUCCESS) throw false;
					if (RegCreateKeyEx(hShellKey, _T("open"), 0, REG_NONE, REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL, &hOpenKey, &dw) != ERROR_SUCCESS) throw false;
						if (RegCreateKeyEx(hOpenKey, _T("command"), 0, REG_NONE, REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL, &hOCmdKey, &dw) != ERROR_SUCCESS) throw false;
							RegSetValueEx(hOCmdKey, NULL, NULL, REG_SZ, (const BYTE *) command.asArray(), command.length() * 2 + 2);
		}
		catch(bool)
		{
		}

		RegCloseKey(hOCmdKey);
		RegCloseKey(hOpenKey);
		RegCloseKey(hShellKey);
		RegCloseKey(hIconKey);
		RegCloseKey(hProgKey);
	}
}

// --------------------------------------------------------------------------------------------------------------------------------

bool	FSRaidDialog::checkFileAssociations()
{
	fstl::wstring	prog = theApp.programFilename();

	// Get the dotted name

	fstl::wstring	dottedString = PROGRAM_NAME_STRING;
	for(;;)
	{
		int	idx = dottedString.find(_T(" "));
		if (idx == -1) break;
		dottedString[idx] = _T('.');
	}

	DWORD	dw;
	HKEY	hKey;

	// Look for .PAR

	if (RegCreateKeyEx(HKEY_CLASSES_ROOT, _T(".par"), 0, REG_NONE, REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL, &hKey, &dw) != ERROR_SUCCESS) return false;
	TCHAR	nameString[MAX_PATH];
	memset(nameString, 0, sizeof(nameString));
	DWORD	len = sizeof(nameString) - 1;
	DWORD	type = REG_SZ;
	RegQueryValueEx(hKey, NULL, NULL, &type, (LPBYTE) nameString, &len);
	RegCloseKey(hKey);

	fstl::wstring	foundStr = nameString;
	if (foundStr != dottedString) return false;

	// Look for .P00 - P30

	for (unsigned int i = 0; i <= 30; ++i)
	{
		TCHAR	ext[90];
		swprintf(ext, _T(".p%02d"), i);
		if (RegCreateKeyEx(HKEY_CLASSES_ROOT, ext, 0, REG_NONE, REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL, &hKey, &dw) != ERROR_SUCCESS) return false;
		TCHAR	nameString[MAX_PATH];
		memset(nameString, 0, sizeof(nameString));
		DWORD	len = sizeof(nameString) - 1;
		DWORD	type = REG_SZ;
		RegQueryValueEx(hKey, NULL, NULL, &type, (LPBYTE) nameString, &len);
		RegCloseKey(hKey);
		fstl::wstring	foundStr = nameString;
		if (foundStr != dottedString) return false;
	}

	// File associations are set, update them to make sure they point at the current executable (wherever it happens to be)

	setFileAssociations();

	return true;
}

// ---------------------------------------------------------------------------------------------------------------------------------

bool	FSRaidDialog::saveStates(const unsigned char setHash[EmDeeFive::HASH_SIZE_IN_BYTES], ParityFileArray & pfa, DataFileArray & dfa)
{
	// If the user has asked not to remember states, then don't

	if (!theApp.GetProfileInt(_T("Options"), _T("rememberStates"), 1)) return false;

	// Get the data from the registry

	unsigned int	maxAllowed;
	RegInfoArray	ria = getRegInfo(maxAllowed);

	// Look for the current hash entry

	int	index = -1;
	for (unsigned int i = 0; index < 0 && i < ria.size(); ++i)
	{
		RegInfo &	ri = ria[i];
		if (ri.hashCount == EmDeeFive::HASH_SIZE_IN_BYTES && !memcmp(setHash, &ri.hash[0], ri.hashCount))
		{
			index = i;
		}
	}

	// Add a new one?

	if (index < 0)
	{
		// Remove the oldest one?

		if (ria.size() >= maxAllowed)
		{
			ria.sort();
			ria.erase(0, 1);
		}

		// This is the index of the new one that we'll add

		index = ria.size();

		// Just add one to the list

		RegInfo	ri;
		ria += ri;
	}

	// Update the entry
	
	RegInfo &	ri = ria[index];

	ri.lastAccessed = static_cast<unsigned int>(time(NULL));
	ri.hashCount = EmDeeFive::HASH_SIZE_IN_BYTES;
	ri.dataCount = dfa.size();
	ri.parityCount = pfa.size();
	ri.hash.erase();
	ri.data.erase();
	ri.parity.erase();

	for (unsigned int i = 0; i < ri.hashCount; ++i)
	{
		ri.hash += setHash[i];
	}
	for (unsigned int i = 0; i < ri.dataCount; ++i)
	{
		ri.data += static_cast<unsigned int>(dfa[i].status());
	}
	for (unsigned int i = 0; i < ri.parityCount; ++i)
	{
		ri.parity += static_cast<unsigned int>(pfa[i].status());
	}

	// Write the suckers out

	putRegInfo(ria, maxAllowed);

	return true;
}

// ---------------------------------------------------------------------------------------------------------------------------------

bool	FSRaidDialog::loadStates(const unsigned char setHash[EmDeeFive::HASH_SIZE_IN_BYTES], ParityFileArray & pfa, DataFileArray & dfa)
{
	// If the user has asked not to remember states, then don't

	if (!theApp.GetProfileInt(_T("Options"), _T("rememberStates"), 1)) return false;

	// Get the data from the registry

	unsigned int	maxAllowed;
	RegInfoArray	ria = getRegInfo(maxAllowed);

	// Look for the current hash entry

	int	index = -1;
	for (unsigned int i = 0; index < 0 && i < ria.size(); ++i)
	{
		RegInfo &	ri = ria[i];
		if (ri.hashCount == EmDeeFive::HASH_SIZE_IN_BYTES && !memcmp(setHash, &ri.hash[0], ri.hashCount))
		{
			index = i;
		}
	}

	// Find it?

	if (index < 0) return false;

	// Found it, parse it into our data/parity arrays

	RegInfo &	ri = ria[index];

	if (ri.dataCount == dfa.size())
	{
		for (unsigned int i = 0; i < ri.dataCount; ++i)
		{
			int	tmp = ri.data[i];
			DataFile::FileStatus	status = *reinterpret_cast<DataFile::FileStatus *>(&tmp);
			if (dfa[i].status() == DataFile::Unknown)
			{
				dfa[i].status() = status;
				dfa[i].statusString() = DataFile::getStatusString(status);
			}
		}
	}

	if (ri.parityCount == pfa.size())
	{
		for (unsigned int i = 0; i < ri.parityCount; ++i)
		{
			int	tmp = ri.parity[i];
			ParityFile::FileStatus	status = *reinterpret_cast<ParityFile::FileStatus *>(&tmp);
			if (pfa[i].status() == ParityFile::Unknown)
			{
				pfa[i].status() = status;
				pfa[i].statusString() = ParityFile::getStatusString(status);
			}
		}
	}

	// Update the last accessed time

	ri.lastAccessed = static_cast<unsigned int>(time(NULL));

	// Write the suckers out

	putRegInfo(ria, maxAllowed);

	return true;
}

// ---------------------------------------------------------------------------------------------------------------------------------

bool	FSRaidDialog::scanForParityFiles()
{
	ParityFileArray	pfa;
	if (!parityInfo().findParFiles(pfa)) return false;

	// Copy the statuses over of known files

	for (unsigned int i = 0; i < pfa.size(); ++i)
	{
		for (unsigned int j = 0; j < parityInfo().parityFiles().size(); ++j)
		{
			if (!parityInfo().parityFiles()[j].fileName().ncCompare(pfa[i].fileName()))
			{
				pfa[i].status() = parityInfo().parityFiles()[j].status();
				pfa[i].statusString() = parityInfo().parityFiles()[j].statusString();
			}
		}
	}
	parityInfo().parityFiles() = pfa;

	return true;
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	FSRaidDialog::killDownloadMonitor()
{
	KillTimer(monitorTimer);
	monitorFlag() = false;
	resizeWindow(false);
	progress().ShowWindow(SW_SHOW);
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	FSRaidDialog::monitorDownload()
{
	// Kill the timer

	KillTimer(monitorTimer);

	bool	doRepair = false;

	if (!monitorFlag())
	{
		killDownloadMonitor();
		return;
	}

	try
	{
		// We'll default to a full-length wait-period...

		unsigned int	downloadMonitorInterval = theApp.GetProfileInt(_T("Options"), _T("downloadMonitorInterval"), 10) * 1000;

		// Inform the user, handle pause, cancel, etc...

		if (!progCallback(this, _T("Monitoring downloads..."), 0)) throw true;

		// Reset these...

		validPercent = 0;
		potentialPercent = 0;

		// If they've requested a stop, then stop...

		if (cancelFlag()) throw true;

		// Scan for new parity files

		scanForParityFiles();

		// Count parity files

		unsigned int	validParityFiles = 0;
		unsigned int	validatedThisPass = false;

		for (unsigned int i = 0; i < parityInfo().parityFiles().size(); ++i)
		{
			ParityFile &	pf = parityInfo().parityFiles()[i];

			// Missing?

			if (!doesFileExist(pf.filespec()))
			{
				// File no longer exists.. update it's status

				pf.status() = ParityFile::Missing;
				pf.statusString() = _T("Missing");
			}

			// If not valid, try to validate...

			if (pf.status() != ParityFile::Valid && !validatedThisPass)
			{
				parityInfo().validateParFile(pf, 1, 0);
				drawMaps();

				// If we just validated a file, stop validating so our stats can update

				if (pf.status() == ParityFile::Valid) validatedThisPass = true;
			}

			// Validated?

			if (pf.volumeNumber() && pf.status() == ParityFile::Valid) ++validParityFiles;

			// Inform the user, handle pause, cancel, etc...

			if (!progCallback(this, _T("Monitoring downloads..."), 0)) throw true;

			// Cancelled?

			if (!monitorFlag()) throw true;
		}

		// Count data files

		unsigned int	dataFilesNeeded = 0;
		unsigned int	validDataFiles = 0;
		float		averageDataFileSize = 0.0f;
		for (unsigned int i = 0; i < parityInfo().dataFiles().size(); ++i)
		{
			// Inform the user, handle pause, cancel, etc...

			if (!progCallback(this, _T("Monitoring downloads..."), 0)) throw true;

			DataFile &	df = parityInfo().dataFiles()[i];

			// Missing?

			if (!doesFileExist(df.filespec()))
			{
				// File no longer exists.. update it's status

				df.status() = DataFile::Missing;
				df.statusString() = _T("Missing");
			}

			// If not valid, try to validate...

			if (df.status() != DataFile::Valid && !validatedThisPass)
			{
				parityInfo().validateDataFile(df, 1, 0);
				drawMaps();

				// If we just validated a file, stop validating so our stats can update

				if (df.status() == DataFile::Valid) validatedThisPass = true;
			}

			// Only track stats on recoverable files

			if (df.recoverable())
			{
				// Average size...

				averageDataFileSize += df.fileSize();

				// Needed file

				++dataFilesNeeded;

				// Validated?

				if (df.status() == DataFile::Valid) ++validDataFiles;
			}

			// Inform the user, handle pause, cancel, etc...

			if (!progCallback(this, _T("Monitoring downloads..."), 0)) throw true;

			// Cancelled?

			if (!monitorFlag()) throw true;
		}

		averageDataFileSize /= dataFilesNeeded;

		// Done?

		if (validParityFiles + validDataFiles >= dataFilesNeeded)
		{
			if (validDataFiles != dataFilesNeeded)
			{
				if (AfxMessageBox(	_T("The download monitor has determined that your download\n")
							_T("can be completed by using parity archives to recover\n")
							_T("the remaining files. Would you like to recover them now?"), MB_YESNO) == IDYES)
				{
					doRepair = true;
				}
			}
			else
			{
				MessageBeep(MB_ICONASTERISK);
			}

			throw true;
		}

		// Not done yet, go through the data files and tally up the partials...

		__int64		dataBytesDownloaded = 0;
		__int64		dataBytesNeeded = 0;
		__int64		dataBytesValid = 0;
		unsigned int	parityFilesUsed = 0;

		for (unsigned int i = 0; i < parityInfo().dataFiles().size(); ++i)
		{
			// Current data file...

			DataFile &	df = parityInfo().dataFiles()[i];

			// Skip non-recoverable files...

			if (!df.recoverable()) continue;

			// Need this file...

			dataBytesNeeded += df.fileSize();

			// Classify the file... Valid, Corrupt, Error, Missing, Misnamed, Unknown

			if (df.status() == DataFile::Valid)
			{
				dataBytesDownloaded += df.fileSize();
				dataBytesValid += df.fileSize();
			}

			// If it's misnamed, ask the user if they want to fix it

			else if (df.status() == DataFile::Misnamed)
			{
				AfxMessageBox(	_T("The download monitor has found at least one misnamed file\n")
						_T("in the dataset.\n")
						_T("\n")
						_T("The Fix Names process can repair this, but for reasons of\n")
						_T("data-integrity, this process should never be automated.\n")
						_T("\n")
						_T("The download monitor cannot proceed until you have run the\n")
						_T("Fix Names process or correct the filenames manually.\n")
						_T("\n")
						_T("This monitor will now stop. Please repair the names and\n")
						_T("restart the monitor."));

				throw true;
			}

			// Corrupt and missing files (files that definately need a repair and are not currently being downloaded)
			// can use a valid parity file, therefore, properly maintaining the progress bars.

			else if ((df.status() == DataFile::Missing || df.status() == DataFile::Corrupt) && parityFilesUsed < validParityFiles)
			{
				dataBytesDownloaded += df.fileSize();
				dataBytesValid += df.fileSize();
				parityFilesUsed++;
			}

			// Everything else gets counted as partial downloads...

			else
			{
				// Get the file size, and count that...

				unsigned int	size = getFileLength(df.filespec());
				dataBytesDownloaded += size;
			}
		}

		// Like we did for data, but do it for parity now...

		for (unsigned int i = 0; i < parityInfo().parityFiles().size(); ++i)
		{
			// Current parity file...

			ParityFile &	pf = parityInfo().parityFiles()[i];

			// Skip PAR files

			if (pf.volumeNumber() == 0) continue;

			// Classify the file... Valid, Corrupt, Error, Missing, Misnamed, Unknown

			if (pf.status() == ParityFile::Valid)
			{
				// Only count those parity files we didn't already use to cover for a missing/corrupt data file

				if (parityFilesUsed < validParityFiles)
				{
					dataBytesDownloaded += static_cast<unsigned int>(averageDataFileSize);
					dataBytesValid += static_cast<unsigned int>(averageDataFileSize);
					parityFilesUsed++;
				}
			}

			// Everything else gets counted as partial downloadeds...

			else
			{
				// Get the file size, and count that...

				unsigned int	size = getFileLength(pf.filespec());
				dataBytesDownloaded += size;
			}
		}

		// Potentially downloaded data (i.e. not validated yet)

		validPercent = static_cast<float>(static_cast<double>(dataBytesValid) / static_cast<double>(dataBytesNeeded));
		potentialPercent = static_cast<float>(static_cast<double>(dataBytesDownloaded) / static_cast<double>(dataBytesNeeded));

		// Update our special progress bar

		drawMonitorProgress();

		// Remember stuff

		saveStates(parityInfo().setHash(), parityInfo().parityFiles(), parityInfo().dataFiles());

		// If we just validated a file, use a shorter interval so we can keep going

		if (validatedThisPass) downloadMonitorInterval = 0;

		// Keep the love alive!

		SetTimer(monitorTimer, downloadMonitorInterval, NULL);
	}
	catch(const bool exitFlag)
	{
		if (exitFlag) killDownloadMonitor();
		if (doRepair) OnBnClickedRepairButton();
	}
}

// ---------------------------------------------------------------------------------------------------------------------------------

void	FSRaidDialog::drawMonitorProgress()
{
	// System colors

	unsigned int	background   = GetSysColor(COLOR_3DFACE);
	unsigned int	outerHilight = GetSysColor(COLOR_3DHILIGHT);
	unsigned int	innerHilight = GetSysColor(COLOR_3DLIGHT);
	unsigned int	outerShadow  = GetSysColor(COLOR_3DDKSHADOW);
	unsigned int	innerShadow  = GetSysColor(COLOR_3DSHADOW);

	// We'll be drawing our own progress bar...

	CDC *	dc = GetDC();
	CRect	r;
	progress().GetWindowRect(r);
	ScreenToClient(r);
	r.right--;
	r.bottom--;
	dc->Draw3dRect(r, outerHilight, outerShadow);
	r.DeflateRect(1,1,1,1);
	dc->Draw3dRect(r, innerShadow, innerHilight);
	r.DeflateRect(1,1,1,1);
	dc->FillSolidRect(r, background);

	// Percentages, in terms of pixels...

	float	potential = potentialPercent;
	float	valid = validPercent;
	if (potential > 1.0f) potential = 1.0f;
	if (valid > 1.0f) valid = 1.0f;
	if (potential < valid) potential = valid;
	float	validWidth = static_cast<float>(r.Width()) * valid;
	float	potentialWidth = static_cast<float>(r.Width()) * potential;

	dc->FillSolidRect(r.left, r.top, static_cast<unsigned int>(potentialWidth), r.Height(), 0xC9786E);
	dc->FillSolidRect(r.left, r.top, static_cast<unsigned int>(validWidth), r.Height(), 0x844F48);

	ReleaseDC(dc);
}

// ---------------------------------------------------------------------------------------------------------------------------------
// FSRaidDialog.cpp - End of file
// ---------------------------------------------------------------------------------------------------------------------------------

