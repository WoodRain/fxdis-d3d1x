#include "D3D11TokenParser.h"
// Text for D3D10_SB_TOKENIZED_PROGRAM_TYPE
const char* ShaderTypeText[] = {
	"ps_",
	"vs_",
	"gs_",
	"hs_",
	"ds_",
	"cs_"
};

// Text for D3D10_SB_OPCODE_TYPE and the names are aligned with SM4/5 assembly. 
const char* OpcodeText[] = {
	"add",
	"and",
	"break",
	"breakc",
	"call",
	"callc",
	"case",
	"continue",
	"continuec",
	"cut",
	"default",
	"deriv_rtx",
	"deriv_rty",
	"discard",
	"div",
	"dp2",
	"dp3",
	"dp4",
	"else",
	"emit",
	"emitThenCut",
	"endif",
	"endloop",
	"endswitch",
	"eq",
	"exp",
	"frc",
	"ftoi",
	"ftou",
	"ge",
	"iadd",
	"if",
	"ieq",
	"ige",
	"ilt",
	"imad",
	"imax",
	"imin",
	"imul",
	"ine",
	"ineg",
	"ishl",
	"ishr",
	"itof",
	"label",
	"ld",
	"ld2dms",
	"log",
	"loop",
	"lt",
	"mad",
	"min",
	"max",
	"CustomData", // For custome data formats
	"mov",
	"movc",
	"mul",
	"ne",
	"nop",
	"not",
	"or",
	"resinfo",
	"ret",
	"retc",
	"round_ne",
	"round_ni",
	"round_pi",
	"round_z",
	"rsq",
	"sample",
	"sample_c",
	"sample_c_lz",
	"sample_l",
	"sample_d",
	"sample_b",
	"sqrt",
	"switch",
	"sincos",
	"udiv",
	"ult",
	"uge",
	"umul",
	"umad",
	"umax",
	"umin",
	"ushr",
	"utof",
	"xor",
	"dcl_resource",
	"dcl_constantBuffer",
	"dcl_sampler",
	"dcl_indexRange",
	"dcl_gsOutputTopology",
	"dcl_gsInputPrimitive",
	"dcl_maxOutputVertexCount",
	"dcl_input",
	"dcl_input_sgv",
	"dcl_input_siv",
	"dcl_input_ps",
	"dcl_input_ps_sgv",
	"dcl_input_ps_siv",
	"dcl_output",
	"dcl_output_sgv",
	"dcl_output_siv",
	"dcl_temps",
	"dcl_indexableTemp",
	"dcl_globalFlags",
	// This is the END of Text for D3D10.0 opcodes
	"// EndOfD3D10Text",
	"lod",
	"gather4",
	"samplepos",
	"sampleinfo",
	// This is the END of Text for D3D10.1 opcodes
	"// EndOfD3D10_1Text",
	"hs_decls",
	"hs_control_point_phase",
	"hs_fork_phase",
	"hs_join_phase",
	"emit_stream",
	"cut_stream",
	"emitThenCut_stream",
	"interface_call",// No exact match in MSDN
	"bufinfo",
	"deriv_rtx_coarse",
	"deriv_rtx_fine",
	"deriv_rty_coarse",
	"deriv_rty_fine",
	"gather4_c",
	"gather4_po",
	"gather4_po_c",
	"rcp",
	"f32to16",
	"f16to32",
	"uaddc",
	"usubb",
	"countbits",
	"firstbit_hi",
	"firstbit_lo",
	"firstbit_shi",
	"ubfe",
	"ibfe",
	"bfi",
	"bfrev",
	"swapc",
	"dcl_stream",
	"dcl_function_body",
	"dcl_function_table",
	"dcl_interface",
	"dcl_input_control_point_count",
	"dcl_output_control_point_count",
	"dcl_tessellator_domain",
	"dcl_tessellator_partitioning",
	"dcl_tessellator_output_primitive",
	"dcl_hs_max_factor",
	"dcl_hs_fork_phase_instance_count",
	"dcl_hs_join_phase_instance_count",
	"dcl_thread_group",
	"dcl_uav_typed",
	"dcl_uav_raw",
	"dcl_uav_structured",
	"dcl_tgsm_raw",
	"dcl_tgsm_structured",
	"dcl_resource_raw",
	"dcl_resource_structured",
	"ld_uav_raw",
	"store_uav_raw",
	"ld_raw",
	"store_raw",
	"ld_structured",
	"store_structured",
	"atomic_and",
	"atomic_or",
	"atomic_xor",
	"atomic_cmp_store",
	"atomic_iadd",
	"atomic_imax",
	"atomic_imin",
	"atomic_umax",
	"atomic_umin",
	"imm_atomic_alloc",
	"imm_atomic_consume",
	"imm_atomic_iadd",
	"imm_atomic_and",
	"imm_atomic_or",
	"imm_atomic_xor",
	"imm_atomic_exch",
	"imm_atomic_cmp_exch",
	"imm_atomic_imax",
	"imm_atomic_imin",
	"imm_atomic_umax",
	"imm_atomic_umin",
	"symc",
	"dadd",
	"dmax",
	"dmin",
	"dmul",
	"deq",
	"dge",
	"dlt",
	"dne",
	"dmov",
	"dmovc",
	"dtof",
	"ftod",
	"eval_snapped",
	"eval_sample_index",
	"eval_centroid",
	"dcl_gs_instance_count",
	"abort",
	"debug_break",
	// This is the END of text for D3D11.0 opcodes
	"// EndOfD3D11Text",
	"ddiv",
	"dfma",
	"drcp",
	"msad",
	"dtoi",
	"dtou",
	"itod",
	"utod",
	// This is the END of text for D3D11.1 opcodes
	"//EndOfD3D11_1Text",
	"gather4_feedback",
	"gather4_c_feedback",
	"gather4_po_feedback",
	"gather4_po_c_feedback",
	"ld_feedback",
	"ld_ms_feedback",
	"ld_uav_typed_feedback",
	"ld_raw_feedback",
	"ld_structured_feedback",
	"sample_l_feedback",
	"sample_c_lz_feedback",
	"sample_clamp_feedback",
	"sample_b_clamp_feedback",
	"sample_d_clamp_feedback",
	"sample_c_clamp_feedback",
	"check_access_fully_mapped",
	// This is the END of text for WDDM1.3 opcodes
	"// EndOfWDDM1_3Text",
};
static_assert(D3D10_SB_NUM_OPCODES == sizeof(OpcodeText) / sizeof(OpcodeText[0]), "OpcodeStringMismatch");

