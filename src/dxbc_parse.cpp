/**************************************************************************
 *
 * Copyright 2010 Luca Barbieri
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE COPYRIGHT OWNER(S) AND/OR ITS SUPPLIERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#include <memory>
#include "dxbc.h"

typedef 
enum D3D_NAME
{
    D3D_NAME_UNDEFINED	= 0,
    D3D_NAME_POSITION	= 1,
    D3D_NAME_CLIP_DISTANCE	= 2,
    D3D_NAME_CULL_DISTANCE	= 3,
    D3D_NAME_RENDER_TARGET_ARRAY_INDEX	= 4,
    D3D_NAME_VIEWPORT_ARRAY_INDEX	= 5,
    D3D_NAME_VERTEX_ID	= 6,
    D3D_NAME_PRIMITIVE_ID	= 7,
    D3D_NAME_INSTANCE_ID	= 8,
    D3D_NAME_IS_FRONT_FACE	= 9,
    D3D_NAME_SAMPLE_INDEX	= 10,
    D3D_NAME_FINAL_QUAD_EDGE_TESSFACTOR	= 11,
    D3D_NAME_FINAL_QUAD_INSIDE_TESSFACTOR	= 12,
    D3D_NAME_FINAL_TRI_EDGE_TESSFACTOR	= 13,
    D3D_NAME_FINAL_TRI_INSIDE_TESSFACTOR	= 14,
    D3D_NAME_FINAL_LINE_DETAIL_TESSFACTOR	= 15,
    D3D_NAME_FINAL_LINE_DENSITY_TESSFACTOR	= 16,
    D3D_NAME_TARGET	= 64,
    D3D_NAME_DEPTH	= 65,
    D3D_NAME_COVERAGE	= 66,
    D3D_NAME_DEPTH_GREATER_EQUAL	= 67,
    D3D_NAME_DEPTH_LESS_EQUAL	= 68,
    D3D10_NAME_UNDEFINED	= D3D_NAME_UNDEFINED,
    D3D10_NAME_POSITION	= D3D_NAME_POSITION,
    D3D10_NAME_CLIP_DISTANCE	= D3D_NAME_CLIP_DISTANCE,
    D3D10_NAME_CULL_DISTANCE	= D3D_NAME_CULL_DISTANCE,
    D3D10_NAME_RENDER_TARGET_ARRAY_INDEX	= D3D_NAME_RENDER_TARGET_ARRAY_INDEX,
    D3D10_NAME_VIEWPORT_ARRAY_INDEX	= D3D_NAME_VIEWPORT_ARRAY_INDEX,
    D3D10_NAME_VERTEX_ID	= D3D_NAME_VERTEX_ID,
    D3D10_NAME_PRIMITIVE_ID	= D3D_NAME_PRIMITIVE_ID,
    D3D10_NAME_INSTANCE_ID	= D3D_NAME_INSTANCE_ID,
    D3D10_NAME_IS_FRONT_FACE	= D3D_NAME_IS_FRONT_FACE,
    D3D10_NAME_SAMPLE_INDEX	= D3D_NAME_SAMPLE_INDEX,
    D3D10_NAME_TARGET	= D3D_NAME_TARGET,
    D3D10_NAME_DEPTH	= D3D_NAME_DEPTH,
    D3D10_NAME_COVERAGE	= D3D_NAME_COVERAGE,
    D3D11_NAME_FINAL_QUAD_EDGE_TESSFACTOR	= D3D_NAME_FINAL_QUAD_EDGE_TESSFACTOR,
    D3D11_NAME_FINAL_QUAD_INSIDE_TESSFACTOR	= D3D_NAME_FINAL_QUAD_INSIDE_TESSFACTOR,
    D3D11_NAME_FINAL_TRI_EDGE_TESSFACTOR	= D3D_NAME_FINAL_TRI_EDGE_TESSFACTOR,
    D3D11_NAME_FINAL_TRI_INSIDE_TESSFACTOR	= D3D_NAME_FINAL_TRI_INSIDE_TESSFACTOR,
    D3D11_NAME_FINAL_LINE_DETAIL_TESSFACTOR	= D3D_NAME_FINAL_LINE_DETAIL_TESSFACTOR,
    D3D11_NAME_FINAL_LINE_DENSITY_TESSFACTOR	= D3D_NAME_FINAL_LINE_DENSITY_TESSFACTOR,
    D3D11_NAME_DEPTH_GREATER_EQUAL	= D3D_NAME_DEPTH_GREATER_EQUAL,
    D3D11_NAME_DEPTH_LESS_EQUAL	= D3D_NAME_DEPTH_LESS_EQUAL
} 	D3D_NAME;

typedef 
enum D3D_REGISTER_COMPONENT_TYPE
{
    D3D_REGISTER_COMPONENT_UNKNOWN	= 0,
    D3D_REGISTER_COMPONENT_UINT32	= 1,
    D3D_REGISTER_COMPONENT_SINT32	= 2,
    D3D_REGISTER_COMPONENT_FLOAT32	= 3,
    D3D10_REGISTER_COMPONENT_UNKNOWN	= D3D_REGISTER_COMPONENT_UNKNOWN,
    D3D10_REGISTER_COMPONENT_UINT32	= D3D_REGISTER_COMPONENT_UINT32,
    D3D10_REGISTER_COMPONENT_SINT32	= D3D_REGISTER_COMPONENT_SINT32,
    D3D10_REGISTER_COMPONENT_FLOAT32	= D3D_REGISTER_COMPONENT_FLOAT32
} 	D3D_REGISTER_COMPONENT_TYPE;

typedef 
enum D3D_MIN_PRECISION
{
    D3D_MIN_PRECISION_DEFAULT	= 0,
    D3D_MIN_PRECISION_FLOAT_16	= 1,
    D3D_MIN_PRECISION_FLOAT_2_8	= 2,
    D3D_MIN_PRECISION_RESERVED	= 3,
    D3D_MIN_PRECISION_SINT_16	= 4,
    D3D_MIN_PRECISION_UINT_16	= 5,
    D3D_MIN_PRECISION_ANY_16	= 0xf0,
    D3D_MIN_PRECISION_ANY_10	= 0xf1
} 	D3D_MIN_PRECISION;

typedef char* LPCSTR;
typedef uint32_t UINT;
typedef uint8_t BYTE;

typedef struct _D3D11_SIGNATURE_PARAMETER_DESC
{
    LPCSTR                      SemanticName;   // Name of the semantic
    UINT                        SemanticIndex;  // Index of the semantic
    UINT                        Register;       // Number of member variables
    D3D_NAME                    SystemValueType;// A predefined system value, or D3D_NAME_UNDEFINED if not applicable
    D3D_REGISTER_COMPONENT_TYPE ComponentType;  // Scalar type (e.g. uint, float, etc.)
    BYTE                        Mask;           // Mask to indicate which components of the register
                                                // are used (combination of D3D10_COMPONENT_MASK values)
    BYTE                        ReadWriteMask;  // Mask to indicate whether a given component is 
                                                // never written (if this is an output signature) or
                                                // always read (if this is an input signature).
                                                // (combination of D3D_MASK_* values)
    UINT                        Stream;         // Stream index
    D3D_MIN_PRECISION           MinPrecision;   // Minimum desired interpolation precision
} D3D11_SIGNATURE_PARAMETER_DESC;

dxbc_container* dxbc_parse(const void* data, int size)
{
   std::auto_ptr<dxbc_container> container(new dxbc_container());
   container->data = data;
   dxbc_container_header* header = (dxbc_container_header*)data;
   uint32_t* chunk_offsets = (uint32_t*)(header + 1);
   if(bswap_le32(header->fourcc) != FOURCC_DXBC)
      return 0;
   unsigned num_chunks = bswap_le32(header->chunk_count);
   for(unsigned i = 0; i < num_chunks; ++i)
   {
      unsigned offset = bswap_le32(chunk_offsets[i]);
      dxbc_chunk_header* chunk = (dxbc_chunk_header*)((char*)data + offset);
      unsigned fourcc = bswap_le32(chunk->fourcc);
      container->chunk_map[fourcc] = i;
      container->chunks.push_back(chunk);
   }
   return container.release();
}

dxbc_chunk_header* dxbc_find_chunk(const void* data, int size, unsigned fourcc)
{
   dxbc_container_header* header = (dxbc_container_header*)data;
   uint32_t* chunk_offsets = (uint32_t*)(header + 1);
   if(bswap_le32(header->fourcc) != FOURCC_DXBC)
      return 0;
   unsigned num_chunks = bswap_le32(header->chunk_count);
   for(unsigned i = 0; i < num_chunks; ++i)
   {
      unsigned offset = bswap_le32(chunk_offsets[i]);
      dxbc_chunk_header* chunk = (dxbc_chunk_header*)((char*)data + offset);
      if(bswap_le32(chunk->fourcc) == fourcc)
         return chunk;
   }
   return 0;
}

int dxbc_parse_signature(dxbc_chunk_signature* sig, D3D11_SIGNATURE_PARAMETER_DESC** params)
{
   unsigned count = bswap_le32(sig->count);
   *params = (D3D11_SIGNATURE_PARAMETER_DESC*)malloc(sizeof(D3D11_SIGNATURE_PARAMETER_DESC) * count);

   for (unsigned i = 0; i < count; ++i)
   {
      D3D11_SIGNATURE_PARAMETER_DESC& param = (*params)[i];
      param.SemanticName = (char*)&sig->count + bswap_le32(sig->elements[i].name_offset);
      param.SemanticIndex = bswap_le32(sig->elements[i].semantic_index);
      param.SystemValueType = (D3D_NAME)bswap_le32(sig->elements[i].system_value_type);
      param.ComponentType = (D3D_REGISTER_COMPONENT_TYPE)bswap_le32(sig->elements[i].component_type);
      param.Register = bswap_le32(sig->elements[i].register_num);
      param.Mask = sig->elements[i].mask;
      param.ReadWriteMask = sig->elements[i].read_write_mask;
      param.Stream = sig->elements[i].stream;
   }
   return count;
}
