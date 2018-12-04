#include "ccct.h"
#include "manifest.h"
#include "strconv.h"
#include <stdlib.h>
#include <string.h>
#include <iostream>
using namespace std;
using namespace aya;

static char* malloc_str(const string& str) {
    char* s = static_cast<char*>(malloc(str.length() + 1));
    memcpy(s, str.c_str(), str.length() + 1);
    return s;
}

static wchar_t* malloc_str(const wstring& str) {
    size_t len = (str.length() + 1) * sizeof(wchar_t);
    wchar_t* s = static_cast<wchar_t*>(malloc(len));
    memcpy(s, str.data(), len);
    return s;
}

char* Ccct::Ucs2ToMbcs(const wchar_t* wstr, int charset) {
    return Ucs2ToMbcs(wstring(wstr), charset);
}
char* Ccct::Ucs2ToMbcs(const wstring& wstr, int charset) {
    std::string utf8 = strconv::convert_ucs_to_utf8<wchar_t>(wstr);
    std::string mbcs;
    if (charset == CHARSET_UTF8) {
	// �Ѵ�����
	mbcs = utf8;
    }
    else if (charset == CHARSET_SJIS || charset == CHARSET_DEFAULT) {
	// ��OS�ǥե���ȡפϾ��SJIS��
	mbcs = strconv::convert(utf8, strconv::UTF8, strconv::CP932);
    }
    return malloc_str(mbcs);
}

wchar_t* Ccct::MbcsToUcs2(const char* mstr, int charset) {
    return MbcsToUcs2(string(mstr), charset);
}
wchar_t* Ccct::MbcsToUcs2(const string &mstr, int charset) {
    std::string utf8;
    if (charset == CHARSET_UTF8) {
	// �Ѵ�����
	utf8 = mstr;
    }
    else if (charset == CHARSET_SJIS || charset == CHARSET_DEFAULT) {
	// ��OS�ǥե���ȡפϾ��SJIS��
	utf8 = strconv::convert(mstr, strconv::CP932, strconv::UTF8);
    }
    return malloc_str(strconv::convert_utf8_to_ucs<wchar_t>(utf8));
}
