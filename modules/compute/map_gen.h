/* map_gen.h */

#ifndef MAP_GEN_H
#define MAP_GEN_H

#include "core/reference.h"
#include "core/math/math_funcs.h"
#include "scene/resources/texture.h"
#include "compute.h"

/*
TODO:
	Better randomization at outlay
	Rather than bouncing, try sliding

Commit:
	Padding of buffers diagnosed, fixed.
	Getting data now uses glMapBuffer.

	Implemented Triple Buffering?
*/

struct TerrainCell {
	//RainData
	Vector3 normal_NW;
	float padding;
	Vector3 normal_SE;
	float height;
};

struct WaterParticle {
	Vector3 position; //  [0, terrain_size]
	Vector3 velocity;
	float mass;
	float padding;
};

class MapGenerator : public Reference {
	GDCLASS(MapGenerator, Reference);
	const uint32_t world_size_x = 3600, world_size_y = 3600;
	const uint32_t particle_count = 12960000;
	//in meters per pixel
	const float terrain_resolution = 30;

	PoolVector<WaterParticle> data;
	PoolVector<TerrainCell> world;
	Compute<TerrainCell, WaterParticle> terrain_shader;

	Ref<Image> image = memnew(Image);

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
