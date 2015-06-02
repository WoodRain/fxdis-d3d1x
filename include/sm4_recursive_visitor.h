#pragma once
#include "sm4_decompile.h"

namespace sm4 {

class recursive_visitor : public ast_visitor
{
public:
	virtual void visit(ast_node* node);
	virtual void visit(super_node* node);

	virtual void visit(dynamic_index_node* node);
	virtual void visit(mask_node* node);
	virtual void visit(scalar_node* node);
	virtual void visit(swizzle_node* node);

	virtual void visit(constant_node* node);
	virtual void visit(global_variable_node* node);
	virtual void visit(vector_node* node);

	virtual void visit(comparison_node* node);

	virtual void visit(call_expr_node* node);

	virtual void visit(unary_expr_node* node);

	virtual void visit(binary_expr_node* node);
};

}