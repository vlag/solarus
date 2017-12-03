/*
 * Copyright (C) 2006-2016 Christopho, Solarus - http://www.solarus-games.org
 *
 * Solarus is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Solarus is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "solarus/lowlevel/shaders/ShaderContext.h"
#include "solarus/lowlevel/shaders/GL_ARBShader.h"
#include "solarus/lowlevel/shaders/GL_2DShader.h"
#include "solarus/lowlevel/Logger.h"
#include "solarus/lowlevel/QuestFiles.h"
#include "solarus/lowlevel/Video.h"
#include "solarus/lua/LuaContext.h"
#include "solarus/lua/LuaTools.h"
#include <string>


namespace Solarus {

SDL_GLContext ShaderContext::gl_context = nullptr;
bool ShaderContext::is_arb_supported = false;

/**
 * \brief Initializes the shader system.
 * \return \c true if any shader system is supported.
 */
bool ShaderContext::initialize() {

  const char* opengl_version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
  const char* shading_language_version = reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION));
  const char* opengl_vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
  const char* opengl_renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));

  Logger::info(std::string("OpenGL: ") + opengl_version);
  Logger::info(std::string("OpenGL vendor: ") + opengl_vendor);
  Logger::info(std::string("OpenGL renderer: ") + opengl_renderer);
  Logger::info(std::string("OpenGL shading language: ") + shading_language_version);

  SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  if (!(gl_context = SDL_GL_CreateContext(Video::get_window()))) {
    Debug::warning("Unable to create OpenGL context : " + std::string(SDL_GetError()));
    return false;
  }

  // Setting some parameters
  glEnable(GL_DEPTH_TEST); // The type of depth test to do.
  glDepthFunc(GL_LESS); // Enables depth testing.

  // Use late swap tearing, or try to use the classic swap interval (aka VSync) if not supported.
  if (SDL_GL_SetSwapInterval(-1) == -1) {
    SDL_GL_SetSwapInterval(1);
  }

  // Try to initialize a gl shader system, in order from the earlier to the older.
  is_arb_supported = GL_ARBShader::initialize();

  return is_arb_supported || GL_2DShader::initialize();
}

/**
 * \brief Free shader-related context.
 */
void ShaderContext::quit() {

  if (gl_context) {
    SDL_GL_DeleteContext(gl_context);
  }
}

/**
 * \brief Construct a shader from a name.
 * \param shader_id The id of the shader to load.
 * \return The created shader, or nullptr if the shader fails to compile.
 */
std::unique_ptr<Shader> ShaderContext::create_shader(const std::string& shader_id) {

  std::unique_ptr<Shader> shader = nullptr;

  if (is_arb_supported) {
    shader = std::unique_ptr<Shader>(new GL_ARBShader(shader_id));
  }
  else {
    shader = std::unique_ptr<Shader>(new GL_2DShader(shader_id));
  }

  if (glGetError() != GL_NO_ERROR) {
    Debug::error("Can't create shader '" + shader_id + "'");
    shader = nullptr;
  }

  return shader;
}

}
