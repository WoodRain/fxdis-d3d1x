#include "sm4_ast_nodes.h"

namespace sm4 {

// Forgive me for I have sinned
#define AST_NODE_CLASS(klass) \
	void ast_visitor::visit(klass* node) \
	{ \
		this->visit((klass::base_class*)node); \
	} \

	AST_NODE_CLASSES

#undef AST_NODE_CLASS

bool super_node::operator==(ast_node const& rhs) 
{ 
	auto ret = base_class::operator==(rhs);
	auto typed_rhs = static_cast<this_class const&>(rhs);

	ret &= (*this->parent == *typed_rhs.parent);
	ret &= (this->children == typed_rhs.children);

	return ret;
}
	
bool type_node::operator==(ast_node const& rhs) 
{ 
	auto ret = base_class::operator==(rhs);
	auto typed_rhs = static_cast<this_class const&>(rhs);

	ret &= (this->name == typed_rhs.name);

	return ret;
}

bool vector_type_node::operator==(ast_node const& rhs) 
{ 
	auto ret = base_class::operator==(rhs);
	auto typed_rhs = static_cast<this_class const&>(rhs);

	ret &= (this->type == typed_rhs.type);
	ret &= (this->count == typed_rhs.count);

	return ret;
}

bool function_node::operator==(ast_node const& rhs) 
{ 
	auto ret = base_class::operator==(rhs);
	auto typed_rhs = static_cast<this_class const&>(rhs);

	ret &= (this->name == typed_rhs.name);
	ret &= (this->return_type && typed_rhs.return_type && 
			(*this->return_type == *typed_rhs.return_type));
	ret &= (this->arguments == typed_rhs.arguments);

	return ret;
}

bool structure_node::operator==(ast_node const& rhs) 
{ 
	auto ret = base_class::operator==(rhs);
	auto typed_rhs = static_cast<this_class const&>(rhs);

	ret &= (this->name == typed_rhs.name);

	return ret;
}

#define CONSTANT_TYPE(cpp_type, enum_type) \
	case value_type::enum_type: this->enum_type = abs(this->enum_type); break;

void constant_node::absolute()
{
	switch (this->current_type)
	{
		SIGNED_TYPES
	}
}

#undef CONSTANT_TYPE
#define CONSTANT_TYPE(cpp_type, enum_type) \
	case value_type::enum_type: return this->enum_type <= 0;

bool constant_node::is_negative()
{
	switch (this->current_type)
	{
		SIGNED_TYPES
	default:
		return true;
	}
}
#undef CONSTANT_TYPE

bool constant_node::is_64bit()
{
	switch (this->current_type)
	{
	case value_type::f32:
	case value_type::i32:
	case value_type::u32:
		return false;
	default:
		return true;
	}
}

bool constant_node::operator==(ast_node const& rhs) 
{ 
	auto ret = base_class::operator==(rhs);
	auto typed_rhs = static_cast<this_class const&>(rhs);

	ret &= (this->current_type == typed_rhs.current_type);

	if (this->is_64bit())
		ret &= (this->u64 == typed_rhs.u64);
	else
		ret &= (this->u32 == typed_rhs.u32);

	return ret;
}

bool global_variable_node::operator==(ast_node const& rhs) 
{ 
	auto ret = base_class::operator==(rhs);
	auto typed_rhs = static_cast<this_class const&>(rhs);

	ret &= (this->index == typed_rhs.index);

	return ret;
}

bool vector_node::operator==(ast_node const& rhs) 
{ 
	auto ret = base_class::operator==(rhs);
	auto typed_rhs = static_cast<this_class const&>(rhs);

	ret &= (this->values == typed_rhs.values);

	return ret;
}

bool variable_access_node::operator==(ast_node const& rhs) 
{ 
	auto ret = base_class::operator==(rhs);
	auto typed_rhs = static_cast<this_class const&>(rhs);

	ret &= (*this->value == *typed_rhs.value);

	return ret;
}

bool static_index_node::operator==(ast_node const& rhs) 
{ 
	auto ret = base_class::operator==(rhs);
	auto typed_rhs = static_cast<this_class const&>(rhs);

	ret &= (this->indices == typed_rhs.indices);

	return ret;
}

bool dynamic_index_node::operator==(ast_node const& rhs) 
{ 
	auto ret = base_class::operator==(rhs);
	auto typed_rhs = static_cast<this_class const&>(rhs);

	ret &= (*this->value == *typed_rhs.value);
	ret &= (this->displacement == typed_rhs.displacement);

	return ret;
}

bool comparison_node::operator==(ast_node const& rhs) 
{ 
	auto ret = base_class::operator==(rhs);
	auto typed_rhs = static_cast<this_class const&>(rhs);

	ret &= (*this->expression == *typed_rhs.expression);

	return ret;
}

bool call_expr_node::operator==(ast_node const& rhs) 
{ 
	auto ret = base_class::operator==(rhs);
	auto typed_rhs = static_cast<this_class const&>(rhs);

	ret &= (this->arguments == typed_rhs.arguments);

	return ret;
}

bool function_call_expr_node::operator==(ast_node const& rhs) 
{ 
	auto ret = base_class::operator==(rhs);
	auto typed_rhs = static_cast<this_class const&>(rhs);

	ret &= (this->name == typed_rhs.name);

	return ret;
}

bool instruction_call_expr_node::operator==(ast_node const& rhs) 
{ 
	auto ret = base_class::operator==(rhs);
	auto typed_rhs = static_cast<this_class const&>(rhs);

	ret &= (this->opcode == typed_rhs.opcode);

	return ret;
}

bool unary_expr_node::operator==(ast_node const& rhs) 
{ 
	auto ret = base_class::operator==(rhs);
	auto typed_rhs = static_cast<this_class const&>(rhs);

	ret &= (*this->value == *typed_rhs.value);

	return ret;
}

bool binary_expr_node::operator==(ast_node const& rhs) 
{ 
	auto ret = base_class::operator==(rhs);
	auto typed_rhs = static_cast<this_class const&>(rhs);

	ret &= (*this->lhs == *typed_rhs.lhs);
	ret &= (*this->rhs == *typed_rhs.rhs);

	return ret;
}

}