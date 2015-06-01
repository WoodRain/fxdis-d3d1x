#include "sm4_instruction_substitution_visitor.h"
#include "sm4.h"
#include <iostream>
#include <string>

namespace sm4 {
	
void rewrite_node(std::shared_ptr<ast_node>& node)
{
	if (node->is_type(node_type::binary_instruction_node))
	{
		auto inst_node = std::static_pointer_cast<binary_instruction_node>(node);

		// rewrite a = mov(b) to a = b
		if (inst_node->opcode == SM4_OPCODE_MOV)
			node = inst_node->input;
	}
	else if (node->is_type(node_type::ternary_instruction_node))
	{
		auto inst_node = std::static_pointer_cast<ternary_instruction_node>(node);
		// rewrite add(a,b) to a + b
		if (inst_node->opcode == SM4_OPCODE_ADD || inst_node->opcode == SM4_OPCODE_IADD)
		{
			auto new_node = std::make_shared<add_node>();
			new_node->lhs = inst_node->lhs;
			new_node->rhs = inst_node->rhs;

			node = new_node;
		}
		// rewrite mul(a,b) to a * b
		else if (inst_node->opcode == SM4_OPCODE_MUL)
		{
			auto new_node = std::make_shared<mul_node>();
			new_node->lhs = inst_node->lhs;
			new_node->rhs = inst_node->rhs;

			node = new_node;
		}
		// rewrite div(a,b) to a / b
		else if (inst_node->opcode == SM4_OPCODE_DIV)
		{
			auto new_node = std::make_shared<div_node>();
			new_node->lhs = inst_node->lhs;
			new_node->rhs = inst_node->rhs;

			node = new_node;
		}
	}
	else if (node->is_type(node_type::quaternary_instruction_node))
	{
		auto inst_node = std::static_pointer_cast<quaternary_instruction_node>(node);
		// rewrite mad(a,b,c) to a*b + c
		if (inst_node->opcode == SM4_OPCODE_MAD)
		{
			auto new_mul_node = std::make_shared<mul_node>();
			new_mul_node->lhs = inst_node->lhs;
			new_mul_node->rhs = inst_node->rhs1;

			auto new_add_node = std::make_shared<add_node>();
			new_add_node->lhs = new_mul_node;
			new_add_node->rhs = inst_node->rhs2;

			node = new_add_node;
		}
	}

	// rewrite a + -b to a - b
	if (node->is_type(node_type::add_node))
	{
		auto old_add_node = std::static_pointer_cast<add_node>(node);
		if (old_add_node->rhs->is_type(node_type::negate_node))
		{
			auto old_negate_node = std::static_pointer_cast<negate_node>(old_add_node->rhs);
			
			auto new_node = std::make_shared<sub_node>();
			new_node->lhs = old_add_node->lhs;
			new_node->rhs = old_negate_node->value;

			node = new_node;
		}
		else if (old_add_node->rhs->is_type(node_type::vector_node))
		{
			auto old_vector_node = std::static_pointer_cast<vector_node>(old_add_node->rhs);

			// If all the values are negative
			bool is_negative = true;
			for (auto value : old_vector_node->values) 
				is_negative &= value->is_negative();

			// Absolute values on vector
			if (is_negative)
				for (auto value : old_vector_node->values)
					value->absolute();

			auto new_node = std::make_shared<sub_node>();
			new_node->lhs = old_add_node->lhs;
			new_node->rhs = old_add_node->rhs;

			node = new_node;
		}
	}
}

void instruction_substitution_visitor::visit(assign_node* node)
{
	rewrite_node(node->rhs);
}

void instruction_substitution_visitor::visit(binary_op* node)
{
	rewrite_node(node->lhs);
	rewrite_node(node->rhs);
}

}