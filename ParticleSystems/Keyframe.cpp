//Julie De Lorenzo
//2-27-13

#include "Keyframe.h"


Keyframe::Keyframe(std::map<std::string, Node*> the_node_map)
{
	node_map = the_node_map;
}

Keyframe::Keyframe() 
{
}

Keyframe::~Keyframe(void)
{
}