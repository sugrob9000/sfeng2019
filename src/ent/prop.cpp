#include "prop.h"

namespace core
{

e_prop::e_prop ()
{
	std::cout << "prop was born" << std::endl;
}

void e_prop::think ()
{
	std::cout << "a" << std::flush;
}

}
