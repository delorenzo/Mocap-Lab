// Geometric Tools, LLC
// Copyright (c) 1998-2012
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
//
// File Version: 5.0.0 (2010/01/01)

#include "Bone.h"
#include "Skeleton.h"
#include "Hierarchy.h"
#include <fstream>
#include <string>
#include <cerrno>
#include <iostream>
#include <sstream>
using namespace std;

WM5_WINDOW_APPLICATION(Skeleton);

//----------------------------------------------------------------------------
Skeleton::Skeleton ()
    :
    WindowApplication3("SampleGraphics/ParticleSystems", 0, 0, 640, 480,
        Float4(0.5f, 0.0f, 1.0f, 1.0f)),
        mTextColor(1.0f, 1.0f, 1.0f, 1.0f)
{
}
//----------------------------------------------------------------------------
bool Skeleton::OnInitialize ()
{
    if (!WindowApplication3::OnInitialize())
    {
        return false;
    }

	mScene = new0 Node(); //set up scene

	//grab the file into a string buffer
	source = get_file_contents("asf1.txt");

	//parse into the correct data structures
	TriMesh *tempmesh;
	std::vector<Bone> bones;
	std::map<std::string, Bone> bonemap;
	//parse the bones (returns a vector::bones)
	parse_asf(source, bones);
	//add the bone vector into a hashmap
	for (int i = 0; i < bones.size(); i++) {
		Bone b = bones[i];
		bonemap[b.name] = b;
	}
	//create a hashmap for the nodes
	std::map<std::string, Node*> nodemap;
	//parse the hierarchy and rotate the bones according to the hierarchy, using the bone and node hashmap
	parse_hierarchy(source, nodemap, bonemap);
	
    // Set up the camera.
    mCamera->SetFrustum(60.0f, GetAspectRatio(), 1.0f, 1000.0f);
    APoint camPosition(4.0f, 0.0f, 0.0f);
    AVector camDVector(-1.0f, 0.0f, 0.0f);
    AVector camUVector(0.0f, 0.0f, 1.0f);
    AVector camRVector = camDVector.Cross(camUVector);
    mCamera->SetFrame(camPosition, camDVector, camUVector, camRVector);

    // Initial update of objects.
    mScene->Update();

    // Initial culling of scene.
    mCuller.SetCamera(mCamera);
    mCuller.ComputeVisibleSet(mScene);

    InitializeCameraMotion(0.001f, 0.001f);
    InitializeObjectMotion(mScene);
	
    return true;
}
//----------------------------------------------------------------------------
void Skeleton::draw_skel() {

}

