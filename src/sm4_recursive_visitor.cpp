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

void recursive_visitor::visit(expr_stmt_node* node)
{
	node->value->accept(*this);
}

void recursive_visitor::visit(dynamic_index_node* node)
{
	if (node->index)
		node->index->accept(*this);
	node->value->accept(*this);
}

void recursive_visitor::visit(static_index_node* node)
{
	node->value->accept(*this);
}

void recursive_visitor::visit(constant_node* node)
{
}

void recursive_visitor::visit(global_variable_node* node)
{
}

void recursive_visitor::visit(vector_node* node)
{
	for (auto value : node->values)
		value->accept(*this);
}

void recursive_visitor::visit(comparison_node* node)
{
	node->expression->accept(*this);
	
	this->visit(static_cast<comparison_node::base_class*>(node));
}

void recursive_visitor::visit(call_expr_node* node)
{
	for (auto arg : node->arguments)
		arg->accept(*this);
}

void recursive_visitor::visit(unary_expr_node* node)
{
	node->value->accept(*this);
}

void recursive_visitor::visit(binary_expr_node* node)
{
	node->lhs->accept(*this);
	node->rhs->accept(*this);
}

}