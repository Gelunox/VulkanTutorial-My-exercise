
#include <string>
#include <vector>
#include <fstream>

using namespace std;

namespace com::gelunox::vulcanUtils
{
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