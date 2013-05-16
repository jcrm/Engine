////////////////////////////////////////////////////////////////////////////////
// Filename: applicationclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "applicationclass.h"


ApplicationClass::ApplicationClass()
{
	m_Input = 0;
	m_Direct3D = 0;
	m_Camera = 0;
	m_Terrain = 0;
	mFluid = 0;
	m_Timer = 0;
	m_Position = 0;
	m_Fps = 0;
	m_Cpu = 0;
	m_FontShader = 0;
	m_Text = 0;
	m_TerrainShader = 0;
	mFluidShader = 0;
	m_Light = 0;
	m_TextureShader = 0;
	m_TextureToTextureShader = 0;
	m_RenderFullSizeTexture = 0;
	m_DownSampleHalfSizeTexure = 0;
	m_FullSizeTexure = 0;
	m_FullScreenWindow = 0;
	m_ConvolutionShader = 0;
	m_ConvolutionHalfSizeTexture = 0;
	mGlowShader = 0;
	mMergerShader = 0;
	m_MergeFullSizeTexture = 0;
	for(int i = 0; i < MODEL_NUMBER; i++){
		m_Model[0] = 0;
	}
	mBlur = false;
}


ApplicationClass::ApplicationClass(const ApplicationClass& other)
{
}


ApplicationClass::~ApplicationClass()
{
}


