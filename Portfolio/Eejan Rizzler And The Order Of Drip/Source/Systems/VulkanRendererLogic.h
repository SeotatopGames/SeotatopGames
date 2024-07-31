// The rendering system is responsible for drawing all objects
#ifndef VULKANRENDERERLOGIC_H
#define VULKANRENDERERLOGIC_H

// Contains our global game settings
#include "../GameConfig.h"
#include "../Font.h"
#define XML_PATH "../xml/"
// example space game (avoid name collisions)
namespace EROD
{
	struct PipelineHandles
	{
		ID3D11DeviceContext* context;
		ID3D11RenderTargetView* targetView;
		ID3D11DepthStencilView* depthStencil;
	};
	struct SPRITE_DATA
	{
		GW::MATH::GVECTORF pos_scale;
		GW::MATH::GVECTORF rotation_depth;
	};

	using HUD = std::vector<Sprite>;
	class D3D11RendererLogic
	{
		// shared connection to the main ECS engine
		std::shared_ptr<flecs::world> game;
		// non-ownership handle to configuration settings
		std::weak_ptr<const GameConfig> gameConfig;
		// handle to our running ECS systems
		flecs::system completeDraw;
		flecs::system startDraw;
		flecs::system updateDraw;
		// Used to query screen dimensions
		GW::SYSTEM::GWindow window;
		GW::GRAPHICS::GDirectX11Surface d3d11;
		GW::CORE::GEventReceiver shutdown;
		ID3D11Device* creator;
		flecs::entity playerEntity;
		//Function Checks
		HRESULT result;
		GW::GReturn gResult;
		std::map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textures;
		std::wstring texturePath;
		unsigned pelletCount = -1;
		bool lose = false;
		bool first = true;
		unsigned firstPelletCount;
		unsigned pelletTransform;
	public:
		~D3D11RendererLogic();
		// attach the required logic to the ECS 
		bool Init(std::shared_ptr<flecs::world> _game,
			std::weak_ptr<const GameConfig> _gameConfig,
			GW::GRAPHICS::GDirectX11Surface _d3d11,
			GW::SYSTEM::GWindow _window, Level_Data& data,unsigned& _pelletCount);
		// control if the system is actively running
		bool Activate(bool runSystem);
		void UpdateScoreText(unsigned int score);
		void UpdateLevel(unsigned int level);
		void UpdateLivesText(unsigned int lives);
		void YouLose();

		//void ChangePelletTransform(unsigned index);
		//void LoadPTransforms();
		void DecrementPelletCount(int num);
		int GetPelletCount();
		void ResetPelletCount();
		// release any resources allocated by the system
		bool Shutdown();
	private:
		// Loading funcs
		void SetYouLose();
		bool LoadShaders();
		bool LoadUniforms();
		void LoadLevelNumBuffer();
		void LoadLevelTextBuffer();
		void LoadMaterialBuffer();
		void LoadLightBuffer();
		void LoadTransformBuffer();
		void LoadLoseTextBuffer();
		void LoadPlayerLivesTextBuffer();
		void LoadPlayerUIBuffer();
		void LoadScoreBuffer();
		void LoadUIBuffer();
		bool LoadGeometry();
		void SetPlayerLivesTextVertices();
		void SetScoreTextVertices();
		bool SetupPipeline();
		bool SetupDrawcalls();
		//void LoadPelletTransforms();
		void TextureSetUp();
		void UISetShaders();
		void DrawUI();
		void DrawScore(const UINT* strides, const UINT* offset);

		void DrawPlayerLives(const UINT* strides, const UINT* offset);
		void DrawLivesText(const UINT* strides, const UINT* offset);
		void DrawLevelText(const UINT* strides, const UINT* offset);
		void DrawLevelNum(const UINT* strides, const UINT* offset);
		void UpdateScore(unsigned int score);
		void UpdateLives(unsigned int lives);
		
		//void CreateTransformPelletBuffer();
		void LevelTextureSetUp();
		PipelineHandles GetCurrentPipelineHandles();
		void ReleasePipelineHandles(PipelineHandles toRelease);
		void SetTexturePixelShader(PipelineHandles curHandles);
		void SetRenderTargets(PipelineHandles handles);
		void SetVertexBuffers(PipelineHandles handles);
		void SetIndexBuffer(PipelineHandles handles);
		void SetShaders(PipelineHandles handles);
		std::wstring FindTextureName(std::string& string);
		SPRITE_DATA UpdateSpriteConstantBufferData(const Sprite& s);
		SPRITE_DATA UpdateTextConstantBufferData(const Text& s);
		bool FillSceneData();
		void LoadSceneBuffer();
		void LoadModelIdsBuffer();
		void LoadSpriteBuffer();
		void LoadVertexBuffer();
		void LoadIndexBuffer();
		void SetScoreVertices();
		void SetLivesVertices();
		void SetLevelTextVertices();
		void SetLevelNumVertices();
		void SetLoseTextVertices();
		// Unloading funcs
		bool FreeVulkanResources();
		void CreateVertexInputLayout(ID3D11Device* creator, Microsoft::WRL::ComPtr<ID3DBlob>& vsBlob);
		void CreateUIVertexInputLayout(ID3D11Device* creator, Microsoft::WRL::ComPtr<ID3DBlob>& vsBlob);
		Microsoft::WRL::ComPtr<ID3DBlob> CompileVertexShader(ID3D11Device* creator, UINT compilerFlags, std::string vertexShaderSource, unsigned type = 0);
		Microsoft::WRL::ComPtr<ID3DBlob> CompilePixelShader(ID3D11Device* creator, UINT compilerFlags, std::string pixelShaderSource, unsigned type = 0);
		// Utility funcs
		std::string ShaderAsString(const char* shaderFilePath);
		GW::MATH::GVECTORF VectorAsString(const char* sting);
	private:

