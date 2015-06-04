#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include "sm4.h"

namespace sm4 {

typedef sm4_program program;
typedef sm4_insn instruction;
typedef sm4_op operand;

#define SIGNED_TYPES \
	CONSTANT_TYPE(float, f32) \
	CONSTANT_TYPE(int32_t, i32) \
	CONSTANT_TYPE(double, f64) \
	CONSTANT_TYPE(int64_t, i64) \

#define CONSTANT_TYPES \
	SIGNED_TYPES \
	CONSTANT_TYPE(uint32_t, u32) \
	CONSTANT_TYPE(uint64_t, u64)

#define CONSTANT_TYPE(cpp_type, enum_type) \
	enum_type,

enum class value_type
{
	CONSTANT_TYPES
};

#undef CONSTANT_TYPE

#define AST_NODE_CLASSES \
	/* General */ \
	AST_NODE_CLASS(ast_node) \
	AST_NODE_CLASS(super_node) \
	AST_NODE_CLASS(type_node) \
	AST_NODE_CLASS(vector_type_node) \
	AST_NODE_CLASS(function_node) \
	AST_NODE_CLASS(structure_node) \
	AST_NODE_CLASS(expr_stmt_node) \
	/* Constants, variables */ \
	AST_NODE_CLASS(variable_node) \
	AST_NODE_CLASS(variable_decl_node) \
	AST_NODE_CLASS(constant_node) \
	AST_NODE_CLASS(global_variable_node) \
	AST_NODE_CLASS(vector_node) \
	AST_NODE_CLASS(register_node) \
	AST_NODE_CLASS(constant_buffer_node) \
	AST_NODE_CLASS(immediate_constant_buffer_node) \
	/* Indexing */ \
	AST_NODE_CLASS(variable_access_node) \
	AST_NODE_CLASS(static_index_node) \
	AST_NODE_CLASS(dynamic_index_node) \
	/* Comparison */ \
	AST_NODE_CLASS(comparison_node) \
	AST_NODE_CLASS(if_node) \
	AST_NODE_CLASS(else_node) \
	AST_NODE_CLASS(ret_node) \
	/* Expressions */ \
	AST_NODE_CLASS(expr_node) \
	/* Call expression nodes */ \
	AST_NODE_CLASS(call_expr_node) \
	AST_NODE_CLASS(function_call_expr_node) \
	AST_NODE_CLASS(instruction_call_expr_node) \
	/* Unary expression nodes */ \
	AST_NODE_CLASS(unary_expr_node) \
	AST_NODE_CLASS(negate_node) \
	/* Binary expression nodes */ \
	AST_NODE_CLASS(binary_expr_node) \
	AST_NODE_CLASS(dot_expr_node) \
	AST_NODE_CLASS(assign_expr_node) \
	AST_NODE_CLASS(add_expr_node) \
	AST_NODE_CLASS(sub_expr_node) \
	AST_NODE_CLASS(mul_expr_node) \
	AST_NODE_CLASS(div_expr_node) \
	AST_NODE_CLASS(eq_expr_node) \
	AST_NODE_CLASS(neq_expr_node) \

#define AST_NODE_CLASS(klass) class klass;
AST_NODE_CLASSES
#undef AST_NODE_CLASS

#define AST_NODE_CLASS(klass) klass,
enum class node_type
{
	AST_NODE_CLASSES
};
#undef AST_NODE_CLASS

class ast_visitor
{
public:
#define AST_NODE_CLASS(klass) \
	virtual void visit(klass* node);