bool ApplicationClass::Initialize(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight)
{
	bool result;
	int downSampleWidth, downSampleHeight;

	// Set the size to sample down to.
	downSampleWidth = screenWidth / 2;
	downSampleHeight = screenHeight / 2;

	// Create the input object.  The input object will be used to handle reading the keyboard and mouse input from the user.
	m_Input = new InputClass;
	if(!m_Input){
		return false;
	}
	// Initialize the input object.
	result = m_Input->Initialize(hinstance, hwnd, screenWidth, screenHeight);
	if(!result){
		MessageBox(hwnd, L"Could not initialize the input object.", L"Error", MB_OK);
		return false;
	}

	// Create the Direct3D object.
	m_Direct3D = new D3DClass;
	if(!m_Direct3D){
		return false;
	}

	// Initialize the Direct3D object.
	result = m_Direct3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if(!result){
		MessageBox(hwnd, L"Could not initialize DirectX 11.", L"Error", MB_OK);
		return false;
	}
	InitCamera();
	InitObjects(hwnd);
	InitText(hwnd, screenWidth, screenHeight);
	// Create the light object.
	m_Light = new LightClass;
	if(!m_Light){
		return false;
	}
	// Initialize the light object.
	m_Light->SetAmbientColor(0.05f, 0.05f, 0.05f, 1.0f);
	m_Light->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
	m_Light->SetDirection(1.0f,0.0f, 0.0f);
	InitTextures(hwnd, screenWidth, screenHeight);
	InitShaders(hwnd);
	// Create the full screen ortho window object.
	m_FullScreenWindow = new OrthoWindowClass;
	if(!m_FullScreenWindow){
		return false;
	}

	// Initialize the full screen ortho window object.
	result = m_FullScreenWindow->Initialize(m_Direct3D->GetDevice(), screenWidth, screenHeight);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize the full screen ortho window object.", L"Error", MB_OK);
		return false;
	}
	m_Terrain->GenerateHeightMap(m_Direct3D->GetDevice());
	mFluid->SetBorders(m_Terrain->getHeightMap(), m_Terrain->getHeightMapSize());
	mFluid->InitValues();
	return true;
}
bool ApplicationClass::InitText(HWND hwnd, int screenWidth , int screenHeight){
	D3DXMATRIX baseViewMatrix;
	char videoCard[128];
	int videoMemory;
	bool result = true;
	// Create the timer object.
	m_Timer = new TimerClass;
	if(!m_Timer){
		return false;
	}
	// Initialize the timer object.
	result = m_Timer->Initialize();
	if(!result){
		MessageBox(hwnd, L"Could not initialize the timer object.", L"Error", MB_OK);
		return false;
	}
	// Create the fps object.
	m_Fps = new FpsClass;
	if(!m_Fps){
		return false;
	}

	// Initialize the fps object.
	m_Fps->Initialize();

	// Create the cpu object.
	m_Cpu = new CpuClass;
	if(!m_Cpu){
		return false;
	}
	// Initialize the cpu object.
	m_Cpu->Initialize();
	// Create the text object.
	m_Text = new TextClass;
	if(!m_Text){
		return false;
	}
	m_Camera->GetViewMatrix(baseViewMatrix);
	// Initialize the text object.
	result = m_Text->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), hwnd, screenWidth, screenHeight, baseViewMatrix);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize the text object.", L"Error", MB_OK);
		return false;
	}

	// Retrieve the video card information.
	m_Direct3D->GetVideoCardInfo(videoCard, videoMemory);

	// Set the video card information in the text object.
	result = m_Text->SetVideoCardInfo(videoCard, videoMemory, m_Direct3D->GetDeviceContext());
	if(!result)
	{
		MessageBox(hwnd, L"Could not set video card info in the text object.", L"Error", MB_OK);
		return false;
	}
	return true;
}
bool ApplicationClass::InitObjects(HWND hwnd){
	bool result;
	for(int i = 0; i < MODEL_NUMBER; i++){
		// Create the model object.
		m_Model[i] = new ModelClass(D3DXVECTOR3(rand()%50,(rand()%5)+10,rand()%50));
		if(!m_Model){
			return false;
		}

		// Initialize the model object.
		result = m_Model[i]->Initialize(m_Direct3D->GetDevice(), "data/cube.txt", L"data/seafloor.dds");
		if(!result){
			MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
			return false;
		}
	}
	// Create the terrain object.
	m_Terrain = new TerrainClass;
	if(!m_Terrain){
		return false;
	}
	// Initialize the terrain object.
	result = m_Terrain->InitializeTerrain(m_Direct3D->GetDevice(), 129, 129, L"data/ground.dds");   //initialise the flat terrain.
	if(!result){
		MessageBox(hwnd, L"Could not initialize the terrain object.", L"Error", MB_OK);
		return false;
	}

	mFluid = new FluidClass;
	if(!mFluid){
		return false;
	}

	result = mFluid->InitializeFluid(m_Direct3D->GetDevice(), 129, 129);   //initialise the flat terrain.
	if(!result){
		MessageBox(hwnd, L"Could not initialize the fluid object.", L"Error", MB_OK);
		return false;
	}
	return true;
}
bool ApplicationClass::InitTextures(HWND hwnd, int screenWidth, int screenHeight){
	bool result;
	int	downSampleWidth = screenWidth / 2;
	int downSampleHeight = screenHeight / 2;
	// Create the render to texture object.
	m_RenderFullSizeTexture = new RenderTextureClass;
	if(!m_RenderFullSizeTexture){
		return false;
	}
	// Initialize the render to texture object.
	result = m_RenderFullSizeTexture->Initialize(m_Direct3D->GetDevice(), screenWidth, screenHeight, SCREEN_DEPTH, SCREEN_NEAR);
	if(!result){
		MessageBox(hwnd, L"Could not initialize the render to texture object.", L"Error", MB_OK);
		return false;
	}
	// Create the down sample render to texture object.
	m_DownSampleHalfSizeTexure = new RenderTextureClass;
	if(!m_DownSampleHalfSizeTexure){
		return false;
	}
	// Initialize the down sample render to texture object.
	result = m_DownSampleHalfSizeTexure->Initialize(m_Direct3D->GetDevice(), downSampleWidth, downSampleHeight, SCREEN_DEPTH, SCREEN_NEAR);
	if(!result){
		MessageBox(hwnd, L"Could not initialize the down sample render to texture object.", L"Error", MB_OK);
		return false;
	}
	m_ConvolutionHalfSizeTexture = new RenderTextureClass;
	if (!m_ConvolutionHalfSizeTexture){
		return false;
	}
	result = m_ConvolutionHalfSizeTexture->Initialize(m_Direct3D->GetDevice(), downSampleWidth, downSampleHeight, SCREEN_DEPTH, SCREEN_NEAR);
	if (!result){
		MessageBox(hwnd, L"Could not initialize the Convolution to texture object.", L"Error", MB_OK);		
		return false;
	}
	m_MergeFullSizeTexture = new RenderTextureClass;
	if (!m_MergeFullSizeTexture){
		return false;
	}
	result = m_MergeFullSizeTexture->Initialize(m_Direct3D->GetDevice(), screenWidth, screenHeight, SCREEN_DEPTH, SCREEN_NEAR);
	if (!result){
		MessageBox(hwnd, L"Could not initialize the Convolution to texture object.", L"Error", MB_OK);		
		return false;
	}
	// Create the up sample render to texture object.
	m_FullSizeTexure = new RenderTextureClass;
	if(!m_FullSizeTexure){
		return false;
	}
	// Initialize the up sample render to texture object.
	result = m_FullSizeTexure->Initialize(m_Direct3D->GetDevice(), screenWidth, screenHeight, SCREEN_DEPTH, SCREEN_NEAR);
	if(!result){
		MessageBox(hwnd, L"Could not initialize the up sample render to texture object.", L"Error", MB_OK);
		return false;
	}
	return true;
}
bool ApplicationClass::InitCamera(){
	float cameraX, cameraY, cameraZ;
	// Create the camera object.
	m_Camera = new CameraClass;
	if(!m_Camera){
		return false;
	}

	// Initialize a base view matrix with the camera for 2D user interface rendering.
	m_Camera->SetPosition(0.0f, 0.0f, -1.0f);
	m_Camera->Render();

	// Set the initial position of the camera.
	cameraX = 0.0f;
	cameraY = 2.0f;
	cameraZ = -7.0f;

	m_Camera->SetPosition(cameraX, cameraY, cameraZ);
	// Create the position object.
	m_Position = new PositionClass;
	if(!m_Position){
		return false;
	}

	// Set the initial position of the viewer to the same as the initial camera position.
	m_Position->SetPosition(cameraX, cameraY, cameraZ);
	return true;
}
bool ApplicationClass::InitShaders(HWND hwnd){
	bool result;
	// Create the font shader object.
	m_FontShader = new FontShaderClass;
	if(!m_FontShader){
		return false;
	}
	// Initialize the font shader object.
	result = m_FontShader->Initialize(m_Direct3D->GetDevice(), hwnd);
	if(!result){
		MessageBox(hwnd, L"Could not initialize the font shader object.", L"Error", MB_OK);
		return false;
	}
	// Create the terrain shader object.
	m_TerrainShader = new TerrainShaderClass;
	if(!m_TerrainShader){
		return false;
	}
	// Initialize the terrain shader object.
	result = m_TerrainShader->Initialize(m_Direct3D->GetDevice(), hwnd);
	if(!result){
		MessageBox(hwnd, L"Could not initialize the terrain shader object.", L"Error", MB_OK);
		return false;
	}
	mFluidShader = new FluidShaderClass;
	if(!mFluidShader){
		return false;
	}
	// Initialize the terrain shader object.
	result = mFluidShader->Initialize(m_Direct3D->GetDevice(), hwnd);
	if(!result){
		MessageBox(hwnd, L"Could not initialize the fluid shader object.", L"Error", MB_OK);
		return false;
	}
	// Create the texture shader object.
	m_TextureToTextureShader = new TextureToTextureShaderClass;
	if(!m_TextureToTextureShader){
		return false;
	}
	// Initialize the texture shader object.
	result = m_TextureToTextureShader->Initialize(m_Direct3D->GetDevice(), hwnd);
	if(!result){
		MessageBox(hwnd, L"Could not initialize the texture shader object.", L"Error", MB_OK);
		return false;
	}
	m_TextureShader = new TextureShaderClass;
	if(!m_TextureShader){
		return false;
	}
	// Initialize the texture shader object.
	result = m_TextureShader->Initialize(m_Direct3D->GetDevice(), hwnd);
	if(!result){
		MessageBox(hwnd, L"Could not initialize the texture shader object.", L"Error", MB_OK);
		return false;
	}
	m_ConvolutionShader = new ConvolutionShaderClass;
	if (!m_ConvolutionShader){
		return false;
	}
	result= m_ConvolutionShader->Initialize(m_Direct3D->GetDevice(),hwnd);
	if (!result){
		MessageBox(hwnd, L"Could not initialize the convolution shader object.", L"Error", MB_OK);
		return false;
	}
	mMergerShader = new MergeTextureShaderClass;
	if (!mMergerShader){
		return false;
	}
	result= mMergerShader->Initialize(m_Direct3D->GetDevice(),hwnd);
	if (!result){
		MessageBox(hwnd, L"Could not initialize the convolution shader object.", L"Error", MB_OK);
		return false;
	}
	mGlowShader = new GlowShaderClass;
	if (!mGlowShader){
		return false;
	}
	result= mGlowShader->Initialize(m_Direct3D->GetDevice(),hwnd);
	if (!result){
		MessageBox(hwnd, L"Could not initialize the convolution shader object.", L"Error", MB_OK);
		return false;
	}
	mVerticalBlurShader = new VerticalBlurShaderClass;
	if (!mVerticalBlurShader){
		return false;
	}
	result= mVerticalBlurShader->Initialize(m_Direct3D->GetDevice(),hwnd);
	if (!result){
		MessageBox(hwnd, L"Could not initialize the convolution shader object.", L"Error", MB_OK);
		return false;
	}
	return true;
}

