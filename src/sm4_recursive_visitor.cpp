#include "sm4_recursive_visitor.h"
#include "sm4.h"
#include <iostream>

namespace sm4 {

void recursive_visitor::visit(ast_node* node)
{
}

void recursive_visitor::visit(super_node* node)
{
	for (auto child : node->children)
		child->accept(*this);
}

void recursive_visitor::visit(index_node* node)
{
	node->index->accept(*this);
	node->value->accept(*this);
}

void recursive_visitor::visit(mask_node* node)
{
	node->value->accept(*this);
}

void recursive_visitor::visit(scalar_node* node)
{
	node->value->accept(*this);
}

void recursive_visitor::visit(swizzle_node* node)
{
	node->value->accept(*this);
}

void recursive_visitor::visit(constant_node* node)
{
}

void recursive_visitor::visit(global_index_node* node)
{
}

void recursive_visitor::visit(vector_node* node)
{
	for (auto value : node->values)
		value->accept(*this);
}

void recursive_visitor::visit(comparison_node* node)
{
	node->value->accept(*this);
	
	for (auto child : node->children)
		child->accept(*this);
}

void recursive_visitor::visit(call_node* node)
{
	for (auto arg : node->arguments)
		arg->accept(*this);
}

void recursive_visitor::visit(unary_node* node)
{
	node->value->accept(*this);
}

void recursive_visitor::visit(binary_op* node)
{
	node->lhs->accept(*this);
	node->rhs->accept(*this);
}

}