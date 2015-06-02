#pragma once
#include "sm4_decompile.h"
#include <ostream>

namespace sm4 {

class dump_visitor : public ast_visitor
{
public:
	dump_visitor(std::ostream& stream) :
		stream_(stream)
	{
	}

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

	virtual void visit(function_call_expr_node* node);
	virtual void visit(instruction_call_expr_node* node);

	virtual void visit(unary_expr_node* node);

	virtual void visit(binary_expr_node* node);

	dump_visitor operator=(dump_visitor const& rhs) = delete;

private:
	void write_spaces();
	void write_newline();

	uint32_t depth_ = 0;
	std::ostream& stream_;
};

}