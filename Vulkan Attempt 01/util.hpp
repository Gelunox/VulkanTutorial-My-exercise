#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>

using namespace std;

namespace com::gelunox::vulcanUtils
{
	inline static void println( const string& str ... )
	{
		cout << str << endl;
	}

	inline static void errln( const string& str ... )
	{
		cerr << str << endl;
	}


	static vector<char> readFile( const string& filename )
	{
		ifstream file( filename, ios::ate | ios::binary );

		if (!file.is_open())
		{
			throw runtime_error( "can't open file" );
		}

		unsigned int fileSize = file.tellg();
		vector<char> buff( fileSize );

		file.seekg( 0 );
		file.read( buff.data(), fileSize );
		file.close();

		return buff;
	}

}
