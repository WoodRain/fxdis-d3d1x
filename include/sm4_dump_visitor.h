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

	virtual void visit(index_node* node);
	virtual void visit(mask_node* node);
	virtual void visit(scalar_node* node);
	virtual void visit(swizzle_node* node);

	virtual void visit(constant_node* node);
	virtual void visit(global_index_node* node);
	virtual void visit(vector_node* node);

	virtual void visit(comparison_node* node);

	virtual void visit(function_call_node* node);

	virtual void visit(unary_node* node);
	virtual void visit(binary_instruction_node* node);
	virtual void visit(ternary_instruction_node* node);
	virtual void visit(quaternary_instruction_node* node);

	virtual void visit(binary_op* node);

	dump_visitor operator=(dump_visitor const& rhs) = delete;

private:
	void write_spaces();
	void write_newline();

	uint32_t depth_ = 0;
	std::ostream& stream_;
};

}