//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/function/scalar/string_functions.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/function/scalar_function.hpp"
#include "duckdb/function/function_set.hpp"

namespace re2 {
class RE2;
}

namespace duckdb {

struct ReverseFun {
	static void RegisterFunction(BuiltinFunctions &set);
};

struct LowerFun {
	static void RegisterFunction(BuiltinFunctions &set);
};

struct UpperFun {
	static void RegisterFunction(BuiltinFunctions &set);
};

struct ConcatFun {
	static void RegisterFunction(BuiltinFunctions &set);
};

struct ConcatWSFun {
	static void RegisterFunction(BuiltinFunctions &set);
};

struct LengthFun {
	static void RegisterFunction(BuiltinFunctions &set);
};

struct LikeFun {
	static void RegisterFunction(BuiltinFunctions &set);
};

struct RegexpFun {
	static void RegisterFunction(BuiltinFunctions &set);
};

struct SubstringFun {
	static void RegisterFunction(BuiltinFunctions &set);
};

struct PrintfFun {
	static void RegisterFunction(BuiltinFunctions &set);
};

struct InstrFun {
	static void RegisterFunction(BuiltinFunctions &set);
};

struct PrefixFun {
	static void RegisterFunction(BuiltinFunctions &set);
};

struct SuffixFun {
	static void RegisterFunction(BuiltinFunctions &set);
};

struct ContainsFun {
	static void RegisterFunction(BuiltinFunctions &set);
};

struct ContainsKMPBindData : public FunctionData {
	ContainsKMPBindData(std::unique_ptr<vector<uint32_t>> table, std::unique_ptr<string_t> pattern);
	~ContainsKMPBindData();

	std::unique_ptr<vector<uint32_t>> kmp_table;
	std::unique_ptr<string_t> pattern;

	unique_ptr<FunctionData> Copy() override;
};

struct RegexpMatchesBindData : public FunctionData {
	RegexpMatchesBindData(std::unique_ptr<re2::RE2> constant_pattern, string range_min, string range_max,
	                      bool range_success);
	~RegexpMatchesBindData();

	std::unique_ptr<re2::RE2> constant_pattern;
	string range_min, range_max;
	bool range_success;

	unique_ptr<FunctionData> Copy() override;
};

} // namespace duckdb
