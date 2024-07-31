#include "VulkanRendererLogic.h"
#include "../Components/Identification.h"
#include "../Components/Visuals.h"
#include "../Components/Physics.h"
#include "../Components/Model_Data.h"

using namespace EROD; // Example Space Game


bool EROD::D3D11RendererLogic::Init(std::shared_ptr<flecs::world> _game,
	std::weak_ptr<const GameConfig> _gameConfig,
	GW::GRAPHICS::GDirectX11Surface _d3d11,
	GW::SYSTEM::GWindow _window, Level_Data& _level,unsigned& _pelletCount)
{
	// save a handle to the ECS & game settings
	game = _game;
	gameConfig = _gameConfig;
	d3d11 = _d3d11;
	window = _window;
	dLevelData = &_level;
	pelletCount = _pelletCount;
	firstPelletCount = _pelletCount;

	if (-d3d11.GetDevice((void**)&creator))
		return false;
	// Setup all vulkan resources
	if (LoadShaders() == false)
		return false;
	if (FillSceneData() == false)
		return false;
	TextureSetUp();
	if (LoadUniforms() == false)
		return false;
	if (LoadGeometry() == false)
		return false;
	creator->Release();
	if (SetupPipeline() == false)
		return false;
	// Setup drawing engine
	if (SetupDrawcalls() == false)
		return false;
	//No falses return true - everything loaded
	return true;
}
EROD::D3D11RendererLogic::~D3D11RendererLogic()
{
	for (auto &i : livesText.GetVertices())
	{
		delete i.pos;
	}
}
bool EROD::D3D11RendererLogic::Activate(bool runSystem)
{
	if (completeDraw.is_alive()) {
		if (runSystem) 
		{
			completeDraw.enable();
		}
		else 
		{
			completeDraw.disable();
		}
		return true;
	}
	return false;
}

bool EROD::D3D11RendererLogic::Shutdown()
{
	completeDraw.destruct();

	return true; // vulkan resource shutdown handled via GEvent in Init()
}

std::string EROD::D3D11RendererLogic::ShaderAsString(const char* shaderFilePath)
{
	std::string output;
	unsigned int stringLength = 0;
	GW::SYSTEM::GFile file; file.Create();
	file.GetFileSize(shaderFilePath, stringLength);
	if (stringLength && +file.OpenBinaryRead(shaderFilePath)) {
		output.resize(stringLength);
		file.Read(&output[0], stringLength);
	}
	else
		std::cout << "ERROR: Shader Source File \"" << shaderFilePath << "\" Not Found!" << std::endl;
	return output;
}
GW::MATH::GVECTORF EROD::D3D11RendererLogic::VectorAsString(const char* string)
{
	GW::MATH::GVECTORF output;
	std::sscanf(string, "%f, %f, %f,%f",
		&output.x, &output.y, &output.z, &output.w);
	return output;
}
bool EROD::D3D11RendererLogic::LoadShaders()//vertex and pixel shaders
{
	UINT compilerFlags = D3DCOMPILE_ENABLE_STRICTNESS;

	std::shared_ptr<const GameConfig> readCfg = gameConfig.lock();
	std::string vertexShaderSource = (*readCfg).at("Shaders").at("vertex").as<std::string>();
	std::string pixelShaderSource = (*readCfg).at("Shaders").at("pixel").as<std::string>();
	 textureSource = (*readCfg).at("Shaders").at("texturepixelshader").as<std::string>();
	 UIsource = (*readCfg).at("Shaders").at("UIvertex").as<std::string>();
	 UIpixelSource = (*readCfg).at("Shaders").at("UIpixel").as<std::string>();
	//	
	if (vertexShaderSource.empty() || pixelShaderSource.empty() ||  textureSource.empty() ||  UIsource.empty() ||  UIpixelSource.empty())
		return false;
	//
	vertexShaderSource = ShaderAsString(vertexShaderSource.c_str());
	pixelShaderSource = ShaderAsString(pixelShaderSource.c_str());
	 textureSource = ShaderAsString( textureSource.c_str());
	 UIsource = ShaderAsString( UIsource.c_str());
	 UIpixelSource = ShaderAsString( UIpixelSource.c_str());

	//
	if (vertexShaderSource.empty() || pixelShaderSource.empty() ||  textureSource.empty() ||  UIsource.empty() ||  UIpixelSource.empty())
		return false;
	Microsoft::WRL::ComPtr<ID3DBlob> vsBlob = CompileVertexShader(creator, compilerFlags, vertexShaderSource);
	Microsoft::WRL::ComPtr<ID3DBlob> psBlob = CompilePixelShader(creator, compilerFlags, pixelShaderSource);
	Microsoft::WRL::ComPtr<ID3DBlob> tpsBlob = CompilePixelShader(creator, compilerFlags,  textureSource, 1);
	Microsoft::WRL::ComPtr<ID3DBlob> UIverBlob = CompileVertexShader(creator, compilerFlags,  UIsource, 1);
	Microsoft::WRL::ComPtr<ID3DBlob> UIpixBlob = CompilePixelShader(creator, compilerFlags,  UIpixelSource, 2);
	CreateVertexInputLayout(creator, vsBlob);
	CreateUIVertexInputLayout(creator, UIverBlob);
	return true;
}

bool EROD::D3D11RendererLogic::LoadUniforms()// create constant buffers
{
	//Create Scene Buffer
	LoadSceneBuffer();
	//Create ModelIds Buffer
	LoadModelIdsBuffer();
	//Structured Buffers
	//Sprite Buffers
	LoadSpriteBuffer();
	//UI Constant Buffer
	LoadUIBuffer();
	LoadScoreBuffer();
	LoadPlayerUIBuffer();
	LoadPlayerLivesTextBuffer();
	LoadLoseTextBuffer();
	LoadLevelTextBuffer();	
	LoadLevelNumBuffer();
	//Create Transform Buffer
	LoadTransformBuffer();
	//Create Light Buffer
	LoadLightBuffer();
	//Create Material Buffer
	LoadMaterialBuffer();

	return true;
}

