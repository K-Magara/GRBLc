// CustomControl.h : �w�b�_�[ �t�@�C��
//

#pragma once

//	�R���{�{�b�N�X�̃L�[���͂�
//	CGRBLdlg::PreTranslateMessage() �ő������Ȃ�(?)�̂�
//	�R���{�{�b�N�X���̂ŏ���
#define	MAXADD_CMD		10
class CMyComboBox : public CComboBox
{
protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
public:
	int		AddString(LPCTSTR);
};

class CMyListBox : public CListBox
{
	void	CopyMessage(void);
protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};

/////////////////////////////////////////////////////////////////////////////
// CIntEdit �E�B���h�E

class CIntEdit : public CEdit
{
public:
	CIntEdit() {};

	operator int();
	CIntEdit& operator =(int);

protected:
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CFloatEdit �E�B���h�E

class CFloatEdit : public CEdit
{
public:
	CFloatEdit() {}

	operator float();
	CFloatEdit& operator =(float);

protected:
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);

	DECLARE_MESSAGE_MAP()
};
