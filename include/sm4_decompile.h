#ifndef SM4_DECOMPILE_H_
#define SM4_DECOMPILE_H_
   
#include <stdint.h>
#include <memory>
#include <vector>

struct sm4_program;
struct sm4_insn;
struct sm4_op;

namespace sm4 {

typedef sm4_program program;
typedef sm4_insn instruction;
typedef sm4_op operand;

#define AST_NODE_CLASSES \
	/* General */ \
	AST_NODE_CLASS(ast_node) \
	AST_NODE_CLASS(super_node) \
	/* Constants, variables */ \
	AST_NODE_CLASS(constant_node) \
	AST_NODE_CLASS(global_variable_node) \
	AST_NODE_CLASS(vector_node) \
	AST_NODE_CLASS(register_node) \
	AST_NODE_CLASS(constant_buffer_node) \
	AST_NODE_CLASS(immediate_constant_buffer_node) \
	AST_NODE_CLASS(input_node) \
	AST_NODE_CLASS(output_node) \
	/* Function calls */ \
	AST_NODE_CLASS(call_expr_node) \
	AST_NODE_CLASS(function_call_expr_node) \
	AST_NODE_CLASS(instruction_call_expr_node) \
	/* Indexing */ \
	AST_NODE_CLASS(mask_node) \
	AST_NODE_CLASS(swizzle_node) \
	AST_NODE_CLASS(scalar_node) \
	AST_NODE_CLASS(dynamic_index_node) \
	/* Comparison */ \
	AST_NODE_CLASS(comparison_node) \
	/* Instruction nodes */ \
	AST_NODE_CLASS(unary_node) \
	AST_NODE_CLASS(if_node) \
	AST_NODE_CLASS(else_node) \
	AST_NODE_CLASS(ret_node) \
	AST_NODE_CLASS(negate_node) \
	/* Rewritten expressions */ \
	AST_NODE_CLASS(binary_expr_node) \
	AST_NODE_CLASS(add_expr_node) \
	AST_NODE_CLASS(sub_expr_node) \
	AST_NODE_CLASS(mul_expr_node) \
	AST_NODE_CLASS(div_expr_node) \
	AST_NODE_CLASS(assign_expr_node)

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
	std::vector<std::shared_ptr<ast_node>> children;
};

// types
class constant_node : public ast_node
{
public:
	virtual ~constant_node() {};
	DECLARE_AST_NODE(constant_node, ast_node)

	enum class type
	{
		f32,
		i32,
		u32,

		f64,
		i64,
		u64
	} current_type;

	union
	{
		float f32;
		int32_t i32;
		uint32_t u32;

		double f64;
		int64_t i64;
		uint64_t u64;
	};

	void absolute()
	{
		switch (this->current_type)
		{
		case type::f32:
			this->f32 = abs(this->f32);
			break;
		case type::i32:
			this->i32 = abs(this->i32);
			break;
		case type::f64:
			this->f64 = abs(this->f64);
			break;
		case type::i64:
			this->i64 = abs(this->i64);
			break;
		default:
			break;
		}
	}

	bool is_negative()
	{
		switch (this->current_type)
		{
		case type::f32:
			return this->f32 <= 0;
		case type::i32:
			return this->i32 <= 0;
		case type::f64:
			return this->f64 <= 0;
		case type::i64:
			return this->i64 <= 0;
		default:
			return true;
		}
	}
};

class global_variable_node : public ast_node
{
public:
	global_variable_node(int64_t index) :
		index(index)
	{
	}

	virtual ~global_variable_node() {};
	DECLARE_AST_NODE(global_variable_node, ast_node)

	int64_t index;
	bool operator==(global_variable_node const& rhs) const
	{
		return rhs.is_type(this->get_type()) && this->index == rhs.index;
	}
};

#define DEFINE_DERIVED_GLOBAL_VARIABLE_NODE(node_name) \
	class node_name : public global_variable_node \
	{ \
		public: \
			node_name(int64_t index) : \
				global_variable_node(index) \
			{ \
			} \
		DECLARE_AST_NODE(node_name, global_variable_node) \
	}; \

