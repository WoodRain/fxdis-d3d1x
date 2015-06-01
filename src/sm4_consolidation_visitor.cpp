#include "sm4_consolidation_visitor.h"
#include "sm4.h"
#include <iostream>
#include <string>

namespace sm4 {

void consolidation_visitor::visit(super_node* node)
{
	if (node->children.empty())
		return;

	std::shared_ptr<ast_node> last_node = node->children.front();

	class roll_visitor : public recursive_visitor
	{
	public:
		void rewrite(std::shared_ptr<ast_node>& node)
		{
			auto lhs = last_node->lhs;

#define HANDLE_TYPE(type) \
	if (node->is_type(node_type::type) && lhs->is_type(node_type::type)) \
	{ \
		auto casted_node = std::static_pointer_cast<type>(node); \
		auto casted_lhs = std::static_pointer_cast<type>(lhs); \
		if (*casted_node == *casted_lhs) \
		{ \
			node = last_node->rhs; \
			this->rewrote = true; \
		} \
	}
			HANDLE_TYPE(swizzle_node)
			HANDLE_TYPE(scalar_node)
			HANDLE_TYPE(mask_node)
		}

		virtual void visit(unary_node* node)
		{
			this->rewrite(node->value);
		}

		virtual void visit(binary_instruction_node* node)
		{
			this->rewrite(node->input);
		}

		virtual void visit(ternary_instruction_node* node)
		{
			this->rewrite(node->lhs);
			this->rewrite(node->rhs);
		}

		virtual void visit(quaternary_instruction_node* node)
		{
			this->rewrite(node->lhs);
			this->rewrite(node->rhs1);
			this->rewrite(node->rhs2);
		}

		virtual void visit(binary_op* node)
		{
			this->rewrite(node->lhs);
			this->rewrite(node->rhs);
		}

		virtual void visit(function_call_node* node)
		{
			for (auto& argument : node->arguments)
				this->rewrite(argument);
		}

		bool rewrote = false;
		std::shared_ptr<assign_node> last_node;
	} roll_visitor;

	std::vector<size_t> to_remove;

	for (size_t i = 0; i < node->children.size(); ++i)
	{
		auto current_node = node->children[i];
		current_node->accept(*this);

		if (last_node->is_type(node_type::assign_node) && current_node->is_type(node_type::assign_node))
		{
			auto last_assign = std::static_pointer_cast<assign_node>(last_node);
			auto current_assign = std::static_pointer_cast<assign_node>(current_node);

			roll_visitor.rewrote = false;
			roll_visitor.last_node = last_assign;
			current_assign->rhs->accept(roll_visitor);

			if (roll_visitor.rewrote)
				to_remove.push_back(i);
		}
		last_node = current_node;
	}

	for (auto it = to_remove.rbegin(); it != to_remove.rend(); ++it)
		node->children.erase(node->children.begin() + (*it-1));
}

}