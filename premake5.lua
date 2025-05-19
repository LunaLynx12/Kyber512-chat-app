workspace "QuantumSecureChat"
    configurations { "Debug", "Release" }
    architecture "x86_64"
    location "build"
    cppdialect "C++17"
    warnings "Extra"
    
    targetdir "bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
    objdir "obj/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "SecureChatApp"
    kind "ConsoleApp"
    language "C++"
    
    files { 
        "src/**.cpp",
        "src/**.c",
        "include/**.h",
        "include/**.hpp"
    }
    
    includedirs { 
        "include",
        "/usr/include/lua5.3",
        "/usr/include/oqs",
        "/usr/local/include"  -- Path for liboqs headers
    }

    filter "system:linux"
        defines { "LINUX" }
        links { 
            "ssl", 
            "crypto", 
            "yara", 
            "lua5.3", 
            "nice", 
            "glib-2.0",
            "pthread",
            "oqs" -- Link against liboqs
        }
        buildoptions { 
            "-Wall",
            "-Wextra",
            "-fPIC",
            "`pkg-config --cflags glib-2.0 nice`"
        }
        linkoptions { 
            "`pkg-config --libs glib-2.0 nice`"
        }

    filter "configurations:Debug"
        defines { "DEBUG", "_DEBUG" }
        symbols "On"
        optimize "Debug"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "Speed"
        flags { "LinkTimeOptimization" }
        -- Removed the strip command as it's not needed in premake5