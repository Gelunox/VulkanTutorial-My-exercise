#pragma once

#include <stdint.h>
#include <set>

using namespace std;

namespace com::gelunox::vulcanUtils
{
	struct QueueIndices
	{
		int graphics = -1;
		int presentation = -1;
		
		bool isComplete()
		{
			return graphics >= 0 && presentation >= 0;
		}

		set<uint32_t> asSet()
		{
			set<uint32_t> set =
			{
				static_cast<uint32_t>(graphics),
				static_cast<uint32_t>(presentation)
			};

			return set;
		}

		vector<uint32_t> asList()
		{
			set<uint32_t> set = this->asSet();
			vector<uint32_t> vec;
			copy( set.begin(), set.end(), back_inserter( vec ) );
			return vec;
		}
	};
}