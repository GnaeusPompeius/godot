/* map_gen.cpp */

#include "map_gen.h"

void MapGenerator::_bind_methods() {
	//ClassDB::bind_method(D_METHOD("add", "value"), &Summator::add);
	ClassDB::bind_method(D_METHOD("get_image"), &MapGenerator::get_image);
	ClassDB::bind_method(D_METHOD("step"), &MapGenerator::step);
}

MapGenerator::MapGenerator() {
	String path = String("C:\\Godot\\modules\\compute\\river.particle.comp");
	
	terrain_shader.generate_program(path);
		
	world.resize(world_size_x * world_size_y);
	data.resize(particle_count);

	load_data();
	generate_normals();

	terrain_shader.set_workgroups(120, 120, 1);
	terrain_shader.set_input_data(data, world);
	terrain_shader.generate_buffers();
}

void MapGenerator::step() {
	OS::get_singleton()->print("Step taken.\n");
	terrain_shader.step();
}

Ref<Image> MapGenerator::get_image() {

	TerrainCell* out = terrain_shader.open_world_data();
	StreamPeerBuffer image_data;
	Ref<Image> image = memnew(Image);
	image_data.resize(world_size_x * world_size_y * 3 * sizeof(float));

	float value = 0;

	for (int i = 0; i < data.size(); i++) {
		//Floats normalized to [0, 1]
		//OS::get_singleton()->print("Normals: %f\n", out[i].height);

		value = out[i].height / 1400;
		image_data.put_float(value); //R
		image_data.put_float(value); //G
		image_data.put_float(value); //B
		//image_data.put_float(1);//A
	}
	
	image->create(world_size_x, world_size_y, false, Image::FORMAT_RGBF, image_data.get_data_array());
	terrain_shader.close_data();
	return image;
}

TerrainCell MapGenerator::get_world_cell(uint32_t x, uint32_t y) {
	return world.get(x + world_size_x * y);
}


void MapGenerator::set_world_cell(uint32_t x, uint32_t y, TerrainCell cell) {
	return world.set(x + world_size_x * y, cell);
}

void MapGenerator::load_data() {
	//Replace file loading w/ generated data
	String path = String("D:\\Data\\Geography\\N42E12_adj.tif");
	FileAccess *file = FileAccess::open(path, FileAccess::READ);
	uint16_t *source = (uint16_t *)calloc(file->get_len(), sizeof(uint8_t));
	file->get_buffer((uint8_t *)source, file->get_len());

	for (int i = 0; i < world_size_x * world_size_y; i++) {
		TerrainCell cell;
		cell.height = (float)(source[i]);
		world.write().ptr()[i] = cell;
	}
	file->close();
	free(source);
}

//For Irrigation
/*
Godot's Plane Mesh:
  NW _____ Oppo
	 |  /|
	 | / |
	 |/__|
Target	   SE

*/
void MapGenerator::generate_normals() {
	Vector3 NW_t, NW_o, SE_t, SE_o;
	TerrainCell target, opposite, NW, SE;

	for (int i = 0; i < world_size_x - 1; i++) {
		for (int j = 0; j < world_size_y - 1; j++) {
			target = get_world_cell(i, j);
			opposite = get_world_cell(i + 1, j + 1);
			NW = get_world_cell(i, j + 1);
			SE = get_world_cell(i + 1, j);


			//NW Normal
			//	Cross( NW->Target, NW->Oppo )
			NW_t = Vector3(0, -1, target.height - NW.height);
			NW_o = Vector3(1, 0, opposite.height - NW.height);
			target.normal_NW = NW_t.cross(NW_o);

			//SE Normal
			//	Cross( SE->Oppo, SE->Target )
			SE_t = Vector3(-1, 0, target.height - SE.height);
			SE_o = Vector3(0, 1, opposite.height - SE.height);
			target.normal_SE = SE_o.cross(SE_t);

			set_world_cell(i, j, target);
		}
	}
}

