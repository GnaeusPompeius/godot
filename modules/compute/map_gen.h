/* map_gen.h */

#ifndef MAP_GEN_H
#define MAP_GEN_H

#include "core/reference.h"
#include "core/math/math_funcs.h"
#include "scene/resources/texture.h"
#include "compute.h"

struct TerrainCell {
	float height;
	float water;
	int32_t normals[2];	// x, y
	
	uint32_t target_index;
	float target_amount;
	uint32_t port_index;
	float port_amount;
	uint32_t star_index;
	float star_amount;
	uint32_t padding[2];
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
	uint32_t size_x = 3600, size_y = 3600;
	
	PoolVector<TerrainCell> data;
	Compute<TerrainCell> terrain_shader;
	
	void load_data();
	void generate_normals();

	TerrainCell get_cell(uint32_t x, uint32_t y);
	void set_cell(uint32_t x, uint32_t y, TerrainCell cell);

protected:
	static void _bind_methods();

public:
	MapGenerator();
	
	Ref<Image> get_image();
	void step();
};

#endif // MAP_GEN_H
