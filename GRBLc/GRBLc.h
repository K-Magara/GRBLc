// GRBLc.h : GRBLc.DLL �̃��C�� �w�b�_�[ �t�@�C��
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH �ɑ΂��Ă��̃t�@�C�����C���N���[�h����O�� 'stdafx.h' ���C���N���[�h���Ă�������"
#endif

#include "resource.h"		// ���C�� �V���{��
#include "GRBLdlg.h"
#include "GRBLcOption.h"
#include "ProbeOption.h"

// CGRBLcApp
// ���̃N���X�̎����Ɋւ��Ă� GRBLc.cpp ���Q�Ƃ��Ă��������B
//

class CGRBLcApp : public CWinApp
{
	CGRBLcOption	m_optGRBL;
	CProbeOption	m_optProbe;

public:
	CGRBLcApp();

//	CGRBLdlg*	m_pDlg;
	CGRBLcOption*	GetOption(void) {
		return &m_optGRBL;
	}
	CProbeOption*	GetProbeOption(void) {
		return &m_optProbe;
	}
//	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
