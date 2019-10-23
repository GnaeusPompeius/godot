/* map_gen.cpp */

#include "map_gen.h"

void MapGenerator::_bind_methods() {
	//ClassDB::bind_method(D_METHOD("add", "value"), &Summator::add);
}

MapGenerator::MapGenerator() {
	String path = String("C:\\Godot\\modules\\compute\\river.comp");
	Vector<String> paths;
	paths.push_back(path);

	print_line(String("onward"));
	terrain_shader.generate_program(&paths);
}
