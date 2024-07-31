#include <d3dcompiler.h>	// required for compiling shaders on the fly, consider pre-compiling instead
#include <Windows.h>
#include <Shobjidl.h>
#include <string>
#pragma comment(lib, "d3dcompiler.lib") 

void PrintLabeledDebugString(const char* label, const char* toPrint)
{
	std::cout << label << toPrint << std::endl;
#if defined WIN32 //OutputDebugStringA is a windows-only function 
	OutputDebugStringA(label);
	OutputDebugStringA(toPrint);
#endif
}


struct ModelData
{
	unsigned int materialId, transformId, numberOfLights;
	float padding_1;
};

struct SceneData
{
	GW::MATH::GVECTORF sunDirection, sunColor, sunAmbient, cameraPosition;
	GW::MATH::GMATRIXF viewMatrix, projectionMatrix;
};

struct MeshData
{
	GW::MATH::GMATRIXF worldMatrix[400];
	H2B::ATTRIBUTES material[400];
};

struct LightData
{
	Level_Data::LIGHT_SETTINGS lightingMatrices[400];
};

// Creation, Rendering & Cleanup
class Renderer
{
	// proxy handles
	Level_Data& levelData;
	GW::SYSTEM::GWindow win;
	GW::GRAPHICS::GDirectX11Surface d3d;
	// what we need at a minimum to draw a triangle
	Microsoft::WRL::ComPtr<ID3D11Buffer>		vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		indexBuffer;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>	vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>	vertexFormat;

	float fov;
	float aspectRatio;

	GW::MATH::GMatrix proxyMatrix;
	GW::MATH::GMATRIXF viewMatrix;
	GW::MATH::GMATRIXF projectionMatrix;
	GW::MATH::GVECTORF lightDirection;
	GW::MATH::GVECTORF lightColor;
	GW::MATH::GVECTORF lightAmbient;
	GW::MATH::GVECTORF cameraPosition;
	std::chrono::steady_clock::time_point start, end;
	// TODO: Part 2B
	SceneData sceneData;
	MeshData meshData;
	ModelData modelData;
	LightData lightData;
	Microsoft::WRL::ComPtr<ID3D11Buffer> sceneConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> meshConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> modelConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> lightConstantBuffer;
	GW::INPUT::GInput proxyKeyboard;
	GW::INPUT::GController proxyController;
	
	D3D11_VIEWPORT viewPorts[2];
	GW::MATH::GMATRIXF viewPortMatrices[2];
	unsigned int playerNumber;
	bool swappable;

public:
	Renderer(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GDirectX11Surface _d3d, Level_Data& _levelData) : levelData(_levelData)
	{
		win = _win;
		d3d = _d3d;
		// TODO: Part 2A
		start = end = std::chrono::steady_clock::now();
		fov = 65.0f;

		proxyKeyboard.Create(_win);
		proxyController.Create();
		proxyMatrix.Create();

		proxyMatrix.IdentityF(viewMatrix);
		GW::MATH::GVECTORF viewTranslation = { 0.75f, 0.25f, -1.5f, 1.0f };
		GW::MATH::GVECTORF viewCenter = { 0.15f, 0.75f, 0.0f, 1.0f };
		GW::MATH::GVECTORF viewUp = { 0.0f, 1.0f, 0.0f, 1.0f };
		proxyMatrix.LookAtLHF(viewTranslation, viewCenter, viewUp, viewMatrix);

		d3d.GetAspectRatio(aspectRatio);
		proxyMatrix.IdentityF(projectionMatrix);
		proxyMatrix.ProjectionDirectXLHF(G_DEGREE_TO_RADIAN(fov), aspectRatio, 0.1f, 100.0f, projectionMatrix);

		lightDirection = { -1.0f, -1.0f, 2.0f, 1.0f };
		lightColor = { 0.9f, 0.9f, 1.0f, 1.0f };
		// TODO: Part 2B
		sceneData.viewMatrix = viewMatrix;
		sceneData.projectionMatrix = projectionMatrix;
		sceneData.sunDirection = lightDirection;
		sceneData.sunColor = lightColor;
		// TODO: Part 4E
		lightAmbient = { 0.25f, 0.25f, 0.35f, 0.0f };
		sceneData.sunAmbient = lightAmbient;
		sceneData.cameraPosition = viewTranslation;

		modelData.materialId = 0;
		modelData.transformId = 0;
		modelData.numberOfLights = levelData.levelLighting.size();

		for (int i = 0; i < levelData.levelTransforms.size(); ++i)
		{
			meshData.worldMatrix[i] = levelData.levelTransforms[i];
		}

		for (int i = 0; i < levelData.levelMaterials.size(); ++i)
		{
			meshData.material[i] = levelData.levelMaterials[i].attrib;
		}

		for (int i = 0; i < levelData.levelLighting.size(); ++i)
		{
			for (int j = 0; j < 16; ++j)
			{
				lightData.lightingMatrices[i].data[j] = levelData.levelLighting[i].data[j];
			}
		}

		proxyMatrix.LookAtLHF(viewTranslation, viewCenter, viewUp, viewPortMatrices[0]);
		proxyMatrix.LookAtLHF(viewTranslation, viewCenter, viewUp, viewPortMatrices[1]);
		playerNumber = 0;
		swappable = false;

		IntializeGraphics();
	}

private:
	//constructor helper functions
	void IntializeGraphics()
	{
		ID3D11Device* creator;
		d3d.GetDevice((void**)&creator);

		InitializeVertexBuffer(creator);
		InitializeIndexBuffer(creator);		
		InitializeConstantBuffers(creator);

		// free temporary handle
		InitializePipeline(creator);
		creator->Release();
	}

