////////////////////////////////////////////////////////////////////////////////
// Filename: applicationclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "applicationclass.h"


ApplicationClass::ApplicationClass(): m_Input(0), m_Direct3D(0), m_Camera(0), m_Terrain(0),
	mFluid(0), m_Timer(0), m_Position(0), m_Fps(0), m_Cpu(0), m_FontShader(0), m_Text(0),
	m_TerrainShader(0), mFluidShader(0), m_Light(0), m_TextureShader(0), m_TextureToTextureShader(0),
	m_RenderFullSizeTexture(0), m_DownSampleHalfSizeTexure(0), m_FullSizeTexure(0), m_FullScreenWindow(0),
	m_ConvolutionShader(0), m_HalfSizeTexture(0), mGlowShader(0), mMergerShader(0), mVerticalBlurShader(0), 
	m_MergeFullSizeTexture(0), mBlur(false)
{
	for(int i = 0; i < MODEL_NUMBER; i++){
		m_Model[0] = 0;
	}
}

ApplicationClass::ApplicationClass(const ApplicationClass& other): m_Input(0), m_Direct3D(0), m_Camera(0), m_Terrain(0),
	mFluid(0), m_Timer(0), m_Position(0), m_Fps(0), m_Cpu(0), m_FontShader(0), m_Text(0),
	m_TerrainShader(0), mFluidShader(0), m_Light(0), m_TextureShader(0), m_TextureToTextureShader(0),
	m_RenderFullSizeTexture(0), m_DownSampleHalfSizeTexure(0), m_FullSizeTexure(0), m_FullScreenWindow(0),
	m_ConvolutionShader(0), m_HalfSizeTexture(0), mGlowShader(0), mMergerShader(0), mVerticalBlurShader(0),
	m_MergeFullSizeTexture(0), mBlur(false)
{
	for(int i = 0; i < MODEL_NUMBER; i++){
		m_Model[0] = 0;
	}
}
ApplicationClass::~ApplicationClass(){
}
bool ApplicationClass::Initialize(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight){
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
	//Initialize the camera and position objects
	InitCamera();
	//Initialize the terrain, water, and cubes
	InitObjects(hwnd);
	//Initialize the text on screen. 
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
	//Initialize the texture to render to
	InitTextures(hwnd, screenWidth, screenHeight);
	//Initialize the sahders to be used.
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
	//Generate a random height map for the terrain
	m_Terrain->GenerateHeightMap(m_Direct3D->GetDevice());
	//set the borders of the water based upon the height of the terrain
	mFluid->SetBorders(m_Terrain->getHeightMap(), m_Terrain->getHeightMapSize());
	//set the init water values based upon borders
	mFluid->InitValues();
	return true;
}
void ApplicationClass::Shutdown(){
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
	//Shutdown functions
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
bool ApplicationClass::Frame(){
	bool result;
	static int waterCounter = 0;
	static const int maxWaterCounter = 15;
	// Read the user input.
	result = m_Input->Frame();
	if(!result){
		return false;
	}
	//every 15 frames add a more drops of water
	if(waterCounter++ > maxWaterCounter){
		waterCounter = 0;
		for(int i = 0; i <5; i++){
			mFluid->AddWater(rand()%129,rand()%129);
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
			mFluid->AddWater(rand()%129,rand()%129);
		}
	}
	keyDown = m_Input->IsRPressed();
	if(keyDown){
		mFluid->ResetWater();
		mFluid->InitValues();
	}
	// Set the frame time for calculating the updated position.
	m_Position->SetFrameTime(frameTime);
	//if left or a was pressed turn left
	keyDown = (m_Input->IsLeftPressed() || m_Input->IsAPressed());
	m_Position->TurnLeft(keyDown);
	//if right or d was pressed turn right
	keyDown = (m_Input->IsRightPressed() || m_Input->IsDPressed());
	m_Position->TurnRight(keyDown);
	//if up or w was pressed turn up
	keyDown = (m_Input->IsUpPressed() || m_Input->IsWPressed());
	m_Position->MoveForward(keyDown);
	//if down or s was pressed turn down
	keyDown = (m_Input->IsDownPressed() || m_Input->IsSPressed());
	m_Position->MoveBackward(keyDown);
	//if q was pressed move up
	keyDown = m_Input->IsQPressed();
	if(keyDown){
		//set blur to true if moving up
		blur = true;
	}
	m_Position->MoveUpward(keyDown);
	//if e was pressed move down
	keyDown = m_Input->IsEPressed();
	if(keyDown){
		//set blur to true if moving down
		blur = true;
	}
	m_Position->MoveDownward(keyDown);
	//if page up or z was pressed look up
	keyDown = (m_Input->IsPgUpPressed() || m_Input->IsZPressed());
	m_Position->LookUpward(keyDown);
	//if page down or x was pressed look down
	keyDown = (m_Input->IsPgDownPressed() || m_Input->IsXPressed());
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
	//set class blur to be true if moved up or down
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
	//next work out which areas need to have glow added
	result = RenderTexture(mGlowShader, m_RenderFullSizeTexture, m_FullSizeTexure, m_FullScreenWindow);
	if(!result){
		return false;
	}
	//now downsample the texture
	result = RenderTexture(m_TextureToTextureShader, m_FullSizeTexure, m_DownSampleHalfSizeTexure, m_FullScreenWindow);
	if(!result){
		return false;
	}
	//next perform three blurs each time ping ponging the textures so the latest data can be altered
	result = RenderTexture(m_ConvolutionShader, m_DownSampleHalfSizeTexure, m_HalfSizeTexture, m_FullScreenWindow);
	if(!result){
		return false;
	}
	//blur number 2
	result = RenderTexture(m_ConvolutionShader, m_HalfSizeTexture, m_DownSampleHalfSizeTexure, m_FullScreenWindow);
	if(!result){
		return false;
	}
	//blur number 3
	result = RenderTexture(m_ConvolutionShader, m_DownSampleHalfSizeTexure, m_HalfSizeTexture, m_FullScreenWindow);
	if(!result){
		return false;
	}
	//next upsample the texture
	result = RenderTexture(m_TextureToTextureShader, m_HalfSizeTexture, m_FullSizeTexure, m_FullScreenWindow);
	if(!result){
		return false;
	}//now merge the blurred glow texture with the original texture
	result= RenderMergeTexture(m_RenderFullSizeTexture,m_FullSizeTexure,m_MergeFullSizeTexture,m_FullScreenWindow);
	if(!result){
		return false;
	}
	//if the user is moving up or down also blur vertically before rendering
	if(mBlur){
		//vertical blur
		result = RenderTexture(mVerticalBlurShader, m_MergeFullSizeTexture, m_RenderFullSizeTexture, m_FullScreenWindow);
		if(!result){
			return false;
		}
		//render the texture to the screen
		result = Render2DTextureScene(m_RenderFullSizeTexture);
		if(!result){
			return false;
		}
	}else{
		//render the texture to the scene
		result = Render2DTextureScene(m_MergeFullSizeTexture);
		if(!result){
			return false;
		}
	}
	return true;
}
/*
*	Renders the objects on screen to a texture ready for post-processing.
*/
bool ApplicationClass::RenderSceneToTexture(RenderTextureClass* write){
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

	//render the models in 3d world space
	for(int i = 0; i< MODEL_NUMBER; i++){
		modelWorldMatrix = worldMatrix;
		// Rotate the world matrix by the rotation value so that the cube will spin.
		D3DXMatrixRotationY(&modelWorldMatrix, m_Model[i]->GetRotation());
		D3DXMATRIX translation = m_Model[i]->GetTranslation();
		D3DXMatrixMultiply(&modelWorldMatrix,&modelWorldMatrix,&translation);
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
	// Reset the render target back to the original back buffer and not the render to texture anymore.
	m_Direct3D->SetBackBufferRenderTarget();

	// Reset the viewport back to the original.
	m_Direct3D->ResetViewport();

	return true;
}
/*
*	Takes a texture and performs shader actions on it.
*/
bool ApplicationClass::RenderTexture(ShaderClass *shader, RenderTextureClass *readTexture, RenderTextureClass *writeTexture, OrthoWindowClass *window){
	D3DXMATRIX orthoMatrix;
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
/*
*	Takes two textures and combines them to make a third texture;
*/
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
/*
* Renders a texture to the screen.
*/
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
	if(!result){
		MessageBox(hwnd, L"Could not initialize the text object.", L"Error", MB_OK);
		return false;
	}
	// Retrieve the video card information.
	m_Direct3D->GetVideoCardInfo(videoCard, videoMemory);
	// Set the video card information in the text object.
	result = m_Text->SetVideoCardInfo(videoCard, videoMemory, m_Direct3D->GetDeviceContext());
	if(!result){
		MessageBox(hwnd, L"Could not set video card info in the text object.", L"Error", MB_OK);
		return false;
	}
	return true;
}
bool ApplicationClass::InitObjects(HWND hwnd){
	bool result;
	//initialize the models in different locations 
	for(int i = 0; i < MODEL_NUMBER; i++){
		// Create the model object.
		m_Model[i] = new ModelClass();
		if(!m_Model){
			return false;
		}
		// Initialize the model object.
		result = m_Model[i]->Initialize(m_Direct3D->GetDevice(), "data/cube.txt", L"data/cubeTexture.dds");
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
	result = m_Terrain->InitializeTerrain(m_Direct3D->GetDevice(), 129, 129, L"data/ground.dds");   //initialize the flat terrain.
	if(!result){
		MessageBox(hwnd, L"Could not initialize the terrain object.", L"Error", MB_OK);
		return false;
	}
	//Create the fluid object
	mFluid = new FluidClass;
	if(!mFluid){
		return false;
	}
	result = mFluid->InitializeFluid(m_Direct3D->GetDevice(), 129, 129);   //initialise the flat water.
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
	// Create the up sample render to texture object.
	m_FullSizeTexure = new RenderTextureClass;
	if(!m_FullSizeTexure){
		return false;
	}
	// Initialize the up sample render to texture object.
	result = m_FullSizeTexure->Initialize(m_Direct3D->GetDevice(), screenWidth, screenHeight, SCREEN_DEPTH, SCREEN_NEAR);
	if(!result){
		MessageBox(hwnd, L"Could not initialize the full size render to texture object.", L"Error", MB_OK);
		return false;
	}
	//create a third full size texture for merging
	m_MergeFullSizeTexture = new RenderTextureClass;
	if (!m_MergeFullSizeTexture){
		return false;
	}
	result = m_MergeFullSizeTexture->Initialize(m_Direct3D->GetDevice(), screenWidth, screenHeight, SCREEN_DEPTH, SCREEN_NEAR);
	if (!result){
		MessageBox(hwnd, L"Could not initialize the merge full size to texture object.", L"Error", MB_OK);		
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
	//create a second down sample texture for performing convolutions on
	m_HalfSizeTexture = new RenderTextureClass;
	if (!m_HalfSizeTexture){
		return false;
	}
	result = m_HalfSizeTexture->Initialize(m_Direct3D->GetDevice(), downSampleWidth, downSampleHeight, SCREEN_DEPTH, SCREEN_NEAR);
	if (!result){
		MessageBox(hwnd, L"Could not initialize the second half size to texture object.", L"Error", MB_OK);		
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
	//create the shader for the fluid
	mFluidShader = new FluidShaderClass;
	if(!mFluidShader){
		return false;
	}
	// Initialize the fluid shader object.
	result = mFluidShader->Initialize(m_Direct3D->GetDevice(), hwnd);
	if(!result){
		MessageBox(hwnd, L"Could not initialize the fluid shader object.", L"Error", MB_OK);
		return false;
	}
	// Create the texture to texture shader object.
	m_TextureToTextureShader = new TextureToTextureShaderClass;
	if(!m_TextureToTextureShader){
		return false;
	}
	// Initialize the texture to texture shader object.
	result = m_TextureToTextureShader->Initialize(m_Direct3D->GetDevice(), hwnd);
	if(!result){
		MessageBox(hwnd, L"Could not initialize the texture shader object.", L"Error", MB_OK);
		return false;
	}
	//create the texture shader object
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
	//create the convolution shader object
	m_ConvolutionShader = new ConvolutionShaderClass;
	if (!m_ConvolutionShader){
		return false;
	}
	result= m_ConvolutionShader->Initialize(m_Direct3D->GetDevice(),hwnd);
	if (!result){
		MessageBox(hwnd, L"Could not initialize the convolution shader object.", L"Error", MB_OK);
		return false;
	}
	//create the merge shader object
	mMergerShader = new MergeTextureShaderClass;
	if (!mMergerShader){
		return false;
	}
	result= mMergerShader->Initialize(m_Direct3D->GetDevice(),hwnd);
	if (!result){
		MessageBox(hwnd, L"Could not initialize the convolution shader object.", L"Error", MB_OK);
		return false;
	}
	//create the glow shader object
	mGlowShader = new GlowShaderClass;
	if (!mGlowShader){
		return false;
	}
	result= mGlowShader->Initialize(m_Direct3D->GetDevice(),hwnd);
	if (!result){
		MessageBox(hwnd, L"Could not initialize the convolution shader object.", L"Error", MB_OK);
		return false;
	}
	//create the vertical blur shader
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
	// Release each model object.
	for(int i = 0; i < MODEL_NUMBER; i++){
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
	//release fluid object
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
	//release half size texture
	if (m_HalfSizeTexture){
		m_HalfSizeTexture->Shutdown();
		delete m_HalfSizeTexture;
		m_HalfSizeTexture = 0;
	}
	//release merge full size texture
	if (m_MergeFullSizeTexture){
		m_MergeFullSizeTexture->Shutdown();
		delete m_MergeFullSizeTexture;
		m_MergeFullSizeTexture = 0;
	}
}
void ApplicationClass::ShutdownCamera(){
	//relase position object
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
	//release texture to texture shader
	if(m_TextureToTextureShader){
		m_TextureToTextureShader->Shutdown();
		delete m_TextureToTextureShader;
		m_TextureToTextureShader = 0;
	}
	//release convolution shader
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
	//Release Fluid Shader
	if(mFluidShader){
		mFluidShader->Shutdown();
		delete mFluidShader;
		mFluidShader = 0;
	}
	//Release merge shader
	if (mMergerShader){
		mMergerShader->Shutdown();
		delete mMergerShader;
		mMergerShader = 0;
	}
	//release glow shader
	if(mGlowShader){
		mGlowShader->Shutdown();
		delete mGlowShader;
		mGlowShader = 0;
	}
	//release vertical blur shader
	if(mVerticalBlurShader){
		mVerticalBlurShader->Shutdown();
		delete mVerticalBlurShader;
		mVerticalBlurShader = 0;
	}
}