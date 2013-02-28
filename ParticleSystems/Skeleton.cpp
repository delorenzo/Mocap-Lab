//Julie De Lorenzo
//2-27-13
//Never ever use WildMagic for anything.  Please with God as my witness just walk away

#include "Bone.h"
#include "Skeleton.h"
#include "Keyframe.h"
#include <fstream>
#include <string>
#include <cerrno>
#include <iostream>
#include <sstream>
#include "Wm5WindowApplication3.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

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
//Function:  OnInitialize()
//Does:  sets up the scene, grabs the files into buffers, parses those files, then sets up the camera
bool Skeleton::OnInitialize ()
{
    if (!WindowApplication3::OnInitialize())
    {
        return false;
    }

	mScene = new0 Node(); //set up scene

	//grab the ASF file into a string buffer
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
	
	//grab the AMC files into two string buffers
	AMC = get_file_contents("02_01.amc");
	AMC2 = get_file_contents("17_06.amc");

	//Now we want to parse the AMC files!
	//Parse this data into individual frames (from root to left toe) for the entire buffer.
	//Store this data EXACTLY how it needs to be used in the animation.  Use atof, multiplying the the angles by Mathf::DEG_TO_RAD
	std::map<int, Keyframe> keyframe_data;
	parse_amc(AMC, keyframe_data, bonemap);

    // Set up the camera.
    mCamera->SetFrustum(60.0f, GetAspectRatio(), 1.0f, 1000.0f);
    APoint camPosition(26.73f, 0.0f, 0.0f);
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
//Function: parse_amc
//Does:  Parses the AMC data from a std::string buffer and places it in a std::map of keyframes (which itself is a std::map of Nodes)
void Skeleton::parse_amc(std::string source, std::map<int, Keyframe> keyframe_map, std::map<std::string, Bone> bone_data)
{
	//we want a stringstream and a line buffer because we're going to parse line by line
	std::istringstream f(source);
	std::string line;
	//get the string file down to where the good stuff starts 
	while(getline(f,line)) {
		if (line == ":DEGREES") {
			std::getline(f, line);
			break;
		}
	}
	//now we start really parsing
	int index = 1;
	while(getline(f,line)) {

		//we want to separate (by spaces) the line into tokens
		std::string buf;
		stringstream ss(line); 
		vector<string> tokens;
		while (ss >> buf) {
			tokens.push_back(buf); 
		}

		
		//go through the tokens
		Float3 a_float, root_transf; //this controls the rotation
		std::map<std::string, Float3> n; 
		int i = 1;

		//check if the first char is an integer value
		if (isdigit(tokens[0].front())) {
			index++; //increment index of the keyframe map
		}

		//check for the root case
		else if (tokens[0] == "root") {
			for (int i = 1; i < tokens.size()-1; i++) {
				for (int k = 1; k < 4; k++) {
					root_transf[k] = atof(tokens[k].c_str());
				}

				for (int j = 4; j < 7; j++) {
					a_float[j] = atof(tokens[0].c_str());
				}
				n["root"] = a_float;
			}
		}

		//read in the values for the rotation
		else {
			if (bone_data[tokens[0]].dof[0] == 1) { 
				a_float[0] = atof(tokens[i].c_str());
				i++;
			}
			if (bone_data[tokens[0]].dof[1] == 1) {
				a_float[1] = atof(tokens[i].c_str());
				i++;
			}
			if (bone_data[tokens[0]].dof[2] == 1) {
				a_float[2] = atof(tokens[i].c_str());
			}
			n[tokens[0]] = a_float;
		}
		
	}

}


//----------------------------------------------------------------------------
//Function:  parse_hierarchy
//Does:  parses the hierarchy from a std::string, performs the necessary transformations on the bone data from a map,
//then sticks the hierarchy data (bone data + child information) in the appropriate map
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

	n["root"] = root; //add the root to the map

	//we want to parse line by line
	std::istringstream f(source);
	std::string line;
	//we want to skip down to wear the hierarchy starts
	while (std::getline(f, line)) {
		if (line == ":hierarchy") {
			std::getline(f, line); //this skips out of the hierarchy
			break;
		}
	}

	std::getline(f, line); //this skips out of the "begin"

	//now for serious parsing time
	while (line != "  end") { //until we hit end

		//we want to space-separate the line and parse through each token
		std::string buf; //buffer
		stringstream ss(line);
		vector<string> tokens; //holds tokens
		//add the tokens
		while (ss >> buf) {
			tokens.push_back(buf);
		}

		//put the data in the map from the tokens
		std::string parent = tokens[0]; ///the parent is always the first token
		for (int i = 1; i < tokens.size(); i++) {
			Node * cur = n[parent];   //parent
			Node * child = new0 Node(); //child
			cur->AttachChild(child); //attach child to parent
			n[tokens[i]] = child; //add child to map
			
			//draw the bone
			TriMesh * ptr = sm.Cylinder(10, 10, 0.1, 1, false);
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

			//Step #2: rotate the cylinder along the crossproduct axis by deg (dot of Z, dir)
			rotate.MakeRotation(crossproduct,deg);
			ptr->LocalTransform.SetRotate(rotate * cur_matrix);

			//translate the node that we're going to attach the cylinder to by the parent's translation information
			float parent_length = 0;
			APoint parent_dir; 
			Bone parentbone = b[parent]; //the node's parent bone
			if (parent == "root") {
				//there's no bone data for the parent
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
			ptr->SetEffectInstance(Texture2DEffect::CreateUniqueInstance(texture, Shader::SF_LINEAR, Shader::SC_CLAMP_EDGE, Shader::SC_CLAMP_EDGE));
			//attach the Node to the bone
			child->AttachChild(ptr);
			
		}
		std::getline(f, line);
	}
	mScene->AttachChild(root);
}
//----------------------------------------------------------------------------
//Function: parse_asf
//Parses the asf file from a std::string and spits it out as a std::vector of bones
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
					if (tokens[i] == "limits") {
						i++;
						if (tokens[i] == "rx") { 
							dof[0] = 1;
							i++;
						}
						if (tokens[i] == "ry") { 
							dof[1] = 1;
							i++;
						}
						if (tokens[i] == "rz") { 
							dof[2] = 1;
							i++;
						}
						
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

//----------------------------------------------------------------------------
//Function: get_file_contents
//Gets the file and returns it as a std::string buffer
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
//Function:  OnTerminate()
//Handles termination of the program
void Skeleton::OnTerminate ()
{
    mScene = 0;
    mWireState = 0;

    WindowApplication3::OnTerminate();
}
//----------------------------------------------------------------------------
//Function:  OnIdle()
//Handles idle behavior
void Skeleton::OnIdle ()
{
	
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
	
}
//----------------------------------------------------------------------------
//Function: OnKeyDown
//This handles key presses (just switches from wire state to not for now)
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