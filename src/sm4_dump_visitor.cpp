#include "sm4_dump_visitor.h"
#include "sm4.h"
#include <iostream>

namespace sm4 {

void dump_visitor::write_spaces()
{
	for (uint32_t i = 0; i < depth_ * 4; ++i)
		std::cout << ' ';
}

void dump_visitor::write_newline()
{
	std::cout << '\n';
}

void dump_visitor::visit(ast_node* node)
{
	write_spaces();
	stream_ << node->get_type_string();
	write_newline();
}

void dump_visitor::visit(super_node* node)
{
	write_spaces();
	stream_ << node->get_type_string();
	write_newline();

	++depth_;
	for (auto child : node->children)
		child->accept(*this);
	--depth_;
}

void dump_visitor::visit(index_node* node)
{
	write_spaces();
	stream_ << node->get_type_string();
	write_newline();

	++depth_;
	node->index->accept(*this);
	node->value->accept(*this);
	--depth_;
}

void dump_visitor::visit(mask_node* node)
{
	write_spaces();
	stream_ << node->get_type_string();
	stream_ << " (";
	for (auto index : node->indices)
		stream_ << "xyzw"[index];
	stream_ << ")";
	write_newline();

	++depth_;
	node->value->accept(*this);
	--depth_;
}

void dump_visitor::visit(scalar_node* node)
{
	write_spaces();
	stream_ << node->get_type_string();
	stream_ << " (" << "xyzw"[node->index] << ")";
	write_newline();

	++depth_;
	node->value->accept(*this);
	--depth_;
}

void dump_visitor::visit(swizzle_node* node)
{
	write_spaces();
	stream_ << node->get_type_string();
	stream_ << " (";
	for (auto index : node->indices)
		stream_ << "xyzw"[index];
	stream_ << ")";
	write_newline();

	++depth_;
	node->value->accept(*this);
	--depth_;
}

void dump_visitor::visit(constant_node* node)
{
	write_spaces();
	stream_ << node->get_type_string();
	write_newline();

	++depth_;
	write_spaces();
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
	write_newline();
	--depth_;
}

void dump_visitor::visit(global_index_node* node)
{
	write_spaces();
	stream_ << node->get_type_string() << " " << node->index;
	write_newline();
}

void dump_visitor::visit(vector_node* node)
{
	write_spaces();
	stream_ << node->get_type_string();
	write_newline();

	++depth_;
	for (auto value : node->values)
		value->accept(*this);
	--depth_;
}

void dump_visitor::visit(comparison_node* node)
{
	write_spaces();
	stream_ << node->get_type_string() << " (nz: " << node->not_zero << ")";
	write_newline();

	++depth_;
	node->value->accept(*this);
	--depth_;
}

void dump_visitor::visit(unary_node* node)
{
	write_spaces();
	stream_ << node->get_type_string();
	write_newline();

	++depth_;
	node->value->accept(*this);
	--depth_;
}

void dump_visitor::visit(binary_instruction_node* node)
{
	write_spaces();
	stream_ << node->get_type_string() << " (" << sm4_opcode_names[node->opcode] << ")";
	write_newline();

	++depth_;
	node->input->accept(*this);
	--depth_;
}

void dump_visitor::visit(ternary_instruction_node* node)
{
	write_spaces();
	stream_ << node->get_type_string() << " (" << sm4_opcode_names[node->opcode] << ")";
	write_newline();

	++depth_;
	node->lhs->accept(*this);
	node->rhs->accept(*this);
	--depth_;
}

void dump_visitor::visit(quaternary_instruction_node* node)
{
	write_spaces();
	stream_ << node->get_type_string() << " (" << sm4_opcode_names[node->opcode] << ")";
	write_newline();

	++depth_;
	node->lhs->accept(*this);
	node->rhs1->accept(*this);
	node->rhs2->accept(*this);
	--depth_;
}

void dump_visitor::visit(binary_op* node)
{
	write_spaces();
	stream_ << node->get_type_string();
	write_newline();

	++depth_;
	node->lhs->accept(*this);
	node->rhs->accept(*this);
	--depth_;
}

}