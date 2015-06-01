#include "sm4_decompile.h"
#include "sm4.h"
#include <ostream>

namespace sm4 {

// decompile
std::shared_ptr<ast_node> decompile_operand(sm4::operand const* operand, sm4_opcode_type opcode_type)
{
	std::shared_ptr<ast_node> node;

	if (operand->file == SM4_FILE_IMMEDIATE32 || operand->file == SM4_FILE_IMMEDIATE64)
	{
		auto new_node = std::make_shared<vector_node>();
		for (unsigned int i = 0; i < operand->comps; ++i)
		{
			auto var_node = std::make_shared<constant_node>();
			auto const& value = operand->imm_values[i];

			if (operand->file == SM4_FILE_IMMEDIATE32)
			{
				if (opcode_type == SM4_OPCODE_TYPE_INT)
				{
					var_node->current_type = constant_node::type::i32;
					var_node->i32 = value.i32;
				}
				else if (opcode_type == SM4_OPCODE_TYPE_UINT)
				{
					var_node->current_type = constant_node::type::u32;
					var_node->u32 = value.u32;
				}
				else
				{
					var_node->current_type = constant_node::type::f32;
					var_node->f32 = value.f32;
				}
			}
			else
			{
				if (opcode_type == SM4_OPCODE_TYPE_INT)
				{
					var_node->current_type = constant_node::type::i64;
					var_node->i64 = value.i64;
				}
				else if (opcode_type == SM4_OPCODE_TYPE_UINT)
				{
					var_node->current_type = constant_node::type::u64;
					var_node->u64 = value.u64;
				}
				else
				{
					var_node->current_type = constant_node::type::f64;
					var_node->f64 = value.f64;
				}
			}

			new_node->values.push_back(var_node);
		}
		node = new_node;
		return node;
	}
	else if (operand->num_indices > 0)
	{
		auto index = operand->indices[0].disp;

		if (operand->file == SM4_FILE_TEMP)
			node = std::make_shared<register_node>(index);
		else if (operand->file == SM4_FILE_CONSTANT_BUFFER)
			node = std::make_shared<constant_buffer_node>(index);
		else if (operand->file == SM4_FILE_IMMEDIATE_CONSTANT_BUFFER)
			node = std::make_shared<immediate_constant_buffer_node>(index);
		else if (operand->file == SM4_FILE_INPUT)
			node = std::make_shared<input_node>(index);
		else if (operand->file == SM4_FILE_OUTPUT)
			node = std::make_shared<output_node>(index);

		// if we've found that this is actually an indexing operation
		if (operand->indices[0].reg.get())
		{
			auto indexing_node = std::make_shared<index_node>();
			indexing_node->index = decompile_operand(
				operand->indices[0].reg.get(), opcode_type);
			indexing_node->value = node;
			node = indexing_node;
		}
	}
	
	if (operand->comps)
	{
		if (operand->mode == SM4_OPERAND_MODE_MASK && operand->mask)
		{
			auto parent_node = std::make_shared<mask_node>();
			parent_node->value = node;

			for (unsigned int i = 0; i < operand->comps; ++i)
			{
				if (operand->mask & (1 << i))
					parent_node->indices.push_back(i);
			}

			node = parent_node;
		}
		else if (operand->mode == SM4_OPERAND_MODE_SWIZZLE)
		{
			auto parent_node = std::make_shared<swizzle_node>();
			parent_node->value = node;

			for (unsigned int i = 0; i < operand->comps; ++i)
				parent_node->indices.push_back(operand->swizzle[i]);

			node = parent_node;
		}
		else if (operand->mode == SM4_OPERAND_MODE_SCALAR)
		{
			auto parent_node = std::make_shared<scalar_node>();
			parent_node->value = node;
			parent_node->index = operand->swizzle[0];

			node = parent_node;
		}
	}

	if (operand->abs)
	{
		auto parent_node = std::make_shared<absolute_node>();
		parent_node->value = node;
		node = parent_node;
	}

	if (operand->neg)
	{
		auto parent_node = std::make_shared<negate_node>();
		parent_node->value = node;
		node = parent_node;
	}

	return node;
}

std::shared_ptr<ast_node> decompile_operand(sm4::instruction const* instruction, int i)
{
	auto operand = instruction->ops[i].get();
	auto opcode_type = sm4_opcode_types[instruction->opcode];
	
	return decompile_operand(operand, opcode_type);
}

template <typename T>
std::shared_ptr<T> decompile_nullary(sm4::instruction const* instruction)
{
	auto node = std::make_shared<T>();

	return node;
}

template <typename T>
std::shared_ptr<T> decompile_unary(sm4::instruction const* instruction)
{
	auto node = std::make_shared<T>();
	node->value = decompile_operand(instruction, 0);

	return node;
}

template <typename T>
std::shared_ptr<T> decompile_binary(sm4::instruction const* instruction)
{
	auto node = std::make_shared<T>();
	node->output = decompile_operand(instruction, 0);
	node->input = decompile_operand(instruction, 1);

	return node;
}

template <typename T>
std::shared_ptr<T> decompile_ternary(sm4::instruction const* instruction)
{
	auto node = std::make_shared<T>();
	node->output = decompile_operand(instruction, 0);
	node->lhs = decompile_operand(instruction, 1);
	node->rhs = decompile_operand(instruction, 2);

	return node;
}

template <typename T>
std::shared_ptr<T> decompile_quaternary(sm4::instruction const* instruction)
{
	auto node = std::make_shared<T>();
	node->output = decompile_operand(instruction, 0);
	node->lhs = decompile_operand(instruction, 1);
	node->rhs1 = decompile_operand(instruction, 2);
	node->rhs2 = decompile_operand(instruction, 3);

	return node;
}

std::shared_ptr<super_node> decompile(program const* p)
{
	std::shared_ptr<super_node> root = std::make_shared<root_node>();

	for (auto const instruction : p->insns)
	{
		std::shared_ptr<ast_node> new_node;

		switch (instruction->opcode)
		{
#define NULLARY(opcode, type) \
		case opcode: \
			new_node = decompile_nullary<type>(instruction); \
			break;

#define UNARY(opcode, type) \
		case opcode: \
			new_node = decompile_unary<type>(instruction); \
			break;

#define BINARY(opcode, type) \
		case opcode: \
			new_node = decompile_binary<type>(instruction); \
			break;

#define TERNARY(opcode, type) \
		case opcode: \
			new_node = decompile_ternary<type>(instruction); \
			break;

#define QUATERNARY(opcode, type) \
		case opcode: \
			new_node = decompile_quaternary<type>(instruction); \
			break;

		case SM4_OPCODE_IF:
		{
			auto node = decompile_unary<if_node>(instruction);
			node->parent = root;
			node->parent->children.push_back(node);
			root = node;
			break;
		}
		
		case SM4_OPCODE_ELSE:
		{
			auto node = decompile_nullary<else_node>(instruction);
			node->parent = root->parent;
			node->parent->children.push_back(node);
			root = node;
			break;
		}

		case SM4_OPCODE_ENDIF:
			root = root->parent;
			break;

		NULLARY(SM4_OPCODE_RET, ret_node)

		BINARY(SM4_OPCODE_FRC, frc_node)
		BINARY(SM4_OPCODE_RSQ, rsq_node)
		BINARY(SM4_OPCODE_ITOF, itof_node)
		BINARY(SM4_OPCODE_FTOI, ftoi_node)
		BINARY(SM4_OPCODE_FTOU, ftou_node)
		BINARY(SM4_OPCODE_MOV, mov_node)
		BINARY(SM4_OPCODE_ROUND_NI, round_ni_node)
		BINARY(SM4_OPCODE_EXP, exp_node)

		TERNARY(SM4_OPCODE_MUL, mul_node)
		TERNARY(SM4_OPCODE_DIV, div_node)
		TERNARY(SM4_OPCODE_ADD, add_node)
		TERNARY(SM4_OPCODE_DP3, dp3_node)
		TERNARY(SM4_OPCODE_DP4, dp4_node)
		TERNARY(SM4_OPCODE_ISHL, ishl_node)
		TERNARY(SM4_OPCODE_USHR, ushr_node)
		TERNARY(SM4_OPCODE_AND, and_node)
		TERNARY(SM4_OPCODE_OR, or_node)
		TERNARY(SM4_OPCODE_IADD, iadd_node)
		TERNARY(SM4_OPCODE_IEQ, ieq_node)
		TERNARY(SM4_OPCODE_GE, ge_node)
		TERNARY(SM4_OPCODE_MAX, max_node)
		TERNARY(SM4_OPCODE_MIN, min_node)
		TERNARY(SM4_OPCODE_LT, lt_node)

		QUATERNARY(SM4_OPCODE_MAD, mad_node)
		QUATERNARY(SM4_OPCODE_MOVC, movc_node)

#undef BINARY
#undef TERNARY
#undef QUATERNARY

		default:
			std::cerr << "Unhandled opcode: " << sm4_opcode_names[instruction->opcode] << "\n";
			break;
		};

		if (new_node)
		{
			if (instruction->insn.sat)
			{
				auto sat_node = std::make_shared<saturate_node>();
				sat_node->value = new_node;

				new_node = sat_node;
			}

			root->children.push_back(new_node);
		}
	}

	return root;
}

}