// Text for D3D10_SB_OPERAND_TYPE
const char* OperandText[] = {
	"r", // temp register
	"v", // generic input register
	"o", // generic output register
	"x", // indexable temp register
	"l", // immediate 32bit value
	"d", // immediate 64bit value
	"s", // sampler state
	"t",
	"cb", // constant buffer
	"icb", // immediate constant buffer
	"label",
	"vPrimID", // input primitive ID
	"oDepth", // output depth
	"null", // null register
	"rasterizer",
	"oCoverageMask", // output coverage mask
	"stream",
	"functionBody",
	"functionTable",
	"interface",
	"functionInput",
	"functionOutput",
	"oControlPointID", // HS output control point ID
	"vForkInstanceID", // HS input fork instance ID
	"vJoinInstanceID",
	"vControlPoint", // HS Fork+Join, DS input control points array
	"oControlPoint",
	"vPatchConstant",
	"vDomainPoint",
	"this",
	"u", // unordered access view
	"g", // thread group shared memory
	"vThreadID", 
	"vThreadGroupID",
	"vThreadIDInGroup",
	"vCoverageMask", // PS coverage mask input
	"vThreadIDInGroupFlattened",
	"vGSInstanceID",
	"oDepthGE",
	"oDepthLE",
	"cycleCounter"
};

// Text for D3D10_SB_OPERAND_MODIFIER
const char* ModifierText[] = {
	"", // Place holder for null modifier.
	"(neg)",
	"(abs)",
	"(abs-neg)"
};

