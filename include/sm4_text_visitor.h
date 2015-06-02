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

	virtual void visit(index_node* node);
	virtual void visit(mask_node* node);
	virtual void visit(scalar_node* node);
	virtual void visit(swizzle_node* node);

	virtual void visit(constant_node* node);
	virtual void visit(global_index_node* node);
	virtual void visit(vector_node* node);

	virtual void visit(comparison_node* node);

	virtual void visit(function_call_node* node);
	virtual void visit(instruction_call_node* node);

	virtual void visit(unary_node* node);

	virtual void visit(ret_node* node);

	virtual void visit(negate_node* node);

	virtual void visit(add_node* node);
	virtual void visit(sub_node* node);
	virtual void visit(mul_node* node);
	virtual void visit(div_node* node);
	virtual void visit(assign_node* node);

	text_visitor operator=(text_visitor const& rhs) = delete;

private:
	void write_spaces();
	void write_newline();

	uint32_t depth_ = 0;
	std::ostream& stream_;
};

}