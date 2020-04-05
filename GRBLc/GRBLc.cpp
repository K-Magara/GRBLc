// GRBLc.cpp : DLL �̏��������[�`���ł��B
//

#include "stdafx.h"
#include "NCVCaddin.h"
#include "GRBLc.h"
#include "boost/system/system_error.hpp"
#include "boost/system/error_code.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern	LPCTSTR		g_szFuncName = "GRBLc";
extern	LPCTSTR		g_szValue = "%.3f";
extern	LPCTSTR		g_szFeed = "F%d";
extern	LPCTSTR		g_szXYZ = "XYZ";
extern	LPCTSTR		g_szErrorAxis = "-.---";
extern	int			g_inputJogKey[] = {
	VK_LEFT, VK_RIGHT, VK_UP, VK_DOWN, VK_PRIOR, VK_NEXT
};
extern	char		g_inputCtrlKey[] = {
	'O', 'R'
};

BEGIN_MESSAGE_MAP(CGRBLcApp, CWinApp)
END_MESSAGE_MAP()

// CGRBLcApp �R���X�g���N�V����

CGRBLcApp::CGRBLcApp()
{
//	m_pDlg = NULL;
}

// �B��� CGRBLcApp �I�u�W�F�N�g�ł��B
CGRBLcApp theApp;
#ifdef _DEBUG
DbgConsole	theDebug;	// �f�o�b�O�p�R���\�[��
#endif

// CGRBLcApp ������
/*
BOOL CGRBLcApp::InitInstance()
{
	BOOL	bResult = CWinApp::InitInstance();
	if ( bResult ) {
		m_optGRBL.Read();
	}
	return bResult;
}
*/
/////////////////////////////////////////////////////////////////////////////
// NCVC ��޲݊֐�

NCADDIN BOOL NCVC_Initialize(NCVCINITIALIZE* nci)
{
#ifdef _DEBUG
	std::cout << "--GRBLc.dll CALL NCVC_Initialize\n";
#endif

	// ��޲݂̕K�v���
	nci->dwSize = sizeof(NCVCINITIALIZE);
	nci->dwType = NCVCADIN_FLG_NCDFILE;
	nci->lpszMenuName[NCVCADIN_ARY_NCDFILE] = "GRBLc...";
	nci->lpszFuncName[NCVCADIN_ARY_NCDFILE] = g_szFuncName;
	nci->lpszAddinName	= g_szFuncName;
	nci->lpszCopyright	= "MNCT-S K.Magara";
	nci->lpszSupport	= "http://s-gikan2.maizuru-ct.ac.jp/";

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// NCVC ����Ċ֐�

NCADDIN void GRBLc(void)
{
	NCVCHANDLE hDoc = NCVC_GetDocument(NULL);
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
/*
	if ( AfxGetGRBLcApp()->m_pDlg )
		AfxGetGRBLcApp()->m_pDlg->SetFocus();
	else {
		AfxGetGRBLcApp()->m_pDlg = new CGRBLdlg;	// delete�� CGRBLdlg::PostNcDestroy()
		AfxGetGRBLcApp()->m_pDlg->Create(IDD_DIALOG1);
	}
*/
	try {
		AfxGetGRBLcApp()->GetOption()->Read();
		CGRBLdlg	dlg(hDoc);
		dlg.DoModal();
	}
	catch (boost::system::system_error& e) {
		CString	strMsg("��O�����o���܂����B�����𒆒f���܂��B\ncode=");
		strMsg += e.what();
		AfxMessageBox(strMsg);
	}
}