		struct ModelIDS
		{
			unsigned int model_id;
			unsigned int material_id;
			int amountOfLights;
			int padding2;
		};
		struct VECTOR {
			float x, y, z;
		};
		struct SceneData
		{
			GW::MATH::GVECTORF sunDirection, sunColor;
			GW::MATH::GMATRIXF viewMatrix, projectionMatrix;;
			GW::MATH::GVECTORF cameraPos, sunAmbient;
		};
			Level_Data* dLevelData;
			GW::SYSTEM::GLog log;
			ModelIDS modelIds;
			SceneData sceneData;
			std::string textureSource;
			std::string UIsource;
			std::string UIpixelSource;
			UINT width = 0, height = 0;
			Sprite LevelHud;
			HUD hud;
			Text staticText;
			Text scoreText;

			Text playerLives;
			Text livesText;
			Text levelText;
			Text levelNum;

			Text youLose;

			Font consolas32;

			SPRITE_DATA constantBufferData = { 0 };

			PipelineHandles curHandles;

			//Buffers
			Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
			Microsoft::WRL::ComPtr<ID3D11Buffer> pelletTransformBuffer;
			Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
			Microsoft::WRL::ComPtr<ID3D11Buffer> sceneDataBuffer;
			Microsoft::WRL::ComPtr<ID3D11Buffer> modelIdBuffer;
			Microsoft::WRL::ComPtr<ID3D11Buffer> spriteDataBuffer;
			Microsoft::WRL::ComPtr<ID3D11Buffer> lightBuffer;
			Microsoft::WRL::ComPtr<ID3D11Buffer> transformBuffer;
			Microsoft::WRL::ComPtr<ID3D11Buffer> materialBuffer;
			Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBufferStaticText;
			Microsoft::WRL::ComPtr<ID3D11Buffer> scoreVertexBufferStaticText;

			Microsoft::WRL::ComPtr<ID3D11Buffer> levelTextVertexBufferStaticText;
			Microsoft::WRL::ComPtr<ID3D11Buffer> levelNumVertexBufferDynamicText;

			Microsoft::WRL::ComPtr<ID3D11Buffer> playerLivesVertexBufferStaticText;
			Microsoft::WRL::ComPtr<ID3D11Buffer> playerLivesTextVertexBufferStaticText;

			Microsoft::WRL::ComPtr<ID3D11Buffer> youLoseVertexBufferStaticText;

			Microsoft::WRL::ComPtr<ID3D11BlendState> blendState;
			
			//SamplerState
			Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;
			//Shader Resource Views
			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureShaderResource;
			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textShaderResource;
			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> lightSRV;
			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> transformSRV;
			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pelletTransformSRV;
			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> materialSRV;

			Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
			Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
			Microsoft::WRL::ComPtr<ID3D11PixelShader> texturePixelShader;
			Microsoft::WRL::ComPtr<ID3D11InputLayout> vertexFormat;

			Microsoft::WRL::ComPtr<ID3D11Buffer> UIconstantBuffer;
			Microsoft::WRL::ComPtr<ID3D11Buffer> ScoreUIconstantBuffer;
			Microsoft::WRL::ComPtr<ID3D11Buffer> PlayerLivesUIconstantBuffer;
			Microsoft::WRL::ComPtr<ID3D11Buffer> PlayerLivesTextUIconstantBuffer;
			Microsoft::WRL::ComPtr<ID3D11Buffer> youLoseTextUIconstantBuffer;
			Microsoft::WRL::ComPtr<ID3D11Buffer> levelNumUIconstantBuffer;
			Microsoft::WRL::ComPtr<ID3D11Buffer> levelTextUIconstantBuffer;

			Microsoft::WRL::ComPtr<ID3D11VertexShader> UIvertexShader;
			Microsoft::WRL::ComPtr<ID3D11InputLayout> UIvertexFormat;
			Microsoft::WRL::ComPtr<ID3D11PixelShader> UIpixelShader;

			struct debug_vertex { // internal debug vertex
				VECTOR pos;
				VECTOR clr;
				VECTOR nrm;
			}; // caches world space shapes converted to lines
			std::wstring fontTexturePath;

		// Uniform Data Definitions
		//static constexpr unsigned int Instance_Max = 240;
		//struct INSTANCE_UNIFORMS
		//{
		//	GW::MATH::GMATRIXF instance_transforms[Instance_Max];
		//	GW::MATH::GVECTORF instance_colors[Instance_Max];
		//}instanceData;
		//// how many instances will be drawn this frame
		//int draw_counter = 0;
		//int counterOfRenderSystems = 0;
	};
};

#endif


