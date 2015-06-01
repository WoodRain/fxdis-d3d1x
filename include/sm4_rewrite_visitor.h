#pragma once
#include "sm4_recursive_visitor.h"

namespace sm4 {

class rewrite_visitor : public recursive_visitor
{
public:
	virtual void visit(binary_op* node);
	virtual void visit(function_call_node* node);
};

}