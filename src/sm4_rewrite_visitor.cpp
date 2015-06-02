#include "sm4_rewrite_visitor.h"
#include "sm4.h"
#include <iostream>
#include <string>

namespace sm4 {

void rewrite_instruction_call_expr_node(std::shared_ptr<ast_node>& node)
{
	auto inst_node = force_node_cast<instruction_call_expr_node>(node);

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
	auto old_add_expr_node = force_node_cast<add_expr_node>(node);

	// rewrite a + -b to a - b
	if (auto rhs_negate_node = node_cast<negate_node>(old_add_expr_node->rhs))
	{		
		auto new_node = std::make_shared<sub_expr_node>();
		new_node->lhs = old_add_expr_node->lhs;
		new_node->rhs = rhs_negate_node->value;

		node = new_node;
	}
	// rewrite -a + b to b - a
	else if (auto lhs_negate_node = node_cast<negate_node>(old_add_expr_node->rhs))
	{	
		auto new_node = std::make_shared<sub_expr_node>();
		new_node->lhs = old_add_expr_node->rhs;
		new_node->rhs = lhs_negate_node->value;

		node = new_node;
	}
	// rewrite a + b to a - b when b is all negative
	else if (auto rhs_vector_node = node_cast<vector_node>(old_add_expr_node->rhs))
	{
		// If all the values are negative
		bool is_negative = true;
		for (auto value : rhs_vector_node->values) 
			is_negative &= value->is_negative();

		// Absolute values on vector
		if (is_negative)
			for (auto value : rhs_vector_node->values)
				value->absolute();

		auto new_node = std::make_shared<sub_expr_node>();
		new_node->lhs = old_add_expr_node->lhs;
		new_node->rhs = old_add_expr_node->rhs;

		node = new_node;
	}
	// rewrite a + a to 2 * a
	else if (old_add_expr_node->lhs == old_add_expr_node->rhs)
	{
		auto new_mul_node = std::make_shared<mul_expr_node>();

		auto new_vector_node = std::make_shared<vector_node>();
		
		auto new_constant_node = std::make_shared<constant_node>();
		new_constant_node->f32 = 2.0f;
		new_constant_node->current_type = constant_node::type::f32;

		new_vector_node->values.push_back(new_constant_node);

		new_mul_node->lhs = new_vector_node;
		new_mul_node->rhs = old_add_expr_node->lhs;

		node = new_mul_node;
	}
}

void rewrite_mask_node(std::shared_ptr<ast_node>& node)
{
	auto old_mask_node = force_node_cast<mask_node>(node);

	// rewrite all mask_nodes to single-element swizzle_nodes
	auto new_node = std::make_shared<swizzle_node>();
	new_node->value = old_mask_node->value;
	new_node->indices = old_mask_node->indices;

	node = new_node;
}

void rewrite_swizzle_node(std::shared_ptr<ast_node>& node)
{
	auto old_swizzle_node = force_node_cast<swizzle_node>(node);
	auto& indices = old_swizzle_node->indices;

	// swizzle_node where all the elements are the same? rewrite to single-element swizzle_node
	auto first_index = indices.front();
	bool same = true;

	for (auto index : indices) 
		same &= (index == first_index);

	if (same)
		indices.assign(1, first_index);
}

void rewrite_scalar_node(std::shared_ptr<ast_node>& node)
{
	// rewrite all scalar_nodes to single-element swizzle_nodes
	auto old_scalar_node = force_node_cast<scalar_node>(node);

	auto new_node = std::make_shared<swizzle_node>();
	new_node->value = old_scalar_node->value;
	new_node->indices.push_back(old_scalar_node->index);

	node = new_node;
}

void rewrite_function_call_expr_node(std::shared_ptr<ast_node>& node)
{
	auto old_fc_node = force_node_cast<function_call_expr_node>(node);

	// rewrite rsqrt(dot(x, x)) to length(x)
	if (old_fc_node->name == "rsqrt")
	{
		auto argument_node = old_fc_node->arguments.front();
		if (auto nested_fc_node = node_cast<function_call_expr_node>(argument_node))
		{
			if (nested_fc_node->name == "dot")
			{
				auto argument1 = node_cast<swizzle_node>(nested_fc_node->arguments[0]);
				auto argument2 = node_cast<swizzle_node>(nested_fc_node->arguments[1]);

				if (argument1 && argument2 && (*argument1 == *argument2))
					node = std::make_shared<function_call_expr_node>("length", argument1);
			}
		}
	}
}
	
void rewrite_node(std::shared_ptr<ast_node>& node)
{
	if (node->is_type(node_type::mask_node))
		rewrite_mask_node(node);

	if (node->is_type(node_type::scalar_node))
		rewrite_scalar_node(node);

	if (node->is_type(node_type::swizzle_node))
		rewrite_swizzle_node(node);

	if (node->is_type(node_type::instruction_call_expr_node))
		rewrite_instruction_call_expr_node(node);

	if (node->is_type(node_type::add_expr_node))
		rewrite_add_expr_node(node);

	if (node->is_type(node_type::function_call_expr_node))
		rewrite_function_call_expr_node(node);
}

void rewrite_visitor::visit(unary_node* node)
{
	rewrite_node(node->value);

	node->value->accept(*this);
}

void rewrite_visitor::visit(binary_expr_node* node)
{
	rewrite_node(node->lhs);
	rewrite_node(node->rhs);

	node->lhs->accept(*this);
	node->rhs->accept(*this);
}

void rewrite_visitor::visit(call_expr_node* node)
{
	for (auto& arg : node->arguments)
	{
		rewrite_node(arg);
		arg->accept(*this);
	}
}

}