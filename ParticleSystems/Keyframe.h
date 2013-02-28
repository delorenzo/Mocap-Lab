//Julie De Lorenzo
//2-27-13

#ifndef KEYFRAME_H
#define KEYFRAME_H

#include <string>
#include <map>
#include "Wm5WindowApplication3.h"
using namespace Wm5;

class Keyframe
{
public:
	Keyframe();
	std::map<std::string, Node*> node_map;
	APoint * root_translation;
	Keyframe(std::map<std::string, Node*> the_node_map);
	~Keyframe(void);
};

#endif