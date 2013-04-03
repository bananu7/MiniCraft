/*#**************************************************#*
 * mNBT  :  NBT manipulation system by Manearrior     *
 *#**************************************************#*/

#include "Block.hpp"

namespace mNBT
{
	void Block::operator>>(char &out) throw(NBTErr)
	{
		out = readByte();
	}

	void Block::operator>>(int16_t &out) throw(NBTErr)
	{
		out = (uint16_t) ((unsigned char)readByte()) << 8;
		out |= readByte();
	}

	void Block::operator>>(int32_t &out) throw(NBTErr)
	{
		out = (uint32_t) ((unsigned char)readByte()) << 24;
		out |= (uint32_t) ((unsigned char)readByte()) << 16;
		out |= (uint32_t) ((unsigned char)readByte()) << 8;
		out |= (uint32_t) ((unsigned char)readByte());
	}

	void Block::operator>>(int64_t &out) throw(NBTErr)
	{
		out = (uint64_t) ((unsigned char)readByte()) << 56;
		out |= (uint64_t) ((unsigned char)readByte()) << 48;
		out |= (uint64_t) ((unsigned char)readByte()) << 40;
		out |= (uint64_t) ((unsigned char)readByte()) << 32;
		out |= (uint64_t) ((unsigned char)readByte()) << 24;
		out |= (uint64_t) ((unsigned char)readByte()) << 16;
		out |= (uint64_t) ((unsigned char)readByte()) << 8;
		out |= (uint64_t) ((unsigned char)readByte());
	}

	void Block::operator>>(float &out) throw(NBTErr)
	{
		int32_t i;
		operator>>(i);
		out = static_cast<float>(i);
	}

	void Block::operator>>(double &out) throw(NBTErr)
	{
		int64_t i;
		operator>>(i);
		out = static_cast<double>(i);
	}

	void Block::operator>>(std::string &out) throw (NBTErr)
	{
		out.clear();
		int16_t size;
		operator>>(size);
		for (int i = 0; i < size; i++)
			out.push_back(readByte());
	}

	void Block::operator<<(const char &in) throw(NBTErr)
	{
		writeByte(in);
	}

	void Block::operator<<(const int16_t &in) throw(NBTErr)
	{
		writeByte((char)(in>>8));
		writeByte((char)in);
	}

	void Block::operator<<(const int32_t &in) throw(NBTErr)
	{
		writeByte((char)(in>>24));
		writeByte((char)(in>>16));
		writeByte((char)(in>>8));
		writeByte((char)(in));
	}

	void Block::operator<<(const int64_t &in) throw(NBTErr)
	{
		writeByte((char)(in>>56));
		writeByte((char)(in>>48));
		writeByte((char)(in>>40));
		writeByte((char)(in>>32));
		writeByte((char)(in>>24));
		writeByte((char)(in>>16));
		writeByte((char)(in>>8));
		writeByte((char)(in));
	}

	void Block::operator<<(const float &in) throw(NBTErr)
	{
		int32_t inI = *((int32_t*) &in);
		operator<<((int32_t) inI);
	}

	void Block::operator<<(const double &in) throw(NBTErr)
	{
		int64_t inI = *((int64_t*) &in);
		operator<<((int64_t) inI);
	}

	void Block::operator<<(const std::string &in) throw(NBTErr)
	{
		int16_t i = in.size();
		operator<<((int16_t) i);
		for (unsigned int i = 0; i < in.size(); i++)
			operator<<((char)in[i]);
	}
}
