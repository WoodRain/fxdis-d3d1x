#include "sm4_rewrite_visitor.h"
#include "sm4.h"
#include <iostream>
#include <string>

namespace sm4 {

void rewrite_instruction_call_expr_node(std::shared_ptr<ast_node>& node)
{
	auto inst_node = std::static_pointer_cast<instruction_call_expr_node>(node);

	auto& args = inst_node->arguments;

	// rewrite a = mov(b) to a = b
	if (inst_node->opcode == SM4_OPCODE_MOV)
	{
		node = args[0];
	}
	// rewrite rsq(a) to rsqrt(a)
	else if (inst_node->opcode == SM4_OPCODE_RSQ)
	{
		node = std::make_shared<function_call_expr_node>("rsqrt", args[0]);
	}
	// rewrite add(a,b) to a + b
	else if (inst_node->opcode == SM4_OPCODE_ADD || inst_node->opcode == SM4_OPCODE_IADD)
	{
		auto new_node = std::make_shared<add_expr_node>();
		new_node->lhs = args[0];
		new_node->rhs = args[1];

		node = new_node;
	}
	// rewrite mul(a,b) to a * b
	else if (inst_node->opcode == SM4_OPCODE_MUL)
	{
		auto new_node = std::make_shared<mul_expr_node>();
		new_node->lhs = args[0];
		new_node->rhs = args[1];

		node = new_node;
	}
	// rewrite div(a,b) to a / b
	else if (inst_node->opcode == SM4_OPCODE_DIV)
	{
		auto new_node = std::make_shared<div_expr_node>();
		new_node->lhs = args[0];
		new_node->rhs = args[1];

		node = new_node;
	}
	// rewrite dp2/3/4 to function calls
	else if (inst_node->opcode == SM4_OPCODE_DP2 || 
			inst_node->opcode == SM4_OPCODE_DP3 || 
			inst_node->opcode == SM4_OPCODE_DP4)
	{
		node = std::make_shared<function_call_expr_node>("dot", args[0], args[1]);
	}
	// rewrite mad(a,b,c) to a*b + c
	else if (inst_node->opcode == SM4_OPCODE_MAD)
	{
		auto new_mul_node = std::make_shared<mul_expr_node>();
		new_mul_node->lhs = args[0];
		new_mul_node->rhs = args[1];

		auto new_add_expr_node = std::make_shared<add_expr_node>();
		new_add_expr_node->lhs = new_mul_node;
		new_add_expr_node->rhs = args[2];

		node = new_add_expr_node;
	}
}

void rewrite_add_expr_node(std::shared_ptr<ast_node>& node)
{
	auto old_add_expr_node = std::static_pointer_cast<add_expr_node>(node);

	// rewrite a + -b to a - b
	if (old_add_expr_node->rhs->is_type(node_type::negate_node))
	{
		auto old_negate_node = std::static_pointer_cast<negate_node>(old_add_expr_node->rhs);
			
		auto new_node = std::make_shared<sub_expr_node>();
		new_node->lhs = old_add_expr_node->lhs;
		new_node->rhs = old_negate_node->value;

		node = new_node;
	}
	// rewrite -a + b to b - a
	else if (old_add_expr_node->lhs->is_type(node_type::negate_node))
	{
		auto old_negate_node = std::static_pointer_cast<negate_node>(old_add_expr_node->lhs);
			
		auto new_node = std::make_shared<sub_expr_node>();
		new_node->lhs = old_add_expr_node->rhs;
		new_node->rhs = old_negate_node->value;

		node = new_node;
	}
	// rewrite a + b to a - b when b is all negative
	else if (old_add_expr_node->rhs->is_type(node_type::vector_node))
	{
		auto old_vector_node = std::static_pointer_cast<vector_node>(old_add_expr_node->rhs);

		// If all the values are negative
		bool is_negative = true;
		for (auto value : old_vector_node->values) 
			is_negative &= value->is_negative();

		// Absolute values on vector
		if (is_negative)
			for (auto value : old_vector_node->values)
				value->absolute();

		auto new_node = std::make_shared<sub_expr_node>();
		new_node->lhs = old_add_expr_node->lhs;
		new_node->rhs = old_add_expr_node->rhs;

		node = new_node;
	}
}

void rewrite_mask_node(std::shared_ptr<ast_node>& node)
{
	auto old_mask_node = std::static_pointer_cast<mask_node>(node);

	// mask_node with a size of 1? rewrite to scalar_node
	if (old_mask_node->indices.size() == 1)
	{
		auto new_node = std::make_shared<scalar_node>();
		new_node->value = old_mask_node->value;
		new_node->index = old_mask_node->indices.front();

		node = new_node;
	}
}

void rewrite_swizzle_node(std::shared_ptr<ast_node>& node)
{
	auto old_swizzle_node = std::static_pointer_cast<swizzle_node>(node);

	// swizzle_node where all the elements are the same? rewrite to scalar_node
	bool same = true;
	auto first_index = old_swizzle_node->indices.front();

	for (auto index : old_swizzle_node->indices)
		same &= (index == first_index);

	if (same)
	{
		auto new_node = std::make_shared<scalar_node>();
		new_node->value = old_swizzle_node->value;
		new_node->index = first_index;

		node = new_node;
	}
}

void rewrite_function_call_expr_node(std::shared_ptr<ast_node>& node)
{
	auto old_fc_node = std::static_pointer_cast<function_call_expr_node>(node);

	// rewrite rsqrt(dot(x, x)) to length(x)
	if (old_fc_node->name == "rsqrt")
	{
		auto argument_node = old_fc_node->arguments.front();
		if (argument_node->is_type(node_type::function_call_expr_node))
		{
			auto nested_fc_node = std::static_pointer_cast<function_call_expr_node>(argument_node);

			if (nested_fc_node->name == "dot")
			{
				auto argument1 = nested_fc_node->arguments[0];
				auto argument2 = nested_fc_node->arguments[1];

				if (index_equal(argument1.get(), argument2.get()))
					node = std::make_shared<function_call_expr_node>("length", argument1);
			}
		}
	}
}
	
void rewrite_node(std::shared_ptr<ast_node>& node)
{
	if (node->is_type(node_type::instruction_call_expr_node))
		rewrite_instruction_call_expr_node(node);

	if (node->is_type(node_type::add_expr_node))
		rewrite_add_expr_node(node);

	if (node->is_type(node_type::mask_node))
		rewrite_mask_node(node);

	if (node->is_type(node_type::swizzle_node))
		rewrite_swizzle_node(node);

	if (node->is_type(node_type::function_call_expr_node))
		rewrite_function_call_expr_node(node);
}

void rewrite_visitor::visit(binary_expr_node* node)
{
	rewrite_node(node->lhs);
	rewrite_node(node->rhs);

	node->lhs->accept(*this);
	node->rhs->accept(*this);
}

void rewrite_visitor::visit(function_call_expr_node* node)
{
	for (auto& arg : node->arguments)
	{
		rewrite_node(arg);
		arg->accept(*this);
	}
}

}