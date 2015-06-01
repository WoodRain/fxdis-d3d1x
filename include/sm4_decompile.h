#ifndef SM4_DECOMPILE_H_
#define SM4_DECOMPILE_H_
   
#include <stdint.h>
#include <memory>
#include <vector>
#include "picojson.h"

struct sm4_program;
struct sm4_insn;
struct sm4_op;

namespace sm4 {

typedef sm4_program program;
typedef sm4_insn instruction;
typedef sm4_op operand;

#define DECLARE_AST_NODE(node_name, base) \
	typedef base base_class; \
	virtual char const* get_type_string() { return #node_name; }

#define DEFINE_DERIVED_AST_NODE(node_name, base) \
	class node_name : public base { public: DECLARE_AST_NODE(node_name, base) };
	
class ast_node
{
public:
	virtual ~ast_node() {};
	DECLARE_AST_NODE(ast_node, ast_node)

	virtual picojson::value dump();
};

class super_node : public ast_node
{
public:
	virtual ~super_node() {};
	DECLARE_AST_NODE(super_node, ast_node)

	virtual picojson::value dump();

	std::shared_ptr<super_node> parent;
	std::vector<std::shared_ptr<ast_node>> children;
};

DEFINE_DERIVED_AST_NODE(root_node, super_node)

// types
template <typename T>
class variable_node : public ast_node
{
public:
	virtual ~variable_node() {};
	DECLARE_AST_NODE(variable_node, ast_node)

	virtual picojson::value dump();

	T value;
};

DEFINE_DERIVED_AST_NODE(f32_value, variable_node<float>)
DEFINE_DERIVED_AST_NODE(i32_value, variable_node<int32_t>)
DEFINE_DERIVED_AST_NODE(ui32_value, variable_node<uint32_t>)

DEFINE_DERIVED_AST_NODE(f64_value, variable_node<double>)
DEFINE_DERIVED_AST_NODE(i64_value, variable_node<int64_t>)
DEFINE_DERIVED_AST_NODE(ui64_value, variable_node<uint64_t>)

class global_index_node : public ast_node
{
public:
	global_index_node(int64_t index) :
		index(index)
	{
	}

	virtual ~global_index_node() {};
	DECLARE_AST_NODE(register_node, ast_node)

	virtual picojson::value dump();

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

	virtual picojson::value dump();

	std::vector<std::shared_ptr<ast_node>> values; 
};

// indexing
class mask_node : public ast_node
{
public:
	virtual ~mask_node() {};
	DECLARE_AST_NODE(mask_node, ast_node)

	virtual picojson::value dump();

	std::shared_ptr<ast_node> value;
	std::vector<uint8_t> indices;
};

class swizzle_node : public ast_node
{
public:
	virtual ~swizzle_node() {};
	DECLARE_AST_NODE(swizzle_node, ast_node)

	virtual picojson::value dump();

	std::shared_ptr<ast_node> value;
	std::vector<uint8_t> indices;
};

class scalar_node : public ast_node
{
public:
	virtual ~scalar_node() {};
	DECLARE_AST_NODE(scalar_node, ast_node)

	virtual picojson::value dump();

	std::shared_ptr<ast_node> value;
	uint8_t index;
};

class index_node : public ast_node
{
public:
	virtual ~index_node() {};
	DECLARE_AST_NODE(index_node, ast_node)

	virtual picojson::value dump();

	std::shared_ptr<ast_node> index;
	std::shared_ptr<ast_node> value;
};

// comparison
class comparison_node : public super_node
{
public:
	virtual ~comparison_node() {};

	virtual picojson::value dump();

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

	virtual picojson::value dump();

	std::shared_ptr<ast_node> value;
};

DEFINE_DERIVED_AST_NODE(negate_node, unary_node)
DEFINE_DERIVED_AST_NODE(absolute_node, unary_node)
DEFINE_DERIVED_AST_NODE(saturate_node, unary_node)

// binary
class binary_node : public ast_node
{
public:
	virtual ~binary_node() {};

	virtual picojson::value dump();

	std::shared_ptr<ast_node> output;
	std::shared_ptr<ast_node> input;
};

DEFINE_DERIVED_AST_NODE(frc_node, binary_node)
DEFINE_DERIVED_AST_NODE(rsq_node, binary_node)
DEFINE_DERIVED_AST_NODE(itof_node, binary_node)
DEFINE_DERIVED_AST_NODE(ftoi_node, binary_node)
DEFINE_DERIVED_AST_NODE(ftou_node, binary_node)
DEFINE_DERIVED_AST_NODE(mov_node, binary_node)
DEFINE_DERIVED_AST_NODE(round_ni_node, binary_node)
DEFINE_DERIVED_AST_NODE(exp_node, binary_node)

// ternary
class ternary_node : public ast_node
{
public:
	virtual ~ternary_node() {};

	virtual picojson::value dump();

	std::shared_ptr<ast_node> output;
	std::shared_ptr<ast_node> lhs;
	std::shared_ptr<ast_node> rhs;
};

DEFINE_DERIVED_AST_NODE(mul_node, ternary_node)
DEFINE_DERIVED_AST_NODE(div_node, ternary_node)
DEFINE_DERIVED_AST_NODE(add_node, ternary_node)
DEFINE_DERIVED_AST_NODE(dp3_node, ternary_node)
DEFINE_DERIVED_AST_NODE(dp4_node, ternary_node)
DEFINE_DERIVED_AST_NODE(ishl_node, ternary_node)
DEFINE_DERIVED_AST_NODE(ushr_node, ternary_node)
DEFINE_DERIVED_AST_NODE(and_node, ternary_node)
DEFINE_DERIVED_AST_NODE(or_node, ternary_node)
DEFINE_DERIVED_AST_NODE(iadd_node, ternary_node)
DEFINE_DERIVED_AST_NODE(ieq_node, ternary_node)
DEFINE_DERIVED_AST_NODE(ge_node, ternary_node)
DEFINE_DERIVED_AST_NODE(max_node, ternary_node)
DEFINE_DERIVED_AST_NODE(min_node, ternary_node)
DEFINE_DERIVED_AST_NODE(lt_node, ternary_node)

// quaternary
class quaternary_node : public ast_node
{
public:
	virtual ~quaternary_node() {};

	virtual picojson::value dump();

	std::shared_ptr<ast_node> output;
	std::shared_ptr<ast_node> lhs;
	std::shared_ptr<ast_node> rhs1;
	std::shared_ptr<ast_node> rhs2;
};

DEFINE_DERIVED_AST_NODE(mad_node, quaternary_node)
DEFINE_DERIVED_AST_NODE(movc_node, quaternary_node)

std::shared_ptr<super_node> decompile(program const* p);

}

#endif