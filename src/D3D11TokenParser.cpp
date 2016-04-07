#include "D3D11TokenParser.h"
using std::cout;
using std::endl;

void TokenParser::Parse()
{
	uint32_t version = *tokenCurrent++;
	cout << ShaderTypeText[DECODE_D3D10_SB_TOKENIZED_PROGRAM_TYPE(version)]
		<< DECODE_D3D10_SB_TOKENIZED_PROGRAM_MAJOR_VERSION(version) << "_"
		<< DECODE_D3D10_SB_TOKENIZED_PROGRAM_MINOR_VERSION(version) << endl;

	uint32_t size = *tokenCurrent++;
	tokenEnd = tokenBegin + (tokenSize > size ? size : tokenSize);
	if (tokenSize != size)
	{
		cout << "// Provided token size and actual size mismatch." << endl;
	}

	while (tokenCurrent != tokenEnd)
	{
		ParseOpcode();
	}
}

void TokenParser::ParseOpcode()
{
	uint32_t opcodeToken = *tokenCurrent++;
	D3D10_SB_OPCODE_TYPE opcodeType = DECODE_D3D10_SB_OPCODE_TYPE(opcodeToken);
	uint32_t instructLen = DECODE_D3D10_SB_TOKENIZED_INSTRUCTION_LENGTH(opcodeToken);
	uint32_t remainLen = instructLen - 1;

	uint32_t extOpcodeToken = 0;
	bool extOpcode = DECODE_IS_D3D10_SB_OPCODE_EXTENDED(opcodeToken) != 0; // It's always zero in custom data.
	if (extOpcode)
	{
		extOpcodeToken = *tokenCurrent++; // only one extend opcode is supported currently.
		assert(!DECODE_IS_D3D10_SB_OPCODE_EXTENDED(extOpcodeToken));
		remainLen -= 1;
	}

	uint32_t* opcodeEnd = tokenCurrent + remainLen;

	if (opcodeType != D3D10_SB_OPCODE_CUSTOMDATA)
	{
		cout << OpcodeText[opcodeType];
	}

	auto ReturnTypeOut = [](uint32_t returnType)->void {
		cout << " (" << ReturnTypeText[DECODE_D3D10_SB_RESOURCE_RETURN_TYPE(returnType, D3D10_SB_4_COMPONENT_X)]
			<< ", " << ReturnTypeText[DECODE_D3D10_SB_RESOURCE_RETURN_TYPE(returnType, D3D10_SB_4_COMPONENT_Y)]
			<< ", " << ReturnTypeText[DECODE_D3D10_SB_RESOURCE_RETURN_TYPE(returnType, D3D10_SB_4_COMPONENT_Z)]
			<< ", " << ReturnTypeText[DECODE_D3D10_SB_RESOURCE_RETURN_TYPE(returnType, D3D10_SB_4_COMPONENT_W)] << ")";};

	switch (opcodeType)
	{
	case D3D10_SB_OPCODE_CUSTOMDATA:
	{
		D3D10_SB_CUSTOMDATA_CLASS customData = DECODE_D3D10_SB_CUSTOMDATA_CLASS(opcodeToken);
		uint32_t customDataLen = *tokenCurrent++;
		if (customData == D3D10_SB_CUSTOMDATA_DCL_IMMEDIATE_CONSTANT_BUFFER)
		{
			cout << "dcl_immediate_const_buffer ";
			for (uint32_t idx = 0; idx < customDataLen - 2; idx++)
			{
				if (idx % 4)
				{
					cout << ", ";
				}
				else
				{
					cout << " ";
				}
				cout << *tokenCurrent++;
			}
		}
		else
		{
			cout << "// Custom data " << CustomDataText[customData] << "skipped";
			tokenCurrent = opcodeEnd;
		}

		break;
	}
	case D3D10_SB_OPCODE_DCL_RESOURCE:
	{
		cout << ResourceDimText[DECODE_D3D10_SB_RESOURCE_DIMENSION(opcodeToken)];
		switch (DECODE_D3D10_SB_RESOURCE_DIMENSION(opcodeToken))
		{
		case D3D10_SB_RESOURCE_DIMENSION_TEXTURE2DMS:
		case D3D10_SB_RESOURCE_DIMENSION_TEXTURE2DMSARRAY:
			cout << "(" << DECODE_D3D10_SB_RESOURCE_SAMPLE_COUNT(opcodeToken) << ")";
			break;
		default:
			break;
		}
		cout << " ";
		ParseOperand(true, opcodeType);
		uint32_t returnType = *tokenCurrent++;
		ReturnTypeOut(returnType);
		break;
	}
	case D3D10_SB_OPCODE_DCL_CONSTANT_BUFFER:
	{
		if (DECODE_D3D10_SB_CONSTANT_BUFFER_ACCESS_PATTERN(opcodeToken))
		{
			cout << " dynamic indexed ";
		}
		else
		{
			cout << " immediate indexed ";
		}
		while (tokenCurrent < opcodeEnd)
		{
			ParseOperand(true, opcodeType);
		}
		break;
	}
	case D3D10_SB_OPCODE_DCL_SAMPLER:
	{
		cout << " " << SampleModeText[DECODE_D3D10_SB_SAMPLER_MODE(opcodeToken)] << " ";
		ParseOperand(true, opcodeType);
		break;
	}
	case D3D10_SB_OPCODE_DCL_INDEX_RANGE:
	{
		while (tokenCurrent < opcodeEnd)
		{
			ParseOperand(true, opcodeType);
			cout << "," << *tokenCurrent++;
		}
		break;
	}
	case D3D10_SB_OPCODE_DCL_GS_OUTPUT_PRIMITIVE_TOPOLOGY:
	{
		cout << " " << PrimTopoText[DECODE_D3D10_SB_GS_OUTPUT_PRIMITIVE_TOPOLOGY(opcodeToken)];
		break;
	}
	case D3D10_SB_OPCODE_DCL_GS_INPUT_PRIMITIVE:
	{
		cout << " " << PrimitiveText[DECODE_D3D10_SB_GS_INPUT_PRIMITIVE(opcodeToken)];
		break;
	}
	case D3D10_SB_OPCODE_DCL_MAX_OUTPUT_VERTEX_COUNT:
	{
		cout << " " << *tokenCurrent++;
		break;
	}
	case D3D10_SB_OPCODE_DCL_INPUT_PS:
		cout << " " << InterpModeText[DECODE_D3D10_SB_INPUT_INTERPOLATION_MODE(opcodeToken)] << " ";
	case D3D10_SB_OPCODE_DCL_INPUT:
	case D3D10_SB_OPCODE_DCL_OUTPUT:
	{
		while (tokenCurrent < opcodeEnd)
		{
			ParseOperand(true, opcodeType);
		}
		break;
	}
	case D3D10_SB_OPCODE_DCL_INPUT_PS_SIV:
		cout << " " << InterpModeText[DECODE_D3D10_SB_INPUT_INTERPOLATION_MODE(opcodeToken)] << " ";
	case D3D10_SB_OPCODE_DCL_INPUT_SGV:
	case D3D10_SB_OPCODE_DCL_INPUT_SIV:
	case D3D10_SB_OPCODE_DCL_INPUT_PS_SGV:
	case D3D10_SB_OPCODE_DCL_OUTPUT_SGV:
	case D3D10_SB_OPCODE_DCL_OUTPUT_SIV:
	{
		while (tokenCurrent < opcodeEnd)
		{
			ParseOperand(true, opcodeType);
			uint32_t nameToken = *tokenCurrent++;
			cout << " " << NameText[DECODE_D3D10_SB_NAME(nameToken)];
		}
		break;
	}
	case D3D10_SB_OPCODE_DCL_TEMPS:
	{
		cout << " " << *tokenCurrent++;
		break;
	}
	case D3D10_SB_OPCODE_DCL_INDEXABLE_TEMP:
	{
		cout << " x" << *tokenCurrent++;
		cout << "[" << *tokenCurrent++ << "]";
		cout << ", " << *tokenCurrent++;
		break;
	}
	case D3D10_SB_OPCODE_DCL_GLOBAL_FLAGS:
	{

		uint32_t separatorIdx = 0;
		if (opcodeToken & D3D10_SB_GLOBAL_FLAG_REFACTORING_ALLOWED)
		{
			cout << " |"[separatorIdx] << "global refactioring allowed";
			separatorIdx = 1;
		}
		if (opcodeToken & D3D11_SB_GLOBAL_FLAG_ENABLE_DOUBLE_PRECISION_FLOAT_OPS)
		{
			cout << " |"[separatorIdx] << "enable double precision float";
			separatorIdx = 1;
		}
		if (opcodeToken & D3D11_SB_GLOBAL_FLAG_FORCE_EARLY_DEPTH_STENCIL)
		{
			cout << " |"[separatorIdx] << "force early depth stencil";
			separatorIdx = 1;
		}
		if (opcodeToken & D3D11_SB_GLOBAL_FLAG_ENABLE_RAW_AND_STRUCTURED_BUFFERS)
		{
			cout << " |"[separatorIdx] << "enable raw and structured buffers";
			separatorIdx = 1;
		}
		if (opcodeToken & D3D11_1_SB_GLOBAL_FLAG_SKIP_OPTIMIZATION)
		{
			cout << " |"[separatorIdx] << "skip optimization";
			separatorIdx = 1;
		}
		if (opcodeToken & D3D11_1_SB_GLOBAL_FLAG_ENABLE_MINIMUM_PRECISION)
		{
			cout << " |"[separatorIdx] << "enable minimum precision";
			separatorIdx = 1;
		}
		if (opcodeToken & D3D11_1_SB_GLOBAL_FLAG_ENABLE_DOUBLE_EXTENSIONS)
		{
			cout << " |"[separatorIdx] << "enable double extensions";
			separatorIdx = 1;
		}
		if (opcodeToken & D3D11_1_SB_GLOBAL_FLAG_ENABLE_SHADER_EXTENSIONS)
		{
			cout << " |"[separatorIdx] << "enable shader extensions";
			separatorIdx = 1;
		}
		break;
	}
	case D3D11_SB_OPCODE_DCL_STREAM:
	{
		cout << "// stream parse skipped";
		tokenCurrent = opcodeEnd;
		break;
	}
	case D3D11_SB_OPCODE_DCL_FUNCTION_BODY:
	{
		cout << "// function body skipped";
		tokenCurrent = opcodeEnd;
		break;
	}
	case D3D11_SB_OPCODE_DCL_FUNCTION_TABLE:
	{
		cout << "// function table skipped";
		tokenCurrent = opcodeEnd;
		break;
	}
	case D3D11_SB_OPCODE_DCL_INTERFACE:
	{
		cout << "// interface skipped";
		tokenCurrent = opcodeEnd;
		break;
	}
	case D3D11_SB_OPCODE_DCL_INPUT_CONTROL_POINT_COUNT:
		cout << " " << DECODE_D3D11_SB_INPUT_CONTROL_POINT_COUNT(opcodeToken);
		break;
	case D3D11_SB_OPCODE_DCL_OUTPUT_CONTROL_POINT_COUNT:
		cout << " " << DECODE_D3D11_SB_OUTPUT_CONTROL_POINT_COUNT(opcodeToken);
		break;
	case D3D11_SB_OPCODE_DCL_TESS_DOMAIN:
		cout << " " << TessDomainText[DECODE_D3D11_SB_TESS_DOMAIN(opcodeToken)];
		break;
	case D3D11_SB_OPCODE_DCL_TESS_PARTITIONING:
		cout << " " << TessPartitionText[DECODE_D3D11_SB_TESS_PARTITIONING(opcodeToken)];
		break;
	case D3D11_SB_OPCODE_DCL_TESS_OUTPUT_PRIMITIVE:
		cout << " " << TessOutputPrimText[DECODE_D3D11_SB_TESS_OUTPUT_PRIMITIVE(opcodeToken)];
		break;
	case D3D11_SB_OPCODE_DCL_HS_MAX_TESSFACTOR:
		cout << " " << *(float*)tokenCurrent;
		tokenCurrent++;
		break;
	case D3D11_SB_OPCODE_DCL_HS_FORK_PHASE_INSTANCE_COUNT:
	case D3D11_SB_OPCODE_DCL_HS_JOIN_PHASE_INSTANCE_COUNT:
		cout << " " << *tokenCurrent++;
		break;
	case D3D11_SB_OPCODE_DCL_THREAD_GROUP:
		cout << " " << "x:" << *tokenCurrent++;
		cout << " " << "y:" << *tokenCurrent++;
		cout << " " << "z:" << *tokenCurrent++;
		break;
	case D3D11_SB_OPCODE_DCL_UNORDERED_ACCESS_VIEW_TYPED:
	{
		cout << ResourceDimText[DECODE_D3D10_SB_RESOURCE_DIMENSION(opcodeToken)];
		if (opcodeToken & D3D11_SB_GLOBALLY_COHERENT_ACCESS)
		{
			cout << " " << "globally coherent access" << " ";
		}
		while (tokenCurrent < opcodeEnd)
		{
			ParseOperand(true, opcodeType);
			uint32_t returnType = *tokenCurrent++;
			ReturnTypeOut(returnType);
		}
		break;
	}
	case D3D11_SB_OPCODE_DCL_UNORDERED_ACCESS_VIEW_RAW:
	{
		cout << ResourceDimText[DECODE_D3D10_SB_RESOURCE_DIMENSION(opcodeToken)];
		if (opcodeToken & D3D11_SB_GLOBALLY_COHERENT_ACCESS)
		{
			cout << " " << "globally coherent access" << " ";
		}
		while (tokenCurrent < opcodeEnd)
		{
			ParseOperand(true, opcodeType);
		}
		break;
	}
	case D3D11_SB_OPCODE_DCL_UNORDERED_ACCESS_VIEW_STRUCTURED:
	{
		cout << ResourceDimText[DECODE_D3D10_SB_RESOURCE_DIMENSION(opcodeToken)];
		if (opcodeToken & D3D11_SB_GLOBALLY_COHERENT_ACCESS)
		{
			cout << " " << "globally coherent access" << " ";
		}
		if (opcodeToken & D3D11_SB_UAV_HAS_ORDER_PRESERVING_COUNTER)
		{
			cout << " " << "order preserving counter" << " ";
		}
		while (tokenCurrent < opcodeEnd)
		{
			ParseOperand(true, opcodeType);
			cout << ", stride(" << *tokenCurrent++ << ")";
		}
		break;
	}
	case D3D11_SB_OPCODE_DCL_THREAD_GROUP_SHARED_MEMORY_RAW:
	{
		while (tokenCurrent < opcodeEnd)
		{
			ParseOperand(true, opcodeType);
			cout << ", count(" << *tokenCurrent++ << ")";
		}
		break;
	}
	case D3D11_SB_OPCODE_DCL_THREAD_GROUP_SHARED_MEMORY_STRUCTURED:
	{
		while (tokenCurrent < opcodeEnd)
		{
			ParseOperand(true, opcodeType);
			cout << ", stride(" << *tokenCurrent++ << ")";
			cout << ", count(" << *tokenCurrent++ << ")";
		}
		break;
	}
	case D3D11_SB_OPCODE_DCL_RESOURCE_RAW:
	{
		while (tokenCurrent < opcodeEnd)
		{
			ParseOperand(true, opcodeType);
		}
		tokenCurrent = opcodeEnd;// The token header file said there're two operands but only one is provided actually.
		break;
	}
	case D3D11_SB_OPCODE_DCL_RESOURCE_STRUCTURED:
	{
		while (tokenCurrent < opcodeEnd)
		{
			ParseOperand(true, opcodeType);
			cout << ", stride(" << *tokenCurrent++ << ")";
		}
		break;
	}
	case D3D11_SB_OPCODE_DCL_GS_INSTANCE_COUNT:
	{
		cout << " " << *tokenCurrent++;
		break;
	}
	case D3D10_SB_OPCODE_BREAKC:
	case D3D10_SB_OPCODE_CALLC:
	case D3D10_SB_OPCODE_CONTINUEC:
	case D3D10_SB_OPCODE_IF:
	case D3D10_SB_OPCODE_MOVC:
	case D3D10_SB_OPCODE_RETC:
	case D3D11_SB_OPCODE_SWAPC:
		cout << (DECODE_D3D10_SB_INSTRUCTION_TEST_BOOLEAN(opcodeToken) ? "_nz" : "_z");
	case D3D10_SB_OPCODE_ADD:
	case D3D10_SB_OPCODE_AND:
	case D3D10_SB_OPCODE_BREAK:
	case D3D10_SB_OPCODE_CALL:
	case D3D10_SB_OPCODE_CASE:
	case D3D10_SB_OPCODE_CONTINUE:
	case D3D10_SB_OPCODE_CUT:
	case D3D10_SB_OPCODE_DEFAULT:
	case D3D10_SB_OPCODE_DERIV_RTX:
	case D3D10_SB_OPCODE_DERIV_RTY:
	case D3D10_SB_OPCODE_DISCARD:
	case D3D10_SB_OPCODE_DIV:
	case D3D10_SB_OPCODE_DP2:
	case D3D10_SB_OPCODE_DP3:
	case D3D10_SB_OPCODE_DP4:
	case D3D10_SB_OPCODE_ELSE:
	case D3D10_SB_OPCODE_EMIT:
	case D3D10_SB_OPCODE_EMITTHENCUT:
	case D3D10_SB_OPCODE_ENDIF:
	case D3D10_SB_OPCODE_ENDLOOP:
	case D3D10_SB_OPCODE_ENDSWITCH:
	case D3D10_SB_OPCODE_EQ:
	case D3D10_SB_OPCODE_EXP:
	case D3D10_SB_OPCODE_FRC:
	case D3D10_SB_OPCODE_FTOI:
	case D3D10_SB_OPCODE_FTOU:
	case D3D10_SB_OPCODE_GE:
	case D3D10_SB_OPCODE_IADD:
	case D3D10_SB_OPCODE_IEQ:
	case D3D10_SB_OPCODE_IGE:
	case D3D10_SB_OPCODE_ILT:
	case D3D10_SB_OPCODE_IMAD:
	case D3D10_SB_OPCODE_IMAX:
	case D3D10_SB_OPCODE_IMIN:
	case D3D10_SB_OPCODE_IMUL:
	case D3D10_SB_OPCODE_INE:
	case D3D10_SB_OPCODE_INEG:
	case D3D10_SB_OPCODE_ISHL:
	case D3D10_SB_OPCODE_ISHR:
	case D3D10_SB_OPCODE_ITOF:
	case D3D10_SB_OPCODE_LABEL:
	case D3D10_SB_OPCODE_LD:
	case D3D10_SB_OPCODE_LD_MS:
	case D3D10_SB_OPCODE_LOG:
	case D3D10_SB_OPCODE_LOOP:
	case D3D10_SB_OPCODE_LT:
	case D3D10_SB_OPCODE_MAD:
	case D3D10_SB_OPCODE_MIN:
	case D3D10_SB_OPCODE_MAX:
	case D3D10_SB_OPCODE_MOV:
	case D3D10_SB_OPCODE_MUL:
	case D3D10_SB_OPCODE_NE:
	case D3D10_SB_OPCODE_NOP:
	case D3D10_SB_OPCODE_NOT:
	case D3D10_SB_OPCODE_OR:
	case D3D10_SB_OPCODE_RESINFO:
	case D3D10_SB_OPCODE_RET:
	case D3D10_SB_OPCODE_ROUND_NE:
	case D3D10_SB_OPCODE_ROUND_NI:
	case D3D10_SB_OPCODE_ROUND_PI:
	case D3D10_SB_OPCODE_ROUND_Z:
	case D3D10_SB_OPCODE_RSQ:
	case D3D10_SB_OPCODE_SAMPLE:
	case D3D10_SB_OPCODE_SAMPLE_C:
	case D3D10_SB_OPCODE_SAMPLE_C_LZ:
	case D3D10_SB_OPCODE_SAMPLE_L:
	case D3D10_SB_OPCODE_SAMPLE_D:
	case D3D10_SB_OPCODE_SAMPLE_B:
	case D3D10_SB_OPCODE_SQRT:
	case D3D10_SB_OPCODE_SWITCH:
	case D3D10_SB_OPCODE_SINCOS:
	case D3D10_SB_OPCODE_UDIV:
	case D3D10_SB_OPCODE_ULT:
	case D3D10_SB_OPCODE_UGE:
	case D3D10_SB_OPCODE_UMUL:
	case D3D10_SB_OPCODE_UMAD:
	case D3D10_SB_OPCODE_UMAX:
	case D3D10_SB_OPCODE_UMIN:
	case D3D10_SB_OPCODE_USHR:
	case D3D10_SB_OPCODE_UTOF:
	case D3D10_SB_OPCODE_XOR:
	case D3D10_1_SB_OPCODE_LOD:
	case D3D10_1_SB_OPCODE_GATHER4:
	case D3D10_1_SB_OPCODE_SAMPLE_POS:
	case D3D10_1_SB_OPCODE_SAMPLE_INFO:
	case D3D11_SB_OPCODE_HS_DECLS:
	case D3D11_SB_OPCODE_HS_CONTROL_POINT_PHASE:
	case D3D11_SB_OPCODE_HS_FORK_PHASE:
	case D3D11_SB_OPCODE_HS_JOIN_PHASE:
	case D3D11_SB_OPCODE_EMIT_STREAM:
	case D3D11_SB_OPCODE_CUT_STREAM:
	case D3D11_SB_OPCODE_EMITTHENCUT_STREAM:
	case D3D11_SB_OPCODE_INTERFACE_CALL:
	case D3D11_SB_OPCODE_BUFINFO:
	case D3D11_SB_OPCODE_DERIV_RTX_COARSE:
	case D3D11_SB_OPCODE_DERIV_RTX_FINE:
	case D3D11_SB_OPCODE_DERIV_RTY_COARSE:
	case D3D11_SB_OPCODE_DERIV_RTY_FINE:
	case D3D11_SB_OPCODE_GATHER4_C:
	case D3D11_SB_OPCODE_GATHER4_PO:
	case D3D11_SB_OPCODE_GATHER4_PO_C:
	case D3D11_SB_OPCODE_RCP:
	case D3D11_SB_OPCODE_F32TOF16:
	case D3D11_SB_OPCODE_F16TOF32:
	case D3D11_SB_OPCODE_UADDC:
	case D3D11_SB_OPCODE_USUBB:
	case D3D11_SB_OPCODE_COUNTBITS:
	case D3D11_SB_OPCODE_FIRSTBIT_HI:
	case D3D11_SB_OPCODE_FIRSTBIT_LO:
	case D3D11_SB_OPCODE_FIRSTBIT_SHI:
	case D3D11_SB_OPCODE_UBFE:
	case D3D11_SB_OPCODE_IBFE:
	case D3D11_SB_OPCODE_BFI:
	case D3D11_SB_OPCODE_BFREV:
	case D3D11_SB_OPCODE_LD_UAV_TYPED:
	case D3D11_SB_OPCODE_STORE_UAV_TYPED:
	case D3D11_SB_OPCODE_LD_RAW:
	case D3D11_SB_OPCODE_STORE_RAW:
	case D3D11_SB_OPCODE_LD_STRUCTURED:
	case D3D11_SB_OPCODE_STORE_STRUCTURED:
	case D3D11_SB_OPCODE_ATOMIC_AND:
	case D3D11_SB_OPCODE_ATOMIC_OR:
	case D3D11_SB_OPCODE_ATOMIC_XOR:
	case D3D11_SB_OPCODE_ATOMIC_CMP_STORE:
	case D3D11_SB_OPCODE_ATOMIC_IADD:
	case D3D11_SB_OPCODE_ATOMIC_IMAX:
	case D3D11_SB_OPCODE_ATOMIC_IMIN:
	case D3D11_SB_OPCODE_ATOMIC_UMAX:
	case D3D11_SB_OPCODE_ATOMIC_UMIN:
	case D3D11_SB_OPCODE_IMM_ATOMIC_ALLOC:
	case D3D11_SB_OPCODE_IMM_ATOMIC_CONSUME:
	case D3D11_SB_OPCODE_IMM_ATOMIC_IADD:
	case D3D11_SB_OPCODE_IMM_ATOMIC_AND:
	case D3D11_SB_OPCODE_IMM_ATOMIC_OR:
	case D3D11_SB_OPCODE_IMM_ATOMIC_XOR:
	case D3D11_SB_OPCODE_IMM_ATOMIC_EXCH:
	case D3D11_SB_OPCODE_IMM_ATOMIC_CMP_EXCH:
	case D3D11_SB_OPCODE_IMM_ATOMIC_IMAX:
	case D3D11_SB_OPCODE_IMM_ATOMIC_IMIN:
	case D3D11_SB_OPCODE_IMM_ATOMIC_UMAX:
	case D3D11_SB_OPCODE_IMM_ATOMIC_UMIN:
	case D3D11_SB_OPCODE_SYNC:
	case D3D11_SB_OPCODE_DADD:
	case D3D11_SB_OPCODE_DMAX:
	case D3D11_SB_OPCODE_DMIN:
	case D3D11_SB_OPCODE_DMUL:
	case D3D11_SB_OPCODE_DEQ:
	case D3D11_SB_OPCODE_DGE:
	case D3D11_SB_OPCODE_DLT:
	case D3D11_SB_OPCODE_DNE:
	case D3D11_SB_OPCODE_DMOV:
	case D3D11_SB_OPCODE_DMOVC:
	case D3D11_SB_OPCODE_DTOF:
	case D3D11_SB_OPCODE_FTOD:
	case D3D11_SB_OPCODE_EVAL_SNAPPED:
	case D3D11_SB_OPCODE_EVAL_SAMPLE_INDEX:
	case D3D11_SB_OPCODE_EVAL_CENTROID:
	case D3D11_1_SB_OPCODE_DDIV:
	case D3D11_1_SB_OPCODE_DFMA:
	case D3D11_1_SB_OPCODE_DRCP:
	case D3D11_1_SB_OPCODE_MSAD:
	case D3D11_1_SB_OPCODE_DTOI:
	case D3D11_1_SB_OPCODE_DTOU:
	case D3D11_1_SB_OPCODE_ITOD:
	case D3D11_1_SB_OPCODE_UTOD:
	{
		bool firstOprnd = true;
		while (tokenCurrent < opcodeEnd)
		{
			ParseOperand(firstOprnd, opcodeType);
			firstOprnd = false;
		}
		break;
	}
	case D3DWDDM1_3_SB_OPCODE_GATHER4_FEEDBACK:
	case D3DWDDM1_3_SB_OPCODE_GATHER4_C_FEEDBACK:
	case D3DWDDM1_3_SB_OPCODE_GATHER4_PO_FEEDBACK:
	case D3DWDDM1_3_SB_OPCODE_GATHER4_PO_C_FEEDBACK:
	case D3DWDDM1_3_SB_OPCODE_LD_FEEDBACK:
	case D3DWDDM1_3_SB_OPCODE_LD_MS_FEEDBACK:
	case D3DWDDM1_3_SB_OPCODE_LD_UAV_TYPED_FEEDBACK:
	case D3DWDDM1_3_SB_OPCODE_LD_RAW_FEEDBACK:
	case D3DWDDM1_3_SB_OPCODE_LD_STRUCTURED_FEEDBACK:
	case D3DWDDM1_3_SB_OPCODE_SAMPLE_L_FEEDBACK:
	case D3DWDDM1_3_SB_OPCODE_SAMPLE_C_LZ_FEEDBACK:
	case D3DWDDM1_3_SB_OPCODE_SAMPLE_CLAMP_FEEDBACK:
	case D3DWDDM1_3_SB_OPCODE_SAMPLE_B_CLAMP_FEEDBACK:
	case D3DWDDM1_3_SB_OPCODE_SAMPLE_D_CLAMP_FEEDBACK:
	case D3DWDDM1_3_SB_OPCODE_SAMPLE_C_CLAMP_FEEDBACK:
		cout << " // Donot know how to parse opcode with _feedback suffix.";
		tokenCurrent = opcodeEnd;
		break;
	case D3DWDDM1_3_SB_OPCODE_CHECK_ACCESS_FULLY_MAPPED:
		cout << " // skipped";
		tokenCurrent = opcodeEnd;
		break;
	case D3D11_SB_OPCODE_ABORT:
	case D3D11_SB_OPCODE_DEBUG_BREAK:
	case D3D11_SB_OPCODE_RESERVED0:
	case D3D10_SB_OPCODE_RESERVED0:
	case D3D10_1_SB_OPCODE_RESERVED1:
	case D3D11_1_SB_OPCODE_RESERVED0:
	case D3DWDDM1_3_SB_OPCODE_RESERVED0:
	case D3D10_SB_NUM_OPCODES:
	default:
		assert(!"Invalid opcode types.");
		tokenCurrent = opcodeEnd;
		break;
	}

	cout << endl;
}