//----------------------------------------------------------------------------
void Skeleton::parse_hierarchy(std::string source, std::map<std::string, Node*> n, std::map<std::string, Bone> b) {
	mWireState = new0 WireState();
	mRenderer->SetOverrideWireState(mWireState);
	Node * root = new0 Node();
	
    VertexFormat* vformat = VertexFormat::Create(2,
        VertexFormat::AU_POSITION, VertexFormat::AT_FLOAT3, 0,
        VertexFormat::AU_TEXCOORD, VertexFormat::AT_FLOAT2, 0);
    int vstride = vformat->GetStride();

	StandardMesh sm(vformat);

	std::string path = Environment::GetPathR("RedSky.wmtf");
	Texture2D* texture = Texture2D::LoadWMTF(path);
    Texture2DEffect* geomEffect = new0 Texture2DEffect(Shader::SF_LINEAR);

    //mRectangle->SetEffectInstance(geomEffect->CreateInstance(texture));

	n["root"] = root;
	std::istringstream f(source);
	std::string line;
	while (std::getline(f, line)) {
		if (line == ":hierarchy") {
			std::getline(f, line);
			break;
		}
	}
	std::getline(f, line);
	while (line != "  end") {
		std::string buf;
		stringstream ss(line);
		vector<string> tokens;
		while (ss >> buf) {
			tokens.push_back(buf);
		}
		std::string parent = tokens[0];
		for (int i = 1; i < tokens.size(); i++) {
			Node * cur = n[parent];   //parent
			Node * child = new0 Node(); //child
			cur->AttachChild(child); //attach child to parent
			n[tokens[i]] = child; //add child to map
			
			//draw the bone
			TriMesh * ptr = sm.Cylinder(10, 10, 0.05, 2, false);
			Bone bone = b[tokens[i]];

			//scale the cylinder
			ptr->LocalTransform.SetScale(APoint(1.0, 1.0, bone.length));

			//translate the cylinder
			float tx = bone.length * bone.dir[0] / 2;
			float ty = bone.length * bone.dir[1] / 2;
			float tz = bone.length * bone.dir[2] / 2;
			ptr->LocalTransform.SetTranslate(APoint(tx, ty, tz));
			HMatrix cur_matrix;

			//rotate the cylinder
			//Step #1:  Get rotation matrix of cylinder
			cur_matrix = ptr->LocalTransform.GetRotate(); //current rotation matrix of the cylinder
			HMatrix rotate;
			AVector z = AVector::UNIT_Z; //unit Z vector (0 0 1)
			AVector dir(bone.dir[0], bone.dir[1], bone.dir[2]); //direction of the bone in a vector
			AVector crossproduct = z.Cross(dir); //cross product of direction of the bone and the Z axis (new axis)
			float deg = Mathf::ACos(z.Dot(dir)); //degrees between Z axis and the direction of the bone

			//so we need to rotate the cylinder along the crossproduct axis by deg (dot of Z, dir)
			rotate.MakeRotation(crossproduct,deg);
			ptr->LocalTransform.SetRotate(rotate * cur_matrix);

			//translate the node that we're going to attach the cylinder to by the parent's translation information
			float parent_length = 0;
			APoint parent_dir; 
			Bone parentbone = b[parent]; //the node's parent bone
			if (parent == "root") {
				//there's no bone data
			}

			else {
				//set the dir from the parent bone
				for (int i = 0; i < 3; i++) {
					parent_dir[i] = parentbone.dir[i];
				}
				//set the length from the parent length
				parent_length = parentbone.length;

				//multiply the parent_dir by the length
				parent_dir *= parent_length;
			}

			//translate the child node
			child->LocalTransform.SetTranslate(parent_dir);

			//attach the Node to the bone
			child->AttachChild(ptr);
			
		}
		std::getline(f, line);
	}
	mScene->AttachChild(root);
}
//----------------------------------------------------------------------------
void Skeleton::parse_asf(std::string source, std::vector<Bone> &bones) {

	std::string buf; //string buffer
    stringstream ss(source); // Insert the string into a stream

    vector<string> tokens; // Create vector to hold our words

    while (ss >> buf) {
        tokens.push_back(buf);
	}

	//then parse the file
	int id;
	std::string name;
	float direction[3];
	float length;
	int axis[3];
	int dof[3];
	float limits[3][2];
	std::string token;

	
	int i = 0;
	do {
		//if we hit :hierarchy we want to handle positional information, then return
		// (positional information is the end of the file)
		if (tokens[i] == ":hierarchy") {
			return;	
		}
		//if we hit a "begin" we have a new bone
		if (tokens[i] == "begin") {
			i++;
			while (tokens[i] != "end") {
				if (tokens[i] =="id") {
					i++;
					id = atoi(tokens[i].c_str());
					i++;
				}
				if (tokens[i] == "name") {
					i++;
					name = tokens[i];
					i++;
				}
				if (tokens[i] == "direction") {
					i++;
					for (int j = 0; j < 3; j++) {
						direction[j] = (float)atof(tokens[i].c_str());
						i++;
					}
				}
				if (tokens[i] == "length") {
					i++;
					length =  (float)atof(tokens[i].c_str());
					i++;
				}
				if (tokens[i] == "axis") {
					i++;
					for (int j = 0 ; j < 3; j++) {
						axis[j] = atoi(tokens[i].c_str());
						i++;
					}
				}
				if (tokens[i] == "dof") {
					i++;
					token = tokens[i];
					while (tokens[i] == "limits") {
						i++;
						if (tokens[i] == "rx") { dof[0] = 1;}
						i++;
						if (tokens[i] == "ry") { dof[1] = 1;}
						i++;
						if (tokens[i] == "rz") { dof[2] = 1;}
						i++;
					}
					for (int j = 0; j < 3; j++) {
						if (dof[j] == 1) {
							std::string firstlimit = tokens[i];
							i++;
							firstlimit.erase(0, 1); //erase parenthesis at front e.g (28
							std::string secondlimit = tokens[i];
							i++;
							secondlimit.erase(secondlimit.size()-1, 1); //erase parenthesis at end e.g. 28) 
							limits[j][0] = (float)atof(firstlimit.c_str());
							limits[j][1] = (float)atof(secondlimit.c_str());
						}
					}
				}
				i++;
			}
			bones.push_back(Bone(id, name, direction, &length, axis, dof, limits));
		}
		i++;
	} while (i < tokens.size());
}



std::string Skeleton::get_file_contents(const char *filename)
{
	 std::ifstream in(filename, std::ios::in | std::ios::binary);
	if (in)
	 {
	  std::string contents;
	 in.seekg(0, std::ios::end);
	 contents.resize(in.tellg());
	 in.seekg(0, std::ios::beg);
		in.read(&contents[0], contents.size());
		in.close();
		return(contents);
	}
	 throw(errno);
}
//----------------------------------------------------------------------------
void Skeleton::OnTerminate ()
{
    mScene = 0;
    mWireState = 0;

    WindowApplication3::OnTerminate();
}
//----------------------------------------------------------------------------
void Skeleton::OnIdle ()
{
	/*
    MeasureTime();

    MoveCamera();
    MoveObject();
    mScene->Update(GetTimeInSeconds());
    mCuller.ComputeVisibleSet(mScene);

    if (mRenderer->PreDraw())
    {
        mRenderer->ClearBuffers();
        mRenderer->Draw(mCuller.GetVisibleSet());
        DrawFrameRate(8, GetHeight()-8, mTextColor);
        mRenderer->PostDraw();
        mRenderer->DisplayColorBuffer();
    }

    UpdateFrameCount();
	*/
}
//----------------------------------------------------------------------------
bool Skeleton::OnKeyDown (unsigned char key, int x, int y)
{
    if (WindowApplication3::OnKeyDown(key, x, y))
    {
        return true;
    }

    switch (key)
    {
    case 'w':
    case 'W':
        mWireState->Enabled = !mWireState->Enabled;
        return true;
    }

    return false;
}