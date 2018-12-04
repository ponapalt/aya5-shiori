// 
// AYA version 5
//
// 構文解析/中間コードの生成を行うクラス　CParser0
// written by umeici. 2004
// 

#ifndef POSIX
#  include "stdafx.h"
#else
#  include "posix_utils.h"
#  include "messages.h"
#endif
#include "basis.h"
#include "parser0.h"
#include "parser1.h"
#include "sysfunc.h"
#include "comment.h"
#include "misc.h"
#include "log.h"
#include "wsex.h"
#include "ccct.h"
#include "wordmatch.h"

extern CBasis				basis;
extern vector<CFunction>	function;
extern CGlobalVariable		variable;
extern CLog					logger;

extern CWordMatch			formulatag_wm;
extern CWordMatch			function_wm;

#ifndef POSIX
#  define for if(0);else for
#endif

/* -----------------------------------------------------------------------
 *  関数名  ：  CParser0::Parse
 *  機能概要：  指定された辞書ファイル群を読み取り、実行可能な関数群を作成します
 *
 *  返値　　：  0/1=正常/エラー
 * -----------------------------------------------------------------------
 */
char	CParser0::Parse(int charset, const vector<wstring>& dics, int &lindex, int &ulindex, int &rindex)
{
	// 読み取り、構文解析、中間コードの生成
	logger.Message(3);
	vector<CDefine>	gdefines;
	int	errcount = 0;
	for(vector<wstring>::const_iterator it = dics.begin(); it != dics.end(); it++) {
		logger.Write(L"// ");
		logger.Filename(*it);
		errcount += LoadDictionary1(*it, gdefines, charset);
	}
	logger.Message(8);
	logger.Message(9);

	errcount += AddSimpleIfBrace();

	errcount += SetCellType();
	errcount += MakeCompleteFormula();

	// 中間コード生成の後処理と検査
	errcount += CParser1::CheckExecutionCode();

	// load/unload/request関数の位置を取得
	lindex  = GetFunctionIndexFromName(FUNCNAME_LOAD);
//	if (lindex == -1) {
//		logger.Error(E_E, 32, FUNCNAME_LOAD);
//		errcount++;
//	}
	ulindex = GetFunctionIndexFromName(FUNCNAME_UNLOAD);
//	if (ulindex == -1) {
//		logger.Error(E_E, 32, FUNCNAME_UNLOAD);
//		errcount++;
//	}
	rindex  = GetFunctionIndexFromName(FUNCNAME_REQUEST);
//	if (rindex == -1) {
//		logger.Error(E_E, 32, FUNCNAME_REQUEST);
//		errcount++;
//	}
	logger.Message(8);

	return (errcount) ? 1 : 0;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CParser0::ParseEmbedString
 *  機能概要：  文字列を演算可能な数式に変換します
 *
 *  返値　　：  0/1=正常/エラー
 * -----------------------------------------------------------------------
 */
char	CParser0::ParseEmbedString(wstring& str, CStatement &st, const wstring& dicfilename, int linecount)
{
	// 文字列を数式に成形
	if (!StructFormula(str, st.cell, dicfilename, linecount))
		return 1;

	// 数式の項の種類を判定
	for(vector<CCell>::iterator it = st.cell.begin(); it != st.cell.end(); it++) {
		if (it->v.GetType() != F_TAG_NOP)
			continue;

		if (SetCellType1(*it, 0, dicfilename, linecount))
			return 1;
	}

	// ()、[]の正当性判定
	if (CheckDepth1(st, dicfilename))
		return 1;

	// 埋め込み要素の分解
	if (ParseEmbeddedFactor1(st, dicfilename))
		return 1;

	// シングルクォート文字列を通常文字列へ置換
	ConvertPlainString1(st, dicfilename);

	// 演算順序決定
	if (CheckDepthAndSerialize1(st, dicfilename))
		return 1;

	// 後処理と検査
	if (CParser1::CheckExecutionCode1(st, dicfilename))
		return 1;

	return 0;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CParser0::LoadDictionary1
 *  機能概要：  一つの辞書ファイルを読み取り、大雑把に構文を解釈して蓄積していきます
 *
 *  返値　　：  1/0=エラー/正常
 * -----------------------------------------------------------------------
 */
char	CParser0::LoadDictionary1(const wstring& filename, vector<CDefine>& gdefines, int charset)
{
        wstring file = filename;
#ifdef POSIX
	fix_filepath(file);
#endif
	// ファイルを開く
	FILE	*fp = w_fopen((wchar_t *)file.c_str(), L"r");
	if (fp == NULL) {
		logger.Error(E_E, 5, file);
		return 1;
	}

	// 読み取り
	CComment	comment;
	char	ciphered = IsCipheredDic(file);
	wstring	linebuffer;
	int	depth = 0;
	int	targetfunction = -1;
	vector<CDefine>	defines;
	int	errcount = 0;
	for (int i = 1; ; i++) {
		// 1行読み込み　暗号化ファイルの場合は復号も行なう
		wstring	readline;
		int	ret;
		ret = ws_fgets(readline, fp, charset, ciphered, i);
		if (ret == WS_EOF)
			break;
		// 終端の改行および不要な空白（インデント等）を消す
		CutCrLf(readline);
		CutSpace(readline);
		// コメント処理
		comment.ExecuteSS_Top(readline);
		comment.Execute(readline);
		// 空行（もしくは全体がコメント行だった）なら次へ
		if (readline.size() == 0)
			continue;
		// 読み取り済バッファへ結合
		linebuffer.append(readline);
		// 終端が"/"なら結合なので"/"を消して次を読む
		if (readline[readline.size() - 1] == L'/') {
			linebuffer.erase(linebuffer.end() - 1);
			continue;
		}
		// プリプロセッサの場合は取得
		int	pp = GetPreProcess(linebuffer, defines, gdefines, file, i);
		// プリプロセッサであったらこの行の処理は終わり、次へ
		// 異常なプリプロセス行はエラー
		if (pp == 1)
			continue;
		else if (pp == 2) {
			errcount = 1;
			continue;
		}
		// プリプロセッサ#define、#globaldefine処理
		ExecDefinePreProcess(linebuffer, defines);	// #define
		ExecDefinePreProcess(linebuffer, gdefines);	// #globaldefine
		// コメント処理(2)
		comment.ExecuteSS_Tail(linebuffer);
		// {、}、;で分割
		vector<wstring>	factors;
		SeparateFactor(factors, linebuffer);
		// 分割された文字列を解析して関数を作成し、内部のステートメントを蓄積していく
		if (DefineFunctions(factors, file, i, depth, targetfunction))
			errcount = 1;
	}

	// ファイルを閉じる
	::fclose(fp);

	return errcount;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CParser0::GetPreProcess
 *  機能概要：  入力が#define/#globaldefineプリプロセッサであった場合、情報を取得します
 *
 *  返値　　：  0/1/2=プリプロセスではない/プリプロセス/エラー
 * -----------------------------------------------------------------------
 */
char	CParser0::GetPreProcess(wstring &str, vector<CDefine>& defines, vector<CDefine>& gdefines, const wstring& dicfilename,
			int linecount)
{
	wstring	base_str = str;

	// 先頭1バイトが"#"であるかを確認
	// （この関数に来るまでに空文字列は除外されているので、いきなり[0]を参照しても問題ない）
	if (base_str[0] != L'#')
		return 0;

	str = L"";

	// プリプロセス名称、変換前文字列、変換後文字列を取得
	// 取得できなければエラー
	wstring	pname, bef, aft;

	for(int i = 0; i < 2; i++) {
		// デリミタである空白もしくはタブを探す
		int	spc_pos = base_str.find(L' ' );
		int	tab_pos = base_str.find(L'\t');
#ifndef POSIX
		int	wsp_pos = base_str.find(L'　');
#else
		int	wsp_pos = base_str.find(L'\u3000');
#endif

		int	sep_pos = -1;
		if (spc_pos != -1)
			sep_pos = spc_pos;
		if (tab_pos != -1 && (sep_pos == -1 || (sep_pos != -1 && tab_pos < sep_pos)))
			sep_pos = tab_pos;
		if (wsp_pos != -1 && (sep_pos == -1 || (sep_pos != -1 && wsp_pos < sep_pos)))
			sep_pos = wsp_pos;
		if (sep_pos == -1) {
			logger.Error(E_E, 74, dicfilename, linecount);
			return 2;
		}
		// デリミタで分割して各要素を取得
		if (!i) {
			pname.assign(base_str, 0, sep_pos);
			CutSpace(pname);
			base_str.erase(0, sep_pos);
			CutSpace(base_str);
		}
		else {
			bef.assign(base_str, 0, sep_pos);
			CutSpace(bef);
			base_str.erase(0, sep_pos);
			aft = base_str;
			CutSpace(aft);
		}
	}

	if (!pname.size() || !bef.size() || !aft.size()) {
		logger.Error(E_E, 75, dicfilename, linecount);
		return 2;
	}

	// 種別の判定と情報の保持
	if (!pname.compare(L"#define")) {
		CDefine	adddefine(bef, aft);
		defines.push_back(adddefine);
	}
	else if (!pname.compare(L"#globaldefine")) {
		CDefine	adddefine(bef, aft);
		gdefines.push_back(adddefine);
	}
	else {
		logger.Error(E_E, 76, pname, dicfilename, linecount);
		return 2;
	}

	return 1;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CParser0::ExecDefinePreProcess
 *  機能概要：  #define/#globaldefine処理。文字列を置換します
 * -----------------------------------------------------------------------
 */
void	CParser0::ExecDefinePreProcess(wstring &str, const vector<CDefine>& defines)
{
	for(vector<CDefine>::const_iterator it = defines.begin(); it != defines.end(); it++)
		ws_replace(str, it->before, it->after);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CParser0::IsCipheredDic
 *  機能概要：  ファイルが暗号化ファイルかをファイル拡張子を見て判断します
 *
 *  返値　　：  1/0=拡張子は.aycである/.aycではない
 * -----------------------------------------------------------------------
 */
char	CParser0::IsCipheredDic(const wstring& filename)
{
	int	len = filename.size();
	if (len < 5)
		return 0;

	return ((filename[len - 3] == L'a' || filename[len - 3] == L'A') &&
		    (filename[len - 2] == L'y' || filename[len - 2] == L'Y') &&
		    (filename[len - 1] == L'c' || filename[len - 1] == L'C'))
			? 1 : 0;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CParser0::SeparateFactor
 *  機能概要：  与えられた文字列を"{"、"}"、";"の位置で分割します。";"は以降不要なので消し込みます
 * -----------------------------------------------------------------------
 */
void	CParser0::SeparateFactor(vector<wstring> &s, wstring &line)
{
	int		separatepoint = 0;
	int		apoint        = 0;
	int		len           = line.size();
	char	executeflg    = 0;
	for( ; ; ) {
		// { 発見(0)
		int	newseparatepoint = -1;
		int	type = 0;
		int	tmppoint = line.find(L'{', separatepoint);
		if (tmppoint != -1)
			newseparatepoint = tmppoint;
		// } 発見(1)
		tmppoint = line.find(L'}', separatepoint);
		if (tmppoint != -1)
			if (newseparatepoint == -1 ||
				(newseparatepoint != -1 && tmppoint < newseparatepoint)) {
				newseparatepoint = tmppoint;
				type = 1;
			}
		// ; 発見(2)
		tmppoint = line.find(L';', separatepoint);
		if (tmppoint != -1)
			if (newseparatepoint == -1 ||
				(newseparatepoint != -1 && tmppoint < newseparatepoint)) {
				newseparatepoint = tmppoint;
				type = 2;
			}
		// 何も見つからなければ終わり
		if (newseparatepoint == -1)
			break;
		// 発見位置がダブルクォート内なら無視して先へ進む
		if (IsInDQ(line, 0, newseparatepoint)) {
			separatepoint = newseparatepoint + 1;
			continue;
		}
		// 発見位置の手前の文字列を取得
		if (newseparatepoint > apoint) {
			wstring	tmpstr;
			tmpstr.assign(line, apoint, newseparatepoint - apoint);
			CutSpace(tmpstr);
			s.push_back(tmpstr);
		}
		// 発見したのが"{"もしくは"}"ならこれも取得
		if (type == 0)
			s.push_back(L"{");
		else if (type == 1)
			s.push_back(L"}");
		// 検索開始位置を更新
		apoint = separatepoint = newseparatepoint + 1;
		if (separatepoint >= len) {
			executeflg = 1;
			break;
		}
	}

	// まだ文字列が残っているならそれも追加
	if (executeflg == 0) {
		wstring	tmpstr;
		tmpstr.assign(line, apoint, len - apoint);
		CutSpace(tmpstr);
		s.push_back(tmpstr);
	}

	// 元の文字列はクリアする
	line = L"";
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CParser0::DefineFunctions
 *  機能概要：  文字列群を解析して関数の原型を定義する
 *
 *  返値　　：  1/0=エラー/正常
 * -----------------------------------------------------------------------
 */
char	CParser0::DefineFunctions(vector<wstring> &s, const wstring& dicfilename, int linecount, int &depth, int &targetfunction)
{
	char	retcode = 0;

	for(vector<wstring>::iterator it = s.begin(); it != s.end(); it++) {
		// 空行はスキップ
		if (!(it->size()))
			continue;

		// {}入れ子の深さを見て関数名を検索する
		// 深さが0なら{}の外なので関数名を取得すべき位置である
		if (!depth) {
			// 関数の作成
			if (targetfunction == -1) {
				// 関数名と重複回避オプションを取得
				wstring	d0, d1;
				if (!Split(*it, d0, d1, L":"))
					d0 = *it;
				// 関数名の正当性検査
				if (IsLegalFunctionName(d0)) {
					if (!it->compare(L"{"))
						logger.Error(E_E, 1, L"{", dicfilename, linecount);
					else if (!it->compare(L"}"))
						logger.Error(E_E, 2, dicfilename, linecount);
					else
						logger.Error(E_E, 3, d0, dicfilename, linecount);
					return 1;
				}
				// 重複回避オプションの判定
				int	chtype = CHOICETYPE_RANDOM;
				if (d1.size()) {
				        int i;
					for(i = 0; i < CHOICETYPE_NUM; i++)
						if (!d1.compare(choicetype[i])) {
							chtype = i;
							break;
						}
					if (i == CHOICETYPE_NUM) {
						logger.Error(E_E, 30, d1, dicfilename, linecount);
						return 1;
					}
				}
				// 作成
				targetfunction = MakeFunction(d0, chtype, dicfilename);
				if (targetfunction == -1) {
				        logger.Error(E_E, 13, *it, dicfilename, linecount);
					return 1;
				}
				continue;
			}
			// 関数名の次のステートメント　これは"{"でなければならない
			else {
				if (it->compare(L"{")) {
					logger.Error(E_E, 4, dicfilename, linecount);
					return 1;
				}
				// 入れ子の深さを加算
				depth++;
			}
		}
		else {
			// 関数内のステートメントの定義　{}入れ子の計算もここで行う
			if (!StoreInternalStatement(targetfunction, *it, depth, dicfilename, linecount))
				retcode = 1;
			// 入れ子深さが0になったらこの関数定義から脱出する
			if (!depth)
				targetfunction = -1;
		}
	}

	return retcode;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CParser0::MakeFunction
 *  機能概要：  名前を指定して関数を作成します
 *
 *  返値　　：  作成された位置を返します
 *  　　　　　  指定された名前の関数が既に作成済の場合はエラーで、-1を返します
 * -----------------------------------------------------------------------
 */
int	CParser0::MakeFunction(const wstring& name, int chtype, const wstring& dicfilename)
{
	int	i = GetFunctionIndexFromName(name);
	if(i != -1)
		return -1;
/*	for(vector<CFunction>::iterator it = function.begin(); it != function.end(); it++)
		if (!name.compare(it->name))
			return -1;
*/

	CFunction	addfunction(name, chtype, dicfilename);
	function.push_back(addfunction);
	function_wm.addWord(name, function.size() - 1);

	return function.size() - 1;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CParser0::StoreInternalStatement
 *  機能概要：  関数内のステートメントを種類を判定して蓄積します
 *
 *  返値　　：  0/1=エラー/正常
 * -----------------------------------------------------------------------
 */
char	CParser0::StoreInternalStatement(int targetfunc, wstring &str, int& depth, const wstring& dicfilename, int linecount)
{
	// パラメータのないステートメント

	// {
	if (!str.compare(L"{")) {
		depth++;
		CStatement	addstatement(ST_OPEN, linecount);
		function[targetfunc].statement.push_back(addstatement);
		return 1;
	}
	// }
	else if (!str.compare(L"}")) {
		depth--;
		CStatement	addstatement(ST_CLOSE, linecount);
		function[targetfunc].statement.push_back(addstatement);
		return 1;
	}
	// others　elseへ書き換えてしまう
	else if (!str.compare(L"others")) {
		CStatement	addstatement(ST_ELSE, linecount);
		function[targetfunc].statement.push_back(addstatement);
		return 1;
	}
	// else
	else if (!str.compare(L"else")) {
		CStatement	addstatement(ST_ELSE, linecount);
		function[targetfunc].statement.push_back(addstatement);
		return 1;
	}
	// break
	else if (!str.compare(L"break")) {
		CStatement	addstatement(ST_BREAK, linecount);
		function[targetfunc].statement.push_back(addstatement);
		return 1;
	}
	// continue
	else if (!str.compare(L"continue")) {
		CStatement	addstatement(ST_CONTINUE, linecount);
		function[targetfunc].statement.push_back(addstatement);
		return 1;
	}
	// return
	else if (!str.compare(L"return")) {
		CStatement	addstatement(ST_RETURN, linecount);
		function[targetfunc].statement.push_back(addstatement);
		return 1;
	}
	// --
	else if (!str.compare(L"--")) {
		CStatement	addstatement(ST_COMBINE, linecount);
		function[targetfunc].statement.push_back(addstatement);
		return 1;
	}

	// パラメータのあるステートメント（制御文）
	wstring	st, par;
	if (!Split(str, st, par, L" "))
		st = str;
	wstring	t_st, t_par;
	if (!Split(str, t_st, t_par, L"\t"))
		t_st = str;
	if (st.size() > t_st.size()) {
		st  = t_st;
		par = t_par;
	}
	// if
	if (!st.compare(L"if")) {
		str = par;
		return MakeStatement(ST_IF, targetfunc, str, dicfilename, linecount);
	}
	// elseif
	else if (!st.compare(L"elseif")) {
		str = par;
		return MakeStatement(ST_ELSEIF, targetfunc, str, dicfilename, linecount);
	}
	// while
	else if (!st.compare(L"while")) {
		str = par;
		return MakeStatement(ST_WHILE, targetfunc, str, dicfilename, linecount);
	}
	// switch
	else if (!st.compare(L"switch")) {
		str = par;
		return MakeStatement(ST_SWITCH, targetfunc, str, dicfilename, linecount);
	}
	// for
	else if (!st.compare(L"for")) {
		str = par;
		return MakeStatement(ST_FOR, targetfunc, str, dicfilename, linecount);
	}
	// foreach
	else if (!st.compare(L"foreach")) {
		str = par;
		return MakeStatement(ST_FOREACH, targetfunc, str, dicfilename, linecount);
	}
	// case　特殊な名前のローカル変数への代入に書き換えてしまう
	else if (!st.compare(L"case")) {
		str = PREFIX_CASE_VAR + function[targetfunc].name;
		wstring	tmpstr;
		ws_itoa(tmpstr, linecount, 10);
		str += tmpstr;
		str += L"=";
		str += par;
		return MakeStatement(ST_FORMULA, targetfunc, str, dicfilename, linecount);
	}
	// when
	else if (!st.compare(L"when")) {
		str = par;
		return MakeStatement(ST_WHEN, targetfunc, str, dicfilename, linecount);
	}
	// parallel
	else if (!st.compare(L"parallel")) {
		str = par;
		return MakeStatement(ST_PARALLEL, targetfunc, str, dicfilename, linecount);
	}

	// これまでのすべてにマッチしない文字列は数式と認識される
	return MakeStatement(ST_FORMULA, targetfunc, str, dicfilename, linecount);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CParser0::MakeStatement
 *  機能概要：  関数内のステートメントを作成、蓄積します
 *
 *  返値　　：  0/1=エラー/正常
 * -----------------------------------------------------------------------
 */
char	CParser0::MakeStatement(int type, int targetfunc, wstring &str, const wstring& dicfilename, int linecount)
{
	if (!str.size()) {
		logger.Error(E_E, 27, dicfilename, linecount);
		return 0;
	}

	CStatement	addstatement(type, linecount);
	if (type == ST_WHEN) {
		if (!StructWhen(str, addstatement.cell, dicfilename, linecount))
			return 0;
	}
	else {
		if (!StructFormula(str, addstatement.cell, dicfilename, linecount))
			return 0;
	}

	function[targetfunc].statement.push_back(addstatement);
	return 1;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CParser0::StructFormula
 *  機能概要：  文字列を数式の項と演算子に分解し、基本的な整形を行います
 *
 *  返値　　：  0/1=エラー/正常
 *
 *  渡された文字列はこの関数で破壊されます
 * -----------------------------------------------------------------------
 */
char	CParser0::StructFormula(wstring &str, vector<CCell> &cells, const wstring& dicfilename, int linecount)
{
	// 演算子と項に分解　項の種別はこの時点では調べていない
	StructFormulaCell(str, cells);

	// 整形と書式エラーの検出
	char	bracket = 0;
	for(vector<CCell>::iterator it = cells.begin(); it != cells.end(); ) {
		// 直前が")""]"の場合、次は演算子が来なければならない
		if (bracket) {
			bracket = 0;
			if (it->v.GetType() == F_TAG_NOP) {
				logger.Error(E_E, 92, dicfilename, linecount);
				return 0;
			}
		}
		// 正符号（+の直前の項が無いかあるいは")""]"以外の演算子　この項は消してよい）
		if (it->v.GetType() == F_TAG_PLUS) {
			if (it == cells.begin()) {
				it = cells.erase(it);
				continue;
			}
			vector<CCell>::iterator	itm = it;
			itm--;
			if (itm->v.GetType() != F_TAG_NOP &&
				itm->v.GetType() != F_TAG_BRACKETOUT &&
				itm->v.GetType() != F_TAG_HOOKBRACKETOUT) {
				it = cells.erase(it);
				continue;
			}
		}
		// 負符号（-の直前の項が無いかあるいは")"以外の演算子　"-1*"に整形する）
		if (it->v.GetType() == F_TAG_MINUS) {
			if (it == cells.begin()) {
				it->v.SetType(F_TAG_NOP);
				it->v.s_value = L"-1";
				it++;
				CCell	addcell2(F_TAG_MUL);
				it = cells.insert(it, addcell2);
				it++;
				continue;
			}
			vector<CCell>::iterator	itm = it;
			itm--;
			if (itm->v.GetType() != F_TAG_NOP &&
				itm->v.GetType() != F_TAG_BRACKETOUT &&
				itm->v.GetType() != F_TAG_HOOKBRACKETOUT) {
				it->v.SetType(F_TAG_NOP);
				it->v.s_value = L"-1";
				it++;
				CCell	addcell2(F_TAG_MUL);
				it = cells.insert(it, addcell2);
				it++;
				continue;
			}
		}
		// インクリメント（"+=1"に整形する）
		if (it->v.GetType() == F_TAG_INCREMENT) {
			it->v.SetType(F_TAG_PLUSEQUAL);
			it++;
			CCell	addcell(F_TAG_NOP);
			addcell.v.s_value = L"1";
			it = cells.insert(it, addcell);
			it++;
			continue;
		}
		// デクリメント（"-=1"に整形する）
		if (it->v.GetType() == F_TAG_DECREMENT) {
			it->v.SetType(F_TAG_MINUSEQUAL);
			it++;
			CCell	addcell(F_TAG_NOP);
			addcell.v.s_value = L"1";
			it = cells.insert(it, addcell);
			it++;
			continue;
		}
		// !、&演算子（手前にダミー項0を追加）
		if (it->v.GetType() == F_TAG_FEEDBACK) {
			if (it == cells.begin()) {
				logger.Error(E_E, 87, dicfilename, linecount);
				return 0;
			}
			CCell	addcell(F_TAG_NOP);
			addcell.v.s_value = L"0";
			it = cells.insert(it, addcell);
			it += 2;
			continue;
		}
		if (it->v.GetType() == F_TAG_EXC) {
			CCell	addcell(F_TAG_NOP);
			addcell.v.s_value = L"0";
			it = cells.insert(it, addcell);
			it += 2;
			continue;
		}
		// 簡易配列序数アクセス演算子（"["直前に付与）
		if (it->v.GetType() == F_TAG_HOOKBRACKETIN) {
			if (it == cells.begin()) {
				logger.Error(E_E, 6, dicfilename, linecount);
				return 0;
			}
			CCell	addcell(F_TAG_ARRAYORDER);
			it = cells.insert(it, addcell);
			it += 2;
			continue;
		}
		// 関数要素指定演算子（"("直前が演算子でなければ付与）
		if (it->v.GetType() == F_TAG_BRACKETIN) {
			if (it != cells.begin()) {
				vector<CCell>::iterator	itm = it;
				itm--;
				if (itm->v.GetType() == F_TAG_NOP) {
					CCell	addcell(F_TAG_FUNCPARAM);
					it = cells.insert(it, addcell);
					it += 2;
					continue;
				}
			}
		}
		// 空のカッコ（"()"　消す　関数要素指定演算子があった場合はそれも消す）
		// 関数引数2個以上で空の引数（",)"の場合、""を補完）
		if (it->v.GetType() == F_TAG_BRACKETOUT) {
			bracket = 1;
			if (it != cells.begin()) {
				vector<CCell>::iterator	itm = it;
				itm--;
				if (itm->v.GetType() == F_TAG_BRACKETIN) {
					it = cells.erase(itm);
					it = cells.erase(it);
					if (it != cells.begin()) {
						itm = it;
						itm--;
						if (itm->v.GetType() == F_TAG_FUNCPARAM)
							it = cells.erase(itm);
					}
					continue;
				}
				else if (itm->v.GetType() == F_TAG_CUMMA) {
					CCell	addcell(F_TAG_NOP);
					addcell.v.s_value = L"\"\"";
					it = cells.insert(it, addcell);
					it += 2;
					continue;
				}
			}
		}
		// 空の鉤カッコ（"[]"　エラー）
		if (it->v.GetType() == F_TAG_HOOKBRACKETOUT) {
			bracket = 1;
			if (it != cells.begin()) {
				vector<CCell>::iterator	itm = it;
				itm--;
				if (itm->v.GetType() == F_TAG_HOOKBRACKETIN) {
					logger.Error(E_E, 14, dicfilename, linecount);
					return 0;
				}
			}
		}
		// 関数引数2個以上で空の引数（"(,"および",,"の場合、""を補完）
		if (it->v.GetType() == F_TAG_CUMMA) {
			if (it != cells.begin()) {
				vector<CCell>::iterator	itm = it;
				itm--;
				if (itm->v.GetType() == F_TAG_BRACKETIN || itm->v.GetType() == F_TAG_CUMMA) {
					CCell	addcell(F_TAG_NOP);
					addcell.v.s_value = L"\"\"";
					it = cells.insert(it, addcell);
					it += 2;
					continue;
				}
			}
		}
		// 次の項へ
		it++;
	}

	return 1;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CParser0::StructWhen
 *  機能概要：  文字列を数式の項と演算子に分解し、基本的な整形を行います（when構文用）
 *
 *  返値　　：  0/1=エラー/正常
 *
 *  渡された文字列はこの関数で破壊されます
 * -----------------------------------------------------------------------
 */
char	CParser0::StructWhen(wstring &str, vector<CCell> &cells, const wstring& dicfilename, int linecount)
{
	// 演算子と項に分解　項の種別はこの時点では調べていない
	StructFormulaCell(str, cells);

	// 整形と書式エラーの検出
	for(vector<CCell>::iterator it = cells.begin(); it != cells.end(); ) {
		// 正符号（+の直前の項が無いかあるいは")"以外の演算子　この項は消してよい）
		if (it->v.GetType() == F_TAG_PLUS) {
			if (it == cells.begin()) {
				it = cells.erase(it);
				continue;
			}
			vector<CCell>::iterator	itm = it;
			itm--;
			if (itm->v.GetType() != F_TAG_NOP && itm->v.GetType() != F_TAG_BRACKETOUT) {
				it = cells.erase(it);
				continue;
			}
		}
		// 負符号（-の直前の項が無い　次の項に単純に-を付加する）
		if (it->v.GetType() == F_TAG_MINUS) {
			if (it == cells.begin()) {
				it = cells.erase(it);
				it->v.s_value.insert(0, L"-");
				continue;
			}
			vector<CCell>::iterator	itm = it;
			itm--;
			if (itm->v.GetType() != F_TAG_NOP && itm->v.GetType() != F_TAG_BRACKETOUT) {
				it = cells.erase(it);
				it->v.s_value.insert(0, L"-");
				continue;
			}
		}
		// デクリメント 正当な形状かを検査の上、- と次項への-付与として処理する
		if (it->v.GetType() == F_TAG_DECREMENT) {
			if (it == cells.begin()) {
				logger.Error(E_E, 65, dicfilename, linecount);
				return 0;
			}
			it->v.SetType(F_TAG_MINUS);
			it++;
			if (it == cells.end()) {
				logger.Error(E_E, 66, dicfilename, linecount);
				return 0;
			}
			it->v.s_value.insert(0, L"-");
			continue;
		}
		// 次の項へ
		it++;
	}
	// ","を"||"へ、"-"を"&&"へ変換する　無効な演算子ではエラー
	for(vector<CCell>::iterator it = cells.begin(); it != cells.end(); it++) {
		if (it->v.GetType() == F_TAG_CUMMA)
			it->v.SetType(F_TAG_OR);
		else if (it->v.GetType() == F_TAG_MINUS)
			it->v.SetType(F_TAG_AND);
		else if (it->v.GetType() >= F_TAG_ORIGIN && it->v.GetType() < F_TAG_ORIGIN_VALUE) {
			logger.Error(E_E, 50, dicfilename, linecount);
			return 0;
		}
	}

	return 1;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CParser0::StructFormulaCell
 *  機能概要：  文字列を数式の項と演算子に分解します
 *
 *  返値　　：  0/1=エラー/正常
 *
 *  渡された文字列はこの関数で破壊されます
 * -----------------------------------------------------------------------
 */
//#include <iostream>
void	CParser0::StructFormulaCell(wstring &str, vector<CCell> &cells)
{
	for( ; ; ) {
		// 分割位置を取得　最も手前で最も名前が長く、クォートされていない演算子を探す
		int	tagpoint = -1;
		int	tagtype  = 0;
		int	taglen   = 0;
		wstring	bstr;
//		wcout << endl << "str: " << str << endl;
//		wcout << "WordMatch:" << endl;
		int in_dq = 0;
		int	in_sq = 0;
		for(unsigned int i = 0; i < str.size(); ++i) {
			if (str[i] == L'\"') {
				if (!in_sq)
					in_dq = 1 - in_dq;
				continue;
			}
			if (str[i] == L'\'') {
				if (!in_dq)
					in_sq = 1 - in_sq;
				continue;
			}
			if (in_dq || in_sq)
				continue;

			int result = formulatag_wm.search(str, i);
			if(result != -1) {
				tagtype = result;
				taglen  = formulatag_len[tagtype];
				
				wstring	d0, d1;
				d0.assign(str, 0, i);
				d1.assign(str, i + taglen, str.size());
				CutSpace(d0);
				CutSpace(d1);
				tagpoint = d0.size();
				bstr = d1;
//				wcout << "d0: " << d0 << endl;
//				wcout << "d1: " << d1 << endl;
				break;
			}
		}
/*		wcout << "  tagpoint: " << tagpoint << endl;
		wcout << "  tagtype: " << tagtype << endl;
		wcout << "  taglen: " << taglen << endl;
		wcout << "  bstr: " << bstr << endl;

		wcout << "Aya5:" << endl;
		tagpoint = -1;
		tagtype  = 0;
		taglen   = 0;

		for(int i = 0; i < FORMULATAG_NUM; i++) {
			wstring	d0, d1;
			if (!Split_IgnoreDQ(str, d0, d1, (wchar_t *)formulatag[i]))
				continue;
			int	d_point = d0.size();
			if (tagpoint == -1 ||
				(tagpoint != -1 && ((tagpoint > d_point) || tagpoint == d_point && taglen < formulatag_len[i]))) {
				tagpoint = d_point;
				tagtype  = i;
				taglen   = formulatag_len[i];
				bstr = d1;
//				wcout << "d0: " << d0 << endl;
//				wcout << "d1: " << d1 << endl;
			}
		}

		wcout << "  tagpoint: " << tagpoint << endl;
		wcout << "  tagtype: " << tagtype << endl;
		wcout << "  taglen: " << taglen << endl;
		wcout << "  bstr: " << bstr << endl;
*/
		// 見つからない場合は最後の項を登録して抜ける
		if (tagpoint == -1) {
			CCell	addcell(F_TAG_NOP);
			addcell.v.s_value = str;
			CutSpace(addcell.v.s_value);
			if (addcell.v.s_value.size())
				cells.push_back(addcell);
			break;
		}
		// 見つかったので登録する
		else {
			// 項の登録　空文字列は登録しない
			if (tagpoint > 0) {
				CCell	addcell(F_TAG_NOP);
				addcell.v.s_value.assign(str, 0, tagpoint);
				CutSpace(addcell.v.s_value);
				if (addcell.v.s_value.size())
					cells.push_back(addcell);
			}
			// 演算子の登録
			CCell	addcell(tagtype + F_TAG_ORIGIN);
			cells.push_back(addcell);
			// 元の文字列から取り出し済の要素を削る
			str = bstr;
		}
	}
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CParser0::AddSimpleIfBrace
 *  機能概要：  if/elseif/else/when/othersの直後に'{'が無い場合、次の行を{}で囲みます
 *
 *  返値　　：  1/0=エラー/正常
 * -----------------------------------------------------------------------
 */
char	CParser0::AddSimpleIfBrace(void)
{
	for(vector<CFunction>::iterator it = function.begin(); it != function.end(); it++) {
		int	beftype = ST_UNKNOWN;
		for(vector<CStatement>::iterator it2 = it->statement.begin(); it2 != it->statement.end(); it2++) {
			if (beftype == ST_IF ||
				beftype == ST_ELSEIF ||
				beftype == ST_ELSE ||
				beftype == ST_WHEN) {
				if (it2->type != ST_OPEN) {
					// { 追加
					CStatement	addopen(ST_OPEN, it2->linecount);
					it2 = it->statement.insert(it2, addopen);
					it2 += 2;
					// } 追加
					CStatement	addclose(ST_CLOSE, it2->linecount);
					it2 = it->statement.insert(it2, addclose);
				}
			}
			beftype = it2->type;
		}
	}
			
	return 0;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CParser0::SetCellType
 *  機能概要：  数式の項の種別（リテラル、関数、変数...など）を調べて格納していきます
 *
 *  返値　　：  1/0=エラー/正常
 * -----------------------------------------------------------------------
 */
char	CParser0::SetCellType(void)
{
	int	errorflg = 0;

	for(vector<CFunction>::iterator it = function.begin(); it != function.end(); it++)
		for(vector<CStatement>::iterator it2 = it->statement.begin(); it2 != it->statement.end(); it2++) {
			// 数式以外は飛ばす
			if (it2->type < ST_FORMULA)
				continue;

			for(vector<CCell>::iterator it3 = it2->cell.begin(); it3 != it2->cell.end(); it3++) {
				// 演算子は飛ばす
				if (it3->v.GetType() != F_TAG_NOP)
					continue;

				// 項種別取得
				errorflg += SetCellType1(*it3, 0, it->dicfilename, it2->linecount);
				// whenの場合、項はリテラルしかあり得ない
				if (it2->type == ST_WHEN) {
					if (it3->v.GetType() != F_TAG_INT && 
						it3->v.GetType() != F_TAG_DOUBLE && 
						it3->v.GetType() != F_TAG_STRING && 
						it3->v.GetType() != F_TAG_STRING_PLAIN && 
						it3->v.GetType() != F_TAG_DOUBLE) {
						logger.Error(E_E, 45, it->dicfilename, it2->linecount);
						errorflg++;
					}
				}
			}
		}
	
	return (errorflg) ? 1 : 0;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CParser0::SetCellType1
 *  機能概要：  渡された項の種別（リテラル、関数、変数...など）を設定します
 *  引数　　：  emb 0/1=通常の数式の項/文字列に埋め込まれていた数式の項
 *
 *  返値　　：  1/0=エラー/正常
 * -----------------------------------------------------------------------
 */
char	CParser0::SetCellType1(CCell& scell, char emb, const wstring& dicfilename, int linecount)
{
	// 関数
	int	i = GetFunctionIndexFromName(scell.v.s_value);
	if(i != -1) {
		scell.v.SetType(F_TAG_USERFUNC);
		scell.index     = i;
		scell.v.s_value = L"";
		return 0;
	}

/*
	int i = 0;
	for(vector<CFunction>::iterator it = function.begin(); it != function.end(); it++, i++)
		if (!scell.v.s_value.compare(it->name)) {
			scell.v.SetType(F_TAG_USERFUNC);
			scell.index     = i;
			scell.v.s_value = L"";
//			wcout << "Aya5:" << endl;
//			wcout << "  result: " << i << endl;
			return 0;
		}
*/

	// システム関数
	for(i = 0; i < SYSFUNC_NUM; i++)
		if (!scell.v.s_value.compare(sysfunc[i])) {
			scell.v.SetType(F_TAG_SYSFUNC);
			scell.index     = i;
			scell.v.s_value = L"";
			return 0;
		}
	// 整数リテラル(DEC)
	if (IsIntString(scell.v.s_value)) {
		scell.v = ws_atoi(scell.v.s_value, 10);
		return 0;
	}
	// 整数リテラル(BIN)
	if (IsIntBinString(scell.v.s_value, 1)) {
		scell.v = ws_atoi(scell.v.s_value, 2);
		return 0;
	}
	// 整数リテラル(HEX)
	if (IsIntHexString(scell.v.s_value, 1)) {
		scell.v = ws_atoi(scell.v.s_value, 16);
		return 0;
	}
	// 実数リテラル
	if (IsDoubleString(scell.v.s_value)) {
		scell.v = ws_atof(scell.v.s_value);
		return 0;
	}
	// 文字列リテラル(ダブルクォート)
	i = IsLegalStrLiteral(scell.v.s_value);
	if (!i) {
		CutDoubleQuote(scell.v.s_value);
		if (!emb)
			scell.v.SetType(F_TAG_STRING);
		else {
			if (!scell.v.s_value.size())
				scell.v.SetType(F_TAG_STRING);
			else {
				if (scell.v.s_value[0] == L'%') {
					if (scell.v.s_value.size() == 1)
						scell.v.SetType(F_TAG_STRING);
					else {
						scell.v.s_value.erase(0, 1);
						scell.v.SetType(F_TAG_STRING_EMBED);
					}
				}
				else
					scell.v.SetType(F_TAG_STRING);
			}
		}
		return 0;
	}
	else if (i == 1) {
		scell.v.s_value = L"";
		logger.Error(E_E, 7, scell.v.s_value, dicfilename, linecount);
		return 1;
	}
	else if (i == 2) {
		scell.v.s_value = L"";
		logger.Error(E_E, 8, scell.v.s_value, dicfilename, linecount);
		return 1;
	}
	// 文字列リテラル(シングルクォート)
	i = IsLegalPlainStrLiteral(scell.v.s_value);
	if (!i) {
		CutSingleQuote(scell.v.s_value);
		scell.v.SetType(F_TAG_STRING_PLAIN);
		return 0;
	}
	else if (i == 1) {
		scell.v.s_value = L"";
		logger.Error(E_E, 7, scell.v.s_value, dicfilename, linecount);
		return 1;
	}
	else if (i == 2) {
		scell.v.s_value = L"";
		logger.Error(E_E, 93, scell.v.s_value, dicfilename, linecount);
		return 1;
	}
	// ここまで残ったものは変数の候補
	CVariable	addvariable;
	string	errstr;
	switch(IsLegalVariableName(scell.v.s_value)) {
	case 0:
		// グローバル変数
		scell.v.SetType(F_TAG_VARIABLE);
		scell.index     = variable.Make(scell.v.s_value, 1);
		scell.v.s_value = L"";
		return 0;
	case 16:
		// ローカル変数
		scell.v.SetType(F_TAG_LOCALVARIABLE);
		scell.name      = scell.v.s_value;
		scell.v.s_value = L"";
		return 0;
	case 3:
		// エラー　変数名の誤り
		logger.Error(E_E, 10, scell.v.s_value, dicfilename, linecount);
		scell.v.s_value = L"";
		return 1;
	case 4:
		// エラー　使用できない文字を含む
		logger.Error(E_E, 11, scell.v.s_value, dicfilename, linecount);
		scell.v.s_value = L"";
		return 1;
	case 6:
		// エラー　予約語と衝突
		logger.Error(E_E, 12, scell.v.s_value, dicfilename, linecount);
		scell.v.s_value = L"";
		return 1;
	default:
		// 内部エラー
		logger.Error(E_E, 9, scell.v.s_value, dicfilename, linecount);
		scell.v.s_value = L"";
		return 1;
	};
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CParser0::MakeCompleteFormula
 *  機能概要：  埋め込み要素の式への展開と演算順序の決定を行ないます
 *
 *  返値　　：  1/0=エラー/正常
 * -----------------------------------------------------------------------
 */
char	CParser0::MakeCompleteFormula(void)
{
	int	errcount = 0;

	errcount += ParseEmbeddedFactor();
	ConvertPlainString();
	errcount += CheckDepthAndSerialize();

	return (errcount) ? 1 : 0;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CParser0::ParseEmbeddedFactor
 *  機能概要：  "%"で埋め込まれた要素を持つ文字列を分解して数式を作り、元の式と結合します
 *
 *  返値　　：  1/0=エラー/正常
 * -----------------------------------------------------------------------
 */
char	CParser0::ParseEmbeddedFactor(void)
{
	int	errcount = 0;

	for(vector<CFunction>::iterator it = function.begin(); it != function.end(); it++)
		for(vector<CStatement>::iterator it2 = it->statement.begin(); it2 != it->statement.end(); it2++)
		    errcount += ParseEmbeddedFactor1(*it2, it->dicfilename);

	return (errcount) ? 1 : 0;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CParser0::ParseEmbeddedFactor1
 *  機能概要：  "%"で埋め込まれた要素を持つ文字列を分解して数式を作り、元の式と結合します
 *
 *  返値　　：  1/0=エラー/正常
 * -----------------------------------------------------------------------
 */
char	CParser0::ParseEmbeddedFactor1(CStatement& st, const wstring& dicfilename)
{
	if (st.type < ST_FORMULA)
		return 0;

	int	errcount = 0;

	// 演算順序を崩さないようにするため、%を含む要素を()で囲む
	for(vector<CCell>::iterator it = st.cell.begin(); it != st.cell.end(); ) {
		if (it->v.GetType() == F_TAG_STRING) {
			if (it->v.s_value.find(L'%') != -1) {
				CCell	addcell(F_TAG_BRACKETIN);
				it = st.cell.insert(it, addcell);
				it += 2;
				CCell	addcell2(F_TAG_BRACKETOUT);
				it = st.cell.insert(it, addcell2);
				it++;
				if (st.type == ST_WHEN) {
					logger.Error(E_E, 46, dicfilename, st.linecount);
					errcount++;
				}
				continue;
			}
		}
		// 次へ
		it++;
	}

	// 埋め込み要素を加算多項式に分解して元の式の該当位置へ挿入
	for(vector<CCell>::iterator it = st.cell.begin(); it != st.cell.end(); ) {
		if (it->v.GetType() == F_TAG_STRING) {
			if (it->v.s_value.find(L'%') != -1) {
				// 加算多項式へ変換
				int	t_errcount = 0;
				wstring	linedata = it->v.s_value;
				int	res = ConvertEmbedStringToFormula(linedata, dicfilename, st.linecount);
				t_errcount += res;
				if (res) {
					it++;
					continue;
				}
				// 数式の項分解と整形
				vector<CCell>	addcells;
				res = 1 - StructFormula(linedata, addcells, dicfilename, st.linecount);
				t_errcount += res;
				if (res) {
					it++;
					continue;
				}
				// 項の種別を設定
				for(vector<CCell>::iterator it2 = addcells.begin(); it2 != addcells.end(); it2++) {
					if (it2->v.GetType() != F_TAG_NOP)
						continue;

					t_errcount += SetCellType1(*it2, 1, dicfilename, st.linecount);
				}
				// 元の式の該当位置へ挿入
				if (!t_errcount) {
					it = st.cell.erase(it);
					int	c_num = addcells.size();
					for(int i = c_num - 1; i >= 0; i--)
						it = st.cell.insert(it, addcells[i]);
					it += c_num;
					continue;
				}
				errcount += t_errcount;
			}
		}
		// 次へ
		it++;
	}

	return (errcount) ? 1 : 0;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CParser0::ConvertPlainString
 *  機能概要：  シングルクォート文字列を通常文字列へ置換します
 * -----------------------------------------------------------------------
 */
void	CParser0::ConvertPlainString(void)
{
	for(vector<CFunction>::iterator it = function.begin(); it != function.end(); it++)
		for(vector<CStatement>::iterator it2 = it->statement.begin(); it2 != it->statement.end(); it2++)
		    ConvertPlainString1(*it2, it->dicfilename);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CParser0::ConvertPlainString1
 *  機能概要：  シングルクォート文字列を通常文字列へ置換します
 * -----------------------------------------------------------------------
 */
void	CParser0::ConvertPlainString1(CStatement& st, const wstring& dicfilename)
{
	if (st.type < ST_FORMULA)
		return;

	for(vector<CCell>::iterator it = st.cell.begin(); it != st.cell.end(); it++)
		if (it->v.GetType() == F_TAG_STRING_PLAIN)
			it->v.SetType(F_TAG_STRING);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CParser0::ConvertEmbedStringToFormula
 *  機能概要：  "%"で埋め込まれた要素を持つ文字列を加算多項式へ変換します
 *
 *  返値　　：  1/0=エラー/正常
 * -----------------------------------------------------------------------
 */
char	CParser0::ConvertEmbedStringToFormula(wstring& str, const wstring& dicfilename, int linecount)
{
	wstring	resstr;
	int	nindex = -1;
	for(int nfirst = 0; ; nfirst++) {
		// "%"の発見
		int	p_pers = str.find(L'%', 0);
		if (p_pers == -1) {
			logger.Error(E_E, 55, dicfilename, linecount);
			return 1;
		}
		// 加算演算子を追加
		if (nfirst)
			resstr += L"+";
		// 先行する文字列項を追加
		if (p_pers > 0) {
			wstring	prestr;
			prestr.assign(str, 0, p_pers);
			AddDoubleQuote(prestr);
			resstr += prestr;
			str.erase(0, p_pers);
			resstr += L"+";
		}
		// "%"しか残らなかったらそれで終わり
		if (str.size() == 1) {
			resstr += L"\"%\"";
			break;
		}
		// "%"の次が"("なら長さ指定付きの埋め込みなのでそれを抜き出す
		if (str[1] == L'(') {
			// 抜き出し位置検索
			int	bdepth = 1;
			int	len = str.size();
			int     spos;
			for(spos = 2; spos < len; spos++) {
				bdepth += ((str[spos] == L'(') - (str[spos] == L')'));
				if (!bdepth)
					break;
			}
			if (spos < len)
				spos++;
			// エラー処理
			if (bdepth != 0) {
				logger.Error(E_E, 60, dicfilename, linecount);
				return 1;
			}
			if (spos == 2) {
				logger.Error(E_E, 61, dicfilename, linecount);
				return 1;
			}
			else if (spos < 2) {
				logger.Error(E_E, 62, dicfilename, linecount);
				return 1;
			}
			// 埋め込み要素を取り出して追加
			wstring	embedstr;
			embedstr.assign(str, 1, spos - 1);
			resstr += L"TOSTR";
			resstr += embedstr;
			str.erase(0, spos);
//			nindex++;
			// 次の"%"を探してみる
			p_pers = str.find(L'%', 0);
			// 見つからなければこれが最後の文字列定数項
			if (p_pers == -1) {
				embedstr = str;
				if (embedstr.size()) {
					AddDoubleQuote(embedstr);
					resstr += L"+";
					resstr += embedstr;
				}
				break;
			}
			continue;
		}
		// "%"の次が"["なら結果の再利用（%[n]）なのでそれを抜き出す
		if (str[1] == L'[') {
			// まだ先行する項が無いならエラー
			if (nindex == -1) {
				logger.Error(E_E, 81, dicfilename, linecount);
				return 1;
			}
			// 抜き出し位置検索
			int	bdepth = 1;
			int	len = str.size();
			int     spos;
			for(spos = 2; spos < len; spos++) {
				bdepth += ((str[spos] == L'[') - (str[spos] == L']'));
				if (!bdepth)
					break;
			}
			if (spos < len)
				spos++;
			// エラー処理
			if (bdepth != 0) {
				logger.Error(E_E, 78, dicfilename, linecount);
				return 1;
			}
			if (spos == 2) {
				logger.Error(E_E, 79, dicfilename, linecount);
				return 1;
			}
			else if (spos < 2) {
				logger.Error(E_E, 80, dicfilename, linecount);
				return 1;
			}
			// 埋め込み要素を取り出し、"結果の再利用処理を行う関数"として追加
			resstr += sysfunc[SYSFUNC_HIS];
			resstr += L"(";
			wstring	embedstr;
			ws_itoa(embedstr, nindex, 10);
			resstr += embedstr;
			resstr += L"-(";
			embedstr.assign(str, 2, spos - 3);
			resstr += embedstr;
			resstr += L"))";
			str.erase(0, spos);
			// 次の"%"を探してみる
			p_pers = str.find(L'%', 0);
			// 見つからなければこれが最後の文字列定数項
			if (p_pers == -1) {
				embedstr = str;
				if (embedstr.size()) {
					AddDoubleQuote(embedstr);
					resstr += L"+";
					resstr += embedstr;
				}
				break;
			}
			continue;
		}
		// 通常の"%"
		else {
			// 次の"%"を発見
			p_pers = str.find(L'%', 1);
			nindex++;
			// 見つからなければこれが最後の項
			if (p_pers == -1) {
				wstring	embedstr = str;
				AddDoubleQuote(embedstr);
				resstr += embedstr;
				break;
			}
			// 見つかったので追加
			wstring	embedstr;
			embedstr.assign(str, 0, p_pers);
			AddDoubleQuote(embedstr);
			resstr += embedstr;
			str.erase(0, p_pers);
		}
	}

	// 返る
	str = resstr;
	return 0;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CParser0::CheckDepthAndSerialize
 *  機能概要：  数式のカッコが正しく閉じているか検査、および式の演算順序を求めます
 *
 *  返値　　：  1/0=エラー/正常
 * -----------------------------------------------------------------------
 */
char	CParser0::CheckDepthAndSerialize(void)
{
	int	errcount = 0;

	// 数式のカッコ検査
	for(vector<CFunction>::iterator it = function.begin(); it != function.end(); it++)
		for(vector<CStatement>::iterator it2 = it->statement.begin(); it2 != it->statement.end(); it2++) {
			if (it2->type < ST_FORMULA)
				continue;

			errcount += CheckDepth1(*it2, it->dicfilename);
		}

	// whenのif変換の最終処理　仮の数式をifで処理可能な判定式に整形する
	errcount += MakeCompleteConvertionWhenToIf();

	// 演算順序の決定
	for(vector<CFunction>::iterator it = function.begin(); it != function.end(); it++)
		for(vector<CStatement>::iterator it2 = it->statement.begin(); it2 != it->statement.end(); it2++) {
			if (it2->type < ST_FORMULA)
				continue;

			errcount += CheckDepthAndSerialize1(*it2, it->dicfilename);
		}

	return (errcount) ? 1 : 0;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CParser0::MakeCompleteConvertionWhenToIf
 *  機能概要：  whenのif変換の最終処理を行います。仮の数式をifで処理可能な判定式に整形します
 *
 *  返値　　：  1/0=エラー/正常
 * -----------------------------------------------------------------------
 */
char	CParser0::MakeCompleteConvertionWhenToIf(void)
{
	int	errcount = 0;

	for(vector<CFunction>::iterator it = function.begin(); it != function.end(); it++) {
		vector<wstring>	caseary;
		wstring	dmystr = L"";
		caseary.push_back(dmystr);
		vector<int>		whencnt;
		whencnt.push_back(0);
		int	depth = 0;
		for(vector<CStatement>::iterator it2 = it->statement.begin(); it2 != it->statement.end(); it2++) {
			if (depth == -1) {
				logger.Error(E_E, 52, it->dicfilename, it2->linecount);
				errcount++;
				break;
			}
			// {
			if (it2->type == ST_OPEN) {
				depth++;
				wstring	dmystr = L"";
				caseary.push_back(dmystr);
				whencnt.push_back(0);
				continue;
			}
			// }
			if (it2->type == ST_CLOSE) {
				caseary[depth] = L"";
				whencnt[depth] = 0;
				depth--;
				continue;
			}
			// case
			if (it2->type == ST_FORMULA) {
				if (it2->cell.size() >= 2) {
					if (it2->cell[0].v.GetType() == F_TAG_LOCALVARIABLE &&
						it2->cell[1].v.GetType() == F_TAG_EQUAL) {
						if (!::wcscmp(PREFIX_CASE_VAR,
							it2->cell[0].name.substr(0, PREFIX_CASE_VAR_SIZE).c_str())) {
							caseary[depth] = it2->cell[0].name;
							whencnt[depth] = 0;
							continue;
						}
					}
				}
			}
			// when
			if (it2->type == ST_WHEN) {
				int	depthm1 = depth - 1;
				if (depthm1 < 0) {
					logger.Error(E_E, 64, it->dicfilename, it2->linecount);
					errcount++;
					break;
				}
				if (!caseary[depthm1].size()) {
					logger.Error(E_E, 63, it->dicfilename, it2->linecount);
					errcount++;
					break;
				}
				// if/elseifへ変換
				if (!whencnt[depthm1])
					it2->type = ST_IF;
				else
					it2->type = ST_ELSEIF;
				(whencnt[depthm1])++;
				// 仮の数式を判定式に書き換える
				int	i = 0;
				for(vector<CCell>::iterator it3 = it2->cell.begin(); it3 != it2->cell.end(); ) {
					// ラベル
					if (!i) {
						if (it3->v.GetType() != F_TAG_INT && 
							it3->v.GetType() != F_TAG_DOUBLE && 
							it3->v.GetType() != F_TAG_STRING && 
							it3->v.GetType() != F_TAG_DOUBLE) {
							logger.Error(E_E, 53, it->dicfilename, it2->linecount);
							errcount++;
							break;
						}
						i = 1;
						it3++;
						// 最後の項　これは必ず==判定
						if (it3 == it2->cell.end()) {
							CCell	addcell1(F_TAG_LOCALVARIABLE);
							addcell1.name    = caseary[depthm1];
							addcell1.v.s_value = L"";
							it3 = it2->cell.insert(it3, addcell1);
							CCell	addcell2(F_TAG_IFEQUAL);
							it3 = it2->cell.insert(it3, addcell2);
							break;
						}
						continue;
					}
					// or/and
					i = 0;
					if (it3->v.GetType() == F_TAG_OR) {
						// or
						CCell	addcell1(F_TAG_LOCALVARIABLE);
						addcell1.name      = caseary[depthm1];
						addcell1.v.s_value = L"";
						it3 = it2->cell.insert(it3, addcell1);
						CCell	addcell2(F_TAG_IFEQUAL);
						it3 = it2->cell.insert(it3, addcell2);
						if (it3 == it2->cell.end())
							break;
						it3++;
						if (it3 == it2->cell.end())
							break;
						it3++;
						if (it3 == it2->cell.end())
							break;
						it3++;
					}
					else if (it3->v.GetType() == F_TAG_AND) {
						// and
						CCell	addcell1(F_TAG_LOCALVARIABLE);
						addcell1.name      = caseary[depthm1];
						addcell1.v.s_value = L"";
						it3 = it2->cell.insert(it3, addcell1);
						CCell	addcell2(F_TAG_IFLTEQUAL);
						it3 = it2->cell.insert(it3, addcell2);
						if (it3 == it2->cell.end())
							break;
						it3++;
						if (it3 == it2->cell.end())
							break;
						it3++;
						if (it3 == it2->cell.end())
							break;
						it3++;
						if (it3 == it2->cell.end())
							break;
						CCell	addcell3(F_TAG_IFLTEQUAL);
						it3 = it2->cell.insert(it3, addcell3);
						CCell	addcell4(F_TAG_LOCALVARIABLE);
						addcell4.name      = caseary[depthm1];
						addcell4.v.s_value = L"";
						it3 = it2->cell.insert(it3, addcell4);
						if (it3 == it2->cell.end())
							break;
						it3++;
						if (it3 == it2->cell.end())
							break;
						it3++;
						if (it3 == it2->cell.end())
							break;
						it3++;
						if (it3 == it2->cell.end())
							break;
						it3++;
					}
					else {
						logger.Error(E_E, 54, it->dicfilename, it2->linecount);
						errcount++;
						break;
					}
				}
			}
		}
	}

	return (errcount) ? 1 : 0;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CParser0::CheckDepth1
 *  機能概要：  数式のカッコが正しく閉じているか検査します
 *
 *  返値　　：  1/0=エラー/正常
 * -----------------------------------------------------------------------
 */
char	CParser0::CheckDepth1(CStatement& st, const wstring& dicfilename)
{
	// ()/[]の対応づけを検査
	vector<int>	hb_depth;
	int	depth = 0;
	for(vector<CCell>::iterator it = st.cell.begin(); it != st.cell.end(); it++) {
		if (it->v.GetType() == F_TAG_BRACKETIN)
			depth++;
		else if (it->v.GetType() == F_TAG_BRACKETOUT)
			depth--;
		else if (it->v.GetType() == F_TAG_HOOKBRACKETIN)
			hb_depth.push_back(depth);
		else if (it->v.GetType() == F_TAG_HOOKBRACKETOUT) {
			int	gb_depth_size = hb_depth.size();
			if (!gb_depth_size) {
				logger.Error(E_E, 20, dicfilename, st.linecount);
				return 1;
			}
			else if (hb_depth[gb_depth_size - 1] != depth) {
				logger.Error(E_E, 20, dicfilename, st.linecount);
				return 1;
			}
			hb_depth.erase(hb_depth.end() - 1);
		}
	}
	if (depth) {
		logger.Error(E_E, 19, dicfilename, st.linecount);
		return 1;
	}

	return 0;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CParser0::CheckDepthAndSerialize1
 *  機能概要：  数式の演算順序を決定します
 *
 *  返値　　：  1/0=エラー/正常
 * -----------------------------------------------------------------------
 */
char	CParser0::CheckDepthAndSerialize1(CStatement& st, const wstring& dicfilename)
{
	// 再度()入れ子の対応を検査しつつ、演算順序算出用のフラグを作成する
	// フラグdepthvecは対応する項の処理状態を示している。
	// -1    処理済もしくは処理対象ではない項
	// -2    処理待ちの項
	// 0以上 演算子、数字が大きいほど優先度が高い

	int	sz = st.cell.size();
	vector<int>	depthvec;
	int depth = 0;
	int i;
	for(i = 0; i < sz; i++) {
		// 演算子
		int	type = st.cell[i].v.GetType();
		if (type >= F_TAG_ORIGIN && type < F_TAG_ORIGIN_VALUE) {
			if (type == F_TAG_BRACKETIN ||
				type == F_TAG_BRACKETOUT ||
				type == F_TAG_HOOKBRACKETIN ||
				type == F_TAG_HOOKBRACKETOUT) {
				depth += formulatag_depth[type];
				depthvec.push_back(-1);
			}
			else
				depthvec.push_back(depth + formulatag_depth[type]);
			continue;
		}
		// 演算子以外
		depthvec.push_back(-2);
	}
	if (depth) {
		logger.Error(E_E, 48, dicfilename, st.linecount);
		return 1;
	}
	if (sz != depthvec.size()) {
		logger.Error(E_E, 21, dicfilename, st.linecount);
		return 1;
	}

	// 演算順序の決定
	for( ; ; ) {
		// 演算対象の演算子を発見
		// 同一深さの演算子の並列は最初のものが選ばれる。つまりAYA5では演算子は常に左から右へ結合される
		// したがって i=j=1 は i=j; j=1 と等価である。i に 1 は代入されない。右から結合されるC/C++とは
		// ここは異なっている。
		int	t_index = -1;
		int	t_depth = -1;
		for(i = 0; i < sz; i++)
			if (depthvec[i] > t_depth) {
				t_depth = depthvec[i];
				t_index = i;
			}
		// 対象が無くなったら抜ける
		if (t_depth == -1)
			break;

		// 定義の開始　演算子の登録
		int	t_type = st.cell[t_index].v.GetType();
		CSerial	addserial(t_index);
		depthvec[t_index] = -2;
		// 左辺の項を取得
		int	f_depth = 1;
		for(i = t_index - 1; i >= 0; i--) {
			// カッコ深さ検査
			if (st.cell[i].v.GetType() == F_TAG_BRACKETIN ||
				st.cell[i].v.GetType() == F_TAG_HOOKBRACKETIN)
				f_depth--;
			else if (st.cell[i].v.GetType() == F_TAG_BRACKETOUT ||
				st.cell[i].v.GetType() == F_TAG_HOOKBRACKETOUT)
				f_depth++;
			if (!f_depth) {
				i = -1;
				break;
			}
			// 取得
			if (depthvec[i] == -2) {
				addserial.index.push_back(i);
				depthvec[i] = -1;
				break;
			}
		}
		if (i == -1) {
			if (t_type == F_TAG_CUMMA)
				logger.Error(E_E, 23, dicfilename, st.linecount);
			else
				logger.Error(E_E, 22, dicfilename, st.linecount);
			return 1;
		}
		// 演算子が","の場合、左項のさらに左へ検索を進め、もし引数を関数に渡すための演算子と関数が
		// 見つかった場合は引数つき関数扱いに書き換える。
		// 関数が見つからない場合は通常の配列ということになる
		if (t_type == F_TAG_CUMMA) {
			f_depth = 1;
			for( ; i >= 0; i--) {
				// カッコ深さ検査
				if (st.cell[i].v.GetType() == F_TAG_BRACKETIN ||
					st.cell[i].v.GetType() == F_TAG_HOOKBRACKETIN)
					f_depth--;
				else if (st.cell[i].v.GetType() == F_TAG_BRACKETOUT ||
					st.cell[i].v.GetType() == F_TAG_HOOKBRACKETOUT)
					f_depth++;
				if (!f_depth) {
					i--;
					break;
				}
			}
			if (i > 0) {
				if (st.cell[i].v.GetType() == F_TAG_FUNCPARAM) {
					// 関数
					depthvec[t_index] = -1;
					addserial.tindex = i;
					depthvec[i] = -2;
					i--;
					if (i < 0) {
						logger.Error(E_E, 25, dicfilename, st.linecount);
						return 1;
					}
					if ((st.cell[i].v.GetType() == F_TAG_SYSFUNC ||
						st.cell[i].v.GetType() == F_TAG_USERFUNC) &&
						depthvec[i] == -2) {
						addserial.index.insert(addserial.index.begin(), i);
						depthvec[i] = -1;
					}
				}
			}
		}
		// 右辺の項を取得　演算子が","の場合は列挙されたすべてを一括して取得する
		if (t_type == F_TAG_CUMMA) {
			// ","
			int	gflg = 0;
			f_depth = 1;
			for(i = t_index + 1; i < sz; i++) {
				// カッコ深さ検査
				if (st.cell[i].v.GetType() == F_TAG_BRACKETIN ||
					st.cell[i].v.GetType() == F_TAG_HOOKBRACKETIN)
					f_depth++;
				else if (st.cell[i].v.GetType() == F_TAG_BRACKETOUT ||
					st.cell[i].v.GetType() == F_TAG_HOOKBRACKETOUT)
					f_depth--;
				if (!f_depth)
					break;
				// 取得
				if (depthvec[i] == -2) {
					addserial.index.push_back(i);
					depthvec[i] = -1;
					gflg = 1;
					continue;
				}
				else if (depthvec[i] == -1)
					continue;
				else if (st.cell[i].v.GetType() == F_TAG_CUMMA) {
					depthvec[i] = -1;
					continue;
				}

				break;
			}
			if (!gflg) {
				logger.Error(E_E, 24, dicfilename, st.linecount);
				return 1;
			}
		}
		else {
			// ","以外
			for(i = t_index + 1; i < sz; i++)
				if (depthvec[i] == -2) {
					addserial.index.push_back(i);
					depthvec[i] = -1;
					break;
				}
			if (i == sz) {
				logger.Error(E_E, 22, dicfilename, st.linecount);
				return 1;
			}
		}
		// 演算定義を登録
		st.serial.push_back(addserial);
	}

	// 演算順序が決定すると、未処理項がひとつだけ残ることになる（これが結果）。これを確認する
	//
	// もし未処理項が演算子でない場合は、有効な項が1つしかないため演算が無かったことを意味している。
	// そのままでは結果が得られないので、「残った項から結果を得る」ことを指示するフラグを追加する
	int	scount = 0;
	for(i = 0; i < sz; i++)
		if (depthvec[i] == -2) {
			scount++;
			if (st.cell[i].v.GetType() >= F_TAG_ORIGIN_VALUE) {
				CSerial	addserial(i);
				addserial.index.push_back(0);	// dmy
				addserial.index.push_back(0);	// dmy
				st.serial.push_back(addserial);
			}
		}
	if (scount != 1) {
		logger.Error(E_E, 86, dicfilename, st.linecount);
		return 1;
	}

	return 0;
}

