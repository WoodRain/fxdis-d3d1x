#ifndef SM4_DECOMPILE_H_
#define SM4_DECOMPILE_H_
   
#include <stdint.h>
#include "sm4_ast_nodes.h"

namespace sm4 {

class decompiler
{
public:
	decompiler(sm4::program const& program);
	std::shared_ptr<super_node> run();

	void operator=(decompiler const& rhs) = delete;

	std::shared_ptr<type_node> get_type(std::string const& s);
	void insert_type(std::string const& s, std::shared_ptr<type_node> node);

private:
	void decompile_declarations(
		std::shared_ptr<super_node> scope,
		std::shared_ptr<structure_node> input, 
		std::shared_ptr<structure_node> output);

	std::shared_ptr<ast_node> decompile_operand(
		std::shared_ptr<super_node> scope, operand const* operand, sm4_opcode_type opcode_type);
	std::shared_ptr<ast_node> decompile_operand(
		std::shared_ptr<super_node> scope, instruction const* instruction, int i);
	std::string get_name(sm4::operand const* operand);
	uint8_t get_size(sm4::operand const* operand);

	program const& program_;
	std::unordered_map<std::string, std::shared_ptr<type_node>> types_;
};

}

#endif