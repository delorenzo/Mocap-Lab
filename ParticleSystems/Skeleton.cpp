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

	step = 0; //init skele animation steps to 0
	step2 = 0;
	mScene = new0 Node(); //set up scene

	//grab the ASF file into a string buffer
	s = get_file_contents("02.asf");
	s2 = get_file_contents("17.asf");

	//parse into the correct data structures
	//TriMesh *tempmesh;
	std::vector<Bone> bones;

	//parse the bones (returns a vector::bones)
	parse_asf(s, bones);

	//add the bone vector into a hashmap
	for (int i = 0; i < bones.size(); i++) {
		Bone b = bones[i];
		bonemap[b.name] = b;
	}

	//parse the second bones and add into a bonemap (skele 2)
	std::vector<Bone> bones2;
	parse_asf(s2, bones2);
	for (int i = 0; i < bones2.size(); i++) {
		Bone b = bones2[i];
		bonemap2[b.name] = b;
	}
	s = get_file_contents("02.asf");
	s2 = get_file_contents("17.asf");

	
	//grab the AMC files into two string buffers
	AMC = get_file_contents("02_01.amc");
	AMC2 = get_file_contents("17_06.amc");


	//Now we want to parse the AMC files!
	//Parse this data into individual frames (from root to left toe) for the entire buffer.
	parse_amc(AMC);
	parse_amc2(AMC2);

    // Set up the camera.
    mCamera->SetFrustum(60.0f, GetAspectRatio(), 1.0f, 1000.0f);
    APoint camPosition(70.0f, 0.0f, 0.0f);
    AVector camDVector(-1.0f, 0.0f, 0.0f);
    AVector camUVector(0.0f, 1.0f, 0.0f);
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
void Skeleton::parse_amc(std::string source)
{
	

	//we want a stringstream and a line buffer because we're going to parse line by line
	std::istringstream f(source);
	std::string line;
	

	//get the string file down to where the good stuff starts 
	while(getline(f,line)) {
		if (line == ":DEGREES\r") {
			std::getline(f, line);
			break;
		}
	}

	//now we start really parsing
	int index = 0;
	
	std::map<std::string, HMatrix> n;  //map of rotation matrices
	while(getline(f,line)) {

		//we want to separate (by spaces) the line into tokens
		Keyframe frame;
		std::string buf;
		stringstream ss(line); 
		vector<string> tokens;
		while (ss >> buf) {
			tokens.push_back(buf); 
		}

		APoint root_transf; //declare root transformation information
		APoint root_rot;
		//go through the tokens
		Float3 a_float; //this controls the rotation
		HMatrix rot_matrix = HMatrix::IDENTITY; //matrix to store final rotation information
		
		
		int k = 1; //index for reading in rotation angles of bones

		//check if the first char is an integer value
		if (isdigit(tokens[0].front())) {

			frame.matrix_map = n;
			keyframe_data[index] = frame;

			index++; //increment index of the keyframe map
		}

		//check for the root case
		else if (tokens[0] == "root") {
			for (int i = 0; i < 3; i++) {
				root_transf[i] = atof(tokens[i+1].c_str());
			}

			for (int j = 0; j < 3; j++) {
				root_rot[j] = Mathf::DEG_TO_RAD * atof(tokens[3+j].c_str());
			}
			frame.root_translation = root_transf;
			frame.root_rotation = root_rot;
			
		}

		
		//read in the values for the rotation
		//take care to convert degrees to radians!!!! :)
		
		else {
			if (bonemap[tokens[0]].dof[0] == 1) { 
				a_float[0] = Mathf::DEG_TO_RAD * atof(tokens[k].c_str());
				k++;
				rot_matrix = rotation(a_float[0], X); 
			}
			if (bonemap[tokens[0]].dof[1] == 1) {
				a_float[1] = Mathf::DEG_TO_RAD * atof(tokens[k].c_str());
				k++;
				if (rot_matrix == HMatrix::IDENTITY) {
					rot_matrix = rotation(a_float[1], Y); 
				}
				else {
					rot_matrix = (rotation(a_float[1], Y) * rot_matrix); 
				}
			}
			if (bonemap[tokens[0]].dof[2] == 1) {
				a_float[2] = Mathf::DEG_TO_RAD * atof(tokens[k].c_str());
				if (rot_matrix == HMatrix::IDENTITY) {
					rot_matrix = rotation(a_float[2], Z); 
				}
				else {
					rot_matrix = (rotation(a_float[2], Z) * rot_matrix); 
				}
			}


			n[tokens[0]] = rot_matrix;
		}

	}
 	printf("Sean makes good tea.");
	keyframe_steps = index;
}

