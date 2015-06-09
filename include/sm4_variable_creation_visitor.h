#pragma once
#include "sm4_recursive_visitor.h"

#include <unordered_set>

namespace sm4 {

class decompiler;

class variable_creation_visitor : public recursive_visitor
{
public:
	variable_creation_visitor(sm4::decompiler* instance);

	virtual void visit(super_node* node);

private:
	sm4::decompiler* decompiler_;
	std::unordered_set<ast_node*> visited_;
};

}