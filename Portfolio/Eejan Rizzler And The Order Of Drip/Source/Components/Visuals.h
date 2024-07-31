// define all ECS components related to drawing
#ifndef VISUALS_H
#define VISUALS_H

// example space game (avoid name collisions)
namespace EROD
{
	struct Color { GW::MATH2D::GVECTOR3F value; };

	struct Material {
		H2B::MATERIAL material;
	};

	struct Mesh {
		H2B::MESH mesh;
	};
};

#endif