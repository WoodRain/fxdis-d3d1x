#include "sm4_rewrite_visitor.h"
#include "sm4_text_visitor.h"
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
		auto new_node = std::make_shared<function_call_expr_node>("rsqrt", args[0]);
		node = new_node;
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
		auto new_node = std::make_shared<function_call_expr_node>("dot", args[0], args[1]);
		node = new_node;
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
	else if (auto lhs_negate_node = node_cast<negate_node>(old_add_expr_node->lhs))
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
		{
			for (auto value : rhs_vector_node->values)
				value->absolute();

			auto new_node = std::make_shared<sub_expr_node>();
			new_node->lhs = old_add_expr_node->lhs;
			new_node->rhs = old_add_expr_node->rhs;

			node = new_node;
		}
	}
	// rewrite a + a to 2 * a
	else if (old_add_expr_node->lhs == old_add_expr_node->rhs)
	{
		auto new_mul_node = std::make_shared<mul_expr_node>();

		new_mul_node->lhs = std::make_shared<vector_node>(2.0f);
		new_mul_node->rhs = old_add_expr_node->lhs;

		node = new_mul_node;
	}
}

void rewrite_static_index_node(std::shared_ptr<ast_node>& node)
{
	auto old_static_index_node = force_node_cast<static_index_node>(node);
	auto& indices = old_static_index_node->indices;

	// static_index_node where all the elements are the same? rewrite to single-element static_index_node
	auto first_index = indices.front();
	bool same = true;

	for (auto index : indices) 
		same &= (index == first_index);

	if (same)
		indices.assign(1, first_index);

	// if the first and last index are the same, drop the last one
	// xyzx -> xyz
	if (indices.size() > 1 && (indices.front() == indices.back()))
		indices.pop_back();
}

void rewrite_mul_expr_node(std::shared_ptr<ast_node>& node)
{
	auto old_mul_node = force_node_cast<mul_expr_node>(node);

	text_visitor visitor(std::cout);

	// rewrite 1/length(a) * a to normalize
	auto rewrite_normalize = 
	[&](std::shared_ptr<ast_node> lhs, std::shared_ptr<ast_node> rhs)
	{
		auto div_lhs = node_cast<div_expr_node>(lhs);
		if (!div_lhs)
			return;

		auto dividend = node_cast<vector_node>(div_lhs->lhs);
		if (!dividend || dividend->values[0]->f32 != 1.0f)
			return;

		auto divisor = node_cast<function_call_expr_node>(div_lhs->rhs);
		if (!divisor || divisor->name != "length")
			return;

		auto argument = divisor->arguments[0];
		if (*argument != *rhs)
			return;

		auto new_node = 
			std::make_shared<function_call_expr_node>("normalize", rhs);

		node = new_node;
	};

	rewrite_normalize(old_mul_node->lhs, old_mul_node->rhs);
	rewrite_normalize(old_mul_node->rhs, old_mul_node->lhs);
}

void rewrite_function_call_expr_node(std::shared_ptr<ast_node>& node)
{
	auto old_fc_node = force_node_cast<function_call_expr_node>(node);

	// rewrite rsqrt(dot(x, x)) to 1/length(x)
	if (old_fc_node->name == "rsqrt")
	{
		auto argument_node = old_fc_node->arguments.front();
		if (auto nested_fc_node = node_cast<function_call_expr_node>(argument_node))
		{
			if (nested_fc_node->name == "dot")
			{
				auto argument1 = nested_fc_node->arguments[0];
				auto argument2 = nested_fc_node->arguments[1];

				if (*argument1 == *argument2)
				{
					auto new_length_node = std::make_shared<function_call_expr_node>("length", argument1);

					auto new_div_node = std::make_shared<div_expr_node>();
					new_div_node->lhs = std::make_shared<vector_node>(1.0f);
					new_div_node->rhs = new_length_node;

					node = new_div_node;
				}
			}
		}
	}
}
	
void rewrite_node(std::shared_ptr<ast_node>& node)
{
	if (node->is_type(node_type::static_index_node))
		rewrite_static_index_node(node);

	if (node->is_type(node_type::instruction_call_expr_node))
		rewrite_instruction_call_expr_node(node);

	if (node->is_type(node_type::add_expr_node))
		rewrite_add_expr_node(node);	

	if (node->is_type(node_type::function_call_expr_node))
		rewrite_function_call_expr_node(node);

	if (node->is_type(node_type::mul_expr_node))
		rewrite_mul_expr_node(node);
}

void rewrite_visitor::visit(variable_node* node)
{
	if (node->semantic_index != SM4_SV_UNDEFINED)
		node->name = sm4_sv_names[node->semantic_index];

	node->type->accept(*this);
}

void rewrite_visitor::visit(unary_expr_node* node)
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