// Text for D3D11_SB_OPERAND_MIN_PRECISION
const char* MinPrecisionText[] = {
	"", // place holder for default value
	", {min16float}",
	", {min10float}",
	", {min16sint}",
	", {min16uint}"
};

// Text for D3D10_SB_INTERPOLATION_MODE
const char* InterpModeText[] = {
	" ", //place holder for undefined
	"constant",
	"linear",
	"linear_centroid",
	"linear_noperspective",
	"linear_noperspective_centroid",
	"linear_sample",
	"linear_noperspective_sample"
};

// Text for D3D10_SB_NAME
const char* NameText[] = {
	"undefined",
	"SV_POSITION",
	"SV_ClipDistance",
	"SV_CullDistance",
	"SV_RenderTargetArrayIndex",
	"SV_ViewportArrayIndex",
	"SV_VertexID",
	"SV_PrimitiveID",
	"SV_InstanceID",
	"SV_IsFrontFace",
	"sampleIndex",
	"finalQuadUEq0EdgeTessFactor",
	"finalQuadVEq0EdgeTessFactor",
	"finalQuadUEq1EdgeTessFactor",
	"finalQuadVEq1EdgeTessFactor",
	"finalQuadUInsideTessFactor",
	"finalQuadVInsideTessFactor",
	"finalTriUEq0EdgeTessFactor",
	"finalTriVEq0EdgeTessFactor",
	"fianlTriWEq0EdgeTessFactor",
	"finalTriInsideTessFactor",
	"finalLineDetailTessFactor",
	"finalLineDensityTessFactor"
};

// Text for D3D10_SB_RESOURCE_DIMENSION
const char* ResourceDimText[] = {
	"_unknown",
	"_buffer",
	"_texture1d",
	"_texture2d",
	"_texture2dMS",
	"_texture3d",
	"_textureCube",
	"_texture1dArray",
	"_texture2dArray",
	"_texture2dMSArray",
	"_textureCubeArray",
	"_textureRawBuffer",
	"_textureStructuredBuffer"
};

// Text for D3D10_SB_CUSTOMDATA_CLASS
const char* CustomDataText[] = {
	"comment",
	"debug info",
	"opaque",
	"immediate constant buffer",
	"shader message",
	"clip plane constant mappings for DX9"
};

// Text for D3D10_SB_RESOURCE_RETURN_TYPE
const char* ReturnTypeText[] = {
	" ",
	"unorm",
	"snorm",
	"sint",
	"uint",
	"float",
	"mixed",
	"double",
	"continued",
	"unused"
};

// Text for D3D10_SB_SAMPLE_MODE
const char* SampleModeText[] = {
	"mode_default",
	"mode_comparision",
	"mode_mono"
};

// Text for D3D10_SB_PRIMITIVE_TOPOLOGY
const char* PrimTopoText[] = {
	"undefined",
	"point list",
	"line list",
	"line strip",
	"triangle list",
	"triangle strip",
	"",
	"",
	"",
	"",
	"line list adj",
	"line strip adj",
	"triangle list adj",
	"triangle strip adj"
};

// Text for D3D10_SB_PRIMITIVE
const char* PrimitiveText[] = {
	"undefined",
	"point",
	"line",
	"triangle",
	"",
	"",
	"line_adj",
	"triangle_adj",
	"patch_1_control_point",
	"patch_2_control_point",
	"patch_3_control_point",
	"patch_4_control_point",
	"patch_5_control_point",
	"patch_6_control_point",
	"patch_7_control_point",
	"patch_8_control_point",
	"patch_9_control_point",
	"patch_10_control_point",
	"patch_11_control_point",
	"patch_12_control_point",
	"patch_13_control_point",
	"patch_14_control_point",
	"patch_15_control_point",
	"patch_16_control_point",
	"patch_17_control_point",
	"patch_18_control_point",
	"patch_19_control_point",
	"patch_20_control_point",
	"patch_21_control_point",
	"patch_22_control_point",
	"patch_23_control_point",
	"patch_24_control_point",
	"patch_25_control_point",
	"patch_26_control_point",
	"patch_27_control_point",
	"patch_28_control_point",
	"patch_29_control_point",
	"patch_30_control_point",
	"patch_31_control_point",
	"patch_32_control_point",
};