void ApplicationClass::ShutdownText(){
	// Release the text object.
	if(m_Text){
		m_Text->Shutdown();
		delete m_Text;
		m_Text = 0;
	}
	// Release the cpu object.
	if(m_Cpu){
		m_Cpu->Shutdown();
		delete m_Cpu;
		m_Cpu = 0;
	}
	// Release the fps object.
	if(m_Fps){
		delete m_Fps;
		m_Fps = 0;
	}
	// Release the timer object.
	if(m_Timer){
		delete m_Timer;
		m_Timer = 0;
	}
}
void ApplicationClass::ShutdownObjects(){
	for(int i = 0; i < MODEL_NUMBER; i++){
		// Release the model object.
		if(m_Model[i]){
			m_Model[i]->Shutdown();
			delete m_Model[i];
			m_Model[i] = 0;
		}
	}
	// Release the terrain object.
	if(m_Terrain){
		m_Terrain->Shutdown();
		delete m_Terrain;
		m_Terrain = 0;
	}
	if(mFluid){
		mFluid->Shutdown();
		delete mFluid;
		mFluid = 0;
	}
}
void ApplicationClass::ShutdownTextures(){
	// Release the up sample render to texture object.
	if(m_FullSizeTexure){
		m_FullSizeTexure->Shutdown();
		delete m_FullSizeTexure;
		m_FullSizeTexure = 0;
	}
	// Release the down sample render to texture object.
	if(m_DownSampleHalfSizeTexure){
		m_DownSampleHalfSizeTexure->Shutdown();
		delete m_DownSampleHalfSizeTexure;
		m_DownSampleHalfSizeTexure = 0;
	}

	// Release the render to texture object.
	if(m_RenderFullSizeTexture){
		m_RenderFullSizeTexture->Shutdown();
		delete m_RenderFullSizeTexture;
		m_RenderFullSizeTexture = 0;
	}
	if (m_ConvolutionHalfSizeTexture){
		m_ConvolutionHalfSizeTexture->Shutdown();
		delete m_ConvolutionHalfSizeTexture;
		m_ConvolutionHalfSizeTexture = 0;
	}
	if (m_MergeFullSizeTexture){
		m_MergeFullSizeTexture->Shutdown();
		delete m_MergeFullSizeTexture;
		m_MergeFullSizeTexture = 0;
	}
}
void ApplicationClass::ShutdownCamera(){
	if(m_Position){
		delete m_Position;
		m_Position = 0;
	}
	// Release the camera object.
	if(m_Camera){
		delete m_Camera;
		m_Camera = 0;
	}
}
void ApplicationClass::ShutdownShaders(){
	// Release the texture shader object.

	if(m_TextureShader){
		m_TextureShader->Shutdown();
		delete m_TextureShader;
		m_TextureShader = 0;
	}
	if(m_TextureToTextureShader){
		m_TextureToTextureShader->Shutdown();
		delete m_TextureToTextureShader;
		m_TextureToTextureShader = 0;
	}
	if (m_ConvolutionShader){
		m_ConvolutionShader->Shutdown();
		delete m_ConvolutionShader;
		m_ConvolutionShader = 0;
	}

	// Release the font shader object.
	if(m_FontShader){
		m_FontShader->Shutdown();
		delete m_FontShader;
		m_FontShader = 0;
	}

	// Release the terrain shader object.
	if(m_TerrainShader){
		m_TerrainShader->Shutdown();
		delete m_TerrainShader;
		m_TerrainShader = 0;
	}
	if(mFluidShader){
		mFluidShader->Shutdown();
		delete mFluidShader;
		mFluidShader = 0;
	}

	if (mMergerShader){
		mMergerShader->Shutdown();
		delete mMergerShader;
		mMergerShader = 0;
	}
	if(mGlowShader){
		mGlowShader->Shutdown();
		delete mGlowShader;
		mGlowShader = 0;
	}
	if(mVerticalBlurShader){
		mVerticalBlurShader->Shutdown();
		delete mVerticalBlurShader;
		mVerticalBlurShader = 0;
	}
}
void ApplicationClass::Shutdown()
{
	if(m_FullScreenWindow){
		m_FullScreenWindow->Shutdown();
		delete m_FullScreenWindow;
		m_FullScreenWindow = 0;
	}
	// Release the light object.
	if(m_Light){
		delete m_Light;
		m_Light = 0;
	}
	ShutdownText();
	ShutdownTextures();
	ShutdownShaders();
	ShutdownObjects();
	ShutdownCamera();
	
	// Release the Direct3D object.
	if(m_Direct3D){
		m_Direct3D->Shutdown();
		delete m_Direct3D;
		m_Direct3D = 0;
	}

	// Release the input object.
	if(m_Input){
		m_Input->Shutdown();
		delete m_Input;
		m_Input = 0;
	}

	return;
}


