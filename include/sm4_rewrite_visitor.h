#pragma once
#include "sm4_recursive_visitor.h"

namespace sm4 {

class rewrite_visitor : public recursive_visitor
{
public:
	virtual void visit(variable_node* node);
	virtual void visit(unary_expr_node* node);
	virtual void visit(binary_expr_node* node);
	virtual void visit(call_expr_node* node);
};

}