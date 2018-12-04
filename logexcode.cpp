// 
// AYA version 5
//
// 中間コードをログに出力するクラス　CLogExCode
// written by umeici. 2004
// 

#ifndef POSIX
#  include "stdafx.h"
#endif
#include "basis.h"
#include "sysfunc.h"
#include "logexcode.h"
#include "misc.h"
#include "log.h"
#include "wsex.h"

extern vector<CFunction>	function;
extern CGlobalVariable		variable;
extern CLog					logger;

#ifndef POSIX
#  define	swprintf	_snwprintf
#endif

/* -----------------------------------------------------------------------
 *  関数名  ：  CLogExCode::OutExecutionCodeForCheck
 *  機能概要：  構文解析結果（中間コード）をログに記録します
 * -----------------------------------------------------------------------
 */
void	CLogExCode::OutExecutionCodeForCheck(void)
{
	logger.Message(4);

	wchar_t	tmpstr[STRMAX];
	int	i = 0;
	for(vector<CFunction>::iterator it = function.begin(); it != function.end(); it++, i++) {
		// 関数の定義番号
	        swprintf(tmpstr, STRMAX, L"[%04d] ", i);
		logger.Write(tmpstr);
		logger.Write(L"------------------------------------------------------------------------\n");
		// 関数の名前
		logger.Write(L"       ");
		logger.Write(it->name);
		logger.Write(L"\n");
		logger.Write(L"       ------------------------------------------------------------------------\n");
		// 関数の重複回避モード
		logger.Write(L"Duplication evasion mode : ");
		logger.Write((wchar_t *)choicetype[it->dupl.GetType()]);
		logger.Write(L"\n");
		// 関数が記述されている辞書ファイル名
		logger.Write(L"dic filename : ");
		logger.Write(it->dicfilename);
		logger.Write(L"\n");
		// 関数の動作記述
		wstring indent = L" ";
		int	j = 0;
		for(vector<CStatement>::iterator it2 = it->statement.begin(); it2 != it->statement.end(); it2++, j++) {
			// 行番号（[関数内の行番号/辞書ファイル中の行番号]）
		        swprintf(tmpstr, STRMAX, L"[%04d / %04d]", j, it2->linecount);
			logger.Write(tmpstr);
			// ステートメントの種類別にログに記録
			wstring	formula;
			switch(it2->type) {
			case ST_UNKNOWN:
				logger.Write(indent);
				logger.Write(L"?unknown statement(unknown)\n");
				break;
			case ST_NOP:
				logger.Write(indent);
				logger.Write(L"(no operation)\n");
				break;
			case ST_OPEN:
				logger.Write(indent);
				logger.Write(L"{\n");
				indent += OUTDIC_INDENT;
				break;
			case ST_CLOSE:
				if (indent.size() >= OUTDIC_INDENT_SIZE)
					indent.erase(indent.size() - OUTDIC_INDENT_SIZE, OUTDIC_INDENT_SIZE);
				logger.Write(indent);
				logger.Write(L"}\n");
				break;
			case ST_COMBINE:
				logger.Write(indent);
				logger.Write(L"--\n");
				break;
			case ST_BREAK:
				logger.Write(indent);
				logger.Write(L"break (jump to : ");
				swprintf(tmpstr, STRMAX, L"%d)\n", it2->jumpto);
				logger.Write(tmpstr);
				break;
			case ST_CONTINUE:
				logger.Write(indent);
				logger.Write(L"continue (jump to : ");
				swprintf(tmpstr, STRMAX, L"%d)\n", it2->jumpto);
				logger.Write(tmpstr);
				break;
			case ST_RETURN:
				logger.Write(indent);
				logger.Write(L"return (jump to : ");
				swprintf(tmpstr, STRMAX, L"%d)\n", it2->jumpto);
				logger.Write(tmpstr);
				break;
			case ST_ELSE:
				logger.Write(indent);
				logger.Write(L"else (jump to : ");
				swprintf(tmpstr, STRMAX, L"%d)\n", it2->jumpto);
				logger.Write(tmpstr);
				break;
			case ST_FORMULA:
				logger.Write(indent);
				logger.Write(L"?unknown statement(formula)\n");
				break;
			case ST_FORMULA_OUT_FORMULA:
				logger.Write(indent);
				StructCellString(&(it2->cell), formula);
				logger.Write(formula);
				logger.Write(L" (formula output)\n             ");
				logger.Write(indent);
				logger.Write(L"operation order : ");
				StructSerialString(&(*it2), formula);
				logger.Write(formula);
				logger.Write(L"\n");
				break;
			case ST_FORMULA_SUBST:
				logger.Write(indent);
				StructCellString(&(it2->cell), formula);
				logger.Write(formula);
				logger.Write(L" (substitution)\n             ");
				logger.Write(indent);
				logger.Write(L"operation order : ");
				StructSerialString(&(*it2), formula);
				logger.Write(formula);
				logger.Write(L"\n");
				break;
			case ST_IF:
				logger.Write(indent);
				StructCellString(&(it2->cell), formula);
				formula.insert(0, L"if conditions : ");
				logger.Write(formula);
				logger.Write(L"\n             ");
				logger.Write(indent);
				logger.Write(L"operation order : ");
				StructSerialString(&(*it2), formula);
				logger.Write(formula);
				logger.Write(L"\n             ");
				logger.Write(indent);
				logger.Write(L"jump to : ");
				swprintf(tmpstr, STRMAX, L"%d\n", it2->jumpto);
				logger.Write(tmpstr);
				break;
			case ST_ELSEIF:
				logger.Write(indent);
				StructCellString(&(it2->cell), formula);
				formula.insert(0, L"elseif conditions : ");
				logger.Write(formula);
				logger.Write(L"\n             ");
				logger.Write(indent);
				logger.Write(L"operation order : ");
				StructSerialString(&(*it2), formula);
				logger.Write(formula);
				logger.Write(L"\n             ");
				logger.Write(indent);
				logger.Write(L"jump to : ");
				swprintf(tmpstr, STRMAX, L"%d\n", it2->jumpto);
				logger.Write(tmpstr);
				break;
			case ST_WHILE:
				logger.Write(indent);
				StructCellString(&(it2->cell), formula);
				formula.insert(0, L"while conditions : ");
				logger.Write(formula);
				logger.Write(L"\n             ");
				logger.Write(indent);
				logger.Write(L"operation order : ");
				StructSerialString(&(*it2), formula);
				logger.Write(formula);
				logger.Write(L"\n             ");
				logger.Write(indent);
				logger.Write(L"jump to : ");
				swprintf(tmpstr, STRMAX, L"%d\n", it2->jumpto);
				logger.Write(tmpstr);
				break;
			case ST_SWITCH:
				logger.Write(indent);
				StructCellString(&(it2->cell), formula);
				formula.insert(0, L"switch conditions : ");
				logger.Write(formula);
				logger.Write(L"\n             ");
				logger.Write(indent);
				logger.Write(L"operation order : ");
				StructSerialString(&(*it2), formula);
				logger.Write(formula);
				logger.Write(L"\n             ");
				logger.Write(indent);
				logger.Write(L"jump to : ");
				swprintf(tmpstr, STRMAX, L"%d\n", it2->jumpto);
				logger.Write(tmpstr);
				break;
			case ST_FOR:
				logger.Write(indent);
				StructCellString(&(it2->cell), formula);
				formula.insert(0, L"for initialization : ");
				logger.Write(formula);
				logger.Write(L"\n             ");
				logger.Write(indent);
				logger.Write(L"operation order : ");
				StructSerialString(&(*it2), formula);
				logger.Write(formula);
				logger.Write(L"\n             ");
				logger.Write(indent);
				logger.Write(L"jump to : ");
				swprintf(tmpstr, STRMAX, L"%d\n", it2->jumpto);
				logger.Write(tmpstr);
				break;
			case ST_FOREACH:
				logger.Write(indent);
				StructCellString(&(it2->cell), formula);
				formula.insert(0, L"foreach array : ");
				logger.Write(formula);
				logger.Write(L"\n             ");
				logger.Write(indent);
				logger.Write(L"operation order : ");
				StructSerialString(&(*it2), formula);
				logger.Write(formula);
				logger.Write(L"\n             ");
				logger.Write(indent);
				logger.Write(L"jump to : ");
				swprintf(tmpstr, STRMAX, L"%d\n", it2->jumpto);
				logger.Write(tmpstr);
				break;
			case ST_PARALLEL:
				logger.Write(indent);
				StructCellString(&(it2->cell), formula);
				logger.Write(formula);
				logger.Write(L" (parallel formula output)\n             ");
				logger.Write(indent);
				logger.Write(L"operation order : ");
				StructSerialString(&(*it2), formula);
				logger.Write(formula);
				logger.Write(L"\n");
				break;
			default:
				logger.Write(indent);
				logger.Write(L"?unknown statement(?)\n");
				break;
			};
		}
		logger.Write(L"\n");
	}

	logger.Message(8);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CLogExCode::StructCellString
 *  機能概要：  OutDicInfoForCheckから呼ばれます。数式を文字列化します
 * -----------------------------------------------------------------------
 */
void	CLogExCode::StructCellString(vector<CCell> *cellvector, wstring &formula)
{
	formula = L"";
	wchar_t	tmpstr[STRMAX];
	for(vector<CCell>::iterator it = cellvector->begin(); it != cellvector->end(); it++) {
		if (it->v.GetType() >= F_TAG_ORIGIN &&
			it->v.GetType() < F_TAG_FUNCPARAM) {
			formula += formulatag[it->v.GetType()];
			formula += L" ";
			continue;
		}
		switch(it->v.GetType()) {
		case F_TAG_NOP:
			formula += L"(?NOP) ";
			break;
		case F_TAG_FUNCPARAM:
			formula += L"@ ";
			break;
		case F_TAG_SYSFUNCPARAM:
			formula += L"$ ";
			break;
		case F_TAG_ARRAYORDER:
			formula += L"# " ;
			break;
		case F_TAG_INT:
			swprintf(tmpstr, STRMAX, L"(int)%d ", it->v.i_value);
			formula += tmpstr;
			break;
		case F_TAG_DOUBLE:
			swprintf(tmpstr, STRMAX, L"(double)%lf ", it->v.d_value);
			formula += tmpstr;
			break;
		case F_TAG_STRING:
			formula += L"(string)";
			formula += it->v.s_value;
			formula += L" ";
			break;
		case F_TAG_STRING_EMBED:
			formula += L"(emb string)";
			formula += it->v.s_value;
			formula += L" ";
			break;
		case F_TAG_SYSFUNC:
			formula += L"(sysfunc)";
			formula += sysfunc[it->index];
			formula += L" ";
			break;
		case F_TAG_USERFUNC:
			formula += L"(func)";
			formula += function[it->index].name;
			formula += L" ";
			break;
		case F_TAG_VARIABLE:
			formula += L"(var)";
			formula += variable.GetName(it->index);
			formula += L" ";
			break;
		case F_TAG_LOCALVARIABLE:
			formula += L"(localvar)";
			formula += it->name;
			formula += L" ";
			break;
		default:
			formula += L"(?UNKNOWN) ";
			break;
		};
	}
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CLogExCode::StructSerialString
 *  機能概要：  OutDicInfoForCheckから呼ばれます。数式の演算順序を文字列化します
 * -----------------------------------------------------------------------
 */
void	CLogExCode::StructSerialString(CStatement *st, wstring &formula)
{
	formula = L"";
	wchar_t	tmpstr[STRMAX];
	for(vector<CSerial>::iterator it = st->serial.begin(); it != st->serial.end(); it++) {
		if (it != st->serial.begin())
			formula += L" | ";

		int	type = st->cell[it->tindex].v.GetType();

		if (type == F_TAG_FUNCPARAM)
			formula += L"@(";
		else if (type == F_TAG_SYSFUNCPARAM)
			formula += L"$(" ;
		else if (type == F_TAG_ARRAYORDER)
			formula += L"#(" ;
		else if (type >= F_TAG_ORIGIN && type < F_TAG_ORIGIN_VALUE) {
			formula += formulatag[type];
			formula += L"(";
		}
		else if (type >= F_TAG_ORIGIN_VALUE) {
			formula += L"direct" ;
			continue;
		}
		else {
			formula += L"unknown" ;
			continue;
		}

		for(vector<int>::iterator it2 = it->index.begin(); it2 != it->index.end(); it2++) {
			if (it2 != it->index.begin())
				formula += L",";
			swprintf(tmpstr, STRMAX, L"%d", *it2);
			formula += tmpstr;
		}

		formula += L")";
	}
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CLogExCode::OutVariableInfoForCheck
 *  機能概要：  変数の定義状態をログに記録します
 * -----------------------------------------------------------------------
 */
void	CLogExCode::OutVariableInfoForCheck(void)
{
	logger.Message(5);

	wchar_t	tmpstr[STRMAX];
	wstring	t_str;
	int	var_num = variable.GetNumber();
	for(int	i = 0; i < var_num; i++) {
		CVariable	*var = variable.GetPtr(i);
		// 変数の定義番号
		swprintf(tmpstr, STRMAX, L"[%04d] ", i);
		logger.Write(tmpstr);
		// 変数の名前
		logger.Write(var->name);
		logger.Write(L" = ");
		// 変数の値
		switch(var->v.GetType()) {
		case F_TAG_INT:
			swprintf(tmpstr, STRMAX, L"(int)%d\n", var->v.i_value);
			logger.Write(tmpstr);
			break;
		case F_TAG_DOUBLE:
			swprintf(tmpstr, STRMAX, L"(double)%ld\n", var->v.d_value);
			logger.Write(tmpstr);
			break;
		case F_TAG_STRING:
			logger.Write(L"(string)");
			logger.Write(var->v.s_value);
			logger.Write(L"\n");
			break;
		case F_TAG_ARRAY:
			logger.Write(L"(array) : ");
			StructArrayString(var->v.array, t_str);
			logger.Write(t_str);
			logger.Write(L"\n");
			break;
		default:
			logger.Write(L"(unknown type)\n");
			break;
		};
	}

	logger.Message(8);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CLogExCode::StructArrayString
 *  機能概要：  OutVariableInfoForCheckから呼ばれます。配列変数の内容を文字列化します
 * -----------------------------------------------------------------------
 */
void	CLogExCode::StructArrayString(vector<CValueSub> &vs, wstring &enlist)
{
	enlist = L"";
	wchar_t	tmpstr[STRMAX];
	for(vector<CValueSub>::iterator it = vs.begin(); it != vs.end(); it++) {
		switch(it->GetType()) {
		case F_TAG_INT:
			swprintf(tmpstr, STRMAX, L"(int)%d ", it->i_value);
			enlist += tmpstr;
			break;
		case F_TAG_DOUBLE:
			swprintf(tmpstr, STRMAX, L"(double)%lf ", it->d_value);
			enlist += tmpstr;
			break;
		case F_TAG_STRING:
			enlist += L"(string)";
			enlist += it->s_value;
			enlist += L" ";
			break;
		default:
			enlist += L"(?UNKNOWN) ";
			break;
		};
	}
}
