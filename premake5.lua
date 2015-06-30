solution "fxdis-d3d1x"
	configurations { "Debug", "Release" }

	project "fxdis-d3d1x"
		kind "StaticLib"
		language "C++"
		targetdir ""

		configuration { "gmake" }
			buildoptions { "-std=c++11" }

		includedirs { "include/" }
		files { "include/**.hpp", "src/**.cpp" }

		filter "configurations:Debug"
			defines { "DEBUG" }
			flags { "Symbols" }

		filter "configurations:Release"
			defines { "NDEBUG" }
			optimize "On"
