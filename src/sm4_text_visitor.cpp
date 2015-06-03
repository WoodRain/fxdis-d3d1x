#include "sm4_text_visitor.h"
#include "sm4.h"
#include <iostream>
#include <string>

namespace sm4 {

std::string sanitized_node_type(ast_node* node)
{
	std::string ret = node->get_type_string();
	ret = ret.substr(0, ret.find("_node"));
	return ret;
}

void text_visitor::write_spaces()
{
	for (uint32_t i = 0; i < depth_ * 4; ++i)
		std::cout << ' ';
}

void text_visitor::write_newline()
{
	std::cout << '\n';
}

void text_visitor::visit(ast_node* node)
{
	write_spaces();
	stream_ << sanitized_node_type(node);
	write_newline();
}

void text_visitor::visit(super_node* node)
{
	for (auto child : node->children)
	{
		child->accept(*this);
		stream_ << "\n";
	}
}

void text_visitor::visit(type_node* node)
{
	stream_ << node->name;
}

void text_visitor::visit(function_node* node)
{
	write_spaces();
	if (node->return_type)
		stream_ << node->return_type->name;
	else
		stream_ << "void";

	stream_ << " ";
	stream_ << node->name;
	stream_ << "(";
	this->print_argument_list(node->arguments);
	stream_ << ")";
	write_newline();

	write_spaces();
	stream_ << "{";
	write_newline();

	++depth_;
	for (auto child : node->children)
		child->accept(*this);
	--depth_;

	write_spaces();
	stream_ << "}";
	write_newline();
}

void text_visitor::visit(structure_node* node)
{
	write_spaces();
	stream_ << "struct ";
	stream_ << node->name;
	write_newline();

	write_spaces();
	stream_ << "{";
	write_newline();

	++depth_;
	for (auto child : node->children)
		child->accept(*this);
	--depth_;

	write_spaces();
	stream_ << "}";
	write_newline();
}

void text_visitor::visit(assign_stmt_node* node)
{
	write_spaces();
	node->lhs->accept(*this);
	stream_ << " = ";
	node->rhs->accept(*this);
	stream_ << ";";
	write_newline();
}

void text_visitor::visit(expr_stmt_node* node)
{
	write_spaces();
	node->value->accept(*this);
	stream_ << ";";
	write_newline();
}

void text_visitor::visit(dynamic_index_node* node)
{
	node->value->accept(*this);
	stream_ << "[";
	
	if (node->index)
	{
		node->index->accept(*this);
		if (node->displacement)
			stream_ << "+" << node->displacement;
	}
	else
	{
		stream_ << node->displacement;
	}

	stream_ << "]";
}

void text_visitor::visit(static_index_node* node)
{
	node->value->accept(*this);
	stream_ << ".";
	for (auto index : node->indices)
		stream_ << "xyzw"[index];
}

void text_visitor::visit(variable_node* node)
{
	stream_ << node->type->name;
	stream_ << " ";
	stream_ << node->name;
}

void text_visitor::visit(constant_node* node)
{
	switch (node->current_type)
	{
	case value_type::f32:
		stream_ << node->f32;
		break;
	case value_type::i32:
		stream_ << node->i32;
		break;
	case value_type::u32:
		stream_ << node->u32;
		break;
	case value_type::f64:
		stream_ << node->f64;
		break;
	case value_type::i64:
		stream_ << node->i64;
		break;
	case value_type::u64:
		stream_ << node->u64;
		break;
	}
}

void text_visitor::visit(global_variable_node* node)
{
	stream_ << sanitized_node_type(node);
	if (node->index >= 0)
		stream_ << node->index;
}

void text_visitor::visit(vector_node* node)
{
	if (node->values.empty())
		return;

	if (node->values.size() > 1)
	{
		auto type = node->values.front()->current_type;
		if (type == value_type::f32)
			stream_ << "float";
		else if (type == value_type::f64)
			stream_ << "double";
		else if (type == value_type::i32 || type == value_type::i64)
			stream_ << "int";
		else if (type == value_type::u32 || type == value_type::u64)
			stream_ << "uint";
		stream_ << node->values.size();

		std::vector<std::shared_ptr<ast_node>> arguments;
		for (auto value : node->values)
			arguments.push_back(value);

		stream_ << "(";
		this->print_argument_list(arguments);
		stream_ << ")";
	}
	else
		node->values.front()->accept(*this);
}

void text_visitor::visit(comparison_node* node)
{
	write_spaces();
	stream_ << sanitized_node_type(node) << " (";
	node->expression->accept(*this);
	stream_ << ")";
	write_newline();

	write_spaces();
	stream_ << "{";
	write_newline();

	++depth_;
	for (auto child : node->children)
		child->accept(*this);
	--depth_;

	write_spaces();
	stream_ << "}";
	write_newline();
}


void text_visitor::visit(else_node* node)
{
	write_spaces();
	stream_ << "else";
	write_newline();

	write_spaces();
	stream_ << "{";
	write_newline();

	++depth_;
	for (auto child : node->children)
		child->accept(*this);
	--depth_;

	write_spaces();
	stream_ << "}";
	write_newline();
}

void text_visitor::visit(function_call_expr_node* node)
{
	stream_ << node->name << "(";
	this->print_argument_list(node->arguments);
	stream_ << ")";
}

void text_visitor::visit(instruction_call_expr_node* node)
{
	stream_ << sm4_opcode_names[node->opcode] << "(";
	this->print_argument_list(node->arguments);
	stream_ << ")";
}

void text_visitor::visit(unary_expr_node* node)
{	
	stream_ << sanitized_node_type(node) << "(";
	node->value->accept(*this);
	stream_ << ")";
}

void text_visitor::visit(ret_node* node)
{
	write_spaces();
	stream_ << "return;";
	write_newline();
}

void text_visitor::visit(negate_node* node)
{	
	stream_ << "-";
	node->value->accept(*this);
}

void text_visitor::visit(add_expr_node* node)
{
	node->lhs->accept(*this);
	stream_ << " + ";
	node->rhs->accept(*this);
}

void text_visitor::visit(sub_expr_node* node)
{
	node->lhs->accept(*this);
	stream_ << " - ";
	node->rhs->accept(*this);
}

bool requires_brackets(std::shared_ptr<ast_node> node)
{
	return node->is_type(node_type::add_expr_node) || node->is_type(node_type::sub_expr_node);
}

void text_visitor::visit(mul_expr_node* node)
{
	if (requires_brackets(node->lhs))
		stream_ << "(";

	node->lhs->accept(*this);

	if (requires_brackets(node->lhs))
		stream_ << ")";

	stream_ << " * ";

	if (requires_brackets(node->rhs))
		stream_ << "(";

	node->rhs->accept(*this);

	if (requires_brackets(node->rhs))
		stream_ << ")";
}

void text_visitor::visit(div_expr_node* node)
{
	if (requires_brackets(node->lhs))
		stream_ << "(";

	node->lhs->accept(*this);

	if (requires_brackets(node->lhs))
		stream_ << ")";

	stream_ << " / ";

	if (requires_brackets(node->rhs))
		stream_ << "(";

	node->rhs->accept(*this);

	if (requires_brackets(node->rhs))
		stream_ << ")";
}

void text_visitor::visit(eq_expr_node* node)
{
	node->lhs->accept(*this);
	stream_ << " == ";
	node->rhs->accept(*this);
}

void text_visitor::visit(neq_expr_node* node)
{
	node->lhs->accept(*this);
	stream_ << " != ";
	node->rhs->accept(*this);
}

void text_visitor::print_argument_list(std::vector<std::shared_ptr<ast_node>> const& v)
{
	bool first = true;
	for (auto argument : v)
	{
		if (!first)
			stream_ << ", ";
		argument->accept(*this);
		first = false;
	}
}

}