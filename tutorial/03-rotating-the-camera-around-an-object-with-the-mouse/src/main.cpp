/*
Copyright (c) 2016 Aerys

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "minko/Minko.hpp"
#include "minko/MinkoSDL.hpp"

using namespace minko;
using namespace minko::math;
using namespace minko::component;

const math::uint WINDOW_WIDTH = 800;
const math::uint WINDOW_HEIGHT = 600;

int	main(int argc, char** argv)
{
	auto canvas = Canvas::create("Minko Tutorial - Rotating the camera around an object with the mouse", WINDOW_WIDTH, WINDOW_HEIGHT);
	auto sceneManager = component::SceneManager::create(canvas);
	sceneManager->assets()->loader()->queue("effect/Basic.effect");

	auto root = scene::Node::create("root")
		->addComponent(sceneManager);

	auto camera = scene::Node::create("camera")
		->addComponent(Renderer::create(0x7f7f7fff))
		->addComponent(Transform::create(lookAt(vec3(0.f, 0.f, -5.f), vec3(), vec3(0.f, 1.f, 0.f))))
		->addComponent(PerspectiveCamera::create((float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, (float)M_PI * 0.25f, .1f, 1000.f));

	root->addChild(camera);

	auto cube = scene::Node::create("cube");

	auto complete = sceneManager->assets()->loader()->complete()->connect([&](file::Loader::Ptr loader)
	{
		auto cubeEffect = sceneManager->assets()->effect("effect/Basic.effect");
		auto cubeMaterial = material::BasicMaterial::create();
		cubeMaterial->diffuseColor(vec4(0.f, 0.f, 1.f, 1.f));
		auto cubeGeometry = geometry::CubeGeometry::create(sceneManager->assets()->context());

		cube->addComponent(Surface::create(cubeGeometry, cubeMaterial, cubeEffect));
		cube->addComponent(Transform::create());

		root->addChild(cube);
	});

	sceneManager->assets()->loader()->load();

	Signal<input::Mouse::Ptr, int, int>::Slot mouseMove;

	float cameraRotationSpeed = 0.f;

	auto mouseDown = canvas->mouse()->leftButtonDown()->connect([&](input::Mouse::Ptr mouse)
	{
		mouseMove = canvas->mouse()->move()->connect([&](input::Mouse::Ptr mouse, int dx, int dy)
		{
			cameraRotationSpeed = (float)dx * .01f;
		});
	});

	auto mouseUp = canvas->mouse()->leftButtonUp()->connect([&](input::Mouse::Ptr mouse)
	{
		mouseMove = nullptr;
	});

	auto enterFrame = canvas->enterFrame()->connect([&](Canvas::Ptr canvas, float t, float dt)
	{
		auto rotation = camera->component<Transform>();

		rotation->matrix(rotate(cameraRotationSpeed, vec3(0.f, 1.f, 0.f)) * rotation->matrix());
		cameraRotationSpeed *= .99f;

		sceneManager->nextFrame(t, dt);
	});

	canvas->run();

	return 0;
}