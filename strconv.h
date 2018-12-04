/* -*- c++ -*-
   文字コード変換ライブラリ(未完成)
   今のところはUTF-8 <-> CP932の変換が出来るだけ。
   strconvはパブリックドメインです。
*/
#ifndef _AYA_STRCONV_H_INCLUDED_
#define _AYA_STRCONV_H_INCLUDED_
#include <string>
#include <exception>
#include <sstream>
#include <map>

namespace aya {
    class strconv;
    class bad_encoding_exception;
};

class aya::bad_encoding_exception : public std::exception {
private:
    std::string what_str;
    
public:
    bad_encoding_exception(int id) throw () {
	std::ostringstream buf;
	buf << "encoding id `" << id << "' is invalid" << std::ends;
	what_str = buf.str();
    }

    bad_encoding_exception(const std::string& name) throw () {
	std::ostringstream buf;
	buf << "encoding name `" << name << "' is invalid" << std::ends;
	what_str = buf.str();
    }

    virtual ~bad_encoding_exception() throw () {}

    virtual const char* what() const throw() {
	return what_str.c_str();
    }
};

class aya::strconv {
public:
    // 変換可能な文字コードの種類
    static const int UTF8; // UTF-8
    static const int CP932; // CP932

    // fromとtoにはintの定数を与える。
    // 正しいエンコード指定定数でなければaya::bad_encoding_exceptionを投げる。
    static std::string convert(
	const std::string& orig, int from, int to) throw (aya::bad_encoding_exception);

    // fromとtoにはエンコード名文字列を与える。
    // 正しい名前でなければaya::bad_encoding_exceptionを投げる。
    static std::string convert(
	const std::string& orig,
	const std::string& from,
	const std::string& to) throw (aya::bad_encoding_exception);

    // UTF-8文字列を任意のbasic_stringに変換する。
    // convert_utf8_to_ucs<wchar_t>(str) のように使う。
    template <typename char_type>
    static std::basic_string<char_type> convert_utf8_to_ucs(const std::string& utf8) {
	typename std::basic_string<char_type> out;

	std::string::const_iterator end = utf8.end();
	for (std::string::const_iterator ite = utf8.begin(); ite != end; ) {
	    unsigned int unicode = fetch_utf8(ite, end);
	    out += static_cast<char_type>(unicode);
	}

	return out;
    }

    // 任意のbasic_stringをUTF-8文字列に変換する。
    template <typename char_type>
    static std::string convert_ucs_to_utf8(const std::basic_string<char_type>& ucs) {
	std::string out;

	for (typename std::basic_string<char_type>::const_iterator ite = ucs.begin();
	     ite != ucs.end(); ite++) {
	    append_utf8(out, static_cast<unsigned int>(*ite));
	}

	return out;
    }

private:
    strconv() {}; // コンストラクタを呼ぶ事は禁止する。

    static std::map<std::string, int> *encoding_name_to_id;
    static inline void init_encoding_name_table() {
	if (encoding_name_to_id == NULL) {
	    encoding_name_to_id = new std::map<std::string, int>();
	    (*encoding_name_to_id)["UTF-8"] = UTF8;
	    (*encoding_name_to_id)["CP932"] = CP932;
	}
    }

    // 次の二つの配列はstrconv_cp932.cppで定義される。
    static const unsigned int table_cp932_to_unicode[];
    static const unsigned int table_unicode_to_cp932[];

    static std::string conv_cp932_to_utf8(const std::string& src) throw ();
    static std::string conv_utf8_to_cp932(const std::string& src) throw ();

