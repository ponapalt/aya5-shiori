// 
// AYA version 5
//
// ���ԃR�[�h�����O�ɏo�͂���N���X�@CLogExCode
// written by umeici. 2004
// 
// �f�o�b�O�p�̋@�\�ł��B������ɂ͊֌W���܂���B
// �����̊֐����f���o���o�͂́A�\�[�X�R�[�h�𗝉����Ă��Ȃ��Ɠǂݓ���̂ł��B
//

#ifndef	LOGEXCODEH
#define	LOGEXCODEH

//----

#ifndef POSIX
#include "stdafx.h"
#endif
#include "function.h"

class	CLogExCode
{
public:
	static void	OutExecutionCodeForCheck(void);
	static void	OutVariableInfoForCheck(void);
protected:
	static void	StructCellString(vector<CCell> *cellvector, wstring &formula);
	static void	StructSerialString(CStatement *st, wstring &formula);
	static void	StructArrayString(vector<CValueSub> &vs, wstring &enlist);
};

//----

#endif
