#include "renderer/GeometryBuffer.hpp"
#include <iostream>

using namespace bey;

GeometryBuffer::GeometryBuffer()
{
}

GeometryBuffer::~GeometryBuffer()
{
}

void GeometryBuffer::initialize(int screen_width, int screen_height)
{
	shader.load_shader_program("../../shaders/geometry_pass.vs", "../../shaders/geometry_pass.fs");

	// Create the FBO for geometry buffer
	glGenFramebuffers(1, &geometry_buffer_fbo_id);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, geometry_buffer_fbo_id);

	// Create the gbuffer textures
	glGenTextures(NUM_TEXTURES, texture_ids);
	glGenTextures(1, &depth_id);

	for (unsigned int i = 0; i < NUM_TEXTURES; i++)
	{
		glBindTexture(GL_TEXTURE_2D, texture_ids[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, screen_width, screen_height, 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, texture_ids[i], 0);
	}

	// depth for geometry buffer
	glBindTexture(GL_TEXTURE_2D, depth_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH32F_STENCIL8, screen_width, screen_height, 0, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, nullptr);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depth_id, 0);

	for (int i = 0; i < NUM_TEXTURES; i++)
	{
		draw_buffers[i] = GL_COLOR_ATTACHMENT0 + i;
	}
	glDrawBuffers(NUM_TEXTURES, draw_buffers);	

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if (status != GL_FRAMEBUFFER_COMPLETE) {
		std::cerr << "FB error, status: 0x" << status << std::endl;
		exit(EXIT_FAILURE);
		return;
	}

	// restore default FBO
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void GeometryBuffer::bind_texture(const Shader* shader, const GLchar* uniform_name, GeometryBuffer::TextureType texture_type)
{
	GLuint uniform_location = glGetUniformLocation(shader->program, uniform_name);
	glUniform1i(uniform_location, texture_type);
	glActiveTexture(GL_TEXTURE0 + texture_type);
	glBindTexture(GL_TEXTURE_2D, texture_ids[texture_type]);	
}

void GeometryBuffer::bind(BindType bind_type, const Shader* light_shader)
{
	if (bind_type == BindType::READ)
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, geometry_buffer_fbo_id);
	}
	else if (bind_type == BindType::WRITE)
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, geometry_buffer_fbo_id);
		setup_draw_buffers();		
		shader.bind();
	}
	else if (bind_type == BindType::READ_AND_WRITE)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, geometry_buffer_fbo_id);
		setup_draw_buffers();
		shader.bind();
	}
}

void GeometryBuffer::setup_draw_buffers()
{
	glDrawBuffers(NUM_TEXTURES, draw_buffers);
}

void GeometryBuffer::unbind(BindType bind_type)
{
	if (bind_type == BindType::READ)
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	}
	else if (bind_type == BindType::WRITE)
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		//unset the shaders
		shader.unbind();
	}
	else if (bind_type == BindType::READ_AND_WRITE)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		shader.unbind();
	}
}

void GeometryBuffer::set_read_buffer(TextureType texture_type)
{
	glReadBuffer(GL_COLOR_ATTACHMENT0 + (int)texture_type);
}

void GeometryBuffer::dump_geometry_buffer(int screen_width, int screen_height)
{
	GLsizei half_width = (GLsizei)(screen_width / 2.0f);
	GLsizei half_height = (GLsizei)(screen_height / 2.0f);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	bind(BindType::READ);

	set_read_buffer(TextureType::LIGHT_ACCUMULATION);
	glBlitFramebuffer(0, 0, screen_width, screen_height, 0, 0, half_width, half_height, GL_COLOR_BUFFER_BIT, GL_LINEAR);

	set_read_buffer(TextureType::DIFFUSE);
	glBlitFramebuffer(0, 0, screen_width, screen_height, 0, half_height, half_width, screen_height, GL_COLOR_BUFFER_BIT, GL_LINEAR);

	set_read_buffer(TextureType::NORMAL);
	glBlitFramebuffer(0, 0, screen_width, screen_height, half_width, half_height, screen_width, screen_height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	
	set_read_buffer(TextureType::SPECULAR);
	glBlitFramebuffer(0, 0, screen_width, screen_height, half_width, 0, screen_width, half_height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
}

const Shader* GeometryBuffer::get_geometry_pass_shader() const
{
	return &shader;
}