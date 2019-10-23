/* map_gen.h */

#ifndef MAP_GEN_H
#define MAP_GEN_H

#include "core/reference.h"
#include "compute.h"

struct TerrainCell {
	float mass;
	float wind;
	int destination;

	float star_wind;
	int star_destination;

	float port_wind;
	int port_destination;
	int padding;
};

class MapGenerator : public Reference {
	GDCLASS(MapGenerator, Reference);

	Compute<TerrainCell> terrain_shader;

protected:
	static void _bind_methods();

public:
	MapGenerator();
};

#endif // MAP_GEN_H
