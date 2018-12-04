// 
// AYA version 5
//
// �����P��ւ̍����}�b�`���s���@CWordMatch
// written by Mikage Sawatari. 2004
// 
// CWordMatch �́C�\�ߕ����̌������o�^���Ă����C�w�蕶����̎w��ʒu����C
// �����ꂪ���݂��Ȃ����}�b�`�������s���܂��D
// �e������ɂ� int �l���P�֘A�Â��邱�Ƃ��o���C�}�b�`�����ꍇ�͊֘A�Â����l�C
// �}�b�`���Ȃ������ꍇ��-1���Ԃ�܂��D
// 

#ifndef	WORDMATCHH
#define	WORDMATCHH

#ifndef POSIX
#  include "stdafx.h"
#else
#  include <map>
#  include <string>
using namespace std;
#endif

class CWordMatch
{
public:
  void addWord(const wstring& str, int value);
  int search(const wstring& str, int pos);
  ~CWordMatch();
  
private:
  
  struct nextmap_s;
  typedef map<wchar_t, nextmap_s*> word_map;
  typedef pair<wchar_t, nextmap_s*> word_pair;
  
  struct nextmap_s
  {
    word_map next;
    int value;
  };
  
  nextmap_s wordmap;

  void freemap(word_map* wordmap);

};


#endif