bool ApplicationClass::Frame()
{
	bool result;
	static int waterCounter = 0;
	static int maxWaterCounter = 10;
	// Read the user input.
	result = m_Input->Frame();
	if(!result){
		return false;
	}
	if(waterCounter++ > maxWaterCounter){
		waterCounter = 0;
		for(int i = 0; i <5; i++){
			mFluid->AddWater(rand()%120+1,rand()%120+1, float(rand()%20)/10);
		}
	}
	// Check if the user pressed escape and wants to exit the application.
	if(m_Input->IsEscapePressed() == true){
		return false;
	}

	// Update the system stats.
	m_Timer->Frame();
	m_Fps->Frame();
	m_Cpu->Frame();

	// Update the FPS value in the text object.
	result = m_Text->SetFps(m_Fps->GetFps(), m_Direct3D->GetDeviceContext());
	if(!result){
		return false;
	}
	
	// Update the CPU usage value in the text object.
	result = m_Text->SetCpu(m_Cpu->GetCpuPercentage(), m_Direct3D->GetDeviceContext());
	if(!result){
		return false;
	}

	// Do the frame input processing.
	result = HandleInput(m_Timer->GetTime());
	if(!result){
		return false;
	}

	mFluid->GenerateHeightMap(m_Direct3D->GetDevice());
	// Render the graphics scene.
	result = Render();
	if(!result){
		return false;
	}
	return result;
}


