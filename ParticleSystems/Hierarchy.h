#ifndef HIERARCHY_H
#define HIERARCHY_H
#include "Bone.h"
class Hierarchy 
{
public:
	Hierarchy(void);
	virtual void addchild(Bone p, Bone c);
protected:
	Bone* root;
};
#endif
