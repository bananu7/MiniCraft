/*#**************************************************#*
 * mNBT  :  NBT manipulation system by Manearrior     *
 *#**************************************************#*/

#include <fstream>
#include <stdint.h>
#include <map>
#include <time.h>
#include <iostream>
#include "zlib.h"

#include "RegionLoader.hpp"
#include "Tag.hpp"


#ifndef NBT_ZLIB_CHUNK_SIZES
#define NBT_ZLIB_CHUNK_SIZES

#define ICHUNK 4096
#define OCHUNK 4096
#endif // NBT_ZLIB_CHUNK_SIZES

namespace mNBT
{
	RegionLoader::RegionLoader(std::string world, int x, int z)
	{
		xpos = x;
		zpos = z;
		path = world;
		std::ifstream test((world + "/region/r." + itos(xpos) + "." + itos(zpos) + ".mca").c_str());
		if (test.good()) {
			try {
				load();
			} catch (NBTErr error) {
				std::cerr << "Error: " << error.getReason();
			}
		} else {
			for (int x = 0; x < 32; x++)
				for (int z = 0; z < 32; z++)
					timestamps[x][z] = 0;
		}
		test.close();
	}

	RegionLoader::~RegionLoader()
	{
	}

	int RegionLoader::getXPos() const
	{
		return xpos;
	}

	int RegionLoader::getZPos() const
	{
		return zpos;
	}

	std::string RegionLoader::getWorld() const
	{
		return path;
	}

	void RegionLoader::load() throw(NBTErr)
	{
		std::string ppath = path + "/region/r." + itos(xpos) + "." + itos(zpos) + ".mca";
		//Get path of region file.

		std::ifstream inFile;
		inFile.open(ppath.c_str(),std::ios::binary);        //Open the file.

		if(!inFile.good() || !inFile.is_open())
		{
			inFile.close();
			throw NBTErr("Error opening file: " + ppath);       //Die if file not there.
		}

		std::map<int,std::pair<int,int> > offsets;      //Temporary data (sector location, size.)
		char count[32][32];
		int32_t temp;                                   //Temporary reading int.

		//Grab the sector offset/size.--------------------------------------------------
		for (int z = 0; z < 32; z++)
			for (int x = 0; x < 32; x++)
			{
				temp = ((int32_t) ((unsigned char) inFile.get())) << 16;
				temp |= ((int32_t) ((unsigned char) inFile.get())) << 8;
				temp |= ((int32_t) ((unsigned char) inFile.get()));
				count[x][z] = inFile.get();
				if(count[x][z])
					offsets[temp] = std::pair<int,int>(x,z);
				chunks[x][z].clear();
			}

		// Get the timestamps.----------------------------------------------------------
		for (int z = 0; z < 32; z++)
			for (int x = 0; x < 32; x++)
			{
				temp = ((int32_t) ((unsigned char) inFile.get())) << 24; //damn endian.
				temp |= ((int32_t) ((unsigned char) inFile.get())) << 16;
				temp |= ((int32_t) ((unsigned char) inFile.get())) << 8;
				temp |= ((int32_t) ((unsigned char) inFile.get()));
				timestamps[x][z] = temp;
			}

		// Check if file is bad.---------------------------------------------------------
		if (!inFile.good() || inFile.eof())
		{
			inFile.close();
			throw NBTErr("Error reading Region file.");
		}

		int size; //Number of bytes to read.
		int pos = 2; //Current sector (pos*4kib)
		int tempx, tempz; //chunk X/Z from map.
		// Read in file sectors.-------------------------------------------------------------
		while(!offsets.empty())
		{
			if (inFile.eof() || !inFile.good())
			{
				inFile.close();
				throw NBTErr("Corrupt region file?"); // If file is bad.---------------------------
			}
			if(offsets.find(pos) == offsets.end()) // If his sector does not contain a chunk. Does this happen?
			{
				inFile.ignore(2 << 11);
				pos++;
				continue;
			}
			tempx = offsets[pos].first; //get chunk's x/z for this sector.
			tempz = offsets[pos].second;
			size = ((int32_t) ((unsigned char) inFile.get())) << 24; //get size. (in bytes)
			size |= ((int32_t) ((unsigned char) inFile.get())) << 16;
			size |= ((int32_t) ((unsigned char) inFile.get())) << 8;
			size |= ((int32_t) ((unsigned char) inFile.get()));
			if (inFile.get() == 1)
			{
				inFile.close();
				throw NBTErr("Gzip not supported at this time."); //Gets compression type.
			}
			char* buff = new char[size];
			inFile.read(buff,size);
			chunks[tempx][tempz].insert(chunks[tempx][tempz].end(),buff,buff+size); //grab ALL the data!
			delete[] buff;
			if(inFile.eof())
			{
				inFile.close();
				throw NBTErr("Corrupt region file?");
			}
			inFile.ignore((count[tempx][tempz] << 12) - size - 5);
			offsets.erase(pos);
			pos += count[tempx][tempz];
		}
		inFile.close();
	}

