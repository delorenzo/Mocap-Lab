//Julie De Lorenzo
//2-27-13

#include "Keyframe.h"


Keyframe::Keyframe(std::map<std::string, HMatrix> m)
{
	matrix_map = m;
}

Keyframe::Keyframe() 
{
}

Keyframe::~Keyframe(void)
{
}
