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
	std::map<std::string, HMatrix> matrix_map;
	APoint root_translation;
	APoint root_rotation;
	Keyframe(std::map<std::string, HMatrix> matrix_map);
	~Keyframe(void);
};

#endif