	void RegionLoader::save() throw(NBTErr)
	{
		std::string ppath = path + "/region/r." + itos(xpos) + "." + itos(zpos) + ".mca";
		// Generate the path of the region file.
		std::ofstream outFile;
		outFile.open(ppath.c_str());
		if(!outFile.good() || !outFile.is_open())
		{
			outFile.close();
			throw NBTErr("Error opening file to save region: " + ppath);
		}
		std::vector<char> sizePos;
		std::vector<int> timeStampsOut;
		std::vector<char> sectorBlocks;
		int sector = 2;
		for (int z = 0; z < 32; z++)
			for (int x = 0; x < 32; x++)
			{
				if(chunks[x][z].size())
				{
					timeStampsOut.push_back(timestamps[x][z]);
					sizePos.push_back((char) (sector >> 16));
					sizePos.push_back((char) (sector >> 8));
					sizePos.push_back((char) (sector));
					int size = chunks[x][z].size();
					char secSize = 1 + ((size + 4) >> 12);
					sizePos.push_back(secSize);
					sectorBlocks.push_back((char)(size >> 24));
					sectorBlocks.push_back((char)(size >> 16));
					sectorBlocks.push_back((char)(size >> 8));
					sectorBlocks.push_back((char)(size));
					sectorBlocks.push_back(2);
					sectorBlocks.insert(sectorBlocks.end(),
										chunks[x][z].data(),
										chunks[x][z].data() + chunks[x][z].size());

					for (int i = 0; i < (secSize << 12) - 5 - size; i++)
						sectorBlocks.push_back(0);
					sector += secSize;

				} else
				{
					sizePos.push_back(0); sizePos.push_back(0);
					sizePos.push_back(0); sizePos.push_back(0);
					timeStampsOut.push_back(0);
				}
			}
		outFile.write((char*)sizePos.data(),sizePos.size());
		for (int i = 0; i < 1024; i++)
		{
			outFile << (char)(timeStampsOut[i] >> 24);
			outFile << (char)(timeStampsOut[i] >> 16);
			outFile << (char)(timeStampsOut[i] >> 8);
			outFile << (char)(timeStampsOut[i]);
		}
		outFile.write((char*) sectorBlocks.data(),sectorBlocks.size());

	}

	Tag* RegionLoader::getChunk(int x, int z) throw(NBTErr)
	{
		if (!chunks[x][z].size())
			return NULL;
		Tag* toReturn;

		unsigned char *buffO  = new unsigned char[ICHUNK];

		z_stream strmData = {0};
		strmData.zalloc = Z_NULL;
		strmData.zfree = Z_NULL;
		strmData.opaque = Z_NULL;
		strmData.next_in = (Bytef*) chunks[x][z].data();
		strmData.avail_in = 0;
		strmData.next_out = buffO;

		if (inflateInit2(&strmData,15 | 32) < 0)
			throw NBTErr("Error initializing ZLIB stream.");

		strmData.avail_in = chunks[x][z].size();

		while (true)
		{
			strmData.next_out = buffO;
			strmData.avail_out = ICHUNK;

			if(inflate(&strmData,Z_NO_FLUSH) < 0)
			{
				inflateEnd(&strmData);
				throw NBTErr("Error inflating chunk.");
			}
			IOBlock.insert(IOBlock.end(),buffO,buffO + ICHUNK - strmData.avail_out);
			if (strmData.avail_out != 0)
				break;
		}
		inflateEnd(&strmData);
		IOBlockIndex = 0;
		delete[] buffO;

		try
		{
			toReturn = getTag(this);
		} catch (NBTErr error)
		{
			IOBlock.clear();
			IOBlockIndex = -1;
			throw NBTErr("Error while reading chunk: " + error.getReason());
		}
		IOBlock.clear();
		IOBlockIndex = -1;
		return toReturn;
	}

