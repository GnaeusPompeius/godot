/* map_gen.h */

#ifndef MAP_GEN_H
#define MAP_GEN_H

#include "core/reference.h"
#include "core/math/math_funcs.h"
#include "scene/resources/texture.h"
#include "compute.h"

struct TerrainCell {
	uint16_t height;
	float water;
	float normals[2];	// x, y
};

/*
	TODO:
		Test input,  output
			Baseline
			Through one cycle of naive effect.

*/

class MapGenerator : public Reference {
	GDCLASS(MapGenerator, Reference);
	uint32_t size_x = 3600, size_y = 3600;

	Vector<TerrainCell> data;
	Compute<TerrainCell> terrain_shader;

	void load_data();
	void generate_normals();

protected:
	static void _bind_methods();

public:
	MapGenerator();
	
	Ref<Image> get_image();
	TerrainCell get_cell(uint32_t x, uint32_t y);
};

#endif // MAP_GEN_H
