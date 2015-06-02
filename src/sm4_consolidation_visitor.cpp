#include "sm4_consolidation_visitor.h"
#include "sm4.h"
#include <iostream>
#include <string>
#include <unordered_set>

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
			// if the last node's stored in is equal to this node,
			// replace the node with the last node's rhs
			auto rhs_stored_in = node_cast<static_index_node>(last_node->rhs->stored_in);
			if (rhs_stored_in && (*rhs_stored_in == *node))
			{
				node = last_node->rhs;
				this->rewrote = true;
			}
		}

		virtual void visit(unary_expr_node* node)
		{
			node->value->accept(*this);
			this->rewrite(node->value);
		}

		virtual void visit(call_expr_node* node)
		{
			std::unordered_set<std::shared_ptr<ast_node>*> arguments;

			// Insert all arguments into the set. If it shows up more than once,
			// we have a dupe, and we shouldn't collapse this.
			for (auto& argument : node->arguments)
				if (!arguments.insert(&argument).second)
					return;

			for (auto argument : arguments)
				this->rewrite(*argument);
		}
		
		virtual void visit(binary_expr_node* node)
		{
			node->lhs->accept(*this);
			node->rhs->accept(*this);

			this->rewrite(node->lhs);
			this->rewrite(node->rhs);
		}

		bool rewrote = false;
		std::shared_ptr<assign_stmt_node> last_node;
	} roll_visitor;

	std::vector<size_t> to_remove;

	for (size_t i = 0; i < node->children.size(); ++i)
	{
		auto current_node = node->children[i];
		current_node->accept(*this);

		auto last_assign = node_cast<assign_stmt_node>(last_node);
		auto current_assign = node_cast<assign_stmt_node>(current_node);

		if (last_assign && current_assign)
		{
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