	void InitializeConstantBuffers(ID3D11Device* creator)
	{
		// TODO: Part 2C
		D3D11_SUBRESOURCE_DATA bDataScene = { &sceneData, 0, 0 };
		CD3D11_BUFFER_DESC bDescScene(sizeof(SceneData), D3D11_BIND_CONSTANT_BUFFER);
		creator->CreateBuffer(&bDescScene, &bDataScene, sceneConstantBuffer.ReleaseAndGetAddressOf());

		D3D11_SUBRESOURCE_DATA bDataMesh = { &meshData, 0, 0 };
		CD3D11_BUFFER_DESC bDescMesh(sizeof(MeshData), D3D11_BIND_CONSTANT_BUFFER);
		creator->CreateBuffer(&bDescMesh, &bDataMesh, meshConstantBuffer.ReleaseAndGetAddressOf());

		D3D11_SUBRESOURCE_DATA bDataModel = { &modelData, 0, 0 };
		CD3D11_BUFFER_DESC bDescModel(sizeof(ModelData), D3D11_BIND_CONSTANT_BUFFER);
		creator->CreateBuffer(&bDescModel, &bDataModel, modelConstantBuffer.ReleaseAndGetAddressOf());

		D3D11_SUBRESOURCE_DATA bDataLight = { &lightData, 0, 0 };
		CD3D11_BUFFER_DESC bDescLight(sizeof(LightData), D3D11_BIND_CONSTANT_BUFFER);
		creator->CreateBuffer(&bDescLight, &bDataLight, lightConstantBuffer.ReleaseAndGetAddressOf());
	}

	void InitializeVertexBuffer(ID3D11Device* creator)
	{
		// TODO: Part 1C
		CreateVertexBuffer(creator, levelData.levelVertices.data(), sizeof(H2B::VERTEX) * levelData.levelVertices.size());
	}
	
	void CreateVertexBuffer(ID3D11Device* creator, const void* data, unsigned int sizeInBytes)
	{
		D3D11_SUBRESOURCE_DATA bData = { data, 0, 0 };
		CD3D11_BUFFER_DESC bDesc(sizeInBytes, D3D11_BIND_VERTEX_BUFFER);
		creator->CreateBuffer(&bDesc, &bData, vertexBuffer.ReleaseAndGetAddressOf());
	}

