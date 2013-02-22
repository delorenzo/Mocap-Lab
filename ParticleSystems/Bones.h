// Geometric Tools, LLC
// Copyright (c) 1998-2012
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
//
// File Version: 5.0.0 (2010/01/01)

#ifndef BONE_H
#define BONE_H

#include "Wm5WindowApplication3.h"
using namespace Wm5;

class Bones : public WindowApplication3
{
    WM5_DECLARE_INITIALIZE;
    WM5_DECLARE_TERMINATE;

public:
    Bones ();

    virtual bool OnInitialize ();
    virtual void OnTerminate ();
    virtual void OnIdle ();
	virtual bool read_buf();

protected:
    void CreateScene ();

	int id;
	std::string name;
	float dir[3];
	float length;
	float axis[3];
	float limits[3][2];
    NodePtr mScene;
    WireStatePtr mWireState;
    Culler mCuller;

    Float4 mTextColor;
};

WM5_REGISTER_INITIALIZE(Bone);
WM5_REGISTER_TERMINATE(Bone);

#endif
