// Geometric Tools, LLC
// Copyright (c) 1998-2012
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
//
// File Version: 5.0.0 (2010/01/01)

#ifndef SKELETON_H
#define SKELETON_H

#include "Wm5WindowApplication3.h"
using namespace Wm5;

class Skeleton : public WindowApplication3
{
    WM5_DECLARE_INITIALIZE;
    WM5_DECLARE_TERMINATE;

public:
    Skeleton ();
	//char * buffer files for the ASF files
	char * asf1;
	char * asf2;
	Hierarchy h;
	
	//functions
    virtual bool OnInitialize ();
    virtual void OnTerminate ();
    virtual void OnIdle ();
    virtual bool OnKeyDown (unsigned char key, int x, int y);
	virtual bool read_buf (char *name, char *src);
	virtual void parse_asf(char *source);

	TriMeshPtr *sm;

protected:
    void CreateScene ();

    NodePtr mScene;
    WireStatePtr mWireState;
    Culler mCuller;

    Float4 mTextColor;
};

WM5_REGISTER_INITIALIZE(Skeleton);
WM5_REGISTER_TERMINATE(Skeleton);

#endif
