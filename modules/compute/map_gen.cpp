/* map_gen.cpp */

#include "map_gen.h"

void MapGenerator::_bind_methods() {
	//ClassDB::bind_method(D_METHOD("add", "value"), &Summator::add);
	ClassDB::bind_method(D_METHOD("get_image"), &MapGenerator::get_image);
	ClassDB::bind_method(D_METHOD("step"), &MapGenerator::step);
}

MapGenerator::MapGenerator() {
	String path = String("C:\\Godot\\modules\\compute\\river.comp");
	Vector<String> paths;
	paths.push_back(path);

	terrain_shader.generate_programs(&paths);

	data.resize(size_x * size_y);
	load_data();
	generate_normals();

	terrain_shader.set_workgroups(120, 120, 1);
	terrain_shader.set_input_data(&data, size_x * size_y);
	terrain_shader.generate_buffers();
}

void MapGenerator::step() {
	OS::get_singleton()->print("Step taken.\n");
	terrain_shader.step();
}

Ref<Image> MapGenerator::get_image() {

	TerrainCell* out = terrain_shader.open_output_data();
	StreamPeerBuffer image_data;
	Ref<Image> image = memnew(Image);
	image_data.resize(size_x * size_y * 3 * sizeof(float));

	float value = 0;

	for (int i = 0; i < data.size(); i++) {
		//Floats normalized to [0, 1]
		//OS::get_singleton()->print("Normals: %f\n", out[i].height);

		value = out[i].water;
		image_data.put_float(value); //R
		image_data.put_float(value); //G
		image_data.put_float(value); //B
		//image_data.put_float(1);//A
	}
	
	image->create(size_x, size_y, false, Image::FORMAT_RGBF, image_data.get_data_array());
	terrain_shader.close_output_data();
	return image;
}

TerrainCell MapGenerator::get_cell(uint32_t x, uint32_t y) {
	return data.get(x + size_x * y);
}

void MapGenerator::set_cell(uint32_t x, uint32_t y, TerrainCell cell) {
	return data.set(x + size_x * y, cell);
}

void MapGenerator::load_data() {
	//Replace file loading w/ generated data
	String path = String("D:\\Data\\Geography\\N42E12_adj.tif");
	FileAccess *file = FileAccess::open(path, FileAccess::READ);
	uint16_t *source = (uint16_t *)calloc(file->get_len(), sizeof(uint8_t));
	file->get_buffer((uint8_t *)source, file->get_len());

	for (int i = 0; i < size_x * size_y; i++) {
		TerrainCell cell;
		cell.height = (float)(source[i]);
		cell.water = 0;
		data.write().ptr()[i] = cell;
	}
	file->close();
	free(source);
}

//For Irrigation
void MapGenerator::generate_normals() {
	float normals[2];
	TerrainCell cell;
	TerrainCell target;

	for (int i = 0; i < size_x; i++) {
		for (int j = 0; j < size_y; j++) {
			float diff = 0;
			float test = 0;
			cell = get_cell(i, j);

			//Don't consider areas off the map.
			for (int x = i - 1 * (i != 0); x <= i + 1 * (i != size_x - 1); x++) {
				for (int y = j - 1 * (j != 0); y <= j + 1 * (j != size_y - 1); y++) {
					target = get_cell(x, y);
					test = (target.height + target.water) - (cell.height + cell.water);

					if (test < diff && test) {
						diff = test;
						normals[0] = x - i;
						normals[1] = y - j;
					}
				}
			}
		cell.normals[0] = normals[0];
		cell.normals[1] = normals[1];

		set_cell(i, j, cell);
		}
	}
}
