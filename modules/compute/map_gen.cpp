#include "map_gen.h"
/* map_gen.cpp */

#include "map_gen.h"

void MapGenerator::_bind_methods() {
	//ClassDB::bind_method(D_METHOD("add", "value"), &Summator::add);
	ClassDB::bind_method(D_METHOD("get_image"), &MapGenerator::get_image);
}

MapGenerator::MapGenerator() {
	String path = String("C:\\Godot\\modules\\compute\\river.comp");
	Vector<String> paths;
	paths.push_back(path);

	terrain_shader.generate_programs(&paths);

	data.resize(size_x * size_y);
	load_data();
	generate_normals();
}

Ref<Image> MapGenerator::get_image() {
	PoolVector<uint8_t> image_data;
	image_data.resize(size_x * size_y * 4);
	for (int i = 0; i < image_data.size(); i+=4) {
		image_data.write()[i] = (uint8_t)(data.get((i + 1) / 4).height >> 4);
		image_data.write()[i + 1] = (uint8_t)(data.get((i + 1) / 4).height >> 4);
		image_data.write()[i + 2] = (uint8_t)(data.get((i + 1) / 4).height >> 4);
		image_data.write()[i + 3] = 255;
	}
	Ref<Image> image = memnew(Image);
	image->create(size_x, size_y, false, Image::FORMAT_RGBA8, image_data);
	return image;
}

TerrainCell MapGenerator::get_cell(uint32_t x, uint32_t y) {
	return data.get(x + size_x * y);
}

void MapGenerator::load_data() {
	//Replace file loading w/ generated data
	String path = String("D:\\Data\\Geography\\N42E12_adj.tif");
	FileAccess *file = FileAccess::open(path, FileAccess::READ);
	uint16_t *source = (uint16_t *)calloc(file->get_len(), sizeof(uint8_t));
	file->get_buffer((uint8_t *)source, file->get_len());

	for (int i = 0; i < size_x * size_y; i++) {
		TerrainCell cell;
		cell.height = source[i];
		data.set(i, cell);
	}
	file->close();
	free(source);
}

void MapGenerator::generate_normals() {
}
