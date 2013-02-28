//Julie De Lorenzo
//2-27-13

#ifndef SKELETON_H
#define SKELETON_H
#include "Bone.h"
#include "Keyframe.h"
#include "Wm5WindowApplication3.h"

using namespace Wm5;

class Skeleton : public WindowApplication3
{
    WM5_DECLARE_INITIALIZE;
    WM5_DECLARE_TERMINATE;

public:
    Skeleton ();
	//char * buffer files for the ASF files
	std::string source, AMC, AMC2;
	//functions
    bool OnInitialize ();
    void OnTerminate ();
    void OnIdle ();
    bool OnKeyDown (unsigned char key, int x, int y);
	std::string get_file_contents(const char *filename);
	void parse_asf(std::string source, std::vector<Bone> &b);
	void parse_hierarchy(std::string source, std::map<std::string, Node*> n, std::map<std::string, Bone> b);
	std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);
	std::vector<std::string> split(const std::string &s, char delim);
	void parse_amc(std::string source, std::map<std::string, Float3> keyframe);
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
