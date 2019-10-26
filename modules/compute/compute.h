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
	//Workgroup Size
	uint32_t wg_x = 1, wg_y = 1, wg_z = 1;
	uint32_t size_data = 1;
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
		programs = Vector<GLuint>();
	}

	void set_workgroups(uint32_t x, uint32_t y, uint32_t z) {
		wg_x = x;
		wg_y = y;
		wg_z = z;
	}
	void set_input_data(Vector<T> *data_in, uint32_t count, GLuint index_in = 1, GLuint index_out = 2) {
		input = data_in;
		in_index = index_in;
		out_index = index_out;
		size_data = count * sizeof(T);
	}

	Vector<T> *get_output_data() {
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, out_buffer);
		GLint bufMask = GL_MAP_READ_BIT;
		T* data = (T *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_CELLS * sizeof(struct cell), bufMask);
		

		//memcpy()

		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	//void set_uniform(uint shader, )

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

			result = GL_FALSE;
			infoLogLength = GL_FALSE;

			glLinkProgram(compute_program);

			glGetProgramiv(compute_program, GL_LINK_STATUS, &result);
			glGetProgramiv(compute_program, GL_INFO_LOG_LENGTH, &infoLogLength);
			char *programErrorMessage = (char *)calloc(infoLogLength, sizeof(char));
			glGetProgramInfoLog(compute_program, infoLogLength, NULL, &(programErrorMessage[0]));
			OS::get_singleton()->print("programErrorMessage: %s\n", programErrorMessage);

			file->close();
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

		GLint bufMask = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT;

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, in_buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, size_data, input->ptrw(), GL_DYNAMIC_DRAW);
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, out_buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, size_data, output->ptrw(), GL_DYNAMIC_DRAW);
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, in_index, in_buffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, out_index, out_buffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	void step(uint32_t delta = 1) {

		GLint old_program;
		glGetIntegerv(GL_CURRENT_PROGRAM, &old_program);
		for (int j = 0; j < delta; j++) {

			for (int i = 0; i < programs.size(); i++) {

				glUseProgram(programs.get(i));

				//Run the thing
				glDispatchCompute(wg_x, wg_y, wg_z);
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
				glBufferData(GL_SHADER_STORAGE_BUFFER, size_data, NULL, GL_DYNAMIC_DRAW);

				glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
			}
		}
		glUseProgram(old_program);
	}
};

#endif