	AST_NODE_CLASSES

#undef AST_NODE_CLASS
};

#define DECLARE_AST_NODE(node_name, base) \
	typedef node_name this_class; \
	typedef base base_class; \
	static const node_type static_type = node_type::node_name; \
	virtual char const* get_type_string() const { return #node_name; } \
	virtual node_type get_type() const { return node_type::node_name; } \
	virtual void accept(ast_visitor& visitor) { visitor.visit(this); }

#define DEFINE_DERIVED_AST_NODE(node_name, base) \
	class node_name : public base { public: DECLARE_AST_NODE(node_name, base) };
	
class ast_node
{
public:
	virtual ~ast_node() {};
	bool is_type(node_type type) const { return this->get_type() == type; }

	virtual bool operator==(ast_node const&) 
	{ 
		return true; 
	}

	bool operator!=(ast_node const& rhs)
	{
		return !(*this == rhs);
	}

	DECLARE_AST_NODE(ast_node, ast_node)
};

template <typename T>
std::shared_ptr<T> force_node_cast(std::shared_ptr<ast_node> node)
{
	return std::static_pointer_cast<T>(node);
}

template <typename T>
std::shared_ptr<T> node_cast(std::shared_ptr<ast_node> node)
{
	if (!node)
		return nullptr;

	if (node->is_type(T::static_type))
		return force_node_cast<T>(node);

	return nullptr;
}

class super_node : public ast_node
{
public:
	virtual ~super_node() {};
	DECLARE_AST_NODE(super_node, ast_node)

	std::shared_ptr<super_node> parent;
	std::unordered_map<std::string, std::shared_ptr<variable_node>> variables;
	std::vector<std::shared_ptr<ast_node>> children;

	std::shared_ptr<variable_node> get_variable(std::string const& name);
	void add_variable(std::shared_ptr<variable_node> variable, bool add_statement = false);

	virtual bool operator==(ast_node const& rhs);
};

class type_node : public ast_node
{
public:
	type_node() {};
	DECLARE_AST_NODE(type_node, ast_node)

	std::string name;

	virtual bool operator==(ast_node const& rhs);
};

class vector_type_node : public type_node
{
public:
	vector_type_node() {};
	DECLARE_AST_NODE(vector_type_node, type_node)

	value_type type;
	uint8_t count;

	virtual bool operator==(ast_node const& rhs);
};

class function_node : public super_node
{
public:
	virtual ~function_node() {};
	DECLARE_AST_NODE(function_node, super_node)

	std::string name;
	std::shared_ptr<type_node> return_type;
	std::vector<std::shared_ptr<ast_node>> arguments;

	void add_argument(std::shared_ptr<variable_node> node);

	virtual bool operator==(ast_node const& rhs);
};

class structure_node : public type_node
{
public:
	virtual ~structure_node() {};
	DECLARE_AST_NODE(structure_node, type_node)

	std::vector<std::shared_ptr<ast_node>> children;
	std::unordered_map<std::string, std::shared_ptr<variable_node>> variables;

	void add_variable(std::shared_ptr<variable_node> variable);
	virtual bool operator==(ast_node const& rhs);
};

class expr_stmt_node : public ast_node
{
public:
	expr_stmt_node() {};
	expr_stmt_node(std::shared_ptr<ast_node> value) :
		value(value)
	{
	}

	DECLARE_AST_NODE(expr_stmt_node, ast_node)

	std::shared_ptr<ast_node> value;
};

// types
#define CONSTANT_TYPE(cpp_type, enum_type) \
	#enum_type,

static char const* const value_type_strings[] = {CONSTANT_TYPES};

#undef CONSTANT_TYPE

class variable_node : public ast_node
{
public:
	variable_node() {};
	variable_node(std::shared_ptr<type_node> type, std::string name) :
		type(type), name(name)
	{
	}
	DECLARE_AST_NODE(variable_node, ast_node)

	std::shared_ptr<type_node> type;
	std::string name;
	uint8_t semantic_index = 0;

	virtual bool operator==(ast_node const& rhs);
};

class variable_decl_node : public ast_node
{
public:
	variable_decl_node() {}
	variable_decl_node(std::shared_ptr<variable_node> variable) :
		variable(variable)
	{
	}
	DECLARE_AST_NODE(variable_decl_node, ast_node)

	std::shared_ptr<variable_node> variable;

	virtual bool operator==(ast_node const& rhs);
};

class constant_node : public ast_node
{
public:
	constant_node() {};

	virtual ~constant_node() {};
	DECLARE_AST_NODE(constant_node, ast_node)

#undef CONSTANT_TYPE
#define CONSTANT_TYPE(cpp_type, enum_type) \
	constant_node(cpp_type value) \
	{ \
		this->enum_type = value; \
		this->current_type = value_type::enum_type; \
	}

	CONSTANT_TYPES

#undef CONSTANT_TYPE

	value_type current_type;

#define CONSTANT_TYPE(cpp_type, enum_type) \
	cpp_type enum_type;

	union
	{
		CONSTANT_TYPES
	};

#undef CONSTANT_TYPE

	void absolute();
	bool is_negative();
	bool is_64bit();

	virtual bool operator==(ast_node const& rhs);
};

class global_variable_node : public ast_node
{
public:
	global_variable_node() {}
	global_variable_node(int64_t index) :
		index(index)
	{
	}

	virtual ~global_variable_node() {};
	DECLARE_AST_NODE(global_variable_node, ast_node)

	int64_t index = -1;

	virtual bool operator==(ast_node const& rhs);
};

#define DEFINE_DERIVED_GLOBAL_VARIABLE_NODE(node_name) \
	class node_name : public global_variable_node \
	{ \
		public: \
			node_name() {} \
			node_name(int64_t index) : \
				global_variable_node(index) \
			{ \
			} \
		DECLARE_AST_NODE(node_name, global_variable_node) \
	}; \

DEFINE_DERIVED_GLOBAL_VARIABLE_NODE(register_node)
DEFINE_DERIVED_GLOBAL_VARIABLE_NODE(constant_buffer_node)
DEFINE_DERIVED_GLOBAL_VARIABLE_NODE(immediate_constant_buffer_node)

class vector_node : public ast_node
{
public:
	template <typename... Values>
	vector_node(Values&&... args)
	{
		this->push_back(args...);
	}

	virtual ~vector_node() {};
	DECLARE_AST_NODE(vector_node, ast_node)

	std::vector<std::shared_ptr<constant_node>> values; 

	virtual bool operator==(ast_node const& rhs);

protected:
	void push_back()
	{
	}

	template <typename Value>
	void push_back_value(Value value)
	{
		this->values.push_back(std::make_shared<constant_node>(value));
	}

	template <typename Value>
	void push_back(Value value)
	{
		push_back_value(value);
	}

	template <typename Value, typename... Values>
	void push_back(Value&& arg, Values&&... args)
	{
		push_back_value(arg);
		push_back(args...);
	}
};

// indexing
class variable_access_node : public ast_node
{
public:
	virtual ~variable_access_node() {};
	DECLARE_AST_NODE(variable_access_node, ast_node)

	std::shared_ptr<ast_node> value;

	virtual bool operator==(ast_node const& rhs);
};

class static_index_node : public variable_access_node
{
public:
	virtual ~static_index_node() {};
	DECLARE_AST_NODE(static_index_node, variable_access_node)

	std::vector<uint8_t> indices;

	virtual bool operator==(ast_node const& rhs);
};

class dynamic_index_node : public variable_access_node
{
public:
	virtual ~dynamic_index_node() {};
	DECLARE_AST_NODE(dynamic_index_node, variable_access_node)

	std::shared_ptr<ast_node> index;
	int64_t displacement;

	virtual bool operator==(ast_node const& rhs);
};

// comparison
class comparison_node : public super_node
{
public:
	virtual ~comparison_node() {};
	DECLARE_AST_NODE(comparison_node, super_node)

	std::shared_ptr<ast_node> expression;

	virtual bool operator==(ast_node const& rhs);
};

DEFINE_DERIVED_AST_NODE(if_node, comparison_node)
DEFINE_DERIVED_AST_NODE(else_node, super_node)
DEFINE_DERIVED_AST_NODE(ret_node, ast_node)

// expressions
class expr_node : public ast_node
{
public:
	expr_node() {}
	DECLARE_AST_NODE(expr_node, ast_node)

	virtual ~expr_node() {};
};

// call expressions
class call_expr_node : public expr_node
{
public:
	call_expr_node() {}
	DECLARE_AST_NODE(call_expr_node, expr_node)

	virtual ~call_expr_node() {};

	std::vector<std::shared_ptr<ast_node>> arguments;

	virtual bool operator==(ast_node const& rhs);

protected:
	void push_back()
	{
	}

	void push_back(std::shared_ptr<ast_node> node)
	{
		this->arguments.push_back(node);
	}

	template <typename Value, typename... Values>
	void push_back(Value&& arg, Values&&... args)
	{
		this->arguments.push_back(arg);
		push_back(args...);
	}
};

class function_call_expr_node : public call_expr_node
{
public:
	function_call_expr_node() {}
	DECLARE_AST_NODE(function_call_expr_node, call_expr_node)

	template <typename... Values>
	function_call_expr_node(std::string const& name, Values&&... args) : 
		name(name)
	{
		this->push_back(args...);
	}

	virtual bool operator==(ast_node const& rhs);

	std::string name;
};

class instruction_call_expr_node : public call_expr_node
{
public:
	instruction_call_expr_node() {}
	DECLARE_AST_NODE(instruction_call_expr_node, call_expr_node)

	template <typename... Values>
	instruction_call_expr_node(uint8_t opcode, Values&&... args) : 
		opcode(opcode)
	{
		this->push_back(args...);
	}

	virtual bool operator==(ast_node const& rhs);

	uint8_t opcode;
};

// unary expressions
class unary_expr_node : public expr_node
{
public:
	virtual ~unary_expr_node() {};
	DECLARE_AST_NODE(unary_expr_node, expr_node)

	std::shared_ptr<ast_node> value;

	virtual bool operator==(ast_node const& rhs);
};

DEFINE_DERIVED_AST_NODE(negate_node, unary_expr_node)

class binary_expr_node : public expr_node
{
public:
	binary_expr_node() {}
	binary_expr_node(std::shared_ptr<ast_node> lhs, std::shared_ptr<ast_node> rhs) :
		lhs(lhs), rhs(rhs)
	{
	}

	virtual ~binary_expr_node() {}
	DECLARE_AST_NODE(binary_expr_node, expr_node)

	std::shared_ptr<ast_node> lhs;
	std::shared_ptr<ast_node> rhs;

	virtual bool operator==(ast_node const& rhs);
};

#define DEFINE_DERIVED_BINARY_EXPR_NODE(node_name) \
	class node_name : public binary_expr_node \
	{ \
	public: \
		node_name() {} \
		node_name(std::shared_ptr<ast_node> lhs, std::shared_ptr<ast_node> rhs) : \
			binary_expr_node(lhs, rhs) \
		{ \
		} \
		DECLARE_AST_NODE(node_name, binary_expr_node) \
	};

DEFINE_DERIVED_BINARY_EXPR_NODE(dot_expr_node)
DEFINE_DERIVED_BINARY_EXPR_NODE(assign_expr_node)
DEFINE_DERIVED_BINARY_EXPR_NODE(add_expr_node)
DEFINE_DERIVED_BINARY_EXPR_NODE(sub_expr_node)
DEFINE_DERIVED_BINARY_EXPR_NODE(mul_expr_node)
DEFINE_DERIVED_BINARY_EXPR_NODE(div_expr_node)
DEFINE_DERIVED_BINARY_EXPR_NODE(eq_expr_node)
DEFINE_DERIVED_BINARY_EXPR_NODE(neq_expr_node)

}