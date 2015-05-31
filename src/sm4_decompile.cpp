#include "sm4_decompile.h"
#include "sm4.h"
#include <ostream>

namespace sm4 {

template <typename T>
std::shared_ptr<variable_node<T>> make_variable_node(T value)
{
	auto node = std::make_shared<variable_node<T>>();
	node->value = value;
	return node;
}

// decompile
std::shared_ptr<ast_node> decompile_operand(sm4::operand const* operand, sm4_opcode_type opcode_type)
{
	std::shared_ptr<ast_node> node;

	if (operand->file == SM4_FILE_IMMEDIATE32 || operand->file == SM4_FILE_IMMEDIATE64)
	{
		auto new_node = std::make_shared<vector_node>();
		for (unsigned int i = 0; i < operand->comps; ++i)
		{
			std::shared_ptr<ast_node> variable_node;
			auto const& value = operand->imm_values[i];

			if (operand->file == SM4_FILE_IMMEDIATE32)
			{
				if (opcode_type == SM4_OPCODE_TYPE_INT)
					variable_node = make_variable_node(value.i32);
				else if (opcode_type == SM4_OPCODE_TYPE_UINT)
					variable_node = make_variable_node(value.u32);
				else
					variable_node = make_variable_node(value.f32);
			}
			else
			{
				if (opcode_type == SM4_OPCODE_TYPE_INT)
					variable_node = make_variable_node(value.i64);
				else if (opcode_type == SM4_OPCODE_TYPE_UINT)
					variable_node = make_variable_node(value.u64);
				else
					variable_node = make_variable_node(value.f64);
			}

			new_node->values.push_back(variable_node);
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

// dump
void write_spaces(std::ostream& stream, int depth)
{
	for (int i = 0; i < depth * 4; ++i)
		stream << ' ';
}

void write_newline(std::ostream& stream, int depth)
{
	if (depth > 0)
		stream << '\n';
}

void ast_node::dump(std::ostream& stream, int depth)
{
	write_spaces(stream, depth);
	stream << this->get_type_string();
	write_newline(stream, depth);
}

void unary_node::dump(std::ostream& stream, int depth)
{
	write_spaces(stream, depth);
	stream << this->get_type_string();
	write_newline(stream, depth);

	++depth;
	write_spaces(stream, depth);
	stream << "value: ";
	write_newline(stream, depth);

	++depth;
	if (this->value)
		this->value->dump(stream, depth);
}

void comparison_node::dump(std::ostream& stream, int depth)
{
	write_spaces(stream, depth);
	stream << this->get_type_string();
	stream << (this->not_zero ? " not zero " : " zero ");
	write_newline(stream, depth);

	++depth;
	write_spaces(stream, depth);
	stream << "value: ";
	write_newline(stream, depth);

	++depth;
	if (this->value)
		this->value->dump(stream, depth);

	--depth;
	write_spaces(stream, depth);
	stream << "children: ";
	write_newline(stream, depth);
	
	++depth;
	for (auto child : this->children)
		child->dump(stream, depth);
}

void binary_node::dump(std::ostream& stream, int depth)
{
	write_spaces(stream, depth);
	stream << this->get_type_string();
	write_newline(stream, depth);

	++depth;
	write_spaces(stream, depth);
	stream << "output: ";
	write_newline(stream, depth);

	++depth;
	if (this->output)	this->output->dump(stream, depth);

	--depth;
	write_spaces(stream, depth);
	stream << "input: ";
	write_newline(stream, depth);

	++depth;
	if (this->input)	this->input->dump(stream, depth);
}

void ternary_node::dump(std::ostream& stream, int depth)
{
	write_spaces(stream, depth);
	stream << this->get_type_string();
	write_newline(stream, depth);

	++depth;
	write_spaces(stream, depth);
	stream << "output: ";
	write_newline(stream, depth);

	++depth;
	if (this->output)	this->output->dump(stream, depth);

	--depth;
	write_spaces(stream, depth);
	stream << "lhs: ";
	write_newline(stream, depth);

	++depth;
	if (this->lhs)		this->lhs->dump(stream, depth);

	--depth;
	write_spaces(stream, depth);
	stream << "rhs: ";
	write_newline(stream, depth);

	++depth;
	if (this->rhs)		this->rhs->dump(stream, depth);
}

void quaternary_node::dump(std::ostream& stream, int depth)
{
	write_spaces(stream, depth);
	stream << this->get_type_string();
	write_newline(stream, depth);

	++depth;
	write_spaces(stream, depth);
	stream << "output: ";
	write_newline(stream, depth);

	++depth;
	if (this->output)	this->output->dump(stream, depth);

	--depth;
	write_spaces(stream, depth);
	stream << "lhs: ";
	write_newline(stream, depth);

	++depth;
	if (this->lhs)		this->lhs->dump(stream, depth);

	--depth;
	write_spaces(stream, depth);
	stream << "rhs1: ";
	write_newline(stream, depth);

	++depth;
	if (this->rhs1)		this->rhs1->dump(stream, depth);

	--depth;
	write_spaces(stream, depth);
	stream << "rhs2: ";
	write_newline(stream, depth);

	++depth;
	if (this->rhs2)		this->rhs2->dump(stream, depth);
}

template <typename T>
void variable_node<T>::dump(std::ostream& stream, int depth)
{
	write_spaces(stream, depth);
	stream << this->value;
	write_newline(stream, depth);
}

void global_index_node::dump(std::ostream& stream, int depth)
{
	write_spaces(stream, depth);
	stream << this->get_type_string() << " " << this->index;
	write_newline(stream, depth);
}

void vector_node::dump(std::ostream& stream, int depth)
{
	write_spaces(stream, depth);
	stream << this->get_type_string() << " (";
	for (unsigned int i = 0; i < this->values.size(); ++i)
	{
		if (i)
			stream << ", ";

		this->values[i]->dump(stream, 0);
	}
	stream << ")";
	write_newline(stream, depth);
}

void mask_node::dump(std::ostream& stream, int depth)
{
	write_spaces(stream, depth);
	stream << this->get_type_string() << " (";
	for (unsigned int i = 0; i < this->indices.size(); ++i)
	{
		if (i)
			stream << ", ";

		stream << "xyzw"[this->indices[i]];
	}
	stream << ")";
	write_newline(stream, depth);

	if (this->value)
		this->value->dump(stream, ++depth);
}

void swizzle_node::dump(std::ostream& stream, int depth)
{
	write_spaces(stream, depth);
	stream << this->get_type_string() << " (";
	for (unsigned int i = 0; i < this->indices.size(); ++i)
	{
		if (i)
			stream << ", ";

		stream << "xyzw"[this->indices[i]];
	}
	stream << ")";
	write_newline(stream, depth);

	if (this->value)
		this->value->dump(stream, ++depth);
}

void scalar_node::dump(std::ostream& stream, int depth)
{
	write_spaces(stream, depth);
	stream << this->get_type_string() << " " << "xyzw"[this->index];
	write_newline(stream, depth);

	if (this->value)
		this->value->dump(stream, ++depth);
}

void index_node::dump(std::ostream& stream, int depth)
{
	write_spaces(stream, depth);
	stream << this->get_type_string();
	write_newline(stream, depth);

	++depth;
	write_spaces(stream, depth);
	stream << "index: ";
	write_newline(stream, depth);

	++depth;
	if (this->index)	this->index->dump(stream, depth);

	--depth;
	write_spaces(stream, depth);
	stream << "value: ";
	write_newline(stream, depth);

	++depth;
	if (this->value)	this->value->dump(stream, depth);
}

void super_node::dump(std::ostream& stream, int depth)
{
	write_spaces(stream, depth);
	stream << this->get_type_string();
	write_newline(stream, 1);

	++depth;
	write_spaces(stream, depth);
	stream << "children: ";
	write_newline(stream, depth);
	
	++depth;
	for (auto child : this->children)
		child->dump(stream, depth);
}

}