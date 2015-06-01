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
	AST_NODE_CLASS(global_index_node) \
	AST_NODE_CLASS(vector_node) \
	AST_NODE_CLASS(register_node) \
	AST_NODE_CLASS(constant_buffer_node) \
	AST_NODE_CLASS(immediate_constant_buffer_node) \
	AST_NODE_CLASS(input_node) \
	AST_NODE_CLASS(output_node) \
	/* Indexing */ \
	AST_NODE_CLASS(mask_node) \
	AST_NODE_CLASS(swizzle_node) \
	AST_NODE_CLASS(scalar_node) \
	AST_NODE_CLASS(index_node) \
	/* Comparison */ \
	AST_NODE_CLASS(comparison_node) \
	/* Instruction nodes */ \
	AST_NODE_CLASS(unary_node) \
	AST_NODE_CLASS(binary_instruction_node) \
	AST_NODE_CLASS(ternary_instruction_node) \
	AST_NODE_CLASS(quaternary_instruction_node) \
	AST_NODE_CLASS(if_node) \
	AST_NODE_CLASS(else_node) \
	AST_NODE_CLASS(ret_node) \
	AST_NODE_CLASS(negate_node) \
	AST_NODE_CLASS(absolute_node) \
	AST_NODE_CLASS(saturate_node) \
	/* Rewritten expressions */ \
	AST_NODE_CLASS(binary_op) \
	AST_NODE_CLASS(add_node) \
	AST_NODE_CLASS(sub_node) \
	AST_NODE_CLASS(mul_node) \
	AST_NODE_CLASS(div_node) \
	AST_NODE_CLASS(assign_node)

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
	virtual char const* get_type_string() { return #node_name; } \
	virtual node_type get_type() { return node_type::node_name; } \
	virtual void accept(ast_visitor& visitor) { visitor.visit(this); }

#define DEFINE_DERIVED_AST_NODE(node_name, base) \
	class node_name : public base { public: DECLARE_AST_NODE(node_name, base) };
	
class ast_node
{
public:
	virtual ~ast_node() {};
	bool is_type(node_type type) { return this->get_type() == type; }

	DECLARE_AST_NODE(ast_node, ast_node)
};

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
};

class global_index_node : public ast_node
{
public:
	global_index_node(int64_t index) :
		index(index)
	{
	}

	virtual ~global_index_node() {};
	DECLARE_AST_NODE(global_index_node, ast_node)

	int64_t index;
};

#define DEFINE_DERIVED_GLOBAL_INDEX_NODE(node_name) \
	class node_name : public global_index_node \
	{ \
		public: \
			node_name(int64_t index) : \
				global_index_node(index) \
			{ \
			} \
		DECLARE_AST_NODE(node_name, global_index_node) \
	}; \

DEFINE_DERIVED_GLOBAL_INDEX_NODE(register_node)
DEFINE_DERIVED_GLOBAL_INDEX_NODE(constant_buffer_node)
DEFINE_DERIVED_GLOBAL_INDEX_NODE(immediate_constant_buffer_node)
DEFINE_DERIVED_GLOBAL_INDEX_NODE(input_node)
DEFINE_DERIVED_GLOBAL_INDEX_NODE(output_node)

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

	std::shared_ptr<ast_node> value;
	std::vector<uint8_t> indices;
};

class swizzle_node : public ast_node
{
public:
	virtual ~swizzle_node() {};
	DECLARE_AST_NODE(swizzle_node, ast_node)

	std::shared_ptr<ast_node> value;
	std::vector<uint8_t> indices;
};

class scalar_node : public ast_node
{
public:
	virtual ~scalar_node() {};
	DECLARE_AST_NODE(scalar_node, ast_node)

	std::shared_ptr<ast_node> value;
	uint8_t index;
};

class index_node : public ast_node
{
public:
	virtual ~index_node() {};
	DECLARE_AST_NODE(index_node, ast_node)

	std::shared_ptr<ast_node> index;
	std::shared_ptr<ast_node> value;
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

// will leave these for later, they're not technically opcodes
DEFINE_DERIVED_AST_NODE(negate_node, unary_node)
DEFINE_DERIVED_AST_NODE(absolute_node, unary_node)
DEFINE_DERIVED_AST_NODE(saturate_node, unary_node)

// binary
class binary_instruction_node : public ast_node
{
public:
	virtual ~binary_instruction_node() {};
	DECLARE_AST_NODE(binary_instruction_node, ast_node)

	uint8_t opcode;
	std::shared_ptr<ast_node> input;
};

// ternary
class ternary_instruction_node : public ast_node
{
public:
	virtual ~ternary_instruction_node() {};
	DECLARE_AST_NODE(ternary_instruction_node, ast_node)

	uint8_t opcode;
	std::shared_ptr<ast_node> lhs;
	std::shared_ptr<ast_node> rhs;
};

// quaternary
class quaternary_instruction_node : public ast_node
{
public:
	virtual ~quaternary_instruction_node() {};
	DECLARE_AST_NODE(quaternary_instruction_node, ast_node)

	uint8_t opcode;
	std::shared_ptr<ast_node> lhs;
	std::shared_ptr<ast_node> rhs1;
	std::shared_ptr<ast_node> rhs2;
};

// fun stuff! actual operators!
class binary_op : public ast_node
{
public:
	virtual ~binary_op() {}
	DECLARE_AST_NODE(binary_op, ast_node)

	std::shared_ptr<ast_node> lhs;
	std::shared_ptr<ast_node> rhs;
};

DEFINE_DERIVED_AST_NODE(add_node, binary_op)
DEFINE_DERIVED_AST_NODE(sub_node, binary_op)
DEFINE_DERIVED_AST_NODE(mul_node, binary_op)
DEFINE_DERIVED_AST_NODE(div_node, binary_op)
DEFINE_DERIVED_AST_NODE(assign_node, binary_op)

std::shared_ptr<super_node> decompile(program const* p);

}

#endif