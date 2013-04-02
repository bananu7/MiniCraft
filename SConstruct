# scons 2.7
import platform
import os

# set up your MinGW path here
mingw_path = "C:/DEV/MinGW32-4.8/mingw32/"
# set your Engine path here
engine_path = "C:/PROJECTS/Engine/"
# set your Boost path here
boost_path = "C:/DEV/boost_1_52_0/"


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
        boost_path,
    ])
    
    env.Append(LIBS=[
        "winmm", 
        "wininet",
        "gdi32"
    ])
else:
    env = Environment(ENV = os.environ)

env.Append(CPPFLAGS="-Wall -g -std=c++11")
env.Append(CPPDEFINES="GLEW_NO_GLU")
env.Append(CPPDEFINES="SFML_STATIC")
env.Append(CPPDEFINES="GLEW_STATIC")

env.Append(CPPPATH= engine_path + "Engine/include/")
env.Append(CPPPATH= engine_path + "dependencies/SFML-2.0/include/")

# TODO : add Debug here
env.Append(LIBPATH=[
    engine_path,
    engine_path + "dependencies/FreeImage/Dist/",
])

env.Prepend(LIBS=[
'Engine',
'SFML',
'opengl32',
'FreeImage',
])

cpp_files = Glob("src/*.cpp")
exe = env.Program(target='Minicraft', source = cpp_files)

