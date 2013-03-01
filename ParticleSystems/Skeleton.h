//Julie De Lorenzo
//2-27-13

#ifndef SKELETON_H
#define SKELETON_H
#include "Bone.h"
#include "Keyframe.h"
#include "Wm5WindowApplication3.h"
#include <string>

#define X 0
#define Y 1
#define Z 2


using namespace Wm5;

class Skeleton : public WindowApplication3
{
    WM5_DECLARE_INITIALIZE;
    WM5_DECLARE_TERMINATE;

public:

    Skeleton ();

	//data structures for bones, nodes, keyframe
	std::map<std::string, Bone> bonemap;
	std::map<int, Keyframe> keyframe_data;
	std::map<std::string, Node*> nodemap;

	std::map<std::string, Bone> bonemap_2;
	std::map<int, Keyframe> keyframe_data_2;
	std::map<std::string, Node*> nodemap_2;

	APoint* root_transf;
	APoint* root_rot;

	//steps for slowly animating the skeleton (debugging)
	int step;

	//char * buffer files for the ASF files
	std::string source, AMC, AMC2;
	//functions
    bool OnInitialize ();
    void OnTerminate ();
    void OnIdle ();
    bool OnKeyDown (unsigned char key, int x, int y);
	std::string get_file_contents(const char *filename);
	void parse_asf(std::string source, std::vector<Bone> &b);
	void parse_hierarchy(std::string source);
	std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);
	std::vector<std::string> split(const std::string &s, char delim);
	void parse_amc(std::string source);
	void animate_skele(int step);

	HMatrix rotation(float deg, int axis) ;
	HMatrix rotation_x(float deg);
	HMatrix rotation_y(float deg);
	HMatrix rotation_z(float deg);
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