// Text for D3D11_SB_TESSELLATOR_DOMAIN
const char* TessDomainText[] = {
	"undefined",
	"isoline",
	"tri",
	"quad"
};

// Text for D3D11_SB_TESSELLATOR_PARTITIONING
const char* TessPartitionText[] = {
	"undefined",
	"integer",
	"pow2",
	"fractional_odd",
	"fractional_even"
};

const char* TessOutputPrimText[] = {
	"undefined",
	"point",
	"line",
	"triangle_cw",
	"triangle_ccw"
};

const OPCODE_DATA_TYPE OpcodeDataType[] = {
	OPCODE_DATA_TYPE::FLOAT, //"add",
	OPCODE_DATA_TYPE::UINT, //"and",
	OPCODE_DATA_TYPE::UNKNOWN, //"break",
	OPCODE_DATA_TYPE::UNKNOWN, //"breakc",
	OPCODE_DATA_TYPE::UNKNOWN, //"call",
	OPCODE_DATA_TYPE::UNKNOWN, //"callc",
	OPCODE_DATA_TYPE::UNKNOWN, //"case",
	OPCODE_DATA_TYPE::UNKNOWN, //"continue",
	OPCODE_DATA_TYPE::UNKNOWN, //"continuec",
	OPCODE_DATA_TYPE::UNKNOWN, //"cut",
	OPCODE_DATA_TYPE::UNKNOWN, //"default",
	OPCODE_DATA_TYPE::UNKNOWN, //"deriv_rtx",
	OPCODE_DATA_TYPE::UNKNOWN, //"deriv_rty",
	OPCODE_DATA_TYPE::UNKNOWN, //"discard",
	OPCODE_DATA_TYPE::FLOAT, //"div",
	OPCODE_DATA_TYPE::FLOAT, //"dp2",
	OPCODE_DATA_TYPE::FLOAT, //"dp3",
	OPCODE_DATA_TYPE::FLOAT, //"dp4",
	OPCODE_DATA_TYPE::UNKNOWN, //"else",
	OPCODE_DATA_TYPE::UNKNOWN, //"emit",
	OPCODE_DATA_TYPE::UNKNOWN, //"emitThenCut",
	OPCODE_DATA_TYPE::UNKNOWN, //"endif",
	OPCODE_DATA_TYPE::UNKNOWN, //"endloop",
	OPCODE_DATA_TYPE::UNKNOWN, //"endswitch",
	OPCODE_DATA_TYPE::FLOAT, //"eq",
	OPCODE_DATA_TYPE::FLOAT, //"exp",
	OPCODE_DATA_TYPE::FLOAT, //"frc",
	OPCODE_DATA_TYPE::UNKNOWN, //"ftoi",
	OPCODE_DATA_TYPE::UNKNOWN, //"ftou",
	OPCODE_DATA_TYPE::FLOAT, //"ge",
	OPCODE_DATA_TYPE::SINT, //"iadd",
	OPCODE_DATA_TYPE::SINT, //"if",
	OPCODE_DATA_TYPE::SINT, //"ieq",
	OPCODE_DATA_TYPE::SINT, //"ige",
	OPCODE_DATA_TYPE::SINT, //"ilt",
	OPCODE_DATA_TYPE::SINT, //"imad",
	OPCODE_DATA_TYPE::SINT, //"imax",
	OPCODE_DATA_TYPE::SINT, //"imin",
	OPCODE_DATA_TYPE::SINT, //"imul",
	OPCODE_DATA_TYPE::SINT, //"ine",
	OPCODE_DATA_TYPE::SINT, //"ineg",
	OPCODE_DATA_TYPE::SINT, //"ishl",
	OPCODE_DATA_TYPE::SINT, //"ishr",
	OPCODE_DATA_TYPE::SINT, //"itof",
	OPCODE_DATA_TYPE::UNKNOWN, //"label",
	OPCODE_DATA_TYPE::SINT, //"ld",
	OPCODE_DATA_TYPE::SINT, //"ld2dms",
	OPCODE_DATA_TYPE::FLOAT, //"log",
	OPCODE_DATA_TYPE::UNKNOWN, //"loop",
	OPCODE_DATA_TYPE::FLOAT, //"lt",
	OPCODE_DATA_TYPE::FLOAT, //"mad",
	OPCODE_DATA_TYPE::FLOAT, //"min",
	OPCODE_DATA_TYPE::FLOAT, //"max",
	OPCODE_DATA_TYPE::UNKNOWN, //"CustomData", // For custome data formats
	OPCODE_DATA_TYPE::FLOAT, //"mov",
	OPCODE_DATA_TYPE::FLOAT, //"movc",
	OPCODE_DATA_TYPE::FLOAT, //"mul",
	OPCODE_DATA_TYPE::FLOAT, //"ne",
	OPCODE_DATA_TYPE::UNKNOWN, //"nop",
	OPCODE_DATA_TYPE::UNKNOWN, //"not",
	OPCODE_DATA_TYPE::SINT, //"or",
	OPCODE_DATA_TYPE::UNKNOWN, //"resinfo",
	OPCODE_DATA_TYPE::UNKNOWN, //"ret",
	OPCODE_DATA_TYPE::UNKNOWN, //"retc",
	OPCODE_DATA_TYPE::FLOAT, //"round_ne",
	OPCODE_DATA_TYPE::FLOAT, //"round_ni",
	OPCODE_DATA_TYPE::FLOAT, //"round_pi",
	OPCODE_DATA_TYPE::FLOAT, //"round_z",
	OPCODE_DATA_TYPE::FLOAT, //"rsq",
	OPCODE_DATA_TYPE::FLOAT, //"sample",
	OPCODE_DATA_TYPE::FLOAT, //"sample_c",
	OPCODE_DATA_TYPE::FLOAT, //"sample_c_lz",
	OPCODE_DATA_TYPE::FLOAT, //"sample_l",
	OPCODE_DATA_TYPE::FLOAT, //"sample_d",
	OPCODE_DATA_TYPE::FLOAT, //"sample_b",
	OPCODE_DATA_TYPE::FLOAT, //"sqrt",
	OPCODE_DATA_TYPE::UNKNOWN, //"switch",
	OPCODE_DATA_TYPE::FLOAT, //"sincos",
	OPCODE_DATA_TYPE::UINT, //"udiv",
	OPCODE_DATA_TYPE::UINT, //"ult",
	OPCODE_DATA_TYPE::UINT, //"uge",
	OPCODE_DATA_TYPE::UINT, //"umul",
	OPCODE_DATA_TYPE::UINT, //"umad",
	OPCODE_DATA_TYPE::UINT, //"umax",
	OPCODE_DATA_TYPE::UINT, //"umin",
	OPCODE_DATA_TYPE::UINT, //"ushr",
	OPCODE_DATA_TYPE::UINT, //"utof",
	OPCODE_DATA_TYPE::UINT, //"xor",
	OPCODE_DATA_TYPE::UNKNOWN, //"dcl_resource",
	OPCODE_DATA_TYPE::UNKNOWN, //"dcl_constantBuffer",
	OPCODE_DATA_TYPE::UNKNOWN, //"dcl_sampler",
	OPCODE_DATA_TYPE::UNKNOWN, //"dcl_indexRange",
	OPCODE_DATA_TYPE::UNKNOWN, //"dcl_gsOutputTopology",
	OPCODE_DATA_TYPE::UNKNOWN, //"dcl_gsInputPrimitive",
	OPCODE_DATA_TYPE::UNKNOWN, //"dcl_maxOutputVertexCount",
	OPCODE_DATA_TYPE::UNKNOWN, //"dcl_input",
	OPCODE_DATA_TYPE::UNKNOWN, //"dcl_input_sgv",
	OPCODE_DATA_TYPE::UNKNOWN, //"dcl_input_siv",
	OPCODE_DATA_TYPE::UNKNOWN, //"dcl_input_ps",
	OPCODE_DATA_TYPE::UNKNOWN, //"dcl_input_ps_sgv",
	OPCODE_DATA_TYPE::UNKNOWN, //"dcl_input_ps_siv",
	OPCODE_DATA_TYPE::UNKNOWN, //"dcl_output",
	OPCODE_DATA_TYPE::UNKNOWN, //"dcl_output_sgv",
	OPCODE_DATA_TYPE::UNKNOWN, //"dcl_output_siv",
	OPCODE_DATA_TYPE::UNKNOWN, //"dcl_temps",
	OPCODE_DATA_TYPE::UNKNOWN, //"dcl_indexableTemp",
	OPCODE_DATA_TYPE::UNKNOWN, //"dcl_globalFlags",
	OPCODE_DATA_TYPE::UNKNOWN, //"// EndOfD3D10Text",
	OPCODE_DATA_TYPE::UNKNOWN, //"lod",
	OPCODE_DATA_TYPE::UNKNOWN, //"gather4",
	OPCODE_DATA_TYPE::UNKNOWN, //"samplepos",
	OPCODE_DATA_TYPE::UNKNOWN, //"sampleinfo",
	OPCODE_DATA_TYPE::UNKNOWN, //"// EndOfD3D10_1Text",
	OPCODE_DATA_TYPE::UNKNOWN, //"hs_decls",
	OPCODE_DATA_TYPE::UNKNOWN, //"hs_control_point_phase",
	OPCODE_DATA_TYPE::UNKNOWN, //"hs_fork_phase",
	OPCODE_DATA_TYPE::UNKNOWN, //"hs_join_phase",
	OPCODE_DATA_TYPE::UNKNOWN, //"emit_stream",
	OPCODE_DATA_TYPE::UNKNOWN, //"cut_stream",
	OPCODE_DATA_TYPE::UNKNOWN, //"emitThenCut_stream",
	OPCODE_DATA_TYPE::UNKNOWN, //"interface_call",// No exact match in MSDN
	OPCODE_DATA_TYPE::UNKNOWN, //"bufinfo",
	OPCODE_DATA_TYPE::UNKNOWN, //"deriv_rtx_coarse",
	OPCODE_DATA_TYPE::UNKNOWN, //"deriv_rtx_fine",
	OPCODE_DATA_TYPE::UNKNOWN, //"deriv_rty_coarse",
	OPCODE_DATA_TYPE::UNKNOWN, //"deriv_rty_fine",
	OPCODE_DATA_TYPE::UNKNOWN, //"gather4_c",
	OPCODE_DATA_TYPE::UNKNOWN, //"gather4_po",
	OPCODE_DATA_TYPE::UNKNOWN, //"gather4_po_c",
	OPCODE_DATA_TYPE::UNKNOWN, //"rcp",
	OPCODE_DATA_TYPE::FLOAT, //"f32to16",
	OPCODE_DATA_TYPE::FLOAT, //"f16to32",
	OPCODE_DATA_TYPE::SINT, //"uaddc",
	OPCODE_DATA_TYPE::SINT, //"usubb",
	OPCODE_DATA_TYPE::SINT, //"countbits",
	OPCODE_DATA_TYPE::SINT, //"firstbit_hi",
	OPCODE_DATA_TYPE::SINT, //"firstbit_lo",
	OPCODE_DATA_TYPE::SINT, //"firstbit_shi",
	OPCODE_DATA_TYPE::SINT, //"ubfe",
	OPCODE_DATA_TYPE::SINT, //"ibfe",
	OPCODE_DATA_TYPE::SINT, //"bfi",
	OPCODE_DATA_TYPE::UNKNOWN, //"bfrev",
	OPCODE_DATA_TYPE::UNKNOWN, //"swapc",
	OPCODE_DATA_TYPE::UNKNOWN, //"dcl_stream",
	OPCODE_DATA_TYPE::UNKNOWN, //"dcl_function_body",
	OPCODE_DATA_TYPE::UNKNOWN, //"dcl_function_table",
	OPCODE_DATA_TYPE::UNKNOWN, //"dcl_interface",
	OPCODE_DATA_TYPE::UNKNOWN, //"dcl_input_control_point_count",
	OPCODE_DATA_TYPE::UNKNOWN, //"dcl_output_control_point_count",
	OPCODE_DATA_TYPE::UNKNOWN, //"dcl_tessellator_domain",
	OPCODE_DATA_TYPE::UNKNOWN, //"dcl_tessellator_partitioning",
	OPCODE_DATA_TYPE::UNKNOWN, //"dcl_tessellator_output_primitive",
	OPCODE_DATA_TYPE::UNKNOWN, //"dcl_hs_max_factor",
	OPCODE_DATA_TYPE::UNKNOWN, //"dcl_hs_fork_phase_instance_count",
	OPCODE_DATA_TYPE::UNKNOWN, //"dcl_hs_join_phase_instance_count",
	OPCODE_DATA_TYPE::UNKNOWN, //"dcl_thread_group",
	OPCODE_DATA_TYPE::UNKNOWN, //"dcl_uav_typed",
	OPCODE_DATA_TYPE::UNKNOWN, //"dcl_uav_raw",
	OPCODE_DATA_TYPE::UNKNOWN, //"dcl_uav_structured",
	OPCODE_DATA_TYPE::UNKNOWN, //"dcl_tgsm_raw",
	OPCODE_DATA_TYPE::UNKNOWN, //"dcl_tgsm_structured",
	OPCODE_DATA_TYPE::UNKNOWN, //"dcl_resource_raw",
	OPCODE_DATA_TYPE::UNKNOWN, //"dcl_resource_structured",
	OPCODE_DATA_TYPE::SINT, //"ld_uav_raw",
	OPCODE_DATA_TYPE::SINT, //"store_uav_raw",
	OPCODE_DATA_TYPE::SINT, //"ld_raw",
	OPCODE_DATA_TYPE::SINT, //"store_raw",
	OPCODE_DATA_TYPE::SINT, //"ld_structured",
	OPCODE_DATA_TYPE::SINT, //"store_structured",
	OPCODE_DATA_TYPE::SINT, //"atomic_and",
	OPCODE_DATA_TYPE::SINT, //"atomic_or",
	OPCODE_DATA_TYPE::SINT, //"atomic_xor",
	OPCODE_DATA_TYPE::SINT, //"atomic_cmp_store",
	OPCODE_DATA_TYPE::SINT, //"atomic_iadd",
	OPCODE_DATA_TYPE::SINT, //"atomic_imax",
	OPCODE_DATA_TYPE::SINT, //"atomic_imin",
	OPCODE_DATA_TYPE::UINT, //"atomic_umax",
	OPCODE_DATA_TYPE::UINT, //"atomic_umin",
	OPCODE_DATA_TYPE::SINT, //"imm_atomic_alloc",
	OPCODE_DATA_TYPE::SINT, //"imm_atomic_consume",
	OPCODE_DATA_TYPE::SINT, //"imm_atomic_iadd",
	OPCODE_DATA_TYPE::SINT, //"imm_atomic_and",
	OPCODE_DATA_TYPE::SINT, //"imm_atomic_or",
	OPCODE_DATA_TYPE::SINT, //"imm_atomic_xor",
	OPCODE_DATA_TYPE::SINT, //"imm_atomic_exch",
	OPCODE_DATA_TYPE::SINT, //"imm_atomic_cmp_exch",
	OPCODE_DATA_TYPE::SINT, //"imm_atomic_imax",
	OPCODE_DATA_TYPE::SINT, //"imm_atomic_imin",
	OPCODE_DATA_TYPE::UINT, //"imm_atomic_umax",
	OPCODE_DATA_TYPE::UINT, //"imm_atomic_umin",
	OPCODE_DATA_TYPE::UNKNOWN, //"symc",
	OPCODE_DATA_TYPE::DOUBLE, //"dadd",
	OPCODE_DATA_TYPE::DOUBLE, //"dmax",
	OPCODE_DATA_TYPE::DOUBLE, //"dmin",
	OPCODE_DATA_TYPE::DOUBLE, //"dmul",
	OPCODE_DATA_TYPE::DOUBLE, //"deq",
	OPCODE_DATA_TYPE::DOUBLE, //"dge",
	OPCODE_DATA_TYPE::DOUBLE, //"dlt",
	OPCODE_DATA_TYPE::DOUBLE, //"dne",
	OPCODE_DATA_TYPE::DOUBLE, //"dmov",
	OPCODE_DATA_TYPE::DOUBLE, //"dmovc",
	OPCODE_DATA_TYPE::DOUBLE, //"dtof",
	OPCODE_DATA_TYPE::FLOAT, //"ftod",
	OPCODE_DATA_TYPE::UNKNOWN, //"eval_snapped",
	OPCODE_DATA_TYPE::UNKNOWN, //"eval_sample_index",
	OPCODE_DATA_TYPE::UNKNOWN, //"eval_centroid",
	OPCODE_DATA_TYPE::UNKNOWN, //"dcl_gs_instance_count",
	OPCODE_DATA_TYPE::UNKNOWN, //"abort",
	OPCODE_DATA_TYPE::UNKNOWN, //"debug_break",
	OPCODE_DATA_TYPE::UNKNOWN, //"// EndOfD3D11Text",
	OPCODE_DATA_TYPE::DOUBLE, //"ddiv",
	OPCODE_DATA_TYPE::DOUBLE, //"dfma",
	OPCODE_DATA_TYPE::DOUBLE, //"drcp",
	OPCODE_DATA_TYPE::UNKNOWN, //"msad",
	OPCODE_DATA_TYPE::DOUBLE, //"dtoi",
	OPCODE_DATA_TYPE::DOUBLE, //"dtou",
	OPCODE_DATA_TYPE::DOUBLE, //"itod",
	OPCODE_DATA_TYPE::DOUBLE, //"utod",
	OPCODE_DATA_TYPE::UNKNOWN, //"//EndOfD3D11_1Text",
	OPCODE_DATA_TYPE::UNKNOWN, //"gather4_feedback",
	OPCODE_DATA_TYPE::UNKNOWN, //"gather4_c_feedback",
	OPCODE_DATA_TYPE::UNKNOWN, //"gather4_po_feedback",
	OPCODE_DATA_TYPE::UNKNOWN, //"gather4_po_c_feedback",
	OPCODE_DATA_TYPE::UNKNOWN, //"ld_feedback",
	OPCODE_DATA_TYPE::UNKNOWN, //"ld_ms_feedback",
	OPCODE_DATA_TYPE::UNKNOWN, //"ld_uav_typed_feedback",
	OPCODE_DATA_TYPE::UNKNOWN, //"ld_raw_feedback",
	OPCODE_DATA_TYPE::UNKNOWN, //"ld_structured_feedback",
	OPCODE_DATA_TYPE::UNKNOWN, //"sample_l_feedback",
	OPCODE_DATA_TYPE::UNKNOWN, //"sample_c_lz_feedback",
	OPCODE_DATA_TYPE::UNKNOWN, //"sample_clamp_feedback",
	OPCODE_DATA_TYPE::UNKNOWN, //"sample_b_clamp_feedback",
	OPCODE_DATA_TYPE::UNKNOWN, //"sample_d_clamp_feedback",
	OPCODE_DATA_TYPE::UNKNOWN, //"sample_c_clamp_feedback",
	OPCODE_DATA_TYPE::UNKNOWN, //"check_access_fully_mapped",
	OPCODE_DATA_TYPE::UNKNOWN, //"// EndOfWDDM1_3Text",
};
static_assert(D3D10_SB_NUM_OPCODES == sizeof(OpcodeDataType) / sizeof(OpcodeDataType[0]), "OpcodeDataType mismatch with opcode numbers");