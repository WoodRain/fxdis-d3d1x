#pragma once
#include "sm4_recursive_visitor.h"

namespace sm4 {

class consolidation_visitor : public recursive_visitor
{
public:
	virtual void visit(super_node* node);
};

}