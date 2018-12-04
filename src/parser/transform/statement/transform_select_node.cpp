#include "parser/statement/select_statement.hpp"
#include "parser/transformer.hpp"
#include "common/exception.hpp"
#include "parser/query_node/list.hpp"

using namespace duckdb;
using namespace postgres;
using namespace std;

unique_ptr<QueryNode> Transformer::TransformSelectNode(postgres::SelectStmt *stmt) {
    unique_ptr<QueryNode> node;
	switch (stmt->op) {
	case SETOP_NONE: {
		node = make_unique<SelectNode>();
        auto result = (SelectNode*) node.get();
        // distinct clause
        result->select_distinct = stmt->distinctClause != NULL ? true : false;
        // from table
		result->from_table = TransformFrom(stmt->fromClause);
        // group by
		TransformGroupBy(stmt->groupClause, result->groupby.groups);
		result->groupby.having = TransformExpression(stmt->havingClause);
        // where
		result->where_clause = TransformExpression(stmt->whereClause);
        // select list
		if (!TransformExpressionList(stmt->targetList, result->select_list)) {
			throw Exception("Failed to transform expression list.");
		}
        break;
	}
	case SETOP_UNION:
	case SETOP_EXCEPT:
	case SETOP_INTERSECT: {
		node = make_unique<SetOperationNode>();
        auto result = (SetOperationNode*) node.get();
		result->left = TransformSelectNode(stmt->larg);
		result->right = TransformSelectNode(stmt->rarg);
		if (!result->left || !result->right) {
			throw Exception("Failed to transform setop children.");
		}

		result->select_distinct = true;
		switch (stmt->op) {
		case SETOP_UNION:
			result->select_distinct = !stmt->all;
			result->setop_type = SetOperationType::UNION;
			break;
		case SETOP_EXCEPT:
			result->setop_type = SetOperationType::EXCEPT;
			break;
		case SETOP_INTERSECT:
			result->setop_type = SetOperationType::INTERSECT;
			break;
		default:
			throw Exception("Unexpected setop type");
		}
		// if we compute the distinct result here, we do not have to do this in
		// the children. This saves a bunch of unnecessary DISTINCTs.
		if (result->select_distinct) {
			result->left->select_distinct = false;
			result->right->select_distinct = false;
		}
        break;
	}
	default:
		throw NotImplementedException("Statement type %d not implemented!", stmt->op);
	}
    // transform the common properties
    // both the set operations and the regular select can have an ORDER BY/LIMIT attached to them
    TransformOrderBy(stmt->sortClause, node->orderby);
    if (stmt->limitCount) {
        node->limit.limit = ((A_Const *)stmt->limitCount)->val.val.ival;
        node->limit.offset = 0;
    }
    if (stmt->limitOffset) {
        node->limit.offset = ((A_Const *)stmt->limitOffset)->val.val.ival;
        if (!stmt->limitCount) {
            node->limit.limit = std::numeric_limits<int64_t>::max();
        }
    }
    return node;
}