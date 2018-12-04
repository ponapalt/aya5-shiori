/* -*- c++ -*-
   POSIX環境下で使うユーティリティ類。
*/
#include "posix_utils.h"
#include <iostream>
#include <fstream>
#include <boost/scoped_array.hpp>
using namespace std;
using namespace boost;

string lc(const string& fname) {
    string result;
    for (string::const_iterator ite = fname.begin(); ite != fname.end(); ite++) {
	if (*ite >= 'A' && *ite <= 'Z') {
	    result += (*ite + 0x20);
	}
	else {
	    result += *ite;
	}
    }
    return result;
}

string get_fname(const string& fpath) {
    string::size_type slash_pos = fpath.rfind('/');
    if (slash_pos == string::npos) {
	return fpath;
    }
    else {
	return fpath.substr(slash_pos + 1);
    }
}

string get_extension(const string& fname) {
    string::size_type period_pos = fname.rfind('.');
    if (period_pos == string::npos) {
	return string();
    }
    else {
	return fname.substr(period_pos + 1);
    }
}

string drop_extension(const string& fname) {
    string::size_type period_pos = fname.rfind('.');
    if (period_pos == string::npos) {
	return fname;
    }
    else {
	return fname.substr(0, period_pos);
    }
}

string change_extension(const string& fname, const string& extension) {
    return drop_extension(fname) + '.' + extension;
}

string::size_type file_content_search(const string& file, const string& str) {
    string content;
    
    ifstream is(file.c_str());
    scoped_array<char> buf(new char[512]);
    while (is.good()) {
	is.read(buf.get(), 512);
	int len = is.gcount();
	if (len == 0) {
	    break;
	}
	content.append(buf.get(), len);
    }
    is.close();

    return bm_search(content, str);
}

string::size_type bm_search(const string& world, const string& data) {
    string::size_type data_len = data.length();
    scoped_array<string::size_type> skip(new string::size_type[256]);
    for (string::size_type i = 0; i < 256; i++) {
	skip[i] = data_len;
    }
    for (string::size_type i = 0; i < data_len-1; i++) {
	skip[static_cast<unsigned char>(data[i])] =
	    data_len - i - 1;
    }
    string::size_type limit = world.length() - data.length();
    for (string::size_type i = 0;
	 i <= limit;
	 i += skip[static_cast<unsigned char>(world[i+data_len-1])]) {
	if (world[i+data_len-1] != data[data_len-1]) {
	    continue;
	}

	bool matched = true;
	for (string::size_type j = 0; j < data_len; j++) {
	    if (world[i+j] != data[j]) {
		matched = false;
		break;
	    }
	}
	if (matched) {
	    return i;
	}
    }
    return string::npos;
}

wstring widen(const string& str) {
    wstring ws;
    for (string::const_iterator ite = str.begin(); ite != str.end(); ite++) {
	ws += static_cast<wchar_t>(*ite);
    }
    return ws;
}

string narrow(const wstring& str) {
    string s;
    for (wstring::const_iterator ite = str.begin(); ite != str.end(); ite++) {
	s += static_cast<char>(*ite);
    }
    return s;
}

void fix_filepath(string& str) {
    // \は/にし、重複した/を消して一つにする。
    for (string::iterator ite = str.begin(); ite != str.end(); ite++) {
	if (*ite == '\\') {
	    *ite = '/';
	}
    }
    while (true) {
	string::size_type pos = str.find("//");
	if (pos == string::npos) {
	    break;
	}
	str.erase(pos, 1);
    }
}

void fix_filepath(wstring& str) {
    // \は/にし、重複した/を消して一つにする。
    for (wstring::iterator ite = str.begin(); ite != str.end(); ite++) {
	if (*ite == L'\\') {
	    *ite = L'/';
	}
    }
    while (true) {
	wstring::size_type pos = str.find(L"//");
	if (pos == wstring::npos) {
	    break;
	}
	str.erase(pos, 1);
    }
}
