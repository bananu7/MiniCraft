MiniCraft
=========

Amateur clone of Minecraft. I want to do it properly.

===
# Build instructions #

Please excuse me for the current state of affairs, but I am the only one actively working on this project right now, and while the feedback is very nice, I still have to code it myself.

Right now Minicraft runs on SFML Window creation and Input; I don't use prebuilt SFML packages; instead, they are compiled from source together with engine code; I found this approach safer and easier to ensure smooth builds on *ix systems.

### Common Mistakes ###
I'm putting these at the top; please, don't get this wrong *again*

1. Make *absolutely* sure you got `develop-2.0` branch of Engine
2. Neither FBX SDK nor Lua is required to build at this moment; they are explicitly marked as optional
3. SOIL image library isn't used; it has been replaced alltogether by FreeImage. If you happen to stumble on `ImageData.cpp`, just delete it.
4. The same goes for rapidxml parser (replaced by PugiXML) and `Scene.cpp`

## Windows ##

My original development platform is hardly obtainable for everyone, because I am using Intel C++ Compiler; I'm however constantly checking the build on VS compiler for compatibility.

0. You will need VS 2012 (it can be without CTP updates, stock release should work fine).
1. Get the source of Minicraft from **master** branch and Engine from **develop-2.0** (That's *very* important)
2. Build the Engine first; One thing you might wanna set up is the Boost path; if you know how to configure VS for that, it shouldn't be a problem. If you are building for the first time, I suggest building `Debug` target first
3. After that, open up the Minicraft `.sln`, set up the path for library directories to point to your newly made Engine `.lib` (or just make a folder inside and copy `.lib` (and presumably `.pdb`) there)
4. If it builds and links, hit Run. Feel free to report any crashes from this point as Issues here.

## Linux ##

Your best bet is probably using what [@sehe](http://stackoverflow.com/users/85371/sehe) did [here](https://github.com/sehe/Bartek)

## Plans for future (regarding build) ##

1. Integrating Engine as subrepo (I haven't done it yet for various reasons) to ease the cloning
2. Putting linux build scripts here