bool ApplicationClass::HandleInput(float frameTime){
	bool keyDown, result, blur;
	float posX, posY, posZ, rotX, rotY, rotZ;
	blur = false;

	keyDown = m_Input->IsHPressed();
	if(keyDown){
		for(int i = 0; i <5; i++){
			mFluid->AddWater(rand()%120+1,rand()%120+1, float(rand()%20)/10);
		}
	}
	keyDown = m_Input->IsRPressed();
	if(keyDown){
		mFluid->ResetWater();
		mFluid->InitValues();
	}
	// Set the frame time for calculating the updated position.
	m_Position->SetFrameTime(frameTime);

	keyDown = m_Input->IsLeftPressed();
	m_Position->TurnLeft(keyDown);

	keyDown = m_Input->IsRightPressed();
	m_Position->TurnRight(keyDown);

	keyDown = m_Input->IsUpPressed();
	m_Position->MoveForward(keyDown);

	keyDown = m_Input->IsDownPressed();
	m_Position->MoveBackward(keyDown);

	keyDown = m_Input->IsAPressed();
	if(keyDown){
		blur = true;
	}
	m_Position->MoveUpward(keyDown);

	keyDown = m_Input->IsZPressed();
	if(keyDown){
		blur = true;
	}
	m_Position->MoveDownward(keyDown);

	keyDown = m_Input->IsPgUpPressed();
	m_Position->LookUpward(keyDown);

	keyDown = m_Input->IsPgDownPressed();
	m_Position->LookDownward(keyDown);
	
	// Get the view point position/rotation.
	m_Position->GetPosition(posX, posY, posZ);
	m_Position->GetRotation(rotX, rotY, rotZ);

	// Set the position of the camera.
	m_Camera->SetPosition(posX, posY, posZ);
	m_Camera->SetRotation(rotX, rotY, rotZ);

	// Update the position values in the text object.
	result = m_Text->SetCameraPosition(posX, posY, posZ, m_Direct3D->GetDeviceContext());
	if(!result){
		return false;
	}
	// Update the rotation values in the text object.
	result = m_Text->SetCameraRotation(rotX, rotY, rotZ, m_Direct3D->GetDeviceContext());
	if(!result){
		return false;
	}
	mBlur = blur;
	return true;
}