bool EROD::D3D11RendererLogic::LoadGeometry()// index and vertex buffers
{
	LoadVertexBuffer();

	//Create Index Buffer
	LoadIndexBuffer();


	std::string filepath = XML_PATH;
	filepath += "font_consolas_32.xml";
	bool success =  consolas32.LoadFromXML(filepath);

	window.GetClientWidth( width);
	window.GetClientHeight( height);

	// setting up the static text object with information
	// keep in mind the position will always be the center of the text
	 staticText = Text();
	 staticText.SetText("Score: ");
	 staticText.SetFont(& consolas32);
	 staticText.SetPosition(-0.85f, 0.85f);
	 staticText.SetScale(0.65f, 0.65f);
	 staticText.SetRotation(0.0f);
	 staticText.SetDepth(0.01f);

	 scoreText = Text();
	 scoreText.SetText("00000000");
	 scoreText.SetFont(& consolas32);
	 scoreText.SetPosition(-0.60f, 0.85f);
	 scoreText.SetScale(0.60f, 0.60f);
	 scoreText.SetRotation(0.0f);
	 scoreText.SetDepth(0.01f);


	 playerLives = Text();
	 playerLives.SetText("Player Lives:");
	 playerLives.SetFont(& consolas32);
	 playerLives.SetPosition(0.65f, 0.85f);
	 playerLives.SetScale(0.60f, 0.60f);
	 playerLives.SetRotation(0.0f);
	 playerLives.SetDepth(0.01f);
	
	 livesText = Text();
	 livesText.SetText("2");
	 livesText.SetFont(& consolas32);
	 livesText.SetPosition(0.90f, 0.85f);
	 livesText.SetScale(0.60f, 0.60f);
	 livesText.SetRotation(0.0f);
	 livesText.SetDepth(0.01f);

	 youLose = Text();
	 youLose.SetText("Y O U  L O S E");
	 youLose.SetFont(&consolas32);
	 youLose.SetPosition(0.0, 0.85f);
	 youLose.SetScale(0.70f, 0.70f);
	 youLose.SetRotation(0.0f);
	 youLose.SetDepth(0.01f);

	 staticText.Update( width,  height);
	 scoreText.Update( width,  height);
	 playerLives.Update( width,  height);
	 livesText.Update( width,  height);
	 youLose.Update(width, height);

	levelText = Text();
	levelText.SetText("LEVEL");
	levelText.SetFont(&consolas32);
	levelText.SetPosition(-0.1, 0.85f);
	levelText.SetScale(0.70f, 0.70f);
	levelText.SetRotation(0.0f);
	levelText.SetDepth(0.01f);

	levelNum = Text();
	levelNum.SetText("1");
	levelNum.SetFont(&consolas32);
	levelNum.SetPosition(0.1, 0.85f);
	levelNum.SetScale(0.70f, 0.70f);
	levelNum.SetRotation(0.0f);
	levelNum.SetDepth(0.01f);

	staticText.Update(width, height);
	scoreText.Update(width, height);
	playerLives.Update(width, height);
	livesText.Update(width, height);
	youLose.Update(width, height);
	levelText.Update(width, height);
	levelNum.Update(width, height);

	SetScoreTextVertices();
	SetScoreVertices();
	SetPlayerLivesTextVertices();
	SetLivesVertices();
	SetLoseTextVertices();
	SetLevelTextVertices();
	SetLevelNumVertices();

	return true;
}

bool EROD::D3D11RendererLogic::SetupPipeline()
{

	 curHandles = GetCurrentPipelineHandles();

	SetTexturePixelShader( curHandles);
	SetRenderTargets( curHandles);
	SetVertexBuffers( curHandles);
	SetIndexBuffer( curHandles);
	SetShaders( curHandles);

	 curHandles.context->VSSetConstantBuffers(0, 1,  sceneDataBuffer.GetAddressOf());
	 curHandles.context->VSSetConstantBuffers(2, 1,  modelIdBuffer.GetAddressOf());
	 curHandles.context->PSSetConstantBuffers(0, 1,  sceneDataBuffer.GetAddressOf());
	 curHandles.context->PSSetConstantBuffers(2, 1,  modelIdBuffer.GetAddressOf());

	 curHandles.context->VSSetShaderResources(3, 1,  transformSRV.GetAddressOf());
	 curHandles.context->PSSetShaderResources(4, 1,  materialSRV.GetAddressOf());
	 curHandles.context->PSSetShaderResources(3, 1,  lightSRV.GetAddressOf());

	 curHandles.context->IASetInputLayout( vertexFormat.Get());
	 curHandles.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	 curHandles.context->OMSetBlendState( blendState.Get(), NULL, 0xFFFFFFFF);
	return true;
}

