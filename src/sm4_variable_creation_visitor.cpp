#include "sm4_variable_creation_visitor.h"
#include "sm4_text_visitor.h"
#include "sm4_decompile.h"
#include <string>

namespace sm4 {

variable_creation_visitor::variable_creation_visitor(sm4::decompiler* instance) :
	decompiler_(instance)
{
}

void variable_creation_visitor::visit(super_node* node)
{
	if (visited_.find(node) != visited_.end())
		return;

	class find_visitor : public recursive_visitor
	{
	public:
		virtual void visit(ast_node* node)
		{		
			text_visitor text(std::cout);
			/*
			std::cout << "COMPARE ";
			node->accept(text);
			std::cout << " TO ";
			to_find->accept(text);
			std::cout << std::endl;
			*/
			if (*node == *to_find)
				found = true;
		}

		bool operator()(std::shared_ptr<ast_node> to_search, std::shared_ptr<ast_node> to_find)
		{
			this->found = false;
			this->to_find = to_find.get();
			to_search->accept(*this);
			return this->found;
		}

	private:
		ast_node* to_find;		
		bool found = false;
	} find;
	
	class find_type_visitor : public recursive_visitor
	{
	public:
		virtual void visit(ast_node* node)
		{
			if (node->is_type(this->type))
				this->found = true;
		}

		bool operator()(std::shared_ptr<ast_node> node, node_type type)
		{
			this->type = type;
			this->found = false;

			node->accept(*this);

			return this->found;
		}

		node_type type;
		bool found = false;
	} find_type;
	
	class find_count_visitor : public recursive_visitor
	{
	public:
		virtual void visit(ast_node* node)
		{
			if (node->is_type(node_type::static_index_node))
			{
				auto static_index = static_cast<static_index_node*>(node);
				
				if (static_index->indices.size() == 1)
				{
					this->size = 1;
					return;
				}

				// We don't want to generate any new variables for non-trivial cases
				// (i.e. register3.yzw is not trivial)
				for (size_t i = 0; i < static_index->indices.size(); ++i)
				{
					if (static_index->indices[i] == i)
					{
						++this->size;
					}
					else
					{
						this->size = 0;
						return;
					}
				}
			}
		}

		size_t operator()(std::shared_ptr<ast_node> node)
		{
			this->size = 0;
			node->accept(*this);
			return this->size;
		}

		size_t size = 0;
	} find_count;

	struct variable_node_splice
	{
		size_t index;
		size_t variable_index;
		size_t variable_size;
	};

	std::vector<variable_node_splice> splices;

	static size_t variable_index = 0;

	for (size_t i = 0; i < node->children.size(); ++i)
	{
		auto child = node->children[i];

		auto expr_stmt = node_cast<expr_stmt_node>(child);
		if (expr_stmt)
		{
			auto assign_expr = node_cast<assign_expr_node>(expr_stmt->value);
			if (assign_expr)
			{
				if (!find_type(assign_expr->lhs, node_type::variable_node))
					continue;

				auto found = find(assign_expr->rhs, assign_expr->lhs);
				auto count = find_count(assign_expr->lhs);

				if (!found && count > 0)
					splices.push_back({i, ++variable_index, count});
			}
		}

		child->accept(*this);
	}

	for (auto it = splices.rbegin(); it < splices.rend(); ++it)
	{
		auto splice = *it;
		auto variable = std::make_shared<variable_node>();
		auto typeName = "float" + (splice.variable_size > 1 ? std::to_string(splice.variable_size) : "");
		variable->type = this->decompiler_->get_type(typeName);
		variable->name = "v" + std::to_string(splice.variable_index);

		node->add_variable(variable);
		node->children.insert(node->children.begin() + (splice.index), 
			std::make_shared<expr_stmt_node>(
				std::make_shared<variable_decl_node>(variable)));
	}

	this->visited_.insert(node);
}

}