#include "sm4_recursive_visitor.h"
#include "sm4.h"
#include <iostream>

namespace sm4 {

void recursive_visitor::visit(ast_node* node)
{
}

void recursive_visitor::visit(super_node* node)
{
	this->visit_base(node);

	for (auto child : node->children)
		child->accept(*this);
}

void recursive_visitor::visit(expr_stmt_node* node)
{
	this->visit_base(node);

	node->value->accept(*this);
}

void recursive_visitor::visit(dynamic_index_node* node)
{
	this->visit_base(node);

	if (node->index)
		node->index->accept(*this);
	node->value->accept(*this);
}

void recursive_visitor::visit(static_index_node* node)
{
	this->visit_base(node);

	node->value->accept(*this);
}

void recursive_visitor::visit(variable_decl_node* node)
{
	this->visit_base(node);

	node->variable->accept(*this);
}

void recursive_visitor::visit(vector_node* node)
{
	this->visit_base(node);

	for (auto value : node->values)
		value->accept(*this);
}

void recursive_visitor::visit(comparison_node* node)
{
	this->visit_base(node);

	node->expression->accept(*this);
	
	this->visit(static_cast<comparison_node::base_class*>(node));
}

void recursive_visitor::visit(call_expr_node* node)
{
	this->visit_base(node);

	for (auto arg : node->arguments)
		arg->accept(*this);
}

void recursive_visitor::visit(unary_expr_node* node)
{
	this->visit_base(node);

	node->value->accept(*this);
}

void recursive_visitor::visit(binary_expr_node* node)
{
	this->visit_base(node);

	node->lhs->accept(*this);
	node->rhs->accept(*this);
}

}