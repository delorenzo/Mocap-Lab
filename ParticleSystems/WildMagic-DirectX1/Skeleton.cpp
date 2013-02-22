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

	//Set up the bones
	//Read the .asf file into a buffer

	//close the file
	//TriMesh *cyl = sm.cylinder(as6dfla6skejrlka6sjer);

	//Try to read the first ASF file into memory
	if (read_buf("asf1.txt", asf1)) { //if successful
		//parse into the correct data structures
		parse_asf(asf1);
		free(asf1); //avoid memory leaks!
	}
	if (read_buf("asf2.txt", asf2)) {
		parse_asf(asf2);
		free(asf2);
	}

    // Set up the camera.
    mCamera->SetFrustum(60.0f, GetAspectRatio(), 1.0f, 1000.0f);
    APoint camPosition(4.0f, 0.0f, 0.0f);
    AVector camDVector(-1.0f, 0.0f, 0.0f);
    AVector camUVector(0.0f, 0.0f, 1.0f);
    AVector camRVector = camDVector.Cross(camUVector);
    mCamera->SetFrame(camPosition, camDVector, camUVector, camRVector);

    CreateScene();

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
void Skeleton::parse_asf(char* source) {
	//first find the number of bones in the file
	int numbones = strspn(source, "id"); 
	std::vector<Bone> bones(numbones);

	//then parse the file
	int id;
	char * name;
	float direction[3];
	float length;
	int axis[3];
	int dof[3];
	float limits[3][2];

	char * token = strtok(source, " ");
	while (token != NULL) {
		//if we hit :hierarchy we want to handle positional information, then return
		// (positional information is the end of the file)
		if (strcmp(token, ":hierarchy") == 0) {
			while (strcmp(token, "end") != 0) {
				if (strtok(token, "root") == 0) {
					token = strtok(source, " ");
					while (strcmp(token, "\n") != 0) {
						//find the bone with the appropriate name
						
						h.addchild(root, token);
						
					}
				}
				token = strtok(source, " ");
			}
			return;	
		}
		//if we hit a "begin" we have a new bone
		if (strcmp(token, "begin") == 0) {
			while (strcmp(token, "end") != 0) {
				if (strcmp("id", strtok(source, " ")) == 0) {
					id = atoi(strtok(source, " "));
				}
				if (strcmp("name", strtok(source, " ")) == 0) {
					name = strtok(source, " ");
				}
				if (strcmp("direction", strtok(source, " ")) == 0) {
					for (int i = 0; i < 3; i++) {
						direction[i] = atof(strtok(source, " "));
					}
				}
				if (strcmp("length", strtok(source, " ")) == 0) {
					length =  atof(strtok(source, " "));
				}
				if (strcmp("axis", strtok(source, " ")) == 0) {
					for (int i = 0 ; i < 3; i++) {
						axis[i] = atoi(strtok(source, " "));
					}
				}
				if (strcmp("dof", strtok(source, " ")) == 0) {
					token = strtok(source, " ");
					while (strcmp(token, "limits")!=0) {
						if (strcmp(token, "rx") == 0) { dof[0] = 1;}
						if (strcmp(token, "ry") == 0) { dof[1] = 1;}
						if (strcmp(token, "rz") == 0) { dof[2] = 1;}
						token = strtok(source, " ");
					}
					for (int i = 0; i < 3; i++) {
						if (dof[i] == 1) {
							limits[i][0] = atoi(strtok(source, " "));
							limits[i][1] = atoi(strtok(source, " "));
						}
					}
				}
				token = strtok(source, " ");
			}
			bones.push_back(Bone(id, name, direction, &length, axis, dof, limits));
		}

		

		token = strtok(source, " ");
	}
}
//----------------------------------------------------------------------------
bool Skeleton::read_buf(char* filename, char * source) {
	FILE *fp = fopen(filename, "r");
	source = NULL;
	if (fp!= NULL) {
		if (fseek(fp, 0L, SEEK_END) == 0) {
			long bufsize = ftell(fp); //get the size of the file
			if (bufsize == -1) { 
				std::cout << "Size of file invalid.";
				return false; 
			}
			source = (char*)malloc(sizeof(char)*(bufsize+1));

			if (fseek(fp, 0L, SEEK_SET) == 0) { 
				std::cout << "Error returning to start of file.";
				return false; 
			}

			//Read the file into memory
			size_t asf_len = fread(source, sizeof(char), bufsize, fp);
			if (asf_len == 0) { 
				std::cout << "Error reading file.";
				return false;
			}
			else {
				source[++asf_len] = '\0';
			}

		}
		fclose(fp);
		return true;
	}
	else {
		return false;
	}
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
//----------------------------------------------------------------------------
void Skeleton::CreateScene ()
{

	
	//note:  look at skinnedbiped.cpp for info on how to represent
	//specifically KeyFrameController*

	/*
    mScene = new0 Node();
    mWireState = new0 WireState();
    mRenderer->SetOverrideWireState(mWireState);

    VertexFormat* vformat = VertexFormat::Create(2,
        VertexFormat::AU_POSITION, VertexFormat::AT_FLOAT3, 0,
        VertexFormat::AU_TEXCOORD, VertexFormat::AT_FLOAT2, 0);
    int vstride = vformat->GetStride();

    const int numBones  = 1;
    VertexBuffer* vbuffer = new0 VertexBuffer(4*numBones, vstride);
    Float4* positionSizes = new1<Float4>(numBones);
	Line3D line = new0 Line3D();
    Particles* particles = new0 Particles(vformat, vbuffer, sizeof(int),
        positionSizes, 1.0f);

    particles->AttachController(new0 BloodCellController());
    mScene->AttachChild(particles);

    // Create an image with transparency.
    const int xsize = 32, ysize = 32;
    Texture2D* texture = new0 Texture2D(Texture::TF_A8R8G8B8, xsize,
        ysize, 1);
    unsigned char* data = (unsigned char*)texture->GetData(0);

    float factor = 1.0f/(xsize*xsize + ysize*ysize);
    for (int y = 0, i = 0; y < ysize; ++y)
    {
        for (int x = 0; x < xsize; ++x)
        {
            // The image is red.
            data[i++] = 0;
            data[i++] = 0;
            data[i++] = 255;

            // Semitransparent within a disk, dropping off to zero outside the
            // disk.
            int dx = 2*x - xsize;
            int dy = 2*y - ysize;
            float value = factor*(dx*dx + dy*dy);
            if (value < 0.125f)
            {
                value = Mathf::Cos(4.0f*Mathf::PI*value);
            }
            else
            {
                value = 0.0f;
            }
            data[i++] = (unsigned char)(255.0f*value);
        }
    }

    Texture2DEffect* effect = new0 Texture2DEffect(Shader::SF_LINEAR);
    effect->GetAlphaState(0, 0)->BlendEnabled = true;
    effect->GetDepthState(0, 0)->Enabled = false;
    particles->SetEffectInstance(effect->CreateInstance(texture));
	*/
}
//----------------------------------------------------------------------------
