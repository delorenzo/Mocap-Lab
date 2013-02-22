//Julie De Lorenzo 2/13

#ifndef BONE_H
#define BONE_H


class Bone
{

public:
	Bone(int theid, char* thename, float thedirection[3], float* thelength, int theaxis[3], int thedof[3], float thelimits[3][2]);
    virtual bool OnInitialize ();
    virtual void OnTerminate ();
    virtual void OnIdle ();

protected:
    

	int id;
	char * name;
	float dir[3];
	float length;
	float axis[3];
	int dof[3];
	float limits[3][2];
	Bone* parent;
};


#endif
