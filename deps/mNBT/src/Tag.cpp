/*#**************************************************#*
 * mNBT  :  NBT manipulation system by Manearrior     *
 *#**************************************************#*/

#include <string>
#include <stdio.h>

#include "Tag.hpp"

namespace mNBT
{
    std::map<char, Tag*> makeTagMap() {
        std::map<char, Tag*> notchTags;
        notchTags.insert(std::make_pair(1, new Byte()));
        notchTags.insert(std::make_pair(2, new Short()));
        notchTags.insert(std::make_pair(3, new Int()));
        notchTags.insert(std::make_pair(4, new Long()));
        notchTags.insert(std::make_pair(5, new Float()));
        notchTags.insert(std::make_pair(6, new Double()));
        notchTags.insert(std::make_pair(7, new ByteArray()));
        notchTags.insert(std::make_pair(8, new String()));
        notchTags.insert(std::make_pair(9, new List()));
        notchTags.insert(std::make_pair(10, new Compound()));
        notchTags.insert(std::make_pair(11, new IntArray()));
        return notchTags;
    }

    std::map<char, Tag*> Tag::notchTags = makeTagMap();

	std::string itos(int64_t i)
	{
		char buf[32];
		sprintf(buf,"%d",(int)i);
		return (std::string)(buf);
	}
	std::string ftos(double i)
	{
		char buf[64]; //I'm scared.
		sprintf(buf,"%f",i);
		return (std::string)(buf);
	}

	Tag* getTag(Block* in) throw(NBTErr)
	{
		char type;
		*in >> type;
		if (Tag::notchTags.size() < (unsigned)type)
			throw NBTErr("Invalid tag: " + itos(type));
		Tag* rIn = Tag::notchTags[type]->clone();
		rIn->read(in);
		return rIn;
	}

	/********************* / Tag / *********************/
	void Tag::setName(const std::string &in)
	{
		name = in;
	}

	std::string Tag::getName() const
	{
		return name;
	}

	unsigned char Tag::getType() const
	{
		return tagType;
	}

	Tag* Tag::getTag(std::string tname) throw(NBTErr)
	{
		if (name == tname)
			return this;
		throw NBTErr("Tag not found: " + tname);
	}

	Tag* Tag::getTag(std::string tname, char type) throw(NBTErr)
	{
		if (name == tname && tagType == type)
			return this;
		throw NBTErr("Tag not found: " + tname + ". Type: " + (char) (64+type));
	}

	void Tag::read(Block* in) throw(NBTErr)
	{
		*in >> name;
		readPayload(in);
	}

	void Tag::write(Block* out) throw(NBTErr)
	{
		*out << (char) tagType;
		*out << name;
		writePayload(out);
	}

	void Tag::deleteNotchTags()
	{
		for (std::map<char,Tag*>::iterator it = notchTags.begin(); it != notchTags.end(); it = notchTags.begin()) {
			delete it->second;
			notchTags.erase(it);
		}
	}
	/********************* / Byte / *********************/

	std::string Byte::toString(std::string indent)
	{
		return indent + "TAG Byte [" + name + "]: " + itos(data) + "\n";
	}

	void Byte::readPayload(Block* in) throw(NBTErr)
	{
		*in >> data;
	}

	void Byte::writePayload(Block* out) throw(NBTErr)
	{
		*out << (char) data;
	}

	char Byte::getPayload() const
	{
		return data;
	}

	void Byte::setPayload(char idata)
	{
		data = idata;
	}

	/********************* / Short / *********************/

	std::string Short::toString(std::string indent)
	{
		return indent + "TAG Short [" + name + "]: " + itos(data) + "\n";
	}

	void Short::readPayload(Block* in) throw(NBTErr)
	{
		*in >> data;
	}

	void Short::writePayload(Block* out) throw(NBTErr)
	{
		*out << (int16_t) data;
	}

	int16_t Short::getPayload() const
	{
		return data;
	}

	void Short::setPayload(int16_t idata)
	{
		data = idata;
	}

	/********************* / Int / *********************/

	std::string Int::toString(std::string indent)
	{
		return indent + "TAG Int [" + name + "]: " + itos(data) + "\n";
	}

	void Int::readPayload(Block* in) throw(NBTErr)
	{
		*in >> data;
	}

	void Int::writePayload(Block* out) throw(NBTErr)
	{
		*out << (int32_t) data;
	}

	int32_t Int::getPayload() const
	{
		return data;
	}

	void Int::setPayload(int32_t idata)
	{
		data = idata;
	}

	/********************* / Long / *********************/
	std::string Long::toString(std::string indent)
	{
		return indent + "TAG Long [" + name + "]: " + itos((int64_t)data) + "\n";
	}

	void Long::readPayload(Block* in) throw(NBTErr)
	{
		*in >> data;
	}

	void Long::writePayload(Block* out) throw(NBTErr)
	{
		*out << (int64_t) data;
	}

