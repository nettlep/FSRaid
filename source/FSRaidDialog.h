// ---------------------------------------------------------------------------------------------------------------------------------
//  ______  _____ _____        _     _ _____  _       _                 _     
// |  ____|/ ____|  __ \      (_)   | |  __ \(_)     | |               | |    
// | |__  | (___ | |__) | __ _ _  __| | |  | |_  __ _| | ___   __ _    | |__  
// |  __|  \___ \|  _  / / _` | |/ _` | |  | | |/ _` | |/ _ \ / _` |   | '_ \ 
// | |     ____) | | \ \| (_| | | (_| | |__| | | (_| | | (_) | (_| | _ | | | |
// |_|    |_____/|_|  \_\\__,_|_|\__,_|_____/|_|\__,_|_|\___/ \__, |(_)|_| |_|
//                                                             __/ |          
//                                                            |___/           
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

#ifndef	_H_FSRAIDDIALOG
#define _H_FSRAIDDIALOG
#include "afxwin.h"
#include "afxcmn.h"

// ---------------------------------------------------------------------------------------------------------------------------------
// Module setup (required includes, macros, etc.)
// ---------------------------------------------------------------------------------------------------------------------------------

#include "ParityInfo.h"

class	AboutBox;
class	HelpDialog;

// ---------------------------------------------------------------------------------------------------------------------------------

class	FSRaidDialog : public CDialog
{
public:
				FSRaidDialog(CWnd* pParent = NULL);
virtual				~FSRaidDialog();

		enum		{ IDD = IDD_FSRAID_DIALOG };

	// Implementation

virtual		void		DoDataExchange(CDataExchange* pDX);
virtual		BOOL		PreTranslateMessage(MSG* pMsg);
virtual		void		resizeWindow(const bool large = true);
virtual		void		enableButtons(const bool enabled = true);
virtual		void		drawMaps();
virtual		void		drawMaps(CDC & dc);
virtual		bool		dataFileIndexFromMousePoint(const CPoint & p, int & index);
virtual		bool		parityFileIndexFromMousePoint(const CPoint & p, int & index);
virtual		void		checkFiles(bool & allOK);
virtual		void		scanForMissingFiles();
virtual		bool		checkDataFile(const int index, const unsigned int totalFiles, const unsigned int curIndex);
virtual		bool		repairDataFile(const int index);
virtual		bool		checkParityFile(const int index, const unsigned int totalFiles, const unsigned int curIndex);
virtual		void		copyStringToClipboard(const fstl::wstring & str);
virtual		void		calcStats(unsigned int & valid, unsigned int & corrupt, unsigned int & misnamed, unsigned int & missing, unsigned int & unknown, unsigned int & error, unsigned int & needed, unsigned int & recoverable, unsigned int & validRecoverable);
virtual		bool		shouldRepair() const;
virtual		void		setFileAssociations();
virtual		bool		checkFileAssociations();
virtual		bool		saveStates(const unsigned char setHash[EmDeeFive::HASH_SIZE_IN_BYTES], ParityFileArray & pfa, DataFileArray & dfa);
virtual		bool		loadStates(const unsigned char setHash[EmDeeFive::HASH_SIZE_IN_BYTES], ParityFileArray & pfa, DataFileArray & dfa);
virtual		bool		scanForParityFiles();
virtual		void		killDownloadMonitor();
virtual		void		monitorDownload();
virtual		void		drawMonitorProgress();

	// Generated message map functions

virtual		BOOL		OnInitDialog();
virtual		BOOL		OnCommand(WPARAM wParam, LPARAM lParam);
afx_msg		void		OnPaint();
afx_msg		HCURSOR		OnQueryDragIcon();
afx_msg		void		OnMouseMove(UINT nFlags, CPoint point);
afx_msg		void		OnRButtonUp(UINT nFlags, CPoint point);
afx_msg 	void 		OnBnClickedLoadButton();
afx_msg 	void 		OnBnClickedCreateButton();
afx_msg 	void 		OnBnClickedCheckButton();
afx_msg 	void 		OnBnClickedCheckallButton();
afx_msg		void		OnBnClickedFixnamesButton();
afx_msg 	void 		OnBnClickedRepairButton();
afx_msg 	void 		OnBnClickedHelpButton();
afx_msg 	void 		OnBnClickedOk();
afx_msg 	void 		OnBnClickedStopButton();
afx_msg		void		OnBnClickedAboutButton();
afx_msg		void		OnBnClickedPrefsButton();
afx_msg		void		OnTimer(UINT nIDEvent);
afx_msg		void		OnBnClickedPauseButton();
afx_msg		void		OnDropFiles(HDROP hDropInfo);
afx_msg		void		OnSize(UINT nType, int cx, int cy);
afx_msg		void		OnGetMinMaxInfo(MINMAXINFO* lpMMI);
afx_msg		void		OnBnClickedMonitordownloadButton();
afx_msg		void		OnClose();
afx_msg		void		OnMove(int x, int y);
afx_msg		void		OnMoving(UINT fwSide, LPRECT pRect);
afx_msg		void		OnNcLButtonDown(UINT nHitTest, CPoint point);
afx_msg		void		OnNcLButtonUp(UINT nHitTest, CPoint point);
afx_msg		BOOL		OnHelpInfo(HELPINFO* pHelpInfo);