bool ApplicationClass::Render(){
	bool result;

	// First render the scene to a render texture.
	result = RenderSceneToTexture(m_RenderFullSizeTexture);
	if(!result){
		return false;
	}
	result = RenderTexture(mGlowShader, m_RenderFullSizeTexture, m_FullSizeTexure, m_FullScreenWindow);
	if(!result){
		return false;
	}
	result = RenderTexture(m_TextureToTextureShader, m_FullSizeTexure, m_DownSampleHalfSizeTexure, m_FullScreenWindow);
	if(!result){
		return false;
	}
	result = RenderTexture(m_ConvolutionShader, m_DownSampleHalfSizeTexure, m_ConvolutionHalfSizeTexture, m_FullScreenWindow);
	if(!result){
		return false;
	}
	result = RenderTexture(m_ConvolutionShader, m_ConvolutionHalfSizeTexture, m_DownSampleHalfSizeTexure, m_FullScreenWindow);
	if(!result){
		return false;
	}
	result = RenderTexture(m_ConvolutionShader, m_DownSampleHalfSizeTexure, m_ConvolutionHalfSizeTexture, m_FullScreenWindow);
	if(!result){
		return false;
	}
	result = RenderTexture(m_TextureToTextureShader, m_ConvolutionHalfSizeTexture, m_FullSizeTexure, m_FullScreenWindow);
	if(!result){
		return false;
	}
	result= RenderMergeTexture(m_RenderFullSizeTexture,m_FullSizeTexure,m_MergeFullSizeTexture,m_FullScreenWindow);
	if(!result){
		return false;
	}
	if(mBlur){
		result = RenderTexture(mVerticalBlurShader, m_MergeFullSizeTexture, m_RenderFullSizeTexture, m_FullScreenWindow);
		if(!result){
			return false;
		}
		result = Render2DTextureScene(m_RenderFullSizeTexture);
		if(!result){
			return false;
		}
	}else{
		result = Render2DTextureScene(m_MergeFullSizeTexture);
		if(!result){
			return false;
		}
	}
	return true;
}

