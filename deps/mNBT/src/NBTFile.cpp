/*#**************************************************#*
 * mNBT  :  NBT manipulation system by Manearrior     *
 *#**************************************************#*/

#include <fstream>
#include "zlib.h"

#include "NBTFile.hpp"
#include "Tag.hpp"

#ifndef NBT_ZLIB_CHUNK_SIZES
#define NBT_ZLIB_CHUNK_SIZES

#define ICHUNK 4096
#define OCHUNK 4096

#endif // NBT_ZLIB_CHUNK_SIZES

namespace mNBT
{
	char NBTFile::readByte() throw(NBTErr)
	{
		if (index >= data.size())
			throw NBTErr("Went beyond bounds while reading NBT node. Size: " + itos(data.size()));
		return data[index++];
	}

	char NBTFile::peekByte() throw(NBTErr)
	{
		if (index >= data.size())
			throw NBTErr("Peeked beyond bounds while reading NBT node.");
		return data[index];
	}

	void NBTFile::writeByte(const char out) throw(NBTErr)
	{
		data.push_back(out);
	}

	void NBTFile::loadFile(const std::string &in) throw(NBTErr)
	{
		std::ifstream inFile;
		inFile.open(in.c_str(),std::ios::binary);
		int length;

		if(!inFile.good() || !inFile.is_open())
		{
			inFile.close();
			throw NBTErr("Error opening file: " + in);
		}

		inFile.seekg(0,std::ios::end);
		length = inFile.tellg();
		inFile.seekg(0,std::ios::beg);

		unsigned char *buffI = new unsigned char[length];
		unsigned char *buffO  = new unsigned char[ICHUNK];

		inFile.read((char*)buffI,length);
		inFile.close();

		z_stream strmData = {0};
		strmData.zalloc = Z_NULL;
		strmData.zfree = Z_NULL;
		strmData.opaque = Z_NULL;
		strmData.next_in = buffI;
		strmData.avail_in = 0;
		strmData.next_out = buffO;

		if (inflateInit2(&strmData, 15 | 32) < 0)
		{
			throw NBTErr("Error initializing GZIP stream.");
		}
		strmData.avail_in = length;

		while (true)
		{
			strmData.next_out = buffO;
			strmData.avail_out = ICHUNK;
			if (inflate(&strmData,Z_NO_FLUSH) < 0)
			{
				inflateEnd(&strmData);
				throw NBTErr("Error inflating file: " + in);
			}
			data.insert(data.end(),buffO,buffO + ICHUNK - strmData.avail_out);
			if (strmData.avail_out != 0)
				break;
		}
		inflateEnd(&strmData);
		index = 0;
		delete buffI;
		delete buffO;
	}

	void NBTFile::saveFile(const std::string &out) throw(NBTErr)
	{

		std::ofstream outFile;
		outFile.open(out.c_str());
		if(!outFile.good() || !outFile.is_open())
		{
			outFile.close();
			throw NBTErr("Error opening file to save NBT: " + out);
		}
		unsigned char *block = new unsigned char[OCHUNK];

		z_stream strmData = {0};
		strmData.zalloc = Z_NULL;
		strmData.zfree = Z_NULL;
		strmData.opaque = Z_NULL;

		strmData.next_in = (unsigned char*) data.data();
		strmData.avail_in = data.size();
		strmData.next_out = block;
		if(deflateInit2(&strmData,Z_DEFAULT_COMPRESSION,Z_DEFLATED, 31, 8, Z_DEFAULT_STRATEGY) < 0)
		{
			deflateEnd(&strmData);
			delete block;
			throw NBTErr("Error initializing output compression stream");
		}
		while (true)
		{
			strmData.avail_out = OCHUNK;
			strmData.next_out = block;
			if(deflate(&strmData, Z_FINISH) < 0)
			{
				deflateEnd(&strmData);
				throw NBTErr("Unknown error while compressing.");
			}
			outFile.write((char*)block,OCHUNK - strmData.avail_out);
			if (strmData.avail_out)
				break;
		}
		deflateEnd(&strmData);
	}

	void NBTFile::clear()
	{
		index = 0;
		data.clear();
	}
}
