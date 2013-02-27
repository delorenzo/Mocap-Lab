#include <stdio.h>
#include <string.h>
#include "Bone.h"
#include "Hierarchy.h"
//Hierarchy of nodes::add bone data
Hierarchy::Hierarchy()
{
	root = NULL; //root is a bone but carries no bone information	
}

void addchild(Bone* parent, Bone* child) {
	child->parent = parent;
}