	int RegionLoader::getChunkTimestamp(int x, int z) const
	{
		return timestamps[x][z];
	}

	void RegionLoader::putChunk(Tag* inChunk, int x, int z, int timestamp) throw(NBTErr)
	{
		try
		{
			if (!timestamp)
				updateTimestamp(x,z);
			if (inChunk == NULL)
				return;
			IOBlockIndex = 0;
			IOBlock.clear();
			inChunk->write(this);
			timestamps[x][z] = timestamp;
			chunks[x][z].clear();

			unsigned char *buffO = new unsigned char[OCHUNK];

			z_stream strmData = {0};
			strmData.zalloc = Z_NULL;
			strmData.zfree = Z_NULL;
			strmData.opaque = Z_NULL;

			strmData.next_in = (Bytef*) IOBlock.data();
			strmData.avail_in = IOBlock.size();

			if (deflateInit2(&strmData,Z_DEFAULT_COMPRESSION,Z_DEFLATED,15,8,Z_DEFAULT_STRATEGY) < 0)
			{
				deflateEnd(&strmData);
				delete buffO;
				throw NBTErr("Error initializing compression stream (regionLoader.)");
			}
			while (true)
			{
				strmData.avail_out = OCHUNK;
				strmData.next_out = buffO;
				if (deflate(&strmData,Z_FINISH) < 0)
				{
					deflateEnd(&strmData);
					throw NBTErr("Unknown error while compressing.");
				}
				chunks[x][z].insert(chunks[x][z].end(),buffO,buffO + OCHUNK - strmData.avail_out);
				if(strmData.avail_out)
					break;
			}
			deflateEnd(&strmData);

			IOBlock.clear();
			IOBlockIndex = -1;
		} catch (NBTErr error)
		{
			chunks[x][z].clear();
			IOBlock.clear();
			IOBlockIndex = -1;
			throw NBTErr("Error while writing out chunk: " + error.getReason());
		}
	}

	void RegionLoader::updateTimestamp(int x, int z)
	{
		time_t now;
		struct tm* unixEpoch;

		time(&now);
		unixEpoch = gmtime(&now);
		unixEpoch->tm_hour = unixEpoch->tm_min = unixEpoch->tm_sec = 0;
		unixEpoch->tm_mon = 0;
		unixEpoch->tm_mday = 1;
		unixEpoch->tm_year = 70;
		int32_t seconds = (int32_t) difftime(now,mktime(unixEpoch));
		timestamps[x][z]=seconds;
	}

	char RegionLoader::readByte() throw(NBTErr)
	{
		if (IOBlockIndex == -1)
			throw NBTErr("Attempted to read RegionLoader when not in chunk IO mode.");
		if ((unsigned) IOBlockIndex >= IOBlock.size())
			throw NBTErr("Went beyond bounds while reading NBT node from RegionLoader.. Size: " + itos(IOBlock.size()));
		return IOBlock[IOBlockIndex++];
	}

	char RegionLoader::peekByte() throw(NBTErr)
	{
		if (IOBlockIndex == -1)
			throw NBTErr("Attempted to peek RegionLoader when not in chunk IO mode.");
		if ((unsigned)IOBlockIndex >= IOBlock.size())
			throw NBTErr("Peeked beyond bounds while in RegionLoader reading NBT node.");
		return IOBlock[IOBlockIndex];
	}

	void RegionLoader::writeByte(const char out) throw(NBTErr)
	{
		if (IOBlockIndex == -1)
			throw NBTErr("Attempted to write RegionLoader when not in chunk IO mode.");
		IOBlock.push_back(out);
	}

	Tag* RegionLoader::getEmptyChunkNBT(int x, int z)
	{
		Compound* root = new Compound("");
		Compound* top = new Compound("Level");
		root->add(top);
		top->add(new Int("xPos",x + (xpos << 5)));
		top->add(new Int("zPos",z + (zpos << 5)));
		top->add(new Long("LastUpdate",0));
		top->add(new Byte("TerrainPopulated",0));
		std::vector<char> biomes;
		std::vector<int> heightMap;
		for (int i = 0; i < 256; i++)
		{
			biomes.push_back(0);
			heightMap.push_back(0);
		}
		top->add(new ByteArray("Biomes",biomes));
		top->add(new IntArray("HeightMap",heightMap));
		top->add(new List("Entities",10));
		top->add(new List("TileEntities",10));
		top->add(new List("Sections",10));
		return root;
	}
}