bool ApplicationClass::RenderSceneToTexture(RenderTextureClass* write)
{
	D3DXMATRIX worldMatrix, modelWorldMatrix, viewMatrix, projectionMatrix;
	bool result;

	
	// Set the render target to be the render to texture.
	write->SetRenderTarget(m_Direct3D->GetDeviceContext());
	// Clear the render to texture.
	write->ClearRenderTarget(m_Direct3D->GetDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);
	// Generate the view matrix based on the camera's position.
	m_Camera->Render();
	// Get the world, view, and projection matrices from the camera and d3d objects.
	m_Direct3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_Direct3D->GetProjectionMatrix(projectionMatrix);	

	// Render the terrain buffers.
	m_Terrain->Render(m_Direct3D->GetDeviceContext());
	// Render the terrain using the terrain shader.
	result = m_TerrainShader->Render(m_Direct3D->GetDeviceContext(), m_Terrain->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, 
		m_Light->GetAmbientColor(), m_Light->GetDiffuseColor(), m_Light->GetDirection(), m_Terrain->GetTexture());
	if(!result){
		return false;
	}


	for(int i = 0; i< MODEL_NUMBER; i++){
		modelWorldMatrix = worldMatrix;
		// Rotate the world matrix by the rotation value so that the cube will spin.
		D3DXMatrixRotationY(&modelWorldMatrix, m_Model[i]->GetRotation());
		// Put the model vertex and index buffers on the graphics pipeline to prepare them for drawing.
		m_Model[i]->Render(m_Direct3D->GetDeviceContext());

		// Render the model using the texture shader.
		result = m_TextureShader->Render(m_Direct3D->GetDeviceContext(), m_Model[i]->GetIndexCount(), modelWorldMatrix, viewMatrix, projectionMatrix, 
			m_Model[i]->GetTexture());
		if(!result){
			return false;
		}
	}
	// Turn on the alpha blending before rendering the text.
	m_Direct3D->TurnOnAlphaBlending();

	mFluid->Render(m_Direct3D->GetDeviceContext());
	result = mFluidShader->Render(m_Direct3D->GetDeviceContext(), mFluid->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, 
		m_Light->GetAmbientColor(), m_Light->GetDiffuseColor(), m_Light->GetDirection());
	if(!result){
		return false;
	}

	// Turn off alpha blending after rendering the text.
	m_Direct3D->TurnOffAlphaBlending();
	m_Direct3D->EndScene();
	// Reset the render target back to the original back buffer and not the render to texture anymore.
	m_Direct3D->SetBackBufferRenderTarget();

	// Reset the viewport back to the original.
	m_Direct3D->ResetViewport();

	return true;
}
bool ApplicationClass::RenderTexture(ShaderClass *shader, RenderTextureClass *readTexture, RenderTextureClass *writeTexture, OrthoWindowClass *window)
{
	D3DXMATRIX worldMatrix, viewMatrix, orthoMatrix;
	float screenSizeX, screenSizeY;
	bool result;

	screenSizeY = (float)writeTexture->GetTextureHeight();
	screenSizeX = (float)writeTexture->GetTextureWidth();

	// Set the render target to be the render to texture.
	writeTexture->SetRenderTarget(m_Direct3D->GetDeviceContext());

	// Clear the render to texture.
	writeTexture->ClearRenderTarget(m_Direct3D->GetDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	// Generate the view matrix based on the camera's position.
	m_Camera->Render();

	// Get the world and view matrices from the camera and d3d objects.
	m_Camera->GetViewMatrix(viewMatrix);
	m_Direct3D->GetWorldMatrix(worldMatrix);
	
	// Get the ortho matrix from the render to texture since texture has different dimensions being that it is smaller.
	writeTexture->GetOrthoMatrix(orthoMatrix);

	// Turn off the Z buffer to begin all 2D rendering.
	m_Direct3D->TurnZBufferOff();

	// Put the small ortho window vertex and index buffers on the graphics pipeline to prepare them for drawing.
	window->Render(m_Direct3D->GetDeviceContext());

	// Render the small ortho window using the texture shader and the render to texture of the scene as the texture resource.
	result = shader->Render(m_Direct3D->GetDeviceContext(), window->GetIndexCount(), orthoMatrix, 
		readTexture->GetShaderResourceView(),screenSizeY,screenSizeX);
	if(!result){
		return false;
	}

	// Turn the Z buffer back on now that all 2D rendering has completed.
	m_Direct3D->TurnZBufferOn();

	// Reset the render target back to the original back buffer and not the render to texture anymore.
	m_Direct3D->SetBackBufferRenderTarget();

	// Reset the viewport back to the original.
	m_Direct3D->ResetViewport();
	return true;
}
bool ApplicationClass::RenderMergeTexture(RenderTextureClass *readTexture, RenderTextureClass *readTexture2, RenderTextureClass *writeTexture, OrthoWindowClass *window){
	bool result;

	// Set the render target to be the render to texture.
	writeTexture->SetRenderTarget(m_Direct3D->GetDeviceContext());

	// Clear the render to texture.
	writeTexture->ClearRenderTarget(m_Direct3D->GetDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	// Generate the view matrix based on the camera's position.
	m_Camera->Render();

	// Turn off the Z buffer to begin all 2D rendering.
	m_Direct3D->TurnZBufferOff();

	// Put the small ortho window vertex and index buffers on the graphics pipeline to prepare them for drawing.
	window->Render(m_Direct3D->GetDeviceContext());

	// Render the small ortho window using the texture shader and the render to texture of the scene as the texture resource.
	result = mMergerShader->Render(m_Direct3D->GetDeviceContext(), window->GetIndexCount(), readTexture->GetShaderResourceView(), 
		readTexture2->GetShaderResourceView());
	if(!result){
		return false;
	}

	// Turn the Z buffer back on now that all 2D rendering has completed.
	m_Direct3D->TurnZBufferOn();

	// Reset the render target back to the original back buffer and not the render to texture anymore.
	m_Direct3D->SetBackBufferRenderTarget();

	// Reset the viewport back to the original.
	m_Direct3D->ResetViewport();
	return true;
}
bool ApplicationClass::Render2DTextureScene(RenderTextureClass* mRead){
	D3DXMATRIX worldMatrix, viewMatrix, orthoMatrix, projectionMatrix;
	bool result;

	// Clear the buffers to begin the scene.
	m_Direct3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	// Generate the view matrix based on the camera's position.
	m_Camera->Render();

	// Get the world, view, and ortho matrices from the camera and d3d objects.
	m_Camera->GetViewMatrix(viewMatrix);
	m_Direct3D->GetWorldMatrix(worldMatrix);
	m_Direct3D->GetOrthoMatrix(orthoMatrix);
	// Turn off the Z buffer to begin all 2D rendering.
	m_Direct3D->TurnZBufferOff();

	// Put the full screen ortho window vertex and index buffers on the graphics pipeline to prepare them for drawing.
	m_FullScreenWindow->Render(m_Direct3D->GetDeviceContext());

	// Render the full screen ortho window using the texture shader and the full screen sized blurred render to texture resource.
	result = m_TextureToTextureShader->Render(m_Direct3D->GetDeviceContext(), m_FullScreenWindow->GetIndexCount(), orthoMatrix, mRead->GetShaderResourceView());
	if(!result){
		return false;
	}
	
	// Turn on the alpha blending before rendering the text.
	m_Direct3D->TurnOnAlphaBlending();

	// Render the text user interface elements.
	result = m_Text->Render(m_Direct3D->GetDeviceContext(), m_FontShader, worldMatrix, orthoMatrix);
	if(!result){
		return false;
	}

	// Turn off alpha blending after rendering the text.
	m_Direct3D->TurnOffAlphaBlending();
	
	// Turn the Z buffer back on now that all 2D rendering has completed.
	m_Direct3D->TurnZBufferOn();

	// Present the rendered scene to the screen.
	m_Direct3D->EndScene();

	return true;
}