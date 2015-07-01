solution "fxdis-d3d1x"
	configurations { "Debug", "Release" }

	project "fxdis-d3d1x"
		kind "StaticLib"
		language "C++"
		targetdir ""

		architecture "x64"

		includedirs { "include/" }
		files { "include/**.hpp", "src/**.cpp" }
		flags { "StaticRuntime" }

		filter "configurations:Debug"
			defines { "DEBUG" }
			flags { "Symbols" }

		filter "configurations:Release"
			defines { "NDEBUG" }
			optimize "On"

		configuration { "gmake" }
			buildoptions { "-std=c++11" }