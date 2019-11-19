/* compute.h */
#ifndef COMPUTE_H
#define COMPUTE_H

#define GL_GLEXT_PROTOTYPES


#include <cstdio>
#include "core/os/file_access.h"
#include "core/reference.h"
#include "core/os/os.h"
#include "glad/glad.h"

#include "core/print_string.h"

/*
	In the abstract, World are the exogenous variables, Particle the endogenous
*/
template <typename World, typename Particle> class Compute {
	//Workgroup Size
	uint32_t wg_x = 1, wg_y = 1, wg_z = 1;
	uint32_t size_data = 1, size_world = 1;
	GLuint delta = 0;

	PoolVector<World> world;
	PoolVector<Particle> input;

	//GL Data
	GLuint program = 0;
	GLuint in_buffer, out_buffer, world_buffer;
	//SSBO indices
		//Godot doesn't natively use SSBOs, so safety/overlap only a concern for you.
	GLuint in_index = 1, out_index = 2, world_index = 3;


public:
	Compute() {

	}
	~Compute() {
	}

	void set_workgroups(uint32_t x, uint32_t y, uint32_t z) {
		wg_x = x;
		wg_y = y;
		wg_z = z;
	}

	void set_input_data(PoolVector<Particle> data_in, PoolVector<World> world_in, GLuint index_in = 1, GLuint index_out = 2, GLuint index_world = 3) {
		input = data_in;
		world = world_in;

		in_index = index_in;
		out_index = index_out;
		world_index = index_world;

		size_data = input.size() * sizeof(Particle);
		size_world = world.size() * sizeof(World);
	}

	Particle *open_particle_data() {
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, in_buffer);
		GLint bufMask = GL_MAP_READ_BIT;
		Particle *data = (Particle *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, size_data, bufMask);
		
		return data;	
	}

	World *open_world_data() {
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, world_buffer);
		GLint bufMask = GL_MAP_READ_BIT;
		World *data = (World *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, size_world, bufMask);

		return data;
	}

	void close_data() {
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	//void set_uniform(uint shader, )

	void generate_program(const String path) {
		print_line(path);
		GLint result = GL_FALSE;
		GLint infoLogLength;

		program = glCreateProgram();
		GLuint shader = glCreateShader(GL_COMPUTE_SHADER);

		FileAccess *file = FileAccess::open(path, FileAccess::READ);
		uint8_t *source = (uint8_t *)calloc(file->get_len(), sizeof(uint8_t));
		file->get_buffer(source, file->get_len());

		glShaderSource(shader, 1, (const GLchar **)&source, NULL);
		glCompileShader(shader);

		glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
		char *computeShaderErrorMessage = (char *)calloc(infoLogLength, sizeof(char));
		glGetShaderInfoLog(shader, infoLogLength, NULL, &(computeShaderErrorMessage[0]));
		OS::get_singleton()->print("computeShaderErrorMessage: %s\n", computeShaderErrorMessage);

		glAttachShader(program, shader);

		result = GL_FALSE;
		infoLogLength = GL_FALSE;

		glLinkProgram(program);

		glGetProgramiv(program, GL_LINK_STATUS, &result);
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
		char *programErrorMessage = (char *)calloc(infoLogLength, sizeof(char));
		glGetProgramInfoLog(program, infoLogLength, NULL, &(programErrorMessage[0]));
		OS::get_singleton()->print("programErrorMessage: %s\n", programErrorMessage);

		file->close();
		free(computeShaderErrorMessage);
		free(programErrorMessage);
		free(source);
	}

	//set_input_data first.
	void generate_buffers() {
		glGenBuffers(1, &in_buffer);
		glGenBuffers(1, &out_buffer);
		glGenBuffers(1, &world_buffer);

		GLint bufMask = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT;

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, in_buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, size_data, input.read().ptr() , GL_DYNAMIC_DRAW);
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, out_buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, size_data, 0, GL_DYNAMIC_DRAW);
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, world_buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, size_world, world.read().ptr(), GL_STATIC_READ);
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, in_index, in_buffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, out_index, out_buffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, world_index, world_buffer);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	void step() {
		GLint old_program;
		glGetIntegerv(GL_CURRENT_PROGRAM, &old_program);

		glUseProgram(program);

		glUniform1ui(0, delta);
		//Run the thing
		glDispatchCompute(wg_x, wg_y, wg_z);
		//Maybe delay this at end, avoid hang
		//or just thread the whole step()
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
			
		//Ping-Pong the buffers
		GLuint temp = in_buffer;
		in_buffer = out_buffer;
		out_buffer = temp;
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, in_index, in_buffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, out_index, out_buffer);

		glUseProgram(old_program);
		delta++;
	}
};

#endif
