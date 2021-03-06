#include "duckdb/main/prepared_statement.hpp"
#include "duckdb/common/exception.hpp"
#include "duckdb/main/client_context.hpp"
#include "duckdb/main/prepared_statement_data.hpp"

namespace duckdb {

PreparedStatement::PreparedStatement(shared_ptr<ClientContext> context, shared_ptr<PreparedStatementData> data_p,
                                     string query, idx_t n_param)
    : context(context), data(move(data_p)), query(query), success(true), n_param(n_param) {
	D_ASSERT(data || !success);
}

PreparedStatement::PreparedStatement(string error) : context(nullptr), success(false), error(error) {
}

PreparedStatement::~PreparedStatement() {
}

idx_t PreparedStatement::ColumnCount() {
	D_ASSERT(data);
	return data ? data->types.size() : 0;
}

StatementType PreparedStatement::GetStatementType() {
	D_ASSERT(data);
	return data->statement_type;
}

const vector<LogicalType> &PreparedStatement::GetTypes() {
	D_ASSERT(data);
	return data->types;
}

const vector<string> &PreparedStatement::GetNames() {
	D_ASSERT(data);
	return data->names;
}

unique_ptr<QueryResult> PreparedStatement::Execute(vector<Value> &values, bool allow_stream_result) {
	if (!success) {
		throw InvalidInputException("Attempting to execute an unsuccessfully prepared statement!");
	}
	D_ASSERT(data);
	return context->Execute(query, data, values, allow_stream_result && data->allow_stream_result);
}

} // namespace duckdb
