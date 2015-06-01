#include "sm4_text_visitor.h"
#include "sm4.h"
#include <iostream>
#include <string>

namespace sm4 {

// I don't even give a fuck
std::string sanitized_node_type(ast_node* node)
{
	// thanks, VS, you just saved me from your own stupidity
	// fu
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
	write_spaces();
	stream_ << sanitized_node_type(node);
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

void text_visitor::visit(index_node* node)
{
	node->value->accept(*this);
	stream_ << "[";
	node->index->accept(*this);
	stream_ << "]";
}

void text_visitor::visit(mask_node* node)
{
	node->value->accept(*this);
	stream_ << ".";
	for (auto index : node->indices)
		stream_ << "xyzw"[index];
}

void text_visitor::visit(scalar_node* node)
{
	node->value->accept(*this);
	stream_ << "." << "xyzw"[node->index];
}

void text_visitor::visit(swizzle_node* node)
{
	node->value->accept(*this);
	stream_ << ".";
	for (auto index : node->indices)
		stream_ << "xyzw"[index];
}

void text_visitor::visit(constant_node* node)
{
	switch (node->current_type)
	{
	case constant_node::type::f32:
		stream_ << node->f32;
		break;
	case constant_node::type::i32:
		stream_ << node->i32;
		break;
	case constant_node::type::u32:
		stream_ << node->u32;
		break;
	case constant_node::type::f64:
		stream_ << node->f64;
		break;
	case constant_node::type::i64:
		stream_ << node->i64;
		break;
	case constant_node::type::u64:
		stream_ << node->u64;
		break;
	}
}

void text_visitor::visit(global_index_node* node)
{
	stream_ << sanitized_node_type(node) << node->index;
}

void text_visitor::visit(vector_node* node)
{
	if (node->values.empty())
		return;

	if (node->values.size() > 1)
	{
		auto type = node->values.front()->current_type;
		if (type == constant_node::type::f32)
			stream_ << "float";
		else if (type == constant_node::type::f64)
			stream_ << "double";
		else if (type == constant_node::type::i32 || type == constant_node::type::i64)
			stream_ << "int";
		else if (type == constant_node::type::u32 || type == constant_node::type::u64)
			stream_ << "uint";

		stream_ << node->values.size();
		stream_ << "(";
		bool first = true;
		for (auto value : node->values)
		{
			if (!first)
				stream_ << ", ";
			value->accept(*this);
			first = false;
		}
		stream_ << ")";
	}
	else
		node->values.front()->accept(*this);
}

void text_visitor::visit(comparison_node* node)
{
	write_spaces();
	stream_ << sanitized_node_type(node) << " (";
	node->value->accept(*this);
	// lol operator precedence
	// don't overload operators with stupid precedence, god damn it C++
	stream_ << (node->not_zero ? " != 0" : " == 0") << ")";
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

void text_visitor::visit(unary_node* node)
{	
	stream_ << sanitized_node_type(node) << "(";
	node->value->accept(*this);
	stream_ << ")";
}

void text_visitor::visit(binary_instruction_node* node)
{
	stream_ << sm4_opcode_names[node->opcode] << "(";
	node->input->accept(*this);
	stream_ << ")";
}

void text_visitor::visit(ternary_instruction_node* node)
{
	stream_ << sm4_opcode_names[node->opcode] << "(";
	node->lhs->accept(*this);
	stream_ << ", ";
	node->rhs->accept(*this);
	stream_ << ")";
}

void text_visitor::visit(quaternary_instruction_node* node)
{
	stream_ << sm4_opcode_names[node->opcode] << "(";
	node->lhs->accept(*this);
	stream_ << ", ";
	node->rhs1->accept(*this);
	stream_ << ", ";
	node->rhs2->accept(*this);
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

void text_visitor::visit(absolute_node* node)
{	
	stream_ << "abs(";
	node->value->accept(*this);
	stream_ << ")";
}

void text_visitor::visit(add_node* node)
{
	node->lhs->accept(*this);
	stream_ << " + ";
	node->rhs->accept(*this);
}

void text_visitor::visit(sub_node* node)
{
	node->lhs->accept(*this);
	stream_ << " - ";
	node->rhs->accept(*this);
}

void text_visitor::visit(mul_node* node)
{
	node->lhs->accept(*this);
	stream_ << " * ";
	node->rhs->accept(*this);
}

void text_visitor::visit(div_node* node)
{
	node->lhs->accept(*this);
	stream_ << " / ";
	node->rhs->accept(*this);
}

void text_visitor::visit(assign_node* node)
{
	write_spaces();
	node->lhs->accept(*this);
	stream_ << " = ";
	node->rhs->accept(*this);
	stream_ << ";";
	write_newline();
}

}