	// Accessors

inline		AboutBox *&	aboutDialog()		{return _aboutDialog;}
inline	const	AboutBox *	aboutDialog() const	{return _aboutDialog;}
inline		HelpDialog *&	helpDialog()		{return _helpDialog;}
inline	const	HelpDialog *	helpDialog() const	{return _helpDialog;}
inline		bool &		monitorFlag()		{return _monitorFlag;}
inline	const	bool		monitorFlag() const	{return _monitorFlag;}
inline		bool &		cancelFlag()		{return _cancelFlag;}
inline	const	bool		cancelFlag() const	{return _cancelFlag;}
inline		bool &		pausedFlag()		{return _pausedFlag;}
inline	const	bool		pausedFlag() const	{return _pausedFlag;}
inline		bool &		silent()		{return _silent;}
inline	const	bool		silent() const		{return _silent;}
inline		bool &		busy()		{return _busy;}
inline	const	bool		busy() const		{return _busy;}
inline		ParityInfo &	parityInfo()		{return _parityInfo;}
inline	const	ParityInfo &	parityInfo() const	{return _parityInfo;}
inline		CToolTipCtrl *&	toolTip()		{return _toolTip;}
inline	const	CToolTipCtrl *	toolTip() const		{return _toolTip;}
inline		CProgressCtrl &	progress()		{return _progress;}
inline	const	CProgressCtrl &	progress() const	{return _progress;}
inline		CStatic &	progressText()		{return _progressText;}
inline	const	CStatic &	progressText() const	{return _progressText;}
inline		unsigned int &	contextIndex()		{return _contextIndex;}
inline	const	unsigned int	contextIndex() const	{return _contextIndex;}
inline		CButton &	stopButton()		{return _stopButton;}
inline	const	CButton &	stopButton() const	{return _stopButton;}
inline		CButton &	pauseButton()		{return _pauseButton;}
inline	const	CButton &	pauseButton() const	{return _pauseButton;}
inline		fstl::wstring &	startupFile()		{return _startupFile;}
inline	const	fstl::wstring &	startupFile() const	{return _startupFile;}

private:

		HICON		m_hIcon;

		AboutBox *	_aboutDialog;
		HelpDialog *	_helpDialog;
		bool		_monitorFlag;
		bool		_cancelFlag;
		bool		_pausedFlag;
		bool		_silent;
		bool		_busy;
		ParityInfo	_parityInfo;
		CToolTipCtrl *	_toolTip;

		fstl::wstring	_startupFile;
		CEdit 		baseEdit;
		CStatic		progressSeparator;
		CProgressCtrl	_progress;
		CStatic		_progressText;
		unsigned int	_contextIndex;
		CRect		initialDialogRect;
		CButton		loadButton;
		CButton		createButton;
		CButton		checkButton;
		CButton		checkAllButton;
		CButton		fixNamesButton;
		CButton		repairButton;
		CButton		helpButton;
		CButton		closeButton;
		CButton		_stopButton;
		CButton		aboutButton;
		CButton		prefsButton;
		CStatic		dataFileMapFrame;
		CEdit		dataFileMapText;
		CStatic		parityFileMapFrame;
		CEdit		parityFileMapText;
		CEdit		archiveStatusEdit;
		CButton		_pauseButton;
		CButton		downloadMonitorButton;
		CStatic		unknownLegend;
		CStatic		validLegend;
		CStatic		corruptLegend;
		CStatic		missingLegend;
		CStatic		misnamedLegend;
		CStatic		errorLegend;
		CStatic		infoBorderFrame;
		CStatic		validLegendText;
		CStatic		corruptLegendText;
		CStatic		errorLegendText;
		CStatic		missingLegendText;
		CStatic		misnamedLegendText;
		CStatic		unknownLegendText;
		unsigned int	resizeDistance;
		float		validPercent;
		float		potentialPercent;
		int		windowX;
		int		windowY;
		int		windowW;
		POINT		dragPos;
		POINT		dragCorner;

				DECLARE_MESSAGE_MAP()
};

#endif // _H_FSRAIDDIALOG
// ---------------------------------------------------------------------------------------------------------------------------------
// FSRaidDialog.h - End of file
// ---------------------------------------------------------------------------------------------------------------------------------