	int64_t Long::getPayload() const
	{
		return data;
	}

	void Long::setPayload(int64_t idata)
	{
		data = idata;
	}

	/********************* / Float / *********************/
	std::string Float::toString(std::string indent)
	{
		return indent + "TAG Float [" + name + "]: " + ftos((double)data) + "\n";
	}

	void Float::readPayload(Block* in) throw(NBTErr)
	{
		*in >> data;
	}

	void Float::writePayload(Block* out) throw(NBTErr)
	{
		*out << (float) data;
	}

	float Float::getPayload() const
	{
		return data;
	}

	void Float::setPayload(float idata)
	{
		data = idata;
	}

	/********************* / Double / *********************/

	std::string Double::toString(std::string indent)
	{
		return indent + "TAG Double [" + name + "]: " + ftos(data) + "\n";
	}

	void Double::readPayload(Block* in) throw(NBTErr)
	{
		*in >> data;
	}

	void Double::writePayload(Block* out) throw(NBTErr)
	{
		*out << (double) data;
	}

	double Double::getPayload() const
	{
		return data;
	}

	void Double::setPayload(double idata)
	{
		data = idata;
	}

	/********************* / ByteArray / *********************/

	std::string ByteArray::toString(std::string indent)
	{
		std::string toReturn;
		toReturn = indent + "TAG ByteArray [" + name + "]:\n" + indent + "[";
		int widthCounter =0;
		for (unsigned int i = 0; i < data.size(); i++)
		{
			toReturn += itos((int)data[i]);
			widthCounter++;
			if (widthCounter == 16)
			{
				toReturn += "\n" + indent + " ";
				widthCounter = 0;
			}
			else
				if (i != data.size()-1)
					toReturn += ",";
		}
		toReturn += "]\n";
		return toReturn;
	}

	void ByteArray::readPayload(Block* in) throw(NBTErr)
	{
		int size;
		char t;
		*in >> size;
		for (int i = 0; i < size; i++)
		{
			*in >> t;
			data.push_back(t);
		}
		return;
	}

	void ByteArray::writePayload(Block* out) throw(NBTErr)
	{
		int t;
		char t2;
		t = data.size();
		*out << t;
		for (unsigned int i = 0; i < data.size(); i++)
		{
			t2 = data[i];
			*out << (char) t2;
		}
		return;
	}

	std::vector<char>* ByteArray::getPayload()
	{
		return &data;
	}

	void ByteArray::setPayload(std::vector<char>* idata)
	{
		data = *idata;
	}

	char& ByteArray::operator[](unsigned int index) throw(NBTErr)
	{
		if (index > data.size())
			throw NBTErr("Index greater than size of ByteArray: " + itos(index));
		return (data[index]);
	}

	int ByteArray::size()
	{
		return data.size();
	}

	/********************* / String / *********************/

	std::string String::toString(std::string indent)
	{
		return indent + "TAG String [" + name + "]: " + data + "\n";
	}

	void String::readPayload(Block* in) throw(NBTErr)
	{
		*in >> data;
	}

	void String::writePayload(Block* out) throw(NBTErr)
	{
		*out << (std::string) data;
	}

	std::string* String::getPayload()
	{
		return &data;
	}

	void String::setPayload(std::string* idata)
	{
		data = *idata;
	}
	/********************* / List / *********************/
	List::~List()
	{
		while (data.size())
			delete data.back(), data.pop_back();
	}

	std::string List::toString(std::string indent)
	{
		std::string toReturn = indent + "TAG List [" + name + "]: Type " + itos(dataType) + "\n" + indent + "{\n";
		for (std::list<Tag*>::iterator it = data.begin(); it != data.end(); it++)
		{
			toReturn += (*it)->toString("   " + indent);
		}
		toReturn += indent + "}\n";
		return toReturn;
	}


	void List::readPayload(Block* in) throw(NBTErr)
	{
		int size;
		*in >> dataType;
		*in >> size;
		Tag* t;
		for(int i = 0; i < size; i++)
		{
			t = notchTags[dataType]->clone();
			t->readPayload(in);
			data.push_back(t);
		}
	}

	void List::writePayload(Block* out) throw(NBTErr)
	{
		*out << (char) dataType;
		*out << (int) data.size();
		for (std::list<Tag*>::iterator it = data.begin(); it != data.end(); it++)
		{
			(*it)->writePayload(out);
		}
	}


	std::list<Tag*>* List::getPayload()
	{
		return &data;
	}


	void List::setPayload(std::list<Tag*>* idata)
	{
		data = *idata;
	}

	char List::getType()
	{
		return dataType;
	}

	std::list<Tag*>::iterator List::begin()
	{
		return data.begin();
	}


	std::list<Tag*>::iterator List::end()
	{
		return data.end();
	}

