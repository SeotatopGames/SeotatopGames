#pragma once
#include "../../gateware-main/Gateware.h"
#include "../h2bParser.h"

namespace EROD
{
	struct BlenderName { std::string blenderName; };
	struct ModelBoundary { GW::MATH::GOBBF boundary; unsigned transformIndex; };
	struct ModelTransform {
		GW::MATH::GMATRIXF* modelTransform;
		unsigned modelIndex;
	};
	struct OrigPlayerPos {
		GW::MATH::GMATRIXF* originalTransform;
	};
	struct ModelMesh { H2B::MESH mesh; };
	struct ModelMaterial { H2B::MATERIAL material; }; 
	struct MeshInstance {
		std::string filename;
		unsigned modelMeshCount = 0;
		unsigned instanceTransformCount = 0;
		unsigned modelVertexStart = 0;
		unsigned modelIndexStart = 0;
		unsigned modelMeshStart = 0;
		unsigned modelMaterialStart = 0;
		unsigned instanceTransformStart = 0;
		Level_Data::LEVEL_MODEL model;
		Level_Data::MODEL_INSTANCES instanceNumber;
	};
	struct SpawnData {
		unsigned instanceTransformStart = 0;
		unsigned instanceTransformCount = 0;
	};
};