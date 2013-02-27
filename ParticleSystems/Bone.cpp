// Geometric Tools, LLC
// Copyright (c) 1998-2012
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
//
// File Version: 5.0.0 (2010/01/01)

#include "Bone.h"
#include <stdio.h>
#include <string>

Bone::Bone() 
{

}
//----------------------------------------------------------------------------
Bone::Bone (int theid, std::string thename, float thedirection[3], float* thelength, int theaxis[3], int thedof[3], float thelimits[3][2])
{
	id = theid;
	name = thename;
	for (int i = 0; i < 3; i++) {
		dir[i] = thedirection[i];
		axis[i] = theaxis[i];
		dof[i] = thedof[i];
		for (int j = 0; j < 2; j++) {
			limits[i][j] = thelimits[i][j];
		}
	}
	length = *thelength;

}
