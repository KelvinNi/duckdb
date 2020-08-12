#include "duckdb/function/scalar/string_functions.hpp"

#include "duckdb/common/exception.hpp"
#include "duckdb/common/vector_operations/vector_operations.hpp"
#include "duckdb/common/vector_operations/ternary_executor.hpp"

#include <string.h>
#include <ctype.h>
#include <unordered_map>
#include <algorithm> // std::max

using namespace std;

namespace duckdb {

static idx_t next_needle(const char *input_haystack, idx_t size_haystack, const char *input_needle,
                         const idx_t size_needle) {
	// Needle needs something to proceed
	if (size_needle > 0) {
		// Haystack should be bigger or equal size to the needle
		for (idx_t string_position = 0; (size_haystack - string_position) >= size_needle; ++string_position) {
			// Compare Needle to the Haystack
			if ((memcmp(input_haystack + string_position, input_needle, size_needle) == 0)) {
				return string_position;
			}
		}

		return size_haystack;
	}
	// Did not find the needle
	return size_haystack;
}

static string_t replace_scalar_function(const string_t &haystack, const string_t &needle, const string_t &thread,
                                        vector<char> &result) {
	// Get information about the needle, the haystack and the "thread"
	auto input_haystack = haystack.GetData();
	auto size_haystack = haystack.GetSize();

	const auto input_needle = needle.GetData();
	const auto size_needle = needle.GetSize();

	const auto input_thread = thread.GetData();
	const auto size_thread = thread.GetSize();

	//  Reuse the buffer
	result.clear();

	for (;;) {
		//  Append the non-matching characters
		auto string_position = next_needle(input_haystack, size_haystack, input_needle, size_needle);
		result.insert(result.end(), input_haystack, input_haystack + string_position);
		input_haystack += string_position;
		size_haystack -= string_position;

		//  Stop when we have read the entire haystack
		if (size_haystack == 0)
			break;

		//  Replace the matching characters
		result.insert(result.end(), input_thread, input_thread + size_thread);
		input_haystack += size_needle;
		size_haystack -= size_needle;
	}

	return string_t(result.data(), result.size());
}

static void replace_function(DataChunk &args, ExpressionState &state, Vector &result) {
	assert(args.column_count() == 3 && args.data[0].type == TypeId::VARCHAR && args.data[1].type == TypeId::VARCHAR &&
	       args.data[2].type == TypeId::VARCHAR);
	auto &haystack_vector = args.data[0];
	auto &needle_vector = args.data[1];
	auto &thread_vector = args.data[2];

	vector<char> buffer;
	TernaryExecutor::Execute<string_t, string_t, string_t, string_t>(
	    haystack_vector, needle_vector, thread_vector, result, args.size(),
	    [&](string_t input_string, string_t needle_string, string_t thread_string) {
		    return StringVector::AddString(result,
		                                   replace_scalar_function(input_string, needle_string, thread_string, buffer));
	    });
}

void ReplaceFun::RegisterFunction(BuiltinFunctions &set) {
	set.AddFunction(ScalarFunction("replace",         // name of the function
	                               {LogicalType::VARCHAR, // argument list
	                                LogicalType::VARCHAR, LogicalType::VARCHAR},
	                               LogicalType::VARCHAR,   // return type
	                               replace_function)); // pointer to function implementation
}

} // namespace duckdb
