// Geometric Tools, LLC
// Copyright (c) 1998-2012
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
//
// File Version: 5.0.0 (2010/01/01)

#include "Bone.h"

WM5_WINDOW_APPLICATION(Bone);

//----------------------------------------------------------------------------
Bone::Bones ()
    :
    WindowApplication3("SampleGraphics/ParticleSystems", 0, 0, 640, 480,
        Float4(0.5f, 0.0f, 1.0f, 1.0f)),
        mTextColor(1.0f, 1.0f, 1.0f, 1.0f)
{
}
bool Bone::read_buf()
{

}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool Bone::OnInitialize ()
{
    if (!WindowApplication3::OnInitialize())
    {
        return false;
    }

    
    return true;
}
//----------------------------------------------------------------------------
void Bone::OnTerminate ()
{
    
    WindowApplication3::OnTerminate();
}
//----------------------------------------------------------------------------
void Bone::OnIdle ()
{
    
}

void Bone::CreateScene ()
{
 
}
//----------------------------------------------------------------------------
