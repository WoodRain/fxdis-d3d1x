#include "sm4_decompile.h"
#include <ostream>
#include <string>

namespace sm4 {

// decompile
decompiler::decompiler(program const& program) :
	program_(program)
{
}

std::shared_ptr<super_node> decompiler::run()
{
	auto root = std::make_shared<super_node>();
	auto const& p = this->program_;

	auto input_struct = std::make_shared<structure_node>();
	input_struct->name = "ShaderInput";
	this->insert_type(input_struct->name, input_struct);
	root->children.push_back(input_struct);

	auto output_struct = std::make_shared<structure_node>();
	output_struct->name = "ShaderOutput";
	this->insert_type(output_struct->name, output_struct);
	root->children.push_back(output_struct);

	this->decompile_declarations(input_struct, output_struct);

	auto main_function = std::make_shared<function_node>();
	main_function->name = "main";
	main_function->parent = root;
	main_function->return_type = output_struct;
	main_function->arguments.push_back(
		std::make_shared<variable_node>(input_struct, "input"));
	
	root->children.push_back(main_function);
	root = main_function;

	for (auto const instruction : p.insns)
	{
		switch (instruction->opcode)
		{
		case SM4_OPCODE_IF:
		{
			auto node = std::make_shared<if_node>();
			auto comparand = decompile_operand(instruction, 0);

			std::shared_ptr<binary_expr_node> expression;
			auto rhs = std::make_shared<vector_node>(0.0f);

			if (instruction->insn.test_nz)
				expression = std::make_shared<neq_expr_node>(comparand, rhs);
			else
				expression = std::make_shared<eq_expr_node>(comparand, rhs);

			node->expression = expression;
			node->parent = root;
			node->parent->children.push_back(node);
			root = node;
			break;
		}
		
		case SM4_OPCODE_ELSE:
		{
			auto node = std::make_shared<else_node>();
			node->parent = root->parent;
			node->parent->children.push_back(node);
			root = node;
			break;
		}

		case SM4_OPCODE_ENDIF:
			root = root->parent;
			break;

		case SM4_OPCODE_RET:
			root->children.push_back(std::make_shared<ret_node>());
			break;

		// Unary
		case SM4_OPCODE_FRC:
		case SM4_OPCODE_RSQ:
		case SM4_OPCODE_ITOF:
		case SM4_OPCODE_FTOI:
		case SM4_OPCODE_FTOU:
		case SM4_OPCODE_MOV:
		case SM4_OPCODE_ROUND_NI:
		case SM4_OPCODE_EXP:
		// Binary
		case SM4_OPCODE_MUL:
		case SM4_OPCODE_DIV:
		case SM4_OPCODE_ADD:
		case SM4_OPCODE_DP3:
		case SM4_OPCODE_DP4:
		case SM4_OPCODE_ISHL:
		case SM4_OPCODE_USHR:
		case SM4_OPCODE_AND:
		case SM4_OPCODE_OR:
		case SM4_OPCODE_IADD:
		case SM4_OPCODE_IEQ:
		case SM4_OPCODE_GE:
		case SM4_OPCODE_MAX:
		case SM4_OPCODE_MIN:
		case SM4_OPCODE_LT:
		// Ternary
		case SM4_OPCODE_MAD:
		case SM4_OPCODE_MOVC:
		{
			auto node = std::make_shared<instruction_call_expr_node>();
			node->opcode = instruction->opcode;

			for (size_t i = 1; i < instruction->num_ops; ++i)
				node->arguments.push_back(decompile_operand(instruction, i));

			std::shared_ptr<ast_node> lhs = decompile_operand(instruction, 0);
			std::shared_ptr<ast_node> rhs = node;
			if (instruction->insn.sat)
				rhs = std::make_shared<function_call_expr_node>("saturate", node);

			auto assign_expr = std::make_shared<assign_expr_node>(lhs, rhs);
			root->children.push_back(std::make_shared<expr_stmt_node>(assign_expr));
			break;
		}

		default:
			std::cerr << "Unhandled opcode: " << sm4_opcode_names[instruction->opcode] << "\n";
			break;
		};
	}

	root = main_function->parent;

	return root;
}

std::shared_ptr<type_node> decompiler::get_type(std::string const& s)
{
	return this->types_[s];
}

void decompiler::insert_type(std::string const& s, std::shared_ptr<type_node> node)
{
	this->types_[s] = node;
}

void decompiler::decompile_declarations(
	std::shared_ptr<structure_node> input, 
	std::shared_ptr<structure_node> output)
{
	auto get_or_insert = 
	[&](std::string const& s, value_type type, uint8_t count) -> std::shared_ptr<type_node>
	{
		auto name = s + (count > 1 ? std::to_string(count) : "");
		auto node = this->get_type(name);

		if (node)
			return node;

		auto new_node = std::make_shared<vector_type_node>();
		new_node->type = type;
		new_node->count = count;
		new_node->name = name;

		this->insert_type(name, new_node);

		return new_node;
	};

	for (auto const declaration : this->program_.dcls)
	{
		switch (declaration->opcode)
		{
		case SM4_OPCODE_DCL_INPUT:
		case SM4_OPCODE_DCL_INPUT_SIV:
		{
			auto var_node = std::make_shared<variable_node>();
			auto size = get_size(declaration->op.get());
			var_node->type = get_or_insert("float", value_type::f32, size);
			var_node->name = get_name(declaration->op.get());

			if (declaration->opcode == SM4_OPCODE_DCL_INPUT_SIV)
				var_node->semantic_index = declaration->sv;

			input->children.push_back(std::make_shared<expr_stmt_node>(var_node));
			break;
		}
		case SM4_OPCODE_DCL_OUTPUT:
		case SM4_OPCODE_DCL_OUTPUT_SIV:
		{
			auto var_node = std::make_shared<variable_node>();
			auto size = get_size(declaration->op.get());
			var_node->type = get_or_insert("float", value_type::f32, size);
			var_node->name = get_name(declaration->op.get());

			if (declaration->opcode == SM4_OPCODE_DCL_OUTPUT_SIV)
				var_node->semantic_index = declaration->sv;

			output->children.push_back(std::make_shared<expr_stmt_node>(var_node));
			break;
		}
		}
	}
}

std::shared_ptr<ast_node> decompiler::decompile_operand(
	sm4::operand const* operand, sm4_opcode_type opcode_type)
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
					var_node->current_type = value_type::i32;
					var_node->i32 = value.i32;
				}
				else if (opcode_type == SM4_OPCODE_TYPE_UINT)
				{
					var_node->current_type = value_type::u32;
					var_node->u32 = value.u32;
				}
				else
				{
					var_node->current_type = value_type::f32;
					var_node->f32 = value.f32;
				}
			}
			else
			{
				if (opcode_type == SM4_OPCODE_TYPE_INT)
				{
					var_node->current_type = value_type::i64;
					var_node->i64 = value.i64;
				}
				else if (opcode_type == SM4_OPCODE_TYPE_UINT)
				{
					var_node->current_type = value_type::u64;
					var_node->u64 = value.u64;
				}
				else
				{
					var_node->current_type = value_type::f64;
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
		else if (operand->file == SM4_FILE_INPUT)
			node = std::make_shared<input_node>(index);
		else if (operand->file == SM4_FILE_OUTPUT)
			node = std::make_shared<output_node>(index);
		// There is only one ICB, and it is dynamically indexed
		else if (operand->file == SM4_FILE_IMMEDIATE_CONSTANT_BUFFER)
			node = std::make_shared<immediate_constant_buffer_node>();

		auto dynamic_index = 
			(operand->file == SM4_FILE_IMMEDIATE_CONSTANT_BUFFER) ? 0 : 1;

		// if we've found that this is actually an indexing operation
		if (operand->num_indices > dynamic_index)
		{
			auto& operand_index = operand->indices[dynamic_index];
			
			auto indexing_node = std::make_shared<dynamic_index_node>();

			if (operand_index.reg.get())
				indexing_node->index = decompile_operand(operand_index.reg.get(), opcode_type);

			indexing_node->value = force_node_cast<global_variable_node>(node);
			indexing_node->displacement = operand_index.disp;

			node = indexing_node;
		}
	}
	
	if (operand->comps)
	{
		auto gi_node = force_node_cast<global_variable_node>(node);

		if (operand->mode == SM4_OPERAND_MODE_MASK && operand->mask)
		{
			auto parent_node = std::make_shared<static_index_node>();
			parent_node->value = gi_node;

			for (unsigned int i = 0; i < operand->comps; ++i)
			{
				if (operand->mask & (1 << i))
					parent_node->indices.push_back(i);
			}

			node = parent_node;
		}
		else if (operand->mode == SM4_OPERAND_MODE_SWIZZLE)
		{
			auto parent_node = std::make_shared<static_index_node>();
			parent_node->value = gi_node;

			for (unsigned int i = 0; i < operand->comps; ++i)
				parent_node->indices.push_back(operand->swizzle[i]);

			node = parent_node;
		}
		else if (operand->mode == SM4_OPERAND_MODE_SCALAR)
		{
			auto parent_node = std::make_shared<static_index_node>();
			parent_node->value = gi_node;
			parent_node->indices.push_back(operand->swizzle[0]);

			node = parent_node;
		}
	}

	if (operand->abs)
		node = std::make_shared<function_call_expr_node>("abs", node);

	if (operand->neg)
	{
		auto parent_node = std::make_shared<negate_node>();
		parent_node->value = node;
		node = parent_node;
	}

	return node;
}

std::shared_ptr<ast_node> decompiler::decompile_operand(
	sm4::instruction const* instruction, int i)
{
	auto operand = instruction->ops[i].get();
	auto opcode_type = sm4_opcode_types[instruction->opcode];
	
	return decompile_operand(operand, opcode_type);
}

std::string decompiler::get_name(sm4::operand const* operand)
{
	std::string ret = sm4_file_names[operand->file];
	
	if (operand->indices[0].reg.get())
		return ret;

	ret += std::to_string(operand->indices[0].disp);

	return ret;
}

uint8_t decompiler::get_size(sm4::operand const* operand)
{
	uint8_t ret = 0;

	if (operand->mode == SM4_OPERAND_MODE_MASK)
	{
		for (unsigned int i = 0; i < operand->comps; ++i)
			if (operand->mask & (1 << i))
				++ret;
	}
	else if (operand->mode == SM4_OPERAND_MODE_SWIZZLE)
	{
		ret = operand->comps;
	}
	else
	{
		ret = 1;
	}

	return ret;
}


}