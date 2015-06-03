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

#include "dxbc.h"
#include "sm4.h"

#include "sm4_dump_visitor.h"
#include "sm4_text_visitor.h"
#include "sm4_rewrite_visitor.h"

#include <iostream>
#include <fstream>
#include <memory>

void usage()
{
    std::cerr << "Gallium Direct3D10/11 Shader Disassembler\n";
    std::cerr << "This program is free software, released under a MIT-like license\n";
    std::cerr << "Not affiliated with or endorsed by Microsoft in any way\n";
    std::cerr << "Latest version available from http://cgit.freedesktop.org/mesa/mesa/\n";
    std::cerr << "\n";
    std::cerr << "Usage: fxdis FILE [--dont-process] [--decompile] [--dump-ast]\n";
    std::cerr << std::endl;
}

int main(int argc, char** argv)
{
    if(argc < 2)
    {
        usage();
        return EXIT_FAILURE;
    }

	bool decompile = false;
	bool dump_ast = false;
	bool process = true;

	if (argc > 2)
	{
		for (int i = 2; i < argc; ++i)
		{
			if (strcmp(argv[i], "--decompile") == 0)
			{
				decompile = true;
			}
			else if (strcmp(argv[i], "--dump-ast") == 0)
			{
				dump_ast = true;
			}
			else if (strcmp(argv[i], "--dont-process") == 0)
			{
				process = false;
			}
			else
			{
				std::cerr << "Unrecognized switch: " << argv[i] << "\n";
				usage();
				return EXIT_FAILURE;
			}
		}
	}

    std::vector<char> data;
    FILE *pFile = NULL;
#ifdef _MSC_VER
    fopen_s(&pFile, argv[1], "rb" );
#else
    pFile = fopen(argv[1], "rb" );
#endif
    if ( !pFile )
    {
       printf("Could not open file: %s\n", pFile );
       return EXIT_FAILURE;
    }

    fseek(pFile, 0, SEEK_END);
    uint32_t nFileSize = ftell(pFile);
    fseek(pFile, 0, SEEK_SET);

    if (nFileSize < sizeof(dxbc_container_header))
    {
      printf("File is too small!\n");
      return EXIT_FAILURE;
    }

    data.resize(nFileSize);
    if (fread(&data[0], 1, nFileSize, pFile) != nFileSize)
    {
       printf("Failed reading file!\n");
       return EXIT_FAILURE;
    }
    fclose(pFile);

    std::unique_ptr<dxbc_container> dxbc(dxbc_parse(data.data(), data.size()));
	if (!dxbc)
	{
		std::cerr << "Failed to parse DXBC!\n";
		return EXIT_FAILURE;
	}

	dxbc_chunk_header* sm4_chunk(dxbc_find_shader_bytecode(data.data(), data.size()));
	if (!sm4_chunk)
	{
		std::cerr << "Failed to parse SM4 chunk header!\n";
		return EXIT_FAILURE;
	}

	std::unique_ptr<sm4::program> sm4_p(sm4_parse(sm4_chunk + 1, bswap_le32(sm4_chunk->size)));
	if (!sm4_p)
	{
		std::cerr << "Failed to parse SM4 program!\n";
		return EXIT_FAILURE;
	}

	if (decompile)
	{
		auto root_node = sm4::decompile(sm4_p.get());
		
		if (process)
		{
			sm4::rewrite_visitor sub_visitor;

			// pass 1
			root_node->accept(sub_visitor);
		}

		if (dump_ast)
		{
			sm4::dump_visitor visitor(std::cout);
			root_node->accept(visitor);
		}
		else
		{
			sm4::text_visitor visitor(std::cout);
			root_node->accept(visitor);
		}
	}
	else
	{
		std::cout << *sm4_p;
	}

    return EXIT_SUCCESS;
}