//----------------------------------------------------------------------------
//Function: parse_amc
//Does:  Parses the AMC data from a std::string buffer and places it in a std::map of keyframes (which itself is a std::map of Nodes)
void Skeleton::parse_amc2(std::string source)
{
	

	//we want a stringstream and a line buffer because we're going to parse line by line
	std::istringstream f(source);
	std::string line;
	

	//get the string file down to where the good stuff starts 
	while(getline(f,line)) {
		if (line == ":DEGREES\r") {
			std::getline(f, line);
			break;
		}
	}

	//now we start really parsing
	int index = 0;
	
	std::map<std::string, HMatrix> n;  //map of rotation matrices
	while(getline(f,line)) {

		//we want to separate (by spaces) the line into tokens
		Keyframe frame;
		std::string buf;
		stringstream ss(line); 
		vector<string> tokens;
		while (ss >> buf) {
			tokens.push_back(buf); 
		}

		APoint root_transf; //declare root transformation information
		APoint root_rot;
		//go through the tokens
		Float3 a_float; //this controls the rotation
		HMatrix rot_matrix = HMatrix::IDENTITY; //matrix to store final rotation information
		
		
		int k = 1; //index for reading in rotation angles of bones

		//check if the first char is an integer value
		if (isdigit(tokens[0].front())) {

			frame.matrix_map = n;
			keyframe_data2[index] = frame;

			index++; //increment index of the keyframe map
		}

		//check for the root case
		else if (tokens[0] == "root") {
			for (int i = 0; i < 3; i++) {
				root_transf[i] = atof(tokens[i+1].c_str());
			}

			for (int j = 0; j < 3; j++) {
				root_rot[j] = Mathf::DEG_TO_RAD * atof(tokens[3+j].c_str());
			}
			frame.root_translation = root_transf;
			frame.root_rotation = root_rot;
			
		}

		
		//read in the values for the rotation
		//take care to convert degrees to radians!!!! :)
		
		else {
			if (bonemap2[tokens[0]].dof[0] == 1) { 
				a_float[0] = Mathf::DEG_TO_RAD * atof(tokens[k].c_str());
				k++;
				rot_matrix = rotation(a_float[0], X); 
			}
			if (bonemap2[tokens[0]].dof[1] == 1) {
				a_float[1] = Mathf::DEG_TO_RAD * atof(tokens[k].c_str());
				k++;
				if (rot_matrix == HMatrix::IDENTITY) {
					rot_matrix = rotation(a_float[1], Y); 
				}
				else {
					rot_matrix = (rotation(a_float[1], Y) * rot_matrix); 
				}
			}
			if (bonemap2[tokens[0]].dof[2] == 1) {
				a_float[2] = Mathf::DEG_TO_RAD * atof(tokens[k].c_str());
				if (rot_matrix == HMatrix::IDENTITY) {
					rot_matrix = rotation(a_float[2], Z); 
				}
				else {
					rot_matrix = (rotation(a_float[2], Z) * rot_matrix); 
				}
			}


			n[tokens[0]] = rot_matrix;
		}

	}
 	printf("Sean makes good tea.");
	keyframe_steps2= index;
}
//----------------------------------------------------------------------------
//Function:  parse_hierarchy
//Does:  parses the hierarchy from a std::string, performs the necessary transformations on the bone data from a map,
//then sticks the hierarchy data (bone data + child information) in the appropriate map
void Skeleton::parse_hierarchy2(std::string source) {
	//init nodemap2
	nodemap2 = map<string, Node*>();

	//skeleton drawing stuff
	mWireState = new0 WireState();
	mRenderer->SetOverrideWireState(mWireState);

	Node * root = new0 Node();
	root->LocalTransform.SetTranslate(*root_transf);
    VertexFormat* vformat = VertexFormat::Create(2,
        VertexFormat::AU_POSITION, VertexFormat::AT_FLOAT3, 0,
        VertexFormat::AU_TEXCOORD, VertexFormat::AT_FLOAT2, 0);
    int vstride = vformat->GetStride();
	StandardMesh sm(vformat);

	std::string path = Environment::GetPathR("SphereMap.wmtf");
	Texture2D* texture = Texture2D::LoadWMTF(path);
    Texture2DEffect* geomEffect = new0 Texture2DEffect(Shader::SF_LINEAR);

    //mRectangle->SetEffectInstance(geomEffect->CreateInstance(texture));

	nodemap2["root"] = root; //add the root to the map

	//we want to parse line by line
	std::istringstream f(source);
	std::string line;
	//we want to skip down to wear the hierarchy starts
	while (std::getline(f, line)) {
		if ((line == ":hierarchy\r") || (line == ":hierarchy")) {
			std::getline(f, line); //this skips out of the hierarchy
			break;
		}
	}

	std::getline(f, line); //this skips out of the "begin"

	//now for serious parsing time
	while ((line != "  end\r") && (line != "  end")) { //until we hit end
		
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
			Node * cur = nodemap2[parent];   //parent
			Node * child = new0 Node(); //child
			cur->AttachChild(child); //attach child to parent
			nodemap2[tokens[i]] = child; //add child to map
			
			//draw the bone
			TriMesh * ptr = sm.Cylinder(10, 10, 0.1f, 1, false);
			//TriMesh * ptr = sm.Sphere(10, 10, 0.1f);
			Bone bone = bonemap2[tokens[i]];

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
			float deg = Mathf::ACos(z.Dot(dir)); //degrees between Z axis and the direction of the bone (inverse cosine of dot product between Z axis, dir)

			//Step #2: rotate the cylinder along the crossproduct axis by deg (dot of Z, dir)
			rotate.MakeRotation(crossproduct,deg);
			ptr->LocalTransform.SetRotate(rotate * cur_matrix);

			//translate the node that we're going to attach the cylinder to by the parent's translation information
			float parent_length = 0;
			APoint parent_dir; 
			Bone parentbone = bonemap2[parent]; //the node's parent bone
			if (parent == "root") {
				//there's no bone data for the parent
				//so we grab our parent info!
				
				
			}

			else {
				//set the dir from the parent bone
				for (int j = 0; j < 3; j++) {
					parent_dir[j] = parentbone.dir[j];
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
//Function:  parse_hierarchy
//Does:  parses the hierarchy from a std::string, performs the necessary transformations on the bone data from a map,
//then sticks the hierarchy data (bone data + child information) in the appropriate map
void Skeleton::parse_hierarchy(std::string source) {
	nodemap = map<string, Node*>();
	//skeleton drawing stuff
	mWireState = new0 WireState();
	mRenderer->SetOverrideWireState(mWireState);

	Node * root = new0 Node();
	root->LocalTransform.SetTranslate(*root_transf);
    VertexFormat* vformat = VertexFormat::Create(2,
        VertexFormat::AU_POSITION, VertexFormat::AT_FLOAT3, 0,
        VertexFormat::AU_TEXCOORD, VertexFormat::AT_FLOAT2, 0);
    int vstride = vformat->GetStride();
	StandardMesh sm(vformat);

	std::string path = Environment::GetPathR("Water.wmtf");
	Texture2D* texture = Texture2D::LoadWMTF(path);
    Texture2DEffect* geomEffect = new0 Texture2DEffect(Shader::SF_LINEAR);

    //mRectangle->SetEffectInstance(geomEffect->CreateInstance(texture));

	nodemap["root"] = root; //add the root to the map

	//we want to parse line by line
	std::istringstream f(source);
	std::string line;
	//we want to skip down to wear the hierarchy starts
	while (std::getline(f, line)) {
		if ((line == ":hierarchy\r") || (line == ":hierarchy")) {
			std::getline(f, line); //this skips out of the hierarchy
			break;
		}
	}

	std::getline(f, line); //this skips out of the "begin"

	//now for serious parsing time
	while ((line != "  end\r") && (line != "  end")) { //until we hit end
		
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
			Node * cur = nodemap[parent];   //parent
			Node * child = new0 Node(); //child
			cur->AttachChild(child); //attach child to parent
			nodemap[tokens[i]] = child; //add child to map
			
			//draw the bone
			TriMesh * ptr = sm.Cylinder(10, 10, 0.1f, 1, false);
			//TriMesh * ptr = sm.Sphere(10, 10, 0.1f);
			Bone bone = bonemap[tokens[i]];

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
			float deg = Mathf::ACos(z.Dot(dir)); //degrees between Z axis and the direction of the bone (inverse cosine of dot product between Z axis, dir)

			//Step #2: rotate the cylinder along the crossproduct axis by deg (dot of Z, dir)
			rotate.MakeRotation(crossproduct,deg);
			ptr->LocalTransform.SetRotate(rotate * cur_matrix);

			//translate the node that we're going to attach the cylinder to by the parent's translation information
			float parent_length = 0;
			APoint parent_dir; 
			Bone parentbone = bonemap[parent]; //the node's parent bone
			if (parent == "root") {
				//there's no bone data for the parent
				//so we grab our parent info!
				
				
			}

			else {
				//set the dir from the parent bone
				for (int j = 0; j < 3; j++) {
					parent_dir[j] = parentbone.dir[j];
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

	//skip past the beginning information to the root position
	int i = 0;
	for (int i = 0; i < tokens.size(); i++) {
	  if (tokens[i] == "position") {
		  i++;
		  break;
	  }
	} 
	root_transf = new APoint(atof(tokens[i].c_str()), atof(tokens[i+1].c_str()), atof(tokens[i+2].c_str()));
	i += 4; //get past three position tokens (typically 0 0 0) and the orientation token
	root_rot = new APoint(atof(tokens[i].c_str()), atof(tokens[i+1].c_str()), atof(tokens[i+2].c_str()));


	//now parse the bones
	for (int i = 0; i < tokens.size(); i++) {
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
					if (tokens[i] == "rx") { 
						dof[0] = 1;
						i++;
					}
					else { dof[0] = 0;}
					if (tokens[i] == "ry") { 
						dof[1] = 1;
						i++;
					}
					else { dof[1] = 0;}
					if (tokens[i] == "rz") { 
						dof[2] = 1;
						i++;
					}
					else {dof[2] = 0;}
					i++;
				}//end if tokens[i] == dof
				if (tokens[i] == "limits") {	
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
						} //end if dof[j] == 1
					} //end for loop
				} //end if tokens[i] == "limits"

				i++;
			}
			bones.push_back(Bone(id, name, direction, &length, axis, dof, limits));
		}
	} 
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
	if (animate_on) {
		if (cur_subject == 1) {

			animate_skele(step);
			step++;
			if (step > keyframe_steps) { step = 0;}
		}

		else if (cur_subject == 2) {
			animate_skele2(step);
			step++;
			if (step > keyframe_steps2) { step = 0;}
		}

		else if (cur_subject == 3) {
			animate_skele(step);
			animate_skele2(step2);
			step++;
			step2++;
			if (step > keyframe_steps) { step = 0;}
			if (step2 > keyframe_steps2) { step2 = 0;}

		}

		else {
		
		}

	}
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
	mScene = new0 Node();
	// Initial update of objects.
    mScene->Update();

    // Initial culling of scene.
    mCuller.SetCamera(mCamera);
    mCuller.ComputeVisibleSet(mScene);

    InitializeCameraMotion(0.001f, 0.001f);
    InitializeObjectMotion(mScene);
    switch (key)
    {

	case '0': { //reset
		animate_on = 0;
		switch (cur_subject) {
		case 1: { 
			parse_hierarchy(s);  
			break;
		}
		case 2: {
			parse_hierarchy2(s2);
			break;
		}
		case 3: {
			parse_hierarchy2(s);
			parse_hierarchy2(s2);
				}
		default: break;

		}
	}
	case '1': { //display subject 1
		parse_hierarchy(s);
		animate_on = 0;
		cur_subject = 1;
		break;
			  }
	case '2': { //animate subject 1
		animate_on = 1;
		int step = 0;
		parse_hierarchy(s);
		cur_subject = 1;
		break;
			  }
	case '3': { //display and animate subject 2
		parse_hierarchy2(s);
		int step = 0;
		cur_subject = 2;
		animate_on = 1;
		break;
			  }
	case '4':  { //display and animate subject 1 & 2
		parse_hierarchy(s);
		parse_hierarchy2(s);
		animate_on = 1;
		cur_subject = 3;
		break;
			   }
	case 'n': {
		
		animate_skele(step);
		step++;
		return true;
	}
    case 'w':
    case 'W':
        mWireState->Enabled = !mWireState->Enabled;
        return true;
    }

    return false;
}
//----------------------------------------------------------------------------
//Function: animate_skele()
//This animates the skeleton one frame at a time
void Skeleton::animate_skele(int step)
{
	/*
		n["root_rotation"] = a_float;
				n["root_translation"] = root_transf;
	*/
	//Step #1:  Apply transformation information of the root
	//(This took like 3 hours)
	Keyframe frame = keyframe_data[step];
	nodemap["root"]->LocalTransform.SetTranslate(frame.root_translation);

	//define local axis transformation L
	//(this never worked - sorry not sorry) #YOLO

	for (std::map<std::string, HMatrix>::iterator it=frame.matrix_map.begin(); it!=frame.matrix_map.end(); ++it) {
		if (it->first == "root") { continue;} //don't fuck with root! 
		
		HMatrix L;
		HMatrix L_inverse;

		float deg = Mathf::DEG_TO_RAD * bonemap[it->first].axis[2];
		L = rotation(deg, Z);
		deg = Mathf::DEG_TO_RAD * bonemap[it->first].axis[1];
		L = L * rotation (deg, Y);
		deg = Mathf::DEG_TO_RAD * bonemap[it->first].axis[0];
		L = L * rotation (deg, X);

		L_inverse = L.Inverse(); //definse L inverse
		HMatrix rot = frame.matrix_map[it->first];
		HMatrix final = L * rot * L_inverse;
		nodemap[it->first]->LocalTransform.SetRotate(final);

	}

	printf("WildMagic is a pizza burn on the roof of the mouth of the world.");
}

//----------------------------------------------------------------------------
//Function: animate_skele()
//This animates the skeleton one frame at a time
void Skeleton::animate_skele2(int step)
{
	/*
		n["root_rotation"] = a_float;
				n["root_translation"] = root_transf;
	*/
	//Step #1:  Apply transformation information of the root
	//(This took like 3 hours)
	Keyframe frame = keyframe_data2[step];
	nodemap2["root"]->LocalTransform.SetTranslate(frame.root_translation);

	//define local axis transformation L
	//(this never worked - sorry not sorry) #YOLO

	for (std::map<std::string, HMatrix>::iterator it=frame.matrix_map.begin(); it!=frame.matrix_map.end(); ++it) {
		if (it->first == "root") { continue;} //don't fuck with root! 
		
		HMatrix L;
		HMatrix L_inverse;

		float deg = Mathf::DEG_TO_RAD * bonemap[it->first].axis[2];
		L = rotation(deg, Z);
		deg = Mathf::DEG_TO_RAD * bonemap[it->first].axis[1];
		L = L * rotation (deg, Y);
		deg = Mathf::DEG_TO_RAD * bonemap[it->first].axis[0];
		L = L * rotation (deg, X);

		L_inverse = L.Inverse(); //definse L inverse
		HMatrix rot = frame.matrix_map[it->first];
		HMatrix final = L_inverse * rot * L;
		nodemap2[it->first]->LocalTransform.SetRotate(final);

	}

	printf("WildMagic is a pizza burn on the roof of the mouth of the world.");
}

/*******************************************************
FUNCTION: rotation
ARGS: angle and axis
RETURN:
DOES: returns a rotation matrix by degree on axis
********************************************************/
HMatrix Skeleton::rotation(float deg, int axis) {
	HMatrix answer;
	if (axis == X) { answer = rotation_x(deg);}
	else if (axis == Y) { answer = rotation_y(deg);}
	else if (axis == Z) { answer = rotation_z(deg);}
	return answer;
}


/*******************************************************
FUNCTION: rotation_x
ARGS: angle and axis
RETURN:
DOES: rotates x axis by deg
********************************************************/
HMatrix Skeleton::rotation_x(float deg) {
	float c = cos(deg);
	float s = sin(deg);
	
	HMatrix *x = new0 HMatrix( 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, c, -s, 0.0f, 0.0f, s, c, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	return *x;
}
/*******************************************************
FUNCTION: rotation_y
ARGS: angle and axis
RETURN:
DOES: rotates y axis by deg
********************************************************/
HMatrix Skeleton::rotation_y(float deg) {
	double c = cos(deg);
	double s = sin(deg);
	HMatrix *y = new0 HMatrix(c, 0.0, s, 0.0, 0.0, 1.0, 0.0, 0.0, -s, 0.0, c, 0.0, 0.0, 0.0, 0.0, 1.0);
	return *y;

}
/*******************************************************
FUNCTION: rotation_z
ARGS: angle and axis
RETURN:
DOES: rotates z axis by deg
********************************************************/
HMatrix Skeleton::rotation_z(float deg) {
	double c = cos(deg);
	double s = sin(deg);
	HMatrix *z = new HMatrix(c, -s, 0.0, 0.0, s, c, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0);
	return *z;
}

//not a function but dogs are really cute
/***********************************************************

                                         do. 
                                        :NOX 
                                       ,NOM@: 
                                       :NNNN: 
                                       :XXXON 
                                       :XoXXX. 
                                       MM;ONO: 
  .oob..                              :MMO;MOM 
 dXOXYYNNb.                          ,NNMX:MXN 
 Mo"'  '':Nbb                        dNMMN MNN: 
 Mo  'O;; ':Mb.                     ,MXMNM MNX: 
 @O :;XXMN..'X@b.                  ,NXOMXM MNX: 
 YX;;NMMMM@M;;OM@o.                dXOOMMN:MNX: 
 'MOONM@@@MMN:':NONb.            ,dXONM@@MbMXX: 
  MOON@M@@MMMM;;:OOONb          ,MX'"':ONMMMMX: 
  :NOOM@@MNNN@@X;""XNN@Mb     .dP"'   ,..OXM@N: 
   MOON@@MMNXXMMO  :M@@M...@o.oN""":OOOXNNXXOo:
   :NOX@@@MNXXXMNo :MMMM@K"`,:;NNM@@NXM@MNO;.'N. 
    NO:X@@MNXXX@@O:'X@@@@MOOOXMM@M@NXXN@M@NOO ''b 
    `MO.'NMNXXN@@N: 'XXM@NMMXXMM@M@XO"'"XM@X;.  :b 
     YNO;'"NXXXX@M;;::"XMNN:""ON@@MO: ,;;.:Y@X: :OX. 
      Y@Mb;;XNMM@@@NO: ':O: 'OXN@@MO" ONMMX:`XO; :X@. 
      '@XMX':OX@@MN:    ;O;  :OX@MO" 'OMM@N; ':OO;N@N 
       YN;":.:OXMX"': ,:NNO;';XMMX:  ,;@@MNN.'.:O;:@X: 
       `@N;;XOOOXO;;:O;:@MOO;:O:"" ,oMP@@K"YM.;NMO;`NM 
        `@@MN@MOX@@MNMN;@@MNXXOO: ,d@NbMMP'd@@OX@NO;.'bb. 
       .odMX@@XOOM@M@@XO@MMMMMMNNbN"YNNNXoNMNMO"OXXNO.."";o. 
     .ddMNOO@@XOOM@@XOONMMM@@MNXXMMo;."' .":OXO ':.'"'"'  '""o. 
    'N@@X;,M@MXOOM@OOON@MM@MXOO:":ONMNXXOXX:OOO               ""ob. 
   ')@MP"';@@XXOOMMOOM@MNNMOO""   '"OXM@MM: :OO.        :...';o;.;Xb. 
  .@@MX" ;X@@XXOOM@OOXXOO:o:'      :OXMNO"' ;OOO;.:     ,OXMOOXXXOOXMb 
 ,dMOo:  oO@@MNOON@N:::"      .    ,;O:""'  .dMXXO:    ,;OX@XO"":ON@M@ 
:Y@MX:.  oO@M@NOXN@NO. ..: ,;;O;.       :.OX@@MOO;..   .OOMNMO.;XN@M@P 
,MP"OO'  oO@M@O:ON@MO;;XO;:OXMNOO;.  ,.;.;OXXN@MNXO;.. oOX@NMMN@@@@@M: 
`' "O:;;OON@@MN::XNMOOMXOOOM@@MMNXO:;XXNNMNXXXN@MNXOOOOOXNM@NM@@@M@MP 
   :XN@MMM@M@M:  :'OON@@XXNM@M@MXOOdN@@@MM@@@@MMNNXOOOXXNNN@@M@MMMM"' 
   .oNM@MM@ONO'   :;ON@@MM@MMNNXXXM@@@@M@PY@@MMNNNNNNNNNNNM@M@M@@P' 
  ;O:OXM@MNOOO.   'OXOONM@MNNMMXON@MM@@b. 'Y@@@@@@@@@@@@@M@@MP"'" 
 ;O':OOXNXOOXX:   :;NMO:":NMMMXOOX@MN@@@@b.:M@@@M@@@MMM@"""" 
 :: ;"OOOOOO@N;:  'ON@MO.'":""OOOO@@NNMN@@@. Y@@@MMM@@@@b 
 :;   ':O:oX@@O;;  ;O@@XO'   "oOOOOXMMNMNNN@MN""YMNMMM@@MMo. 
 :N:.   ''oOM@NMo.::OX@NOOo.  ;OOOXXNNNMMMNXNM@bd@MNNMMM@MM@bb 
  @;O .  ,OOO@@@MX;;ON@NOOO.. ' ':OXN@NNN@@@@@M@@@@MNXNMM@MMM@, 
  M@O;;  :O:OX@@M@NXXOM@NOO:;;:,;;ON@NNNMM'`"@@M@@@@@MXNMMMMM@N 
  N@NOO;:oO;O:NMMM@M@OO@NOO;O;oOOXN@NNM@@'   `Y@NM@@@@MMNNMM@MM 
  ::@MOO;oO:::OXNM@@MXOM@OOOOOOXNMMNNNMNP      ""MNNM@@@MMMM@MP 
    @@@XOOO':::OOXXMNOO@@OOOOXNN@NNNNNNNN        '`YMM@@@MMM@P' 
    MM@@M:'''' O:":ONOO@MNOOOOXM@NM@NNN@P  -hrr-     "`"""MM' 
    ''MM@:     "' 'OOONMOYOOOOO@MM@MNNM" 
      YM@'         :OOMN: :OOOO@MMNOXM' 
      `:P           :oP''  "'OOM@NXNM' 
       `'                    ':OXNP' 
*************************************************************/