void TokenParser::ParseOperand(bool firstOperand, D3D10_SB_OPCODE_TYPE opcodeType)
{
	if (!firstOperand)
	{
		cout << ",";
	}

	auto ReadImm = [](D3D10_SB_OPCODE_TYPE type, uint32_t* ptr)->void {
		switch (OpcodeDataType[type])
		{
		case OPCODE_DATA_TYPE::UNKNOWN:
			cout << "// (float is used for unknown opcode data types)";
		case OPCODE_DATA_TYPE::FLOAT:
			cout << *(float*)ptr;
			break;
		case OPCODE_DATA_TYPE::SINT:
			cout << *(int32_t*)ptr;
			break;
		case OPCODE_DATA_TYPE::UINT:
			cout << *(uint32_t*)ptr;
			break;
		case OPCODE_DATA_TYPE::DOUBLE:
			cout << *(double*)ptr;
			break;
		default:
			assert(!"It should never be reached.");
			break;
		}
	};
	uint32_t oprndToken = *tokenCurrent++;
	D3D10_SB_OPERAND_TYPE oprndType = DECODE_D3D10_SB_OPERAND_TYPE(oprndToken);
	cout << " " << OperandText[oprndType];
	bool extOprnd = DECODE_IS_D3D10_SB_OPERAND_EXTENDED(oprndToken) != 0;
	uint32_t extOprndToken = 0;
	if (extOprnd)
	{
		extOprndToken = *tokenCurrent++;
	}

	uint32_t numComp = 0;
	switch ((D3D10_SB_OPERAND_NUM_COMPONENTS)DECODE_D3D10_SB_OPERAND_NUM_COMPONENTS(oprndToken))
	{
	case D3D10_SB_OPERAND_0_COMPONENT:
		numComp = 0;
		break;
	case D3D10_SB_OPERAND_1_COMPONENT:
		numComp = 1;
		break;
	case D3D10_SB_OPERAND_4_COMPONENT:
		numComp = 4;
		break;
	case D3D10_SB_OPERAND_N_COMPONENT:
		assert(!"This type is not used.");
	default:
		break;
	}

	bool compSuffix = true;
	D3D10_SB_OPERAND_INDEX_DIMENSION indexDim = DECODE_D3D10_SB_OPERAND_INDEX_DIMENSION(oprndToken);
	if (oprndType == D3D10_SB_OPERAND_TYPE_IMMEDIATE32)
	{
		compSuffix = false;
		cout << "(";
		for (uint32_t immIdx = 0; immIdx < numComp; immIdx++)
		{
			if (immIdx)
			{
				cout << ",";
			}
			ReadImm(opcodeType, tokenCurrent);
			tokenCurrent++;
		}
		cout << ")";
	}
	else if (oprndType == D3D10_SB_OPERAND_TYPE_IMMEDIATE64)
	{
		compSuffix = false;
		cout << "(";
		for (uint32_t immIdx = 0; immIdx < numComp; immIdx++)
		{
			if (immIdx)
			{
				cout << ",";
			}
			ReadImm(opcodeType, tokenCurrent);
			tokenCurrent += 2;
		}
		cout << ")";
	}
	else
	{
		for (uint32_t idx = 0; idx < (uint32_t)indexDim; idx++)
		{
			if (idx)
			{
				cout << "[";
			}

			switch ((D3D10_SB_OPERAND_INDEX_REPRESENTATION)(DECODE_D3D10_SB_OPERAND_INDEX_REPRESENTATION(idx, oprndToken)))
			{
			case D3D10_SB_OPERAND_INDEX_IMMEDIATE32:
				cout << *tokenCurrent++;
				break;
			case D3D10_SB_OPERAND_INDEX_IMMEDIATE64:
				cout << *tokenCurrent++; // 64 HI
				cout << *tokenCurrent++; // 64 LO
				break;
			case D3D10_SB_OPERAND_INDEX_RELATIVE:
				ParseOperand(true, opcodeType);
				break;
			case D3D10_SB_OPERAND_INDEX_IMMEDIATE32_PLUS_RELATIVE:
				cout << *tokenCurrent++;
				ParseOperand(true, opcodeType);
				break;
			case D3D10_SB_OPERAND_INDEX_IMMEDIATE64_PLUS_RELATIVE:
				cout << *tokenCurrent++;
				cout << *tokenCurrent++;
				ParseOperand(true, opcodeType);
				break;
			default:
				assert(!"It should never be reached.");
				break;
			}

			if (idx)
			{
				cout << "]";
			}
		}
	}

	if (numComp && compSuffix)
	{
		cout << ".";
		switch ((D3D10_SB_OPERAND_4_COMPONENT_SELECTION_MODE)DECODE_D3D10_SB_OPERAND_4_COMPONENT_SELECTION_MODE(oprndToken))
		{
		case D3D10_SB_OPERAND_4_COMPONENT_MASK_MODE:
		{
			uint32_t compMasks[] = {
				D3D10_SB_OPERAND_4_COMPONENT_MASK_X,
				D3D10_SB_OPERAND_4_COMPONENT_MASK_Y,
				D3D10_SB_OPERAND_4_COMPONENT_MASK_Z,
				D3D10_SB_OPERAND_4_COMPONENT_MASK_W,
			};
			for (uint32_t compIndex = 0;compIndex < numComp;compIndex++)
			{
				if (compMasks[compIndex] & ENCODE_D3D10_SB_OPERAND_4_COMPONENT_MASK(oprndToken))
				{
					cout << "xyzw"[compIndex];
				}
			}
			break;
		}
		case D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE_MODE:
		{
			for (uint32_t compIndex = 0; compIndex < numComp; compIndex++)
			{
				cout << "xyzw"[DECODE_D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE_SOURCE(oprndToken, compIndex)];
			}
			break;
		}
		case D3D10_SB_OPERAND_4_COMPONENT_SELECT_1_MODE:
			cout << "xyzw"[DECODE_D3D10_SB_OPERAND_4_COMPONENT_SELECT_1(oprndToken)];
			break;
		default:
			assert(!"It should never be reached.");
			break;
		}
	}

	if (extOprnd)
	{
		switch ((D3D10_SB_EXTENDED_OPERAND_TYPE)DECODE_D3D10_SB_EXTENDED_OPERAND_TYPE(extOprndToken))
		{
		case D3D10_SB_EXTENDED_OPERAND_MODIFIER:
			cout << ModifierText[DECODE_D3D10_SB_OPERAND_MODIFIER(extOprndToken)];
			cout << MinPrecisionText[DECODE_D3D11_SB_OPERAND_MIN_PRECISION(extOprndToken)];
			break;
		case D3D10_SB_EXTENDED_OPERAND_EMPTY:
			// nothing
			break;
		default:
			assert(!"It should never be reached.");
			break;
		}
	}
}

