#include <d3d11TokenizedProgramFormat.hpp>
#include <d3d11shader.h>
#include <stdint.h>
#include <iostream>
#include <assert.h>

// Human readiable texts for SM4/5 tokens
extern const char* ShaderTypeText[];
extern const char* OpcodeText[];
extern const char* OperandText[];
extern const char* ModifierText[];
extern const char* MinPrecisionText[];
extern const char* InterpModeText[];
extern const char* NameText[];
extern const char* ResourceDimText[];
extern const char* CustomDataText[];
extern const char* ReturnTypeText[];
extern const char* SampleModeText[];
extern const char* PrimTopoText[];
extern const char* PrimitiveText[];
extern const char* TessDomainText[];
extern const char* TessPartitionText[];
extern const char* TessOutputPrimText[];
// Different opcodes have different requirements for immediate values
enum class OPCODE_DATA_TYPE {
	UNKNOWN = 0,
	FLOAT = 1,
	DOUBLE = 2,
	UINT = 3,
	SINT = 4,
};
extern const OPCODE_DATA_TYPE OpcodeDataType[];

// Output assembly directly. No intermidiate structure is used in the parser.
class TokenParser
{
public:
	TokenParser(uint32_t* tokens, uint32_t sizeInBytes)
	{
		tokenBegin = tokens;
		tokenCurrent = tokenBegin;
		tokenSize = sizeInBytes / 4;
	}
	~TokenParser() { ; };
	void Parse();
private:
	void ParseOpcode();
	void ParseOperand(bool firstOperand, D3D10_SB_OPCODE_TYPE opcodeType);
	uint32_t* tokenBegin;
	uint32_t tokenSize;
	uint32_t* tokenCurrent;
	uint32_t* tokenEnd;
};