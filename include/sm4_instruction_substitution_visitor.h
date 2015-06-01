#pragma once
#include "sm4_recursive_visitor.h"

namespace sm4 {

class instruction_substitution_visitor : public recursive_visitor
{
public:
	virtual void visit(assign_node* node);
	virtual void visit(binary_op* node);

	instruction_substitution_visitor operator=(instruction_substitution_visitor const& rhs) = delete;
};

}