    // 動作速度優先。
    static inline void append_utf8(
	std::string& dest, unsigned int unicode) throw () {
	if (unicode <= 0x7f) {
	    // 1バイト
	    dest += static_cast<char>(unicode & 0x7f);
	}
	else if (unicode <= 0x07ff) {
	    // 2バイト
	    dest += static_cast<char>(0xc0 | ((unicode >>  6) & 0x1f));
	    dest += static_cast<char>(0x80 | ( unicode        & 0x3f));
	}
	else if (unicode <= 0x00ffff) {
	    // 3バイト
	    dest += static_cast<char>(0xe0 | ((unicode >> 12) & 0x0f));
	    dest += static_cast<char>(0x80 | ((unicode >>  6) & 0x3f));
	    dest += static_cast<char>(0x80 | ( unicode        & 0x3f));
	}
	else if (unicode <= 0x1fffff) {
	    // 4バイト
	    dest += static_cast<char>(0xf0 | ((unicode >> 18) & 0x07));
	    dest += static_cast<char>(0x80 | ((unicode >> 12) & 0x3f));
	    dest += static_cast<char>(0x80 | ((unicode >>  6) & 0x3f));
	    dest += static_cast<char>(0x80 | ( unicode        & 0x3f));
	}
	else if (unicode <= 0x03ffffff) {
	    // 5バイト
	    dest += static_cast<char>(0xf8 | ((unicode >> 24) & 0x03));
	    dest += static_cast<char>(0x80 | ((unicode >> 18) & 0x3f));
	    dest += static_cast<char>(0x80 | ((unicode >> 12) & 0x3f));
	    dest += static_cast<char>(0x80 | ((unicode >>  6) & 0x3f));
	    dest += static_cast<char>(0x80 | ( unicode        & 0x3f));
	}
	else if (unicode <= 0x7fffffff) {
	    // 6バイト
	    dest += static_cast<char>(0xfc | ((unicode >> 30) & 0x01));
	    dest += static_cast<char>(0x80 | ((unicode >> 24) & 0x3f));
	    dest += static_cast<char>(0x80 | ((unicode >> 18) & 0x3f));
	    dest += static_cast<char>(0x80 | ((unicode >> 12) & 0x3f));
	    dest += static_cast<char>(0x80 | ((unicode >>  6) & 0x3f));
	    dest += static_cast<char>(0x80 | ( unicode        & 0x3f));
	}
    }

    // 動作速度優先。
    static inline unsigned int fetch_utf8(
	std::string::const_iterator &ite, std::string::const_iterator &end) throw () {
	// まず始めの1バイトを取る。
	unsigned char c = *ite; ite++;
	unsigned int unicode;
	if ((c & 0x80) == 0) {
	    // 1バイト
	    unicode = c;
	}
	else if ((c & 0xe0) == 0xc0) {
	    // 2バイト
	    unicode  = (c & 0x1f) <<  6;
#define _AYA_STRCONV_H_REPEATATION_1_ if (ite == end) { return 0; } else { c = *ite; ite++; }
	    _AYA_STRCONV_H_REPEATATION_1_;
	    unicode |=  c & 0x3f;
	}
	else if ((c & 0xf0) == 0xe0) {
	    // 3バイト
	    unicode  = (c & 0x0f) << 12; _AYA_STRCONV_H_REPEATATION_1_;
	    unicode |= (c & 0x3f) <<  6; _AYA_STRCONV_H_REPEATATION_1_;
	    unicode |=  c & 0x3f;
	}
	else if ((c & 0xf8) == 0xf0) {
	    // 4バイト
	    unicode  = (c & 0x07) << 18; _AYA_STRCONV_H_REPEATATION_1_;
	    unicode |= (c & 0x3f) << 12; _AYA_STRCONV_H_REPEATATION_1_;
	    unicode |= (c & 0x3f) <<  6; _AYA_STRCONV_H_REPEATATION_1_;
	    unicode |=  c & 0x3f;
	}
	else if ((c & 0xfc) == 0xf8) {
	    // 5バイト
	    unicode  = (c & 0x03) << 24; _AYA_STRCONV_H_REPEATATION_1_;
	    unicode |= (c & 0x3f) << 18; _AYA_STRCONV_H_REPEATATION_1_;
	    unicode |= (c & 0x3f) << 12; _AYA_STRCONV_H_REPEATATION_1_;
	    unicode |= (c & 0x3f) <<  6; _AYA_STRCONV_H_REPEATATION_1_;
	    unicode |=  c & 0x3f;
	}
	else if ((c & 0xfe) == 0xfc) {
	    // 6バイト
	    unicode  = (c & 0x01) << 30; _AYA_STRCONV_H_REPEATATION_1_;
	    unicode |= (c & 0x3f) << 24; _AYA_STRCONV_H_REPEATATION_1_;
	    unicode |= (c & 0x3f) << 18; _AYA_STRCONV_H_REPEATATION_1_;
	    unicode |= (c & 0x3f) << 12; _AYA_STRCONV_H_REPEATATION_1_;
	    unicode |= (c & 0x3f) <<  6; _AYA_STRCONV_H_REPEATATION_1_;
	    unicode |=  c & 0x3f;
	}
	return unicode;
    }
};

#endif /* _AYA_STRCONV_H_INCLUDED_ */
