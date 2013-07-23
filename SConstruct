# scons 2.7
import platform
import os

# set up your MinGW path here
mingw_path = "C:/DEV/MinGW32-4.8/mingw32/"
# set your Engine path here
engine_path = "C:/PROJECTS/Engine/"
# set your Boost path here
boost_path = "C:/DEV/boost_1_52_0/"
# set your Lua path here
lua_path = "C:/DEV/Lua5.1/"

glload_path = "C:/DEV/glsdk_0.5.0/glload/"
assimp_path = "C:/DEV/Assimp/"
freeimage_path = "C:/DEV/FreeImage/Dist/"

#---------------------------------------
# you shouldn't need to modify that part 

if platform.system() == 'Windows':
    env = Environment(tools = ['mingw'], ENV = os.environ)
    env.PrependENVPath('PATH', mingw_path + "bin/")
    env.PrependENVPath('LIB', mingw_path + "lib/")
    
    # these aren't needed on *ix systems, since libraries are allready
    # in some sort of /usr/lib/
    env.Append(CPPPATH=[
        engine_path + "dependencies/pugixml/include/",
        engine_path + "dependencies/glew-1.9.0/include/",
        engine_path + "dependencies/glm-0.9.4.1/",
        engine_path + "dependencies/pugixml-1.2/src/",
        "deps/lundi/",
        "deps/GLDR/src/",
        glload_path + "include/",
        assimp_path + "include/",
        lua_path + "include/",
        boost_path,
        freeimage_path,
    ])
    
    env.Append(LIBPATH=[
        lua_path + "lib/",
        #glload_path + "lib/1700/"
        "C:/PROJECTS/glload-scons/",
        freeimage_path,
        assimp_path + "lib/x86/",
    ])

    env.Append(LIBS=[
        "winmm", 
        "wininet",
        "gdi32",
        "FreeImage",
        "assimp"
    ])
else:
    env = Environment(ENV = os.environ)

env.Append(CPPFLAGS="-g -std=c++11")
env.Append(CPPDEFINES="SFML_STATIC")
env.Append(CPPDEFINES="GLDR_HAS_DSA")

#SFML///////////////////////////
env.Append(CPPPATH="deps/SFML-2.0/include/")
sfml_files = Glob("dependencies/SFML-2.0/src/System/*.cpp")
sfml_files += Glob("dependencies/SFML-2.0/src/Window/*.cpp")

if platform.system() == 'Windows':
    sfml_files += Glob("dependencies/SFML-2.0/src/System/Win32/*.cpp")
    sfml_files += Glob("dependencies/SFML-2.0/src/Window/Win32/*.cpp")
else:
    sfml_files += Glob("dependencies/SFML-2.0/src/System/Unix/*.cpp")
    sfml_files += Glob("dependencies/SFML-2.0/src/Window/Linux/*.cpp")

libsfml = env.StaticLibrary(target='SFML', source = sfml_files)
#///////////////////////////////


env.Append(CPPPATH= "src/")
env.Append(CPPPATH= "src/Engine/")
env.Append(CPPPATH= engine_path + "dependencies/SFML-2.0/include/")

# TODO : add Debug here
env.Append(LIBPATH=[
    engine_path,
    engine_path + "dependencies/FreeImage/Dist/",
])

env.Prepend(LIBS=[
'SFML',
'opengl32',
'FreeImage',
'Lua5.1',
'glloadD'
])

cpp_files = Glob("src/*.cpp")
cpp_files += Glob("src/Engine/*.cpp")
exe = env.Program(target='Minicraft', source = cpp_files)


