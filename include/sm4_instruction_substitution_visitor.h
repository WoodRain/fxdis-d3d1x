#pragma once
#include "sm4_recursive_visitor.h"

namespace sm4 {

class instruction_substitution_visitor : public recursive_visitor
{
public:
	virtual void visit(binary_op* node);
};

}