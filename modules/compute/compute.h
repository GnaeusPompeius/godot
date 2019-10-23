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

template <typename T> class Compute {

	uint32_t size_x = 1, size_y = 1, size_z = 1;
	Vector<T> *input;
	Vector<T> *output;

	//GL Data
	Vector<GLuint> programs;
	GLuint in_buffer, out_buffer;
	//SSBO indices
		//Godot doesn't natively use SSBOs, so safety/overlap only a concern for you.
	GLuint in_index = 1, out_index = 2;

public:
	Compute() {
		shaders = Vector<GLuint>();
	}

	void set_size(uint32_t x, uint32_t y, uint32_t z) {
		size_x = x;
		size_y = y;
		size_z = z;
	}
	void set_input_data(Vector<T> *data_in, in_index = 1, out_index = 2) {
		input = data_in;
	}

	Vector<T> *get_output_data() {
		return output;
	}

	//Setuniform

	void generate_programs(Vector<String> *shader_code_path) {
		for (int i = 0; i < shader_code_path->size(); i++) {
			print_line(shader_code_path->get(i));
			GLint result = GL_FALSE;
			GLint infoLogLength;

			GLuint compute_program = glCreateProgram();
			GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
			programs.push_back(compute_program);

			String path = shader_code_path->get(i);
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

			glAttachShader(compute_program, shader);
			free(computeShaderErrorMessage);
			free(source);
			result = GL_FALSE;
			infoLogLength = GL_FALSE;

			glLinkProgram(compute_program);

			glGetProgramiv(compute_program, GL_LINK_STATUS, &result);
			glGetProgramiv(compute_program, GL_INFO_LOG_LENGTH, &infoLogLength);
			char *programErrorMessage = (char *)calloc(infoLogLength, sizeof(char));
			glGetProgramInfoLog(compute_program, infoLogLength, NULL, &(programErrorMessage[0]));
			OS::get_singleton()->print("programErrorMessage: %s\n", programErrorMessage);

			free(computeShaderErrorMessage);
			free(programErrorMessage);
			free(source);
		}
	}

	void generate_buffers(Vector<T> *data_in = input) {
		input = data_in;
		output = input;

		glGenBuffers(1, &in_buffer);
		glGenBuffers(1, &out_buffer);

		GLuint size = size_x * size_y * size_z * sizeof(T);
		GLint bufMask = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT;

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, in_buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, size, input->ptrw(), GL_DYNAMIC_DRAW);
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, out_buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, size, output->ptrw(), GL_DYNAMIC_DRAW);
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, in_index, in_buffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, out_index, out_buffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	void step() {
		GLint old_program;
		glGetIntegerv(GL_CURRENT_PROGRAM, &old_program);

		for (int i = 0; i < programs.size(); i++) {

			glUseProgram(programs.get(i));

			//Run the thing
			glDispatchCompute(32, 32, 1);
			//Maybe delay this at end, avoid hang
				//or just thread the whole step()
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

			//Ping-Pong the buffers, and wipe the new output
			GLuint temp = input;
			input = output;
			output = temp;
			bufMask = GL_MAP_WRITE_BIT;

			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, input);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, output);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, output);
			//Rebuffering zeros is also dumb. Set output to 0 in the shader.
			glBufferData(GL_SHADER_STORAGE_BUFFER, size_x * size_y * size_z * sizeof(T), NULL, GL_DYNAMIC_DRAW);

			glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		}

		glUseProgram(old_program);
	}
};

#endif
