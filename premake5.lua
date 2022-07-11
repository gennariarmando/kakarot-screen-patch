workspace "kakarot-screen-patch"
	configurations { "Release", "Debug" }
	platforms { "Win64" }
	architecture "x64"
	location "project_files"
   
project "kakarot-screen-patch"
	files {
		"source/**.*",
	}
	
	includedirs { 
		"source/**"
	}
	
	kind "SharedLib"
	language "C++"
	targetdir "output/asi"
	objdir ("output/obj")
	targetextension ".asi"
	characterset ("MBCS")
	linkoptions "/SAFESEH:NO"
	buildoptions { "-std:c++latest", "/Zc:threadSafeInit-", "/Zc:strictStrings" }
	defines { "_CRT_SECURE_NO_WARNINGS", "_CRT_NON_CONFORMING_SWPRINTFS", "_USE_MATH_DEFINES" }
    disablewarnings { "4244", "4800", "4305", "4073", "4838", "4996", "4221", "4430" }
	
	filter "configurations:Debug"
		targetname "KakarotScreenPatch"
		defines { "DEBUG" }
		staticruntime "on"
		symbols "On"
		debugdir "$(DBZ_KAKAROT_DIR)"
		debugcommand "$(DBZ_KAKAROT_DIR)/AT/Binaries/Win64/AT-Win64-Shipping.exe"

	filter "configurations:Release"

		targetname "KakarotScreenPatch"
		defines { "NDEBUG" }
		optimize "On"
		staticruntime "on"
		debugdir "$(DBZ_KAKAROT_DIR)"
		debugcommand "$(DBZ_KAKAROT_DIR)/AT/Binaries/Win64/AT-Win64-Shipping.exe"