DEFINE_DERIVED_GLOBAL_VARIABLE_NODE(register_node)
DEFINE_DERIVED_GLOBAL_VARIABLE_NODE(constant_buffer_node)
DEFINE_DERIVED_GLOBAL_VARIABLE_NODE(immediate_constant_buffer_node)
DEFINE_DERIVED_GLOBAL_VARIABLE_NODE(input_node)
DEFINE_DERIVED_GLOBAL_VARIABLE_NODE(output_node)

class vector_node : public ast_node
{
public:
	virtual ~vector_node() {};
	DECLARE_AST_NODE(vector_node, ast_node)

	std::vector<std::shared_ptr<constant_node>> values; 
};

// indexing
class mask_node : public ast_node
{
public:
	virtual ~mask_node() {};
	DECLARE_AST_NODE(mask_node, ast_node)

	std::shared_ptr<global_variable_node> value;
	std::vector<uint8_t> indices;

	bool operator==(mask_node const& rhs) const
	{
		return *this->value == *rhs.value && this->indices == rhs.indices;
	}
};

class swizzle_node : public ast_node
{
public:
	virtual ~swizzle_node() {};
	DECLARE_AST_NODE(swizzle_node, ast_node)

	std::shared_ptr<global_variable_node> value;
	std::vector<uint8_t> indices;

	bool operator==(swizzle_node const& rhs) const
	{
		return *this->value == *rhs.value && this->indices == rhs.indices;
	}
};

class scalar_node : public ast_node
{
public:
	virtual ~scalar_node() {};
	DECLARE_AST_NODE(scalar_node, ast_node)

	std::shared_ptr<global_variable_node> value;
	uint8_t index;

	bool operator==(scalar_node const& rhs) const
	{
		return *this->value == *rhs.value && this->index == rhs.index;
	}
};

// todo: clean up, I feel like I'm writing PHP here
// checks whether lhs and rhs are equal, assuming they're one of the three index operators
bool index_equal(ast_node const* lhs, ast_node const* rhs);

class dynamic_index_node : public ast_node
{
public:
	virtual ~dynamic_index_node() {};
	DECLARE_AST_NODE(dynamic_index_node, ast_node)

	std::shared_ptr<global_variable_node> value;
	std::shared_ptr<ast_node> index;
};

// function call
class call_expr_node : public ast_node
{
public:
	call_expr_node() {}
	DECLARE_AST_NODE(call_expr_node, ast_node)

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
		push_back(arg);
		push_back(args...);
	}

	virtual ~call_expr_node() {};

	std::vector<std::shared_ptr<ast_node>> arguments;
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

	uint8_t opcode;
};

// comparison
class comparison_node : public super_node
{
public:
	virtual ~comparison_node() {};
	DECLARE_AST_NODE(comparison_node, ast_node)

	std::shared_ptr<ast_node> value;
	bool not_zero;
};

DEFINE_DERIVED_AST_NODE(if_node, comparison_node)

// nullary
DEFINE_DERIVED_AST_NODE(else_node, super_node)
DEFINE_DERIVED_AST_NODE(ret_node, ast_node)

// unary
class unary_node : public ast_node
{
public:
	virtual ~unary_node() {};
	DECLARE_AST_NODE(unary_node, ast_node)

	std::shared_ptr<ast_node> value;
};

DEFINE_DERIVED_AST_NODE(negate_node, unary_node)

// fun stuff! actual operators!
class binary_expr_node : public ast_node
{
public:
	virtual ~binary_expr_node() {}
	DECLARE_AST_NODE(binary_expr_node, ast_node)

	std::shared_ptr<ast_node> lhs;
	std::shared_ptr<ast_node> rhs;
};

DEFINE_DERIVED_AST_NODE(add_expr_node, binary_expr_node)
DEFINE_DERIVED_AST_NODE(sub_expr_node, binary_expr_node)
DEFINE_DERIVED_AST_NODE(mul_expr_node, binary_expr_node)
DEFINE_DERIVED_AST_NODE(div_expr_node, binary_expr_node)
DEFINE_DERIVED_AST_NODE(assign_expr_node, binary_expr_node)

std::shared_ptr<super_node> decompile(program const* p);

}

#endif