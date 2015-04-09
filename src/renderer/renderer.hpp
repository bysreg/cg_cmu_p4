#ifndef _RENDERER_H_
#define _RENDERER_H_

#include <renderer/camera.hpp>
#include <renderer/RendererInitData.hpp>
#include <scene/scene.hpp>

namespace bey
{
	class Renderer {
	public:

		// You may want to build some scene-specific OpenGL data before the first frame
		bool initialize(const Camera& camera, const Scene& scene, const RendererInitData& data);

		void initialize_static_models(const StaticModel* static_models, size_t num_static_models);

		/*
		 * Render a frame to the currently active OpenGL context.
		 * It's best to keep all your OpenGL-specific data in the renderer; keep the Scene class clean.
		 * This function should not modify the scene or camera.
		 */
		void render(const Camera& camera, const Scene& scene);

		void render_static_model(const StaticModel& static_model);

		// release all OpenGL data and allocated memory
		// you can do this in the destructor instead, but a callable function lets you swap scenes at runtime
		void release();

	};
}

#endif // #ifndef _RENDERER_H_