#include <stdio.h>
#include <string.h>
#include "Bone.h"
//Hierarchy of nodes::add bone data
Hierarchy::Hierarchy()
{
	Bone* root; //root is a bone but carries no bone information	
}

	void addchild(Bone* parent, Bone* child) {
		child->parent = parent;
	}


