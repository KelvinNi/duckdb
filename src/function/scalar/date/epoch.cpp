#include "duckdb/function/scalar/date_functions.hpp"
#include "duckdb/common/types/time.hpp"
#include "duckdb/common/types/date.hpp"
#include "duckdb/common/types/timestamp.hpp"
#include "duckdb/common/vector_operations/vector_operations.hpp"
#include "duckdb/common/vector_operations/unary_executor.hpp"

namespace duckdb {

static void epoch_sec_function(DataChunk &input, ExpressionState &state, Vector &result) {
	D_ASSERT(input.ColumnCount() == 1);

	string output_buffer;
	UnaryExecutor::Execute<int64_t, timestamp_t, true>(
	    input.data[0], result, input.size(), [&](int64_t input) { return Timestamp::FromEpochSeconds(input); });
}

static void epoch_ms_function(DataChunk &input, ExpressionState &state, Vector &result) {
	D_ASSERT(input.ColumnCount() == 1);

	string output_buffer;
	UnaryExecutor::Execute<int64_t, timestamp_t, true>(input.data[0], result, input.size(),
	                                                   [&](int64_t input) { return Timestamp::FromEpochMs(input); });
}

void EpochFun::RegisterFunction(BuiltinFunctions &set) {
	ScalarFunctionSet epoch("epoch_ms");
	epoch.AddFunction(ScalarFunction({LogicalType::BIGINT}, LogicalType::TIMESTAMP, epoch_ms_function));
	set.AddFunction(epoch);
	// to_timestamp is an alias from Postgres that converts the time in seconds to a timestamp
	ScalarFunctionSet to_timestamp("to_timestamp");
	to_timestamp.AddFunction(ScalarFunction({LogicalType::BIGINT}, LogicalType::TIMESTAMP, epoch_sec_function));
	set.AddFunction(to_timestamp);
}

} // namespace duckdb