	void InitializeIndexBuffer(ID3D11Device* creator)
	{
		// TODO: Part 1C
		CreateIndexBuffer(creator, levelData.levelIndices.data(), sizeof(unsigned int) * levelData.levelIndices.size());
	}

	void CreateIndexBuffer(ID3D11Device* creator, const void* data, unsigned int sizeInBytes)
	{
		D3D11_SUBRESOURCE_DATA ibData = { data, 0, 0 };
		CD3D11_BUFFER_DESC ibDesc(sizeInBytes, D3D11_BIND_INDEX_BUFFER);
		creator->CreateBuffer(&ibDesc, &ibData, indexBuffer.ReleaseAndGetAddressOf());
	}

	void InitializePipeline(ID3D11Device* creator)
	{
		UINT compilerFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if _DEBUG
		compilerFlags |= D3DCOMPILE_DEBUG;
#endif
		Microsoft::WRL::ComPtr<ID3DBlob> vsBlob = CompileVertexShader(creator, compilerFlags);
		Microsoft::WRL::ComPtr<ID3DBlob> psBlob = CompilePixelShader(creator, compilerFlags);
		
		CreateVertexInputLayout(creator, vsBlob);
	}

	Microsoft::WRL::ComPtr<ID3DBlob> CompileVertexShader(ID3D11Device* creator, UINT compilerFlags)
	{
		std::string vertexShaderSource = ReadFileIntoString("../Shaders/VertexShader.hlsl");

		Microsoft::WRL::ComPtr<ID3DBlob> vsBlob, errors;

		HRESULT compilationResult =
			D3DCompile(vertexShaderSource.c_str(), vertexShaderSource.length(),
				nullptr, nullptr, nullptr, "main", "vs_4_0", compilerFlags, 0,
				vsBlob.GetAddressOf(), errors.GetAddressOf());

		if (SUCCEEDED(compilationResult))
		{
			creator->CreateVertexShader(vsBlob->GetBufferPointer(),
				vsBlob->GetBufferSize(), nullptr, vertexShader.GetAddressOf());
		}
		else
		{
			PrintLabeledDebugString("Vertex Shader Errors:\n", (char*)errors->GetBufferPointer());
			abort();
			return nullptr;
		}

		return vsBlob;
	}

	Microsoft::WRL::ComPtr<ID3DBlob> CompilePixelShader(ID3D11Device* creator, UINT compilerFlags)
	{
		std::string pixelShaderSource = ReadFileIntoString("../Shaders/PixelShader.hlsl");
		
		Microsoft::WRL::ComPtr<ID3DBlob> psBlob, errors;

		HRESULT compilationResult =
			D3DCompile(pixelShaderSource.c_str(), pixelShaderSource.length(),
				nullptr, nullptr, nullptr, "main", "ps_4_0", compilerFlags, 0,
				psBlob.GetAddressOf(), errors.GetAddressOf());

		if (SUCCEEDED(compilationResult))
		{
			creator->CreatePixelShader(psBlob->GetBufferPointer(),
				psBlob->GetBufferSize(), nullptr, pixelShader.GetAddressOf());
		}
		else
		{
			PrintLabeledDebugString("Pixel Shader Errors:\n", (char*)errors->GetBufferPointer());
			abort();
			return nullptr;
		}

		return psBlob;
	}

