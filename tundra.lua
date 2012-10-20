local common = {
	Env = {
		CPPPATH = { ".", "editor", "$(OBJECTDIR)", "external/glew/include" },
		R6_INIGENTOOL = "$(OBJECTDIR)$(SEP)cg-tools$(SEP)inicodegen$(HOSTPROGSUFFIX)",
		CSCOPTS = { "-debug+" },
		CSLIBPATH = { "external/opentk" },

		CCOPTS = {
			-- clang and GCC
			{ "-g"; Config = { "*-gcc-debug", "*-clang-debug" } },
			{ "-g -O2"; Config = { "*-gcc-production", "*-clang-production" } },
			{ "-O3"; Config = { "*-gcc-release", "*-clang-release" } },
			{ "-Wall", "-Werror", "-Wextra", "-Wno-unused-parameter", "-Wno-unused-function"; Config = { "*-gcc-*", "*-clang-*" } },

			-- MSVC config
			{ "/MD"; Config = "*-msvc-debug" },
			{ "/MT"; Config = { "*-msvc-production", "*-msvc-release" } },
			{
				"/wd4127", -- conditional expression is constant
				"/wd4100", -- unreferenced formal parameter
				"/wd4324", -- structure was padded due to __declspec(align())
				Config = "*-msvc-*"
			},
		},

		CPPDEFS = {
			{ "RS_BUILD_FEAT_CHECKED", "RS_BUILD_FEAT_LOGGING", "RS_BUILD_FEAT_DEVELOPER", "RS_BUILD_FEAT_DEBUG_REND", "_DEBUG"; Config = "*-*-debug" },
			{ "RS_BUILD_FEAT_CHECKED", "RS_BUILD_FEAT_LOGGING", "RS_BUILD_FEAT_DEVELOPER", "RS_BUILD_FEAT_DEBUG_REND"; Config = "*-*-production" },
			{ "NDEBUG"; Config = "*-*-release" },
			{ "_CRT_SECURE_NO_WARNINGS"; Config = "*-msvc-*" },
		},
	},
}

Build {
	Configs = {
		Config {
			Name = "generic-gcc",
			DefaultOnHost = "linux",
			Tools = { "gcc" },
			Inherit = common,
		},
		Config {
			Name = "macosx-gcc",
			DefaultOnHost = "macosx",
			Tools = { "gcc-osx" },
			Inherit = common,
		},
		Config {
			Name = "win64-msvc",
			DefaultOnHost = "windows",
			Tools = { "msvc-vs2008"; TargetPlatform = "x64" },
			Inherit = common,
		},
	},
	Units = function()
		local mmtest = Program {
			Name = "mmtest",
			Sources = { "json.c", "main.c" },
		}

		Default(mmtest)
	end,
}
