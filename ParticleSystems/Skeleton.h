// Geometric Tools, LLC
// Copyright (c) 1998-2012
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
//
// File Version: 5.0.0 (2010/01/01)

#ifndef SKELETON_H
#define SKELETON_H
#include "Hierarchy.h"

#include "Wm5WindowApplication3.h"
using namespace Wm5;

class Skeleton : public WindowApplication3
{
    WM5_DECLARE_INITIALIZE;
    WM5_DECLARE_TERMINATE;

public:
    Skeleton ();
	//char * buffer files for the ASF files
	std::string source;
	Hierarchy *h;
	
	//functions
    bool OnInitialize ();
    void OnTerminate ();
    void OnIdle ();
    bool OnKeyDown (unsigned char key, int x, int y);
	std::string get_file_contents(const char *filename);
	void parse_asf(std::string source, std::vector<Bone> &b);
	void draw_skel();
	void parse_hierarchy(std::string source, std::map<std::string, Node*> n, std::map<std::string, Bone> b);
	std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);
	std::vector<std::string> split(const std::string &s, char delim);

	TriMeshPtr *sm;

protected:
    

    NodePtr mScene;
    WireStatePtr mWireState;
    Culler mCuller;

    Float4 mTextColor;
};

WM5_REGISTER_INITIALIZE(Skeleton);
WM5_REGISTER_TERMINATE(Skeleton);

#endif
