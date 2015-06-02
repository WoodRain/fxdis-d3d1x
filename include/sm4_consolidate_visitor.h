#pragma once
#include "sm4_recursive_visitor.h"

namespace sm4 {

class consolidate_visitor : public recursive_visitor
{
public:
	virtual void visit(super_node* node);
};

}