	void List::add(Tag* in) throw(NBTErr)
	{
		if (in->getType() != dataType)
			throw NBTErr("Attempted to add tag of incorrect type to TagList.");
		data.push_back(in);
	}

	void List::remove(Tag* in)
	{
		delete in;
		data.remove(in);
	}

	int List::size()
	{
		return data.size();
	}

	/********************* / Compound / *********************/

	Compound::~Compound()
	{
		clear();
	}


	std::string Compound::toString(std::string indent)
	{
		std::string toReturn = indent + "TAG Compound [" + name + "]:\n" + indent + "{\n";
		for (std::map<std::string,Tag*>::iterator it = data.begin(); it != data.end(); it++)
		{
			toReturn += (*it).second->toString("   " + indent);
		}
		toReturn += indent + "}\n";
		return toReturn;
	}


	void Compound::readPayload(Block* in) throw(NBTErr)
	{
		char nextType;
		Tag* temp;
		while (in->peekByte())
		{
			*in >> nextType;
			temp = notchTags[nextType]->clone();
			temp->read(in);
			data[temp->getName()] = temp;
		}
		in->readByte();//Gets rid of ending TAG_END.
	}

	void Compound::writePayload(Block* out) throw(NBTErr)
	{
		for (std::map<std::string,Tag*>::iterator it = data.begin(); it != data.end(); it++)
			it->second->write(out);
		out->writeByte(0);
	}

	std::map<std::string,Tag*>* Compound::getPayload()
	{
		return &data;
	}

	void Compound::setPayload(std::map<std::string,Tag*>* idata)
	{
		if (!data.empty())
			clear();
		data = *idata;
	}

	void Compound::add(Tag* in)
	{
		data[in->getName()] = in;
		return;
	}


	bool Compound::remove(Tag* in)
	{
		if(data.find(in->getName()) == data.end())
			return false;
		delete &(*data.find(in->getName()));
		data.erase(data.find(in->getName()));
		return true;
	}

	void Compound::clear()
	{
		for (std::map<std::string,Tag*>::iterator it = data.begin(); it != data.end(); it++)
			delete it->second;
	}


	Tag* Compound::operator[](std::string index) throw(NBTErr)
	{
		if(data.count(index))
			return data[index];
		throw NBTErr("Error: Attempted to access non-existant node: " + index);
	}

	int Compound::size()
	{
		return data.size();
	}

	Tag* Compound::getTag(std::string tname) throw(NBTErr)
	{
		if (name == tname)
			return this;
		tname.erase(0,tname.find_first_of(".")+1);
		std::string tempString = tname;
		if(tempString.find_first_of(".") != tempString.npos)
			tempString.erase(tempString.find_first_of("."));
		if (data.find(tempString) == data.end())
			throw NBTErr("Tag not found: " + tname);
		return data[tempString]->getTag(tempString);
	}

	Tag* Compound::getTag(std::string tname, char type) throw(NBTErr)
	{
		if (name == tname)
		{
			if (type == tagType)
				return this;
			else
				throw NBTErr("Tag not found: " + tname);
		}
		tname.erase(0,tname.find_first_of("."));
		std::string tempString = tname;
		if(tempString.find_first_of(".") != tempString.npos)
			tempString.erase(tempString.find_first_of("."));
		if (data.find(tempString) == data.end())
			throw NBTErr("Tag not found: " + tname);
		return data[tempString]->getTag(tempString,type);
	}

	/********************* / IntArray / *********************/
	std::string IntArray::toString(std::string indent)
	{
		std::string toReturn;
		toReturn = indent + "TAG IntArray [" + name + "]:\n" + indent + "[";
		int widthCounter = 0;
		for (unsigned int i = 0; i < data.size(); i++)
		{
			widthCounter++;
			toReturn += itos(data[i]);
			if (widthCounter == 16)
			{
				toReturn += "\n" + indent + " ";
				widthCounter = 0;
			}
			else
				if (i != data.size()-1)
					toReturn += ",";
		}
		toReturn += "]\n";
		return toReturn;
	}

	void IntArray::readPayload(Block* in) throw(NBTErr)
	{
		int size, t;
		*in >> size;
		for (int i = 0; i < size; i++)
		{
			*in >> t;
			data.push_back(t);
		}
		return;
	}

	void IntArray::writePayload(Block* out) throw(NBTErr)
	{
		int t;
		t = data.size();
		*out << t;
		for (unsigned int i = 0; i < data.size(); i++)
		{
			t = data[i];
			*out << (int) t;
		}
		return;
	}

	std::vector<int>* IntArray::getPayload()
	{
		return &data;
	}

	void IntArray::setPayload(std::vector<int>* idata)
	{
		data = *idata;
	}

	int& IntArray::operator[](unsigned int index) throw(NBTErr)
	{
		if (index > data.size())
			throw NBTErr("Index greater than size of IntArray: " + itos(index));
		return data[index];
	}

	int IntArray::size()
	{
		return data.size();
	}
}
