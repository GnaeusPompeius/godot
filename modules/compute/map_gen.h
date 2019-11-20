/* map_gen.h */

#ifndef MAP_GEN_H
#define MAP_GEN_H

#include "core/reference.h"
#include "core/math/math_funcs.h"
#include "scene/resources/texture.h"
#include "compute.h"

struct TerrainCell {
	float height;
	Vector3 normal_NW;
	Vector3 normal_SE;
};

struct WaterParticle {
	Vector2 position; //  [0, terrain_size]
	Vector2 velocity;
	float mass;
	float padding[3];
};

/*
	TODO:
		Update normals

		spill over the edges

		smooth out | steepen slopes
			It's pooling too much
		introduce evaporation
			
*/

class MapGenerator : public Reference {
	GDCLASS(MapGenerator, Reference);
	const uint32_t world_size_x = 3600, world_size_y = 3600;
	const uint32_t particle_count = 12960000;
	//in meters per pixel
	const float terrain_resolution = 30;

	PoolVector<WaterParticle> data;
	PoolVector<TerrainCell> world;
	Compute<TerrainCell, WaterParticle> terrain_shader;
	
	void load_data();
	void generate_normals();

	TerrainCell get_world_cell(uint32_t x, uint32_t y);
	void set_world_cell(uint32_t x, uint32_t y, TerrainCell cell);
	
protected:
	static void _bind_methods();

public:
	MapGenerator();
	
	Ref<Image> get_image();
	void step();
};

#endif // MAP_GEN_H
