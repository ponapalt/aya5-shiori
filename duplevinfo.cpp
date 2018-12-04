// 
// AYA version 5
//
// �d����𐧌���s�Ȃ��N���X�@CDuplEvInfo
// - �又����
// written by umeici. 2004
// 

#ifndef POSIX
#  include "stdafx.h"
#endif
#include "selecter.h"
#include "sysfunc.h"
#include "value.h"
#include "misc.h"
#include "log.h"
#include "wsex.h"
#include "mt19937ar.h"

extern CSystemFunction	sysfunction;
extern CLog				logger;

/* -----------------------------------------------------------------------
 *  �֐���  �F  CDuplEvInfo::Choice
 *  �@�\�T�v�F  ��₩��I�����ďo�͂��܂�
 * -----------------------------------------------------------------------
 */
CValue	CDuplEvInfo::Choice(int areanum, const vector<CVecValue> &values, int mode)
{
	// �̈斈�̌�␔�Ƒ������X�V�@�ω����������ꍇ�͏��񏇏�������������
	if (UpdateNums(areanum, values))
		InitRoundOrder(mode);

	// �l�̎擾�Ə��񐧌�
	CValue	result = GetValue(areanum, values);

	// ����ʒu��i�߂�@���񂪊��������珄�񏇏�������������
	index++;
	if (index >= total)
		InitRoundOrder(mode);

	return result;
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CDuplEvInfo::InitRoundOrder
 *  �@�\�T�v�F  ���񏇏������������܂�
 * -----------------------------------------------------------------------
 */
void	CDuplEvInfo::InitRoundOrder(int mode)
{
	// ������
	index = 0;
	roundorder.clear();
	int	t_total = total;

	for(int i = 0; i < t_total; i++)
		roundorder.push_back(i);

	// nonoverlap�̏ꍇ�͏��񏇏�������������
	// �����_���ɑI��ŃX���b�v�A��(�L�[��/2)��J��Ԃ��Ƃ����r�����Ȃ����n�I�ȕ��@
	if (mode == CHOICETYPE_NONOVERLAP) {
		int	shufflenum = (total + 1) >> 1;
		for(int i = 0; i < shufflenum; i++) {
			int i0 = (int)(genrand_real2()*(double)t_total);
			int i1 = (int)(genrand_real2()*(double)t_total);
		    
			if (i0 != i1)
				exchange(roundorder[i0], roundorder[i1]);
		}
	}

}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CDuplEvInfo::UpdateNums
 *  �@�\�T�v�F  �̈斈�̌�␔�Ƒ������X�V���܂�
 *  �Ԓl�@�@�@  0/1=�ω��Ȃ�/����
 * -----------------------------------------------------------------------
 */
char	CDuplEvInfo::UpdateNums(int areanum, const vector<CVecValue> &values)
{
	// ���̌�␔��ۑ����Ă���
	vector<int>	bef_num(num.begin(), num.end());
	int	bef_numlenm1 = bef_num.size() - 1;

	// �̈斈�̌�␔�Ƒg�ݍ��킹�������X�V
	// ��␔�ɕω����������ꍇ�̓t���O�ɋL�^����
	num.clear();
	total = 1;
	char	changed = (areanum != bef_numlenm1) ? 1 : 0;
	for(int i = 0; i <= areanum; i++) {
		int	t_num = values[i].array.size();
		num.push_back(t_num);
		total *= t_num;
		if (i <= bef_numlenm1)
			if (bef_num[i] != t_num)
				changed = 1;
	}

	return changed;
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CDuplEvInfo::GetValue
 *  �@�\�T�v�F  ���݂̏���ʒu����l���擾���܂�
 *
 *  �i�[�̈悪������Ȃ��ꍇ�͂�������̂܂܏o���̂Œl�̌^���ی삳��܂��B
 *  �̈悪��������ꍇ�͂����͕�����Ƃ��Č�������܂��̂ŁA������^�ł̏o�͂ƂȂ�܂��B
 * -----------------------------------------------------------------------
 */
CValue	CDuplEvInfo::GetValue(int areanum, const vector<CVecValue> &values)
{
	int	t_index = roundorder[index];

	sysfunction.SetLso(t_index);

	if (areanum) {
		wstring	result;
		for(int i = 0; i <= areanum; i++) {
			int	next = t_index/num[i];
			result += values[i].array[t_index - next*(num[i])].GetValueString();
			t_index = next;
		}
		return CValue(result);
	}
	else
		return values[0].array[t_index];
}
