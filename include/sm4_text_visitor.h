#pragma once
#include "sm4_decompile.h"
#include <ostream>

namespace sm4 {

class text_visitor : public ast_visitor
{
public:
	text_visitor(std::ostream& stream) :
		stream_(stream)
	{
	}

	virtual void visit(ast_node* node);
	virtual void visit(super_node* node);
	virtual void visit(type_node* node);
	virtual void visit(function_node* node);
	virtual void visit(structure_node* node);
	virtual void visit(expr_stmt_node* node);

	virtual void visit(dynamic_index_node* node);
	virtual void visit(static_index_node* node);

	virtual void visit(variable_node* node);
	virtual void visit(constant_node* node);
	virtual void visit(global_variable_node* node);
	virtual void visit(vector_node* node);

	virtual void visit(comparison_node* node);
	virtual void visit(else_node* node);

	virtual void visit(function_call_expr_node* node);
	virtual void visit(instruction_call_expr_node* node);

	virtual void visit(unary_expr_node* node);

	virtual void visit(ret_node* node);

	virtual void visit(negate_node* node);

	virtual void visit(assign_expr_node* node);
	virtual void visit(add_expr_node* node);
	virtual void visit(sub_expr_node* node);
	virtual void visit(mul_expr_node* node);
	virtual void visit(div_expr_node* node);
	virtual void visit(eq_expr_node* node);
	virtual void visit(neq_expr_node* node);

	void print_argument_list(std::vector<std::shared_ptr<ast_node>> const& v);

	text_visitor operator=(text_visitor const& rhs) = delete;

private:
	void write_spaces();
	void write_newline();

	uint32_t depth_ = 0;
	std::ostream& stream_;
};

}