bool EROD::D3D11RendererLogic::SetupDrawcalls() // I SCREWED THIS UP MAKES SO MUCH SENSE NOW
{
	// runs once per frame after updateDraw
	struct RenderingSystem {};
	game->entity("Rendering System").add<RenderingSystem>();
	startDraw = game->system<RenderingSystem>().kind(flecs::PreUpdate)//Create draw instances
		.each([this](flecs::entity e, RenderingSystem& s) {});
	updateDraw = game->system<RenderingTime>().kind(flecs::OnUpdate)//update subresources
		.each([this](flecs::entity& e, RenderingTime) {});

	completeDraw = game->system<MeshInstance>().kind(flecs::PostUpdate)
		.each([this](flecs::entity e, MeshInstance& s) {
		curHandles.context->UpdateSubresource( transformBuffer.Get(), 0, nullptr,  dLevelData->levelTransforms.data(), 0, 0);
		for (int j = 0; j < s.modelMeshCount; ++j)
		{
			const auto& submesh = dLevelData->levelMeshes[s.modelMeshStart + j];
			//auto& test =  levelData.levelMaterials[s.modelMaterialStart];
			modelIds.material_id = s.modelMaterialStart + submesh.materialIndex;
			modelIds.model_id = s.instanceTransformStart;
			auto map_KdToCheck = dLevelData->levelMaterials[modelIds.material_id].map_Kd;
			if (map_KdToCheck != nullptr)
			{
				if (textures.find(map_KdToCheck) != textures.end())
				{
					curHandles.context->PSSetShader(texturePixelShader.Get(), nullptr, 0);
					curHandles.context->PSSetShaderResources(0, 1, textures[map_KdToCheck].GetAddressOf());
				}
			}
			else
			{
				curHandles.context->PSSetShader(pixelShader.Get(), nullptr, 0);
			}
			D3D11_MAPPED_SUBRESOURCE msr = { 0 };
			curHandles.context->Map(modelIdBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
			memcpy(msr.pData, &modelIds, sizeof(modelIds));
			curHandles.context->Unmap(modelIdBuffer.Get(), 0);
			curHandles.context->DrawIndexedInstanced(
				submesh.drawInfo.indexCount,
				s.instanceTransformCount,
				s.modelIndexStart + submesh.drawInfo.indexOffset,
				s.modelVertexStart,
				0);
		}
		DrawUI();
		SetupPipeline();
		});
	ReleasePipelineHandles(curHandles);
	return true;
}

void EROD::D3D11RendererLogic::TextureSetUp()
{
	LevelTextureSetUp();
	std::wstring UItexPathCopy = texturePath;
	std::wstring fontFilePath = L"font_consolas_32.dds";
	UItexPathCopy += fontFilePath;
	result = DirectX::CreateDDSTextureFromFile(creator, UItexPathCopy.c_str(), nullptr, textShaderResource.GetAddressOf());
	
		//texturePath += texture_names[i];
		// load texture from disk 
		//result = DirectX::CreateDDSTextureFromFile(creator, texturePath.c_str(), nullptr,  textureShaderResourceViewer[i].GetAddressOf());
	CD3D11_SAMPLER_DESC samp_desc = CD3D11_SAMPLER_DESC(CD3D11_DEFAULT());
	result = creator->CreateSamplerState(&samp_desc,  samplerState.GetAddressOf());

	CD3D11_BLEND_DESC blendDesc = CD3D11_BLEND_DESC(CD3D11_DEFAULT());
	blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	creator->CreateBlendState(&blendDesc,  blendState.GetAddressOf());
}

void EROD::D3D11RendererLogic::LevelTextureSetUp()
{
	std::shared_ptr<const GameConfig> readCfg = gameConfig.lock();
	std::string sTexturePath = (*readCfg).at("Textures").at("LevelTexturesPath1").as<std::string>();

	// create a wide string to store the file path and file name
	std::wstring wTexturePath(sTexturePath.begin(), sTexturePath.end());
	texturePath = wTexturePath;
	game->system<MeshInstance>().each([this](flecs::entity e, MeshInstance& s)
	{
		for (int j = 0; j < s.modelMeshCount; ++j)
		{
			if ( dLevelData->levelMaterials[s.modelMaterialStart + j].map_Kd)
			{
				std::string textureFilePath =  dLevelData->levelMaterials[s.modelMaterialStart + j].map_Kd;
				std::wstring texPathCopy = texturePath;
				std::wstring temp = FindTextureName(textureFilePath);
				texPathCopy += temp;
				if (textures.find( dLevelData->levelMaterials[s.modelMaterialStart + j].map_Kd) == textures.end())
				{
					// load texture from disk 
					result = DirectX::CreateDDSTextureFromFile(creator, texPathCopy.c_str(), nullptr,  textureShaderResource.GetAddressOf());
					textures[ dLevelData->levelMaterials[s.modelMaterialStart + j].map_Kd] =  textureShaderResource;
				}
			}
		}
	});
}

void EROD::D3D11RendererLogic::UISetShaders()
{
	 curHandles.context->VSSetShader( UIvertexShader.Get(), nullptr, 0);
	 curHandles.context->PSSetShader( UIpixelShader.Get(), nullptr, 0);
	 curHandles.context->IASetInputLayout( UIvertexFormat.Get());
	 curHandles.context->PSSetSamplers(0, 1, samplerState.GetAddressOf());
}

void EROD::D3D11RendererLogic::DrawUI()
{
	const UINT strides[] = { sizeof(float) * 4 };
	const UINT offsets[] = { 0 };
	UISetShaders();
	curHandles.context->VSSetConstantBuffers(0, 1, UIconstantBuffer.GetAddressOf());
	curHandles.context->IASetVertexBuffers(0, 1, vertexBufferStaticText.GetAddressOf(), strides, offsets);
	constantBufferData = UpdateTextConstantBufferData(staticText);
	curHandles.context->PSSetShaderResources(0, 1, textShaderResource.GetAddressOf());
	curHandles.context->UpdateSubresource(UIconstantBuffer.Get(), 0, nullptr, & constantBufferData, 0, 0);
	curHandles.context->Draw(staticText.GetVertices().size(), 0);
	DrawScore(strides,offsets);
	DrawPlayerLives(strides,offsets);
	DrawLivesText(strides, offsets);
	if (lose)
	{
		SetYouLose();
	}
	else
	{
		DrawLevelText(strides, offsets);
		DrawLevelNum(strides, offsets);
	}
}

void EROD::D3D11RendererLogic::DrawScore(const UINT* strides, const UINT* offset)
{
	 curHandles.context->VSSetConstantBuffers(0, 1,  ScoreUIconstantBuffer.GetAddressOf());
	 curHandles.context->IASetVertexBuffers(0, 1,  scoreVertexBufferStaticText.GetAddressOf(), strides, offset);
	 constantBufferData = UpdateTextConstantBufferData( scoreText);
	 curHandles.context->UpdateSubresource( ScoreUIconstantBuffer.Get(), 0, nullptr, & constantBufferData, 0, 0);
	 curHandles.context->Draw( scoreText.GetVertices().size(), 0);
}

void EROD::D3D11RendererLogic::DrawPlayerLives(const UINT* strides, const UINT* offset)
{
	 curHandles.context->VSSetConstantBuffers(0, 1,  PlayerLivesUIconstantBuffer.GetAddressOf());
	 curHandles.context->IASetVertexBuffers(0, 1,  playerLivesVertexBufferStaticText.GetAddressOf(), strides, offset);
	 constantBufferData = UpdateTextConstantBufferData(playerLives);
	 curHandles.context->UpdateSubresource( PlayerLivesUIconstantBuffer.Get(), 0, nullptr, & constantBufferData, 0, 0);
	 curHandles.context->Draw( playerLives.GetVertices().size(), 0);
}

void EROD::D3D11RendererLogic::DrawLivesText(const UINT* strides, const UINT* offset)
{
	curHandles.context->VSSetConstantBuffers(0, 1,  PlayerLivesTextUIconstantBuffer.GetAddressOf());
	curHandles.context->IASetVertexBuffers(0, 1,  playerLivesTextVertexBufferStaticText.GetAddressOf(), strides, offset);
	constantBufferData = UpdateTextConstantBufferData(livesText);
	curHandles.context->UpdateSubresource( PlayerLivesTextUIconstantBuffer.Get(), 0, nullptr, & constantBufferData, 0, 0);
	curHandles.context->Draw( livesText.GetVertices().size(), 0);
}

void EROD::D3D11RendererLogic::DrawLevelText(const UINT* strides, const UINT* offset)
{
	curHandles.context->VSSetConstantBuffers(0, 1, levelTextUIconstantBuffer.GetAddressOf());
	curHandles.context->IASetVertexBuffers(0, 1, levelTextVertexBufferStaticText.GetAddressOf(), strides, offset);
	constantBufferData = UpdateTextConstantBufferData(levelText);
	curHandles.context->UpdateSubresource(levelTextUIconstantBuffer.Get(), 0, nullptr, &constantBufferData, 0, 0);
	curHandles.context->Draw(levelText.GetVertices().size(), 0);
}

void EROD::D3D11RendererLogic::DrawLevelNum(const UINT* strides, const UINT* offset)
{
	curHandles.context->VSSetConstantBuffers(0, 1, levelNumUIconstantBuffer.GetAddressOf());
	curHandles.context->IASetVertexBuffers(0, 1, levelNumVertexBufferDynamicText.GetAddressOf(), strides, offset);
	constantBufferData = UpdateTextConstantBufferData(levelNum);
	curHandles.context->UpdateSubresource(levelNumUIconstantBuffer.Get(), 0, nullptr, &constantBufferData, 0, 0);
	curHandles.context->Draw(levelNum.GetVertices().size(), 0);
}

void EROD::D3D11RendererLogic::SetYouLose()
{
	const UINT strides[] = { sizeof(float) * 4 };
	const UINT offsets[] = { 0 };
	curHandles.context->VSSetConstantBuffers(0, 1, youLoseTextUIconstantBuffer.GetAddressOf());
	curHandles.context->IASetVertexBuffers(0, 1, youLoseVertexBufferStaticText.GetAddressOf(), strides, offsets);
	constantBufferData = UpdateTextConstantBufferData(youLose);
	curHandles.context->UpdateSubresource(youLoseTextUIconstantBuffer.Get(), 0, nullptr, &constantBufferData, 0, 0);
	curHandles.context->Draw(youLose.GetVertices().size(), 0);
}
void EROD::D3D11RendererLogic::UpdateScore(unsigned int score)
{
	std::string scoreUpdate = std::to_string(score);
	 scoreText.SetText(scoreUpdate);
	 scoreText.Update(width, height);
	SetScoreVertices();
}
void EROD::D3D11RendererLogic::UpdateLevel(unsigned int level)
{
	std::string levelUpdate = std::to_string(level);
	levelNum.SetText(levelUpdate);
	levelNum.Update(width, height);
	SetLevelNumVertices();
}

void EROD::D3D11RendererLogic::UpdateLives(unsigned int lives)
{
	std::string LivesUpdate = std::to_string(lives);
	livesText.SetText(LivesUpdate);
	livesText.Update(width, height);
	SetLivesVertices();
}

void EROD::D3D11RendererLogic::UpdateScoreText(unsigned int score) 
{
	UpdateScore(score);
}

void EROD::D3D11RendererLogic::UpdateLivesText(unsigned int lives)
{
	UpdateLives(lives);
}

void EROD::D3D11RendererLogic::YouLose()
{
	lose = true;
}

void EROD::D3D11RendererLogic::DecrementPelletCount(int num)
{
	pelletCount -= num;
}

int EROD::D3D11RendererLogic::GetPelletCount()
{
	return pelletCount;
}

void EROD::D3D11RendererLogic::ResetPelletCount()
{
	pelletCount = firstPelletCount;
}

PipelineHandles EROD::D3D11RendererLogic::GetCurrentPipelineHandles()
{
	PipelineHandles retval;
	d3d11.GetImmediateContext((void**)&retval.context);
	d3d11.GetRenderTargetView((void**)&retval.targetView);
	d3d11.GetDepthStencilView((void**)&retval.depthStencil);
	return retval;
}

void EROD::D3D11RendererLogic::ReleasePipelineHandles(PipelineHandles toRelease)
{
	toRelease.depthStencil->Release();
	toRelease.targetView->Release();
	toRelease.context->Release();
}

void EROD::D3D11RendererLogic::SetTexturePixelShader(PipelineHandles curHandles)
{
	curHandles.context->PSSetShader( texturePixelShader.Get(), nullptr, 0);
	curHandles.context->PSSetSamplers(0, 1,  samplerState.GetAddressOf());
}

void EROD::D3D11RendererLogic::SetRenderTargets(PipelineHandles handles)
{
	ID3D11RenderTargetView* const views[] = { handles.targetView };
	handles.context->OMSetRenderTargets(ARRAYSIZE(views), views, handles.depthStencil);
}

void EROD::D3D11RendererLogic::SetVertexBuffers(PipelineHandles handles)
{
	const UINT strides[] = { sizeof(H2B::VERTEX) }; // TODO: Part 1E 
	const UINT offsets[] = { 0 };
	ID3D11Buffer* const buffs[] = {  vertexBuffer.Get() };
	handles.context->IASetVertexBuffers(0, 1,  vertexBuffer.GetAddressOf(), strides, offsets);
}

void EROD::D3D11RendererLogic::SetIndexBuffer(PipelineHandles handles)
{
	handles.context->IASetIndexBuffer( indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
}

void EROD::D3D11RendererLogic::SetShaders(PipelineHandles handles)
{
	handles.context->VSSetShader( vertexShader.Get(), nullptr, 0);
	handles.context->PSSetShader( pixelShader.Get(), nullptr, 0);
}

std::wstring EROD::D3D11RendererLogic::FindTextureName(std::string& string)
{
	auto index = string.find_last_of("/");
	std::string textureName = string.substr(index + 1, string.length() - index);
	std::wstring pop(textureName.begin(), textureName.end());
	return pop;
}

bool EROD::D3D11RendererLogic::FillSceneData()
{
	//View Matrix
	std::shared_ptr<const GameConfig> readCfg = gameConfig.lock();
	std::string camPos = (*readCfg).at("Camera").at("pos").as<std::string>();
	std::string lookAt = (*readCfg).at("Camera").at("lookat").as<std::string>();
	std::string up = (*readCfg).at("Camera").at("up").as<std::string>();
	GW::MATH::GVECTORF pos = VectorAsString(camPos.c_str());
	GW::MATH::GVECTORF look = VectorAsString(lookAt.c_str());
	GW::MATH::GVECTORF worldUp = VectorAsString(up.c_str());
	GW::MATH::GMatrix::LookAtLHF(pos, look, worldUp,  sceneData.viewMatrix);
	 sceneData.cameraPos = pos;

	//Projection Matrix
	float aspectRatio = 1;
	float fieldOfView = (*readCfg).at("Projection").at("fieldofview").as<float>();
	float nearPlane = (*readCfg).at("Projection").at("near").as<float>();
	float farPlane = (*readCfg).at("Projection").at("far").as<float>();
	d3d11.GetAspectRatio(aspectRatio);
	GW::MATH::GMatrix::ProjectionDirectXLHF(G_DEGREE_TO_RADIAN(fieldOfView),
		aspectRatio, nearPlane, farPlane,
		 sceneData.projectionMatrix);

	//BaseLight
	std::string mainLightColor = (*readCfg).at("BaseDirectionalLight").at("sunColor").as<std::string>();
	std::string mainLightDirection = (*readCfg).at("BaseDirectionalLight").at("sunDirection").as<std::string>();
	std::string mainAmbientLight = (*readCfg).at("BaseDirectionalLight").at("ambient").as<std::string>();
	GW::MATH::GVECTORF lightColor = VectorAsString(mainLightColor.c_str());
	GW::MATH::GVECTORF lightDirection = VectorAsString(mainLightDirection.c_str());
	GW::MATH::GVECTORF ambientLight = VectorAsString(mainAmbientLight.c_str());
	 sceneData.sunAmbient = ambientLight;
	 sceneData.sunDirection = lightDirection;
	 sceneData.sunColor = lightColor;
	 modelIds.amountOfLights =  dLevelData->levelLighting.size();

	return true;

}

void EROD::D3D11RendererLogic::LoadMaterialBuffer()
{
	std::vector<H2B::ATTRIBUTES> v;
	for (const auto& mat : dLevelData->levelMaterials)
		v.push_back(mat.attrib);

	unsigned int msbSize = v.size() * sizeof(H2B::ATTRIBUTES);
	D3D11_SUBRESOURCE_DATA bData = { v.data(), 0, 0 };
	CD3D11_BUFFER_DESC bDesc(msbSize, D3D11_BIND_SHADER_RESOURCE, D3D11_USAGE_DEFAULT, 0, D3D11_RESOURCE_MISC_BUFFER_STRUCTURED, sizeof(H2B::ATTRIBUTES));
	creator->CreateBuffer(&bDesc, &bData, materialBuffer.ReleaseAndGetAddressOf());
	CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(materialBuffer.Get(), DXGI_FORMAT_UNKNOWN, 0, msbSize / sizeof(H2B::ATTRIBUTES));
	creator->CreateShaderResourceView(materialBuffer.Get(), &srvDesc, materialSRV.ReleaseAndGetAddressOf());
}

void EROD::D3D11RendererLogic::LoadLightBuffer()
{
	unsigned int lsbSize = dLevelData->levelTransforms.size() * sizeof(GW::MATH::GMATRIXF);
	D3D11_SUBRESOURCE_DATA lsbData = { dLevelData->levelLighting.data(), 0, 0 };
	CD3D11_BUFFER_DESC lsbDesc(lsbSize, D3D11_BIND_SHADER_RESOURCE, D3D11_USAGE_DEFAULT, 0, D3D11_RESOURCE_MISC_BUFFER_STRUCTURED, sizeof(GW::MATH::GMATRIXF));
	creator->CreateBuffer(&lsbDesc, &lsbData, lightBuffer.ReleaseAndGetAddressOf());
	CD3D11_SHADER_RESOURCE_VIEW_DESC lsrvDesc(lightBuffer.Get(), DXGI_FORMAT_UNKNOWN, 0, lsbSize / sizeof(GW::MATH::GMATRIXF));
	creator->CreateShaderResourceView(lightBuffer.Get(), &lsrvDesc, lightSRV.ReleaseAndGetAddressOf());
}

void EROD::D3D11RendererLogic::LoadTransformBuffer()
{
	unsigned int tsbSize = dLevelData->levelTransforms.size() * sizeof(GW::MATH::GMATRIXF);
	D3D11_SUBRESOURCE_DATA tsbData = { dLevelData->levelTransforms.data(), 0, 0 };
	CD3D11_BUFFER_DESC tsbDesc(tsbSize, D3D11_BIND_SHADER_RESOURCE, D3D11_USAGE_DEFAULT, 0, D3D11_RESOURCE_MISC_BUFFER_STRUCTURED, sizeof(GW::MATH::GMATRIXF));
	creator->CreateBuffer(&tsbDesc, &tsbData, transformBuffer.ReleaseAndGetAddressOf());
	CD3D11_SHADER_RESOURCE_VIEW_DESC tsrvDesc(transformBuffer.Get(), DXGI_FORMAT_UNKNOWN, 0, tsbSize / sizeof(GW::MATH::GMATRIXF));
	creator->CreateShaderResourceView(transformBuffer.Get(), &tsrvDesc, transformSRV.ReleaseAndGetAddressOf());
}

void EROD::D3D11RendererLogic::LoadLoseTextBuffer()
{
	D3D11_SUBRESOURCE_DATA yltuicbData = { &constantBufferData, 0, 0 };
	CD3D11_BUFFER_DESC yltuicbDesc(sizeof(constantBufferData), D3D11_BIND_CONSTANT_BUFFER);
	result = creator->CreateBuffer(&yltuicbDesc, &yltuicbData, youLoseTextUIconstantBuffer.ReleaseAndGetAddressOf());
}
void EROD::D3D11RendererLogic::LoadLevelNumBuffer()
{
	D3D11_SUBRESOURCE_DATA yltuicbData = { &constantBufferData, 0, 0 };
	CD3D11_BUFFER_DESC yltuicbDesc(sizeof(constantBufferData), D3D11_BIND_CONSTANT_BUFFER);
	result = creator->CreateBuffer(&yltuicbDesc, &yltuicbData, levelNumUIconstantBuffer.ReleaseAndGetAddressOf());
}
void EROD::D3D11RendererLogic::LoadLevelTextBuffer()
{
	D3D11_SUBRESOURCE_DATA ltuicbData = { &constantBufferData, 0, 0 };
	CD3D11_BUFFER_DESC ltuicbDesc(sizeof(constantBufferData), D3D11_BIND_CONSTANT_BUFFER);
	result = creator->CreateBuffer(&ltuicbDesc, &ltuicbData, levelTextUIconstantBuffer.ReleaseAndGetAddressOf());
}
void EROD::D3D11RendererLogic::LoadPlayerLivesTextBuffer()
{
	D3D11_SUBRESOURCE_DATA ltuicbData = { &constantBufferData, 0, 0 };
	CD3D11_BUFFER_DESC ltuicbDesc(sizeof(constantBufferData), D3D11_BIND_CONSTANT_BUFFER);
	result = creator->CreateBuffer(&ltuicbDesc, &ltuicbData, PlayerLivesTextUIconstantBuffer.ReleaseAndGetAddressOf());
}

void EROD::D3D11RendererLogic::LoadPlayerUIBuffer()
{
	D3D11_SUBRESOURCE_DATA pluicbData = { &constantBufferData, 0, 0 };
	CD3D11_BUFFER_DESC pluicbDesc(sizeof(constantBufferData), D3D11_BIND_CONSTANT_BUFFER);
	result = creator->CreateBuffer(&pluicbDesc, &pluicbData, PlayerLivesUIconstantBuffer.ReleaseAndGetAddressOf());
}

void EROD::D3D11RendererLogic::LoadScoreBuffer()
{
	D3D11_SUBRESOURCE_DATA scuicbData = { &constantBufferData, 0, 0 };
	CD3D11_BUFFER_DESC scuicbDesc(sizeof(constantBufferData), D3D11_BIND_CONSTANT_BUFFER);
	result = creator->CreateBuffer(&scuicbDesc, &scuicbData, ScoreUIconstantBuffer.ReleaseAndGetAddressOf());
}

void EROD::D3D11RendererLogic::LoadUIBuffer()
{
	D3D11_SUBRESOURCE_DATA uicbData = { &constantBufferData, 0, 0 };
	CD3D11_BUFFER_DESC uicbDesc(sizeof(constantBufferData), D3D11_BIND_CONSTANT_BUFFER);
	result = creator->CreateBuffer(&uicbDesc, &uicbData, UIconstantBuffer.ReleaseAndGetAddressOf());
}

void EROD::D3D11RendererLogic::LoadSceneBuffer()
{
	D3D11_SUBRESOURCE_DATA sbData = { &sceneData, 0, 0 };
	CD3D11_BUFFER_DESC sbDesc(sizeof(sceneData), D3D11_BIND_CONSTANT_BUFFER);
	sbDesc.Usage = D3D11_USAGE_DYNAMIC;
	sbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	creator->CreateBuffer(&sbDesc, &sbData, sceneDataBuffer.ReleaseAndGetAddressOf());
}

void EROD::D3D11RendererLogic::LoadModelIdsBuffer()
{
	D3D11_SUBRESOURCE_DATA mbData = { &modelIds, 0, 0 };
	CD3D11_BUFFER_DESC mbDesc(sizeof(modelIds), D3D11_BIND_CONSTANT_BUFFER);
	mbDesc.Usage = D3D11_USAGE_DYNAMIC;
	mbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	creator->CreateBuffer(&mbDesc, &mbData, modelIdBuffer.ReleaseAndGetAddressOf());
}

void EROD::D3D11RendererLogic::LoadSpriteBuffer()
{
	D3D11_SUBRESOURCE_DATA cbData = { &constantBufferData, 0, 0 };
	CD3D11_BUFFER_DESC cbDesc(sizeof(&constantBufferData), D3D11_BIND_CONSTANT_BUFFER);
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	creator->CreateBuffer(&cbDesc, &cbData, spriteDataBuffer.ReleaseAndGetAddressOf());
}

void EROD::D3D11RendererLogic::LoadVertexBuffer()
{
	D3D11_SUBRESOURCE_DATA vbData = { dLevelData->levelVertices.data(), 0, 0 };
	CD3D11_BUFFER_DESC vbDesc(dLevelData->levelVertices.size() * sizeof(H2B::VERTEX), D3D11_BIND_VERTEX_BUFFER);
	result = creator->CreateBuffer(&vbDesc, &vbData, vertexBuffer.ReleaseAndGetAddressOf());
}

void EROD::D3D11RendererLogic::LoadIndexBuffer()
{
	D3D11_SUBRESOURCE_DATA ibData = { dLevelData->levelIndices.data(), 0, 0 };
	CD3D11_BUFFER_DESC ibDesc(dLevelData->levelIndices.size() * sizeof(unsigned int), D3D11_BIND_INDEX_BUFFER);
	result = creator->CreateBuffer(&ibDesc, &ibData, indexBuffer.ReleaseAndGetAddressOf());
}

void EROD::D3D11RendererLogic::SetScoreVertices()
{
	const auto& ScoreStaticVerts =  scoreText.GetVertices();
	D3D11_SUBRESOURCE_DATA scbData = { ScoreStaticVerts.data(), 0, 0 };
	CD3D11_BUFFER_DESC scbDesc(sizeof(TextVertex) * ScoreStaticVerts.size(), D3D11_BIND_VERTEX_BUFFER);
	creator->CreateBuffer(&scbDesc, &scbData,  scoreVertexBufferStaticText.GetAddressOf());
}

void EROD::D3D11RendererLogic::SetPlayerLivesTextVertices()
{
	const auto& livesStaticVerts = playerLives.GetVertices();
	D3D11_SUBRESOURCE_DATA livescbData = { livesStaticVerts.data(), 0, 0 };
	CD3D11_BUFFER_DESC livescbDesc(sizeof(TextVertex) * livesStaticVerts.size(), D3D11_BIND_VERTEX_BUFFER);
	creator->CreateBuffer(&livescbDesc, &livescbData, playerLivesVertexBufferStaticText.ReleaseAndGetAddressOf());
}

void EROD::D3D11RendererLogic::SetScoreTextVertices()
{
	const auto& staticVerts = staticText.GetVertices();
	D3D11_SUBRESOURCE_DATA svbData = { staticVerts.data(), 0, 0 };
	CD3D11_BUFFER_DESC svbDesc(sizeof(TextVertex) * staticVerts.size(), D3D11_BIND_VERTEX_BUFFER);
	creator->CreateBuffer(&svbDesc, &svbData, vertexBufferStaticText.ReleaseAndGetAddressOf());
}

void EROD::D3D11RendererLogic::SetLivesVertices()
{
	const auto& livesTextStaticVerts = livesText.GetVertices();
	D3D11_SUBRESOURCE_DATA textcbData = { livesTextStaticVerts.data(), 0, 0 };
	CD3D11_BUFFER_DESC textcbDesc(sizeof(TextVertex) * livesTextStaticVerts.size(), D3D11_BIND_VERTEX_BUFFER);
	creator->CreateBuffer(&textcbDesc, &textcbData, playerLivesTextVertexBufferStaticText.GetAddressOf());
}

void EROD::D3D11RendererLogic::SetLevelTextVertices()
{
	const auto& TextStaticVerts = levelText.GetVertices();
	D3D11_SUBRESOURCE_DATA textcbData = { TextStaticVerts.data(), 0, 0 };
	CD3D11_BUFFER_DESC textcbDesc(sizeof(TextVertex) * TextStaticVerts.size(), D3D11_BIND_VERTEX_BUFFER);
	creator->CreateBuffer(&textcbDesc, &textcbData, levelTextVertexBufferStaticText.GetAddressOf());
}

void EROD::D3D11RendererLogic::SetLevelNumVertices()
{
	const auto& TextDynamicVerts = levelNum.GetVertices();
	D3D11_SUBRESOURCE_DATA textcbData = { TextDynamicVerts.data(), 0, 0 };
	CD3D11_BUFFER_DESC textcbDesc(sizeof(TextVertex) * TextDynamicVerts.size(), D3D11_BIND_VERTEX_BUFFER);
	creator->CreateBuffer(&textcbDesc, &textcbData, levelNumVertexBufferDynamicText.GetAddressOf());
}

void EROD::D3D11RendererLogic::SetLoseTextVertices()
{
	const auto& livesTextStaticVerts = youLose.GetVertices();
	D3D11_SUBRESOURCE_DATA textcbData = { livesTextStaticVerts.data(), 0, 0 };
	CD3D11_BUFFER_DESC textcbDesc(sizeof(TextVertex) * livesTextStaticVerts.size(), D3D11_BIND_VERTEX_BUFFER);
	creator->CreateBuffer(&textcbDesc, &textcbData, youLoseVertexBufferStaticText.GetAddressOf());
}

bool EROD::D3D11RendererLogic::FreeVulkanResources()
{
	return true;
}

void EROD::D3D11RendererLogic::CreateVertexInputLayout(ID3D11Device* creator, Microsoft::WRL::ComPtr<ID3DBlob>& vsBlob)
{
	D3D11_INPUT_ELEMENT_DESC attributes[3];

	attributes[0].SemanticName = "POS";
	attributes[0].SemanticIndex = 0;
	attributes[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	attributes[0].InputSlot = 0;
	attributes[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	attributes[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	attributes[0].InstanceDataStepRate = 0;

	attributes[1].SemanticName = "UVW";
	attributes[1].SemanticIndex = 0;
	attributes[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	attributes[1].InputSlot = 0;
	attributes[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	attributes[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	attributes[1].InstanceDataStepRate = 0;

	attributes[2].SemanticName = "NRM";
	attributes[2].SemanticIndex = 0;
	attributes[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	attributes[2].InputSlot = 0;
	attributes[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	attributes[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	attributes[2].InstanceDataStepRate = 0;

	result = creator->CreateInputLayout(attributes, ARRAYSIZE(attributes),
		vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
		 vertexFormat.GetAddressOf());
}

void EROD::D3D11RendererLogic::CreateUIVertexInputLayout(ID3D11Device* creator, Microsoft::WRL::ComPtr<ID3DBlob>& vsBlob)
{
	D3D11_INPUT_ELEMENT_DESC attributes[2];

	attributes[0].SemanticName = "POSITION";
	attributes[0].SemanticIndex = 0;
	attributes[0].Format = DXGI_FORMAT_R32G32_FLOAT;
	attributes[0].InputSlot = 0;
	attributes[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	attributes[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	attributes[0].InstanceDataStepRate = 0;

	attributes[1].SemanticName = "TEXCOORD";
	attributes[1].SemanticIndex = 0;
	attributes[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	attributes[1].InputSlot = 0;
	attributes[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	attributes[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	attributes[1].InstanceDataStepRate = 0;

	result = creator->CreateInputLayout(attributes, ARRAYSIZE(attributes),
		vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
		 UIvertexFormat.GetAddressOf());
}

Microsoft::WRL::ComPtr<ID3DBlob> EROD::D3D11RendererLogic::CompileVertexShader(ID3D11Device* creator, UINT compilerFlags, std::string vertexShaderSource, unsigned type)
{
	Microsoft::WRL::ComPtr<ID3DBlob> vsBlob, errors;

	HRESULT compilationResult =
		D3DCompile(vertexShaderSource.c_str(), vertexShaderSource.length(),
			nullptr, nullptr, nullptr, "main", "vs_4_0", compilerFlags, 0,
			vsBlob.ReleaseAndGetAddressOf(), errors.ReleaseAndGetAddressOf());

	if (SUCCEEDED(compilationResult))
	{
		switch (type)
		{
		case 1:
			creator->CreateVertexShader(vsBlob->GetBufferPointer(),
				vsBlob->GetBufferSize(), nullptr,  UIvertexShader.ReleaseAndGetAddressOf());
			break;
		default:
			creator->CreateVertexShader(vsBlob->GetBufferPointer(),
				vsBlob->GetBufferSize(), nullptr,  vertexShader.ReleaseAndGetAddressOf());
			break;
		}
	}
	else
	{
		 log.LogCategorized("Vertex Shader Errors:\n", (char*)errors->GetBufferPointer());
		abort();
		return nullptr;
	}
	return vsBlob;
}

Microsoft::WRL::ComPtr<ID3DBlob> EROD::D3D11RendererLogic::CompilePixelShader(ID3D11Device* creator, UINT compilerFlags, std::string pixelShaderSource, unsigned type)
{

	Microsoft::WRL::ComPtr<ID3DBlob> psBlob, errors;

	HRESULT compilationResult =
		D3DCompile(pixelShaderSource.c_str(), pixelShaderSource.length(),
			nullptr, nullptr, nullptr, "main", "ps_4_0", compilerFlags, 0,
			psBlob.ReleaseAndGetAddressOf(), errors.ReleaseAndGetAddressOf());

	if (SUCCEEDED(compilationResult))
	{
		switch (type)
		{
		case 1:
			creator->CreatePixelShader(psBlob->GetBufferPointer(),
				psBlob->GetBufferSize(), nullptr,  texturePixelShader.ReleaseAndGetAddressOf());
			break;
		case 2:
			creator->CreatePixelShader(psBlob->GetBufferPointer(),
				psBlob->GetBufferSize(), nullptr,  UIpixelShader.ReleaseAndGetAddressOf());
			break;
		default:
			creator->CreatePixelShader(psBlob->GetBufferPointer(),
				psBlob->GetBufferSize(), nullptr,  pixelShader.ReleaseAndGetAddressOf());
			break;
		}
	}
	else
	{
		 log.LogCategorized("Pixel Shader Errors:\n", (char*)errors->GetBufferPointer());
		abort();
		return nullptr;
	}

	return psBlob;
}

std::vector<Sprite>	LoadHudFromXML(std::string filepath)
{
	std::vector<Sprite> result;

	tinyxml2::XMLDocument document;
	tinyxml2::XMLError error_message = document.LoadFile(filepath.c_str());
	if (error_message != tinyxml2::XML_SUCCESS)
	{
		std::cout << "XML file [" + filepath + "] did not load properly." << std::endl;
		return std::vector<Sprite>();
	}

	std::string name = document.FirstChildElement("Eejan Rizzler and the Order of Drip")->FindAttribute("Hud")->Value();
	GW::MATH2D::GVECTOR2F screen_size;
	screen_size.x = atof(document.FirstChildElement("hud")->FindAttribute("width")->Value());
	screen_size.y = atof(document.FirstChildElement("hud")->FindAttribute("height")->Value());

	tinyxml2::XMLElement* current = document.FirstChildElement("hud")->FirstChildElement("element");
	while (current)
	{
		Sprite s = Sprite();
		name = current->FindAttribute("name")->Value();
		FLOAT x = atof(current->FindAttribute("pos_x")->Value());
		FLOAT y = atof(current->FindAttribute("pos_y")->Value());
		FLOAT sx = atof(current->FindAttribute("scale_x")->Value());
		FLOAT sy = atof(current->FindAttribute("scale_y")->Value());
		FLOAT r = atof(current->FindAttribute("rotation")->Value());
		FLOAT d = atof(current->FindAttribute("depth")->Value());

		GW::MATH2D::GVECTOR2F s_min, s_max;
		s_min.x = atof(current->FindAttribute("sr_x")->Value());
		s_min.y = atof(current->FindAttribute("sr_y")->Value());
		s_max.x = atof(current->FindAttribute("sr_w")->Value());
		s_max.y = atof(current->FindAttribute("sr_h")->Value());
		UINT tid = atoi(current->FindAttribute("textureID")->Value());

		s.SetName(name);
		s.SetScale(sx, sy);
		s.SetPosition(x, y);
		s.SetRotation(r);
		s.SetDepth(d);
		s.SetScissorRect({ s_min, s_max });
		s.SetTextureIndex(tid);

		result.push_back(s);

		current = current->NextSiblingElement();
	}
	return result;
}
SPRITE_DATA EROD::D3D11RendererLogic::UpdateSpriteConstantBufferData(const Sprite& s)
{
	SPRITE_DATA temp = { 0 };
	temp.pos_scale.x = s.GetPosition().x;
	temp.pos_scale.y = s.GetPosition().y;
	temp.pos_scale.z = s.GetScale().x;
	temp.pos_scale.w = s.GetScale().y;
	temp.rotation_depth.x = s.GetRotation();
	temp.rotation_depth.y = s.GetDepth();
	return temp;
}
SPRITE_DATA EROD::D3D11RendererLogic::UpdateTextConstantBufferData(const Text& s)
{
	SPRITE_DATA temp = { 0 };
	temp.pos_scale.x = s.GetPosition().x;
	temp.pos_scale.y = s.GetPosition().y;
	temp.pos_scale.z = s.GetScale().x;
	temp.pos_scale.w = s.GetScale().y;
	temp.rotation_depth.x = s.GetRotation();
	temp.rotation_depth.y = s.GetDepth();
	return temp;
}