	void CreateVertexInputLayout(ID3D11Device* creator, Microsoft::WRL::ComPtr<ID3DBlob>& vsBlob)
	{
		// TODO: Part 1E 
		D3D11_INPUT_ELEMENT_DESC attributes[3];

		attributes[0].SemanticName = "POSITION";
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

		attributes[2].SemanticName = "NORMAL";
		attributes[2].SemanticIndex = 0;
		attributes[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		attributes[2].InputSlot = 0;
		attributes[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		attributes[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		attributes[2].InstanceDataStepRate = 0;

		creator->CreateInputLayout(attributes, ARRAYSIZE(attributes),
			vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
			vertexFormat.GetAddressOf());
	}

public:
	void Render()
	{
		PipelineHandles curHandles = GetCurrentPipelineHandles();
		SetUpPipeline(curHandles/*, 0*/);

		for (int k = 0; k < 2; k++)
		{
			curHandles.context->RSSetViewports(1, &viewPorts[k]);
			sceneData.viewMatrix = viewPortMatrices[k];

			curHandles.context->UpdateSubresource(sceneConstantBuffer.Get(), 0, nullptr, &sceneData, 0, 0);
			curHandles.context->UpdateSubresource(meshConstantBuffer.Get(), 0, nullptr, &meshData, 0, 0);
			curHandles.context->UpdateSubresource(lightConstantBuffer.Get(), 0, nullptr, &lightData, 0, 0);

			for (int i = 0; i < levelData.levelInstances.size(); ++i)
			{
				const auto& instance = levelData.levelInstances[i];
				const auto object = levelData.levelModels[instance.modelIndex];
				modelData.transformId = instance.transformStart;
				for (unsigned int j = 0; j < object.meshCount; ++j)
				{
					modelData.materialId = levelData.levelMeshes[object.meshStart + j].materialIndex + object.materialStart;

					curHandles.context->UpdateSubresource(modelConstantBuffer.Get(), 0, nullptr, &modelData, 0, 0);
					curHandles.context->DrawIndexedInstanced(levelData.levelMeshes[object.meshStart + j].drawInfo.indexCount,
						instance.transformCount,
						levelData.levelMeshes[object.meshStart + j].drawInfo.indexOffset + object.indexStart,
						object.vertexStart, 0);
				}
			}
		}

		ReleasePipelineHandles(curHandles);

		//PipelineHandles curHandles = GetCurrentPipelineHandles();
		//SetUpPipeline(curHandles, 1);

		//curHandles.context->UpdateSubresource(sceneConstantBuffer.Get(), 0, nullptr, &sceneData, 0, 0);
		//curHandles.context->UpdateSubresource(meshConstantBuffer.Get(), 0, nullptr, &meshData, 0, 0);
		//curHandles.context->UpdateSubresource(lightConstantBuffer.Get(), 0, nullptr, &lightData, 0, 0);
		//curHandles.context->UpdateSubresource(modelConstantBuffer.Get(), 0, nullptr, &modelData, 0, 0);

		//ReleasePipelineHandles(curHandles);
	}

	void UpdateMatrices()
	{
		end = std::chrono::steady_clock::now();
		std::chrono::duration<float> deltaTime = end - start;

		GW::MATH::GMATRIXF cameraMatrix;
		proxyMatrix.InverseF(viewPortMatrices[playerNumber], cameraMatrix);

		const float cameraSpeed = 5.0f;
		float spaceBar, leftShift, w, a, s, d, mouseY, mouseX;
		float forward_back, right_left, up_down;
		forward_back = right_left = up_down = 0.0f;

		GW::MATH::GMATRIXF translationMatrix;
		GW::MATH::GMATRIXF rotateXMatrix;
		GW::MATH::GMATRIXF rotateYMatrix;

		proxyMatrix.IdentityF(translationMatrix);
		proxyMatrix.IdentityF(rotateXMatrix);
		proxyMatrix.IdentityF(rotateYMatrix);

		proxyKeyboard.GetState(G_KEY_SPACE, spaceBar);
		proxyKeyboard.GetState(G_KEY_LEFTSHIFT, leftShift);
		up_down = spaceBar - leftShift;

		proxyKeyboard.GetState(G_KEY_W, w);
		proxyKeyboard.GetState(G_KEY_S, s);
		forward_back = w - s;

		proxyKeyboard.GetState(G_KEY_A, a);
		proxyKeyboard.GetState(G_KEY_D, d);
		right_left = d - a;

		GW::MATH::GVECTORF translationVector = { right_left * cameraSpeed * deltaTime.count(), 
			up_down * cameraSpeed * deltaTime.count(), 
			forward_back * cameraSpeed * deltaTime.count(), 1.0f 
		};
		proxyMatrix.TranslateLocalF(translationMatrix, translationVector, translationMatrix);

		if (proxyKeyboard.GetMouseDelta(mouseX, mouseY) == GW::GReturn::SUCCESS)
		{
			float aspectRatio;
			d3d.GetAspectRatio(aspectRatio);

			unsigned int screenWidth;
			win.GetClientWidth(screenWidth);
			mouseX *= fov * aspectRatio / screenWidth;

			unsigned int screenHeight;
			win.GetClientHeight(screenHeight);
			mouseY *= fov / screenHeight;
		}
		else
		{
			mouseY = 0.0f;
			mouseX = 0.0f;
		}

		proxyMatrix.RotateXLocalF(rotateXMatrix, G_DEGREE_TO_RADIAN_F(mouseY), rotateXMatrix);
		proxyMatrix.RotateYGlobalF(rotateYMatrix, G_DEGREE_TO_RADIAN_F(mouseX), rotateYMatrix);

		proxyMatrix.MultiplyMatrixF(translationMatrix, cameraMatrix, cameraMatrix);
		proxyMatrix.MultiplyMatrixF(rotateXMatrix, cameraMatrix, cameraMatrix);
		GW::MATH::GVECTORF positionLock = cameraMatrix.row4;
		proxyMatrix.MultiplyMatrixF(cameraMatrix, rotateYMatrix, cameraMatrix);
		cameraMatrix.row4 = positionLock;

		proxyMatrix.InverseF(cameraMatrix, viewPortMatrices[playerNumber]);
		start = std::chrono::steady_clock::now();
	}

	void UpdateBuffers()
	{
		ID3D11Device* creator;
		d3d.GetDevice((void**)&creator);
		InitializeConstantBuffers(creator);
		for (int i = 0; i < levelData.levelTransforms.size(); ++i)
		{
			meshData.worldMatrix[i] = levelData.levelTransforms[i];
		}

		for (int i = 0; i < levelData.levelMaterials.size(); ++i)
		{
			meshData.material[i] = levelData.levelMaterials[i].attrib;
		}

		for (int i = 0; i < levelData.levelLighting.size(); ++i)
		{
			for (int j = 0; j < 16; ++j)
			{
				lightData.lightingMatrices[i].data[j] = levelData.levelLighting[i].data[j];
			}
		}

		modelData.numberOfLights = levelData.levelLighting.size();

		InitializeVertexBuffer(creator);
		InitializeIndexBuffer(creator);
		creator->Release();
	}

	void LevelSelect(GW::SYSTEM::GLog& gLog)
	{
		float fOne = 0.0f;
		proxyKeyboard.GetState(G_KEY_F1, fOne);
		if (fOne != 0.0f)
		{
			IShellItem* shellItem = nullptr;
			COMDLG_FILTERSPEC filterDialogSpec[1] = { {L"Text Files", L"*.txt"} };
			if (SUCCEEDED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)))
			{
				IFileDialog* openDialog = nullptr;
				if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, (void**)(&openDialog))))
				{
					openDialog->SetFileTypes(1, filterDialogSpec);
					openDialog->SetTitle(L"Level Select");
					if (SUCCEEDED(openDialog->Show(0)))
					{
						wchar_t* filePath;
						if (SUCCEEDED(openDialog->GetResult(&shellItem)))
						{
							shellItem->GetDisplayName(SIGDN_FILESYSPATH, &filePath);
							if (filePath != 0)
							{
								std::wstring file = std::wstring(filePath);
								std::string fileName(file.begin(), file.end());
								std::string base_file = fileName.substr(fileName.find_last_of("/\\") + 1);
								std::string search = "../" + base_file;
								bool levelLoaded = levelData.LoadLevel(search.c_str(), "../Models", gLog);

								UpdateBuffers();

								CoTaskMemFree(filePath);
								shellItem->Release();
							}
						}
					}
					openDialog->Release();
				}
				CoUninitialize();
			}
		}
	}

	void UpdateWindowSize()
	{
		unsigned int width = 0;
		unsigned int height = 0;
		win.GetClientWidth(width);
		win.GetClientHeight(height);


		viewPorts[0].TopLeftX = 0;
		viewPorts[0].TopLeftY = 0;
		viewPorts[0].MinDepth = 0;
		viewPorts[0].MaxDepth = 1;
		viewPorts[0].Height = height;
		viewPorts[0].Width = width * 0.5;

		viewPorts[1].TopLeftX = width * 0.5;
		viewPorts[1].TopLeftY = 0;
		viewPorts[1].MinDepth = 0;
		viewPorts[1].MaxDepth = 1;
		viewPorts[1].Height = height;
		viewPorts[1].Width = width * 0.5;
	}

	void PlayerSwap()
	{
		float fTwo = 0.0f;
		proxyKeyboard.GetState(G_KEY_F2, fTwo);
		if (fTwo != 0.0f)
		{
			swappable = true;
		}

		if (fTwo == 0.0f && swappable == true)
		{
			if (playerNumber == 0)
			{
				playerNumber = 1;
			}
			else
			{
				playerNumber = 0;
			}
			swappable = false;
		}
	}

