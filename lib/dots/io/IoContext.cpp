#include "IoContext.h"

namespace dots
{
	IoContext& IoContext::Instance()
	{
		static IoContext ioContext;
		return ioContext;
	}
}