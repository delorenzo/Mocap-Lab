//Julie De Lorenzo 2/13

#ifndef BONE_H
#define BONE_H
#include <string>

class Bone
{

public:
	Bone();
	Bone(int theid, std::string thename, float thedirection[3], float* thelength, int theaxis[3], int thedof[3], float thelimits[3][2]);
    bool OnInitialize ();
    void OnTerminate ();
    void OnIdle ();
	std::string name;
	int id;
	float dir[3];
	float length;
	float axis[3];
	int dof[3];
	float limits[3][2];
protected:
};


#endif
