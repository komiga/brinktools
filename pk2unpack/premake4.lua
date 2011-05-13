--brinktools pk2unpack premake file

local name="pk2unpack"
local outpath="out/"
local execpath=outpath..name

if _ACTION == "clean" then
	os.rmdir(outpath)
	os.remove("pk2unpack")
end

solution("pk2unpack")
	configurations { "debug", "release" }

local proj=project(name)
proj.language="C++"
proj.kind="ConsoleApp"

configuration {"debug"}
	targetdir(outpath)
	objdir(outpath)
	defines {"DEBUG", "_DEBUG"}
	flags {"Symbols", "ExtraWarnings"}

configuration {"release"}
	targetdir(outpath)
	objdir(outpath)
	defines{"NDEBUG", "RELEASE"}
	flags {"Optimize", "ExtraWarnings"}

configuration {"gmake"}
	links {"duct", "icui18n", "icudata", "icuio", "icuuc"}
	postbuildcommands {"cp "..execpath.." ../"..name}

configuration {"linux"}
	defines{"PLATFORM_CHECKED", "UNIX_BUILD"}

configuration {}

files {"include/*.hpp", "src/*.cpp"}
includedirs {
	"include/"
}