private:
	struct PipelineHandles
	{
		ID3D11DeviceContext* context;
		ID3D11RenderTargetView* targetView;
		ID3D11DepthStencilView* depthStencil;
	};

	PipelineHandles GetCurrentPipelineHandles()
	{
		PipelineHandles retval;
		d3d.GetImmediateContext((void**)&retval.context);
		d3d.GetRenderTargetView((void**)&retval.targetView);
		d3d.GetDepthStencilView((void**)&retval.depthStencil);
		return retval;
	}

	void SetUpPipeline(PipelineHandles handles/*, unsigned int mode*/)
	{
		SetRenderTargets(handles);
		SetVertexBuffers(handles);
		SetShaders(handles);

		handles.context->VSSetConstantBuffers(0, 1, sceneConstantBuffer.GetAddressOf());
		handles.context->VSSetConstantBuffers(1, 1, meshConstantBuffer.GetAddressOf());
		handles.context->VSSetConstantBuffers(2, 1, modelConstantBuffer.GetAddressOf());
		handles.context->VSSetConstantBuffers(3, 1, lightConstantBuffer.GetAddressOf());

		handles.context->PSSetConstantBuffers(0, 1, sceneConstantBuffer.GetAddressOf());
		handles.context->PSSetConstantBuffers(1, 1, meshConstantBuffer.GetAddressOf());
		handles.context->PSSetConstantBuffers(2, 1, modelConstantBuffer.GetAddressOf());
		handles.context->PSSetConstantBuffers(3, 1, lightConstantBuffer.GetAddressOf());

		handles.context->IASetInputLayout(vertexFormat.Get());
		handles.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//if (mode == 0)
		//{
		///*handles.context->IASetInputLayout(vertexFormat.Get());
		//handles.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);*/
		//}
		//else if (mode == 1)
		//{
		//	handles.context->IASetInputLayout(vertexFormat.Get());
		//	handles.context->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_LINELIST);
		//}
	}

	void SetRenderTargets(PipelineHandles handles)
	{
		ID3D11RenderTargetView* const views[] = { handles.targetView };
		handles.context->OMSetRenderTargets(ARRAYSIZE(views), views, handles.depthStencil);
	}

	void SetVertexBuffers(PipelineHandles handles)
	{
		const UINT strides[] = { sizeof(H2B::VERTEX) }; // TODO: Part 1E 
		const UINT offsets[] = { 0 };
		ID3D11Buffer* const buffs[] = { vertexBuffer.Get() };
		handles.context->IASetVertexBuffers(0, ARRAYSIZE(buffs), buffs, strides, offsets);
		handles.context->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	}

	void SetShaders(PipelineHandles handles)
	{
		handles.context->VSSetShader(vertexShader.Get(), nullptr, 0);
		handles.context->PSSetShader(pixelShader.Get(), nullptr, 0);
	}

	void ReleasePipelineHandles(PipelineHandles toRelease)
	{
		toRelease.depthStencil->Release();
		toRelease.targetView->Release();
		toRelease.context->Release();
	}


public:
	~Renderer()
	{
		// ComPtr will auto release so nothing to do here yet 
	}
};
