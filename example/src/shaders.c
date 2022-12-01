#include <glad/glad.h>

#include <stdio.h>
#include <stdlib.h>

/* Read a text file. Adds a null terminator, not included in *size. */
static char *
read_file(const char *filename, long *const size)
{
	FILE *file;
	char *buf;
	size_t read;

	fopen_s(&file, filename, "r");
	if (!file) return 0;

	fseek(file, 0, SEEK_END);
	*size = ftell(file);
	fseek(file, 0, SEEK_SET);

	buf = malloc(((size_t)*size) + 1);
	if (!buf)
	{
		fclose(file);
		return 0;
	}

	read = fread_s(buf, *size, 1, *size, file);

	fclose(file);

	buf[read] = 0;
	return buf;
}

/* Compile a shader from its source file */
GLuint
compile_shader(const char *name, GLenum type)
{
	GLuint result;
	char *buf;
	long bufsize;
	GLint success;
	GLchar error_log[1024];

	buf = read_file(name, &bufsize);
	result = glCreateShader(type);
	if (!result)
	{
		free(buf);
		return 0;
	}

	glShaderSource(result, 1, &buf, NULL);
	glCompileShader(result);

	glGetShaderiv(result, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		printf("'%s':\n", name);
		glGetShaderInfoLog(result, sizeof(error_log), NULL, error_log);
		puts(error_log);

		glDeleteShader(result);
		return 0;
	}

	free(buf);

	return result;
}

/* Link a vertex and fragment shader and return a program */
GLuint
link_shaders(GLuint vert, GLuint frag)
{
	GLuint program;
	GLint success;
	GLchar error_log[1024];

	if (!vert || !frag)
	{
		glDeleteShader(vert);
		glDeleteShader(frag);
		return 0;
	}

	program = glCreateProgram();
	if (!program) return 0;

	glAttachShader(program, vert);
	glAttachShader(program, frag);

	glLinkProgram(program);

	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success)
	{
		printf("error linking '%u' and '%u'\n", vert, frag);
		glGetProgramInfoLog(program, sizeof(error_log), NULL, error_log);
		puts(error_log);

		glDeleteProgram(program);
		glDeleteShader(vert);
		glDeleteShader(frag);
		return 0;
	}

	glDeleteShader(vert);
	glDeleteShader(frag);
	return program;
}