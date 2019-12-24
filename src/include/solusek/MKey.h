/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#ifndef __M_KEY_INCLUDED__
#define __M_KEY_INCLUDED__

namespace solusek
{
	class MKey
	{
	public:
		unsigned char *Data;
		unsigned int Size;

		MKey(unsigned char *data, unsigned int size) : Data(data), Size(size) {}
		~MKey()
		{
			delete[] Data;
		}
	};
}

#endif
