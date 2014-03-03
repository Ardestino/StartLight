/*
Copyright (c) 2013 Aerys

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
#include "minko/MinkoPNG.hpp"
#include "minko/MinkoSDL.hpp"
#include "minko/MinkoCEF.hpp"

using namespace minko;
using namespace minko::component;
using namespace minko::math;

const std::string TEXTURE_FILENAME = "texture/box.png";

int main(int argc, char** argv)
{
	Cef* cef = new Cef();

	if (!cef->load(argc, argv))
	{
		return cef->cefProcessResult;
	}

	auto canvas = Canvas::create("Minko Example - Cube", 800, 600);

	auto sceneManager = SceneManager::create(canvas->context());
	
	// setup assets
	sceneManager->assets()->defaultOptions()->resizeSmoothly(true);
	sceneManager->assets()->defaultOptions()->generateMipmaps(true);
	sceneManager->assets()
		->registerParser<file::PNGParser>("png")
		->queue(TEXTURE_FILENAME)
		->queue("effect/Basic.effect")
		->queue("effect/Overlay.effect");

	sceneManager->assets()->context()->errorsEnabled(true);

	auto cubeGeometry = geometry::CubeGeometry::create(sceneManager->assets()->context());
	sceneManager->assets()->geometry("cubeGeometry", cubeGeometry);

	auto root = scene::Node::create("root")
		->addComponent(sceneManager);

	auto mesh = scene::Node::create("mesh")
		->addComponent(Transform::create());
	root->addChild(mesh);

	auto camera = scene::Node::create("camera")
		->addComponent(Renderer::create(0x7f7f7fff))
		->addComponent(Transform::create(
		Matrix4x4::create()->lookAt(Vector3::zero(), Vector3::create(0.f, 0.f, 3.f))
		))
		->addComponent(PerspectiveCamera::create(800.f / 600.f, (float)PI * 0.25f, .1f, 1000.f));
	root->addChild(camera);

	auto resized = canvas->resized()->connect([&](AbstractCanvas::Ptr canvas, uint w, uint h)
	{
		camera->component<PerspectiveCamera>()->aspectRatio((float)w / (float)h);
	});

	auto _ = sceneManager->assets()->complete()->connect([=](file::AssetLibrary::Ptr assets)
	{
		cef->initialize(canvas, sceneManager);
		//cef->setURL("http://static3.wikia.nocookie.net/__cb20071019155932/uncyclopedia/images/7/7b/Dancing_banana.gif");
		//cef->setHTML("<html><body color='blue'>Hello World</body></html>");
		//cef->setURL("http://www.benjisbrk.com/public/mousetest");

		cef->loadLocal("html/menu.html");

		cef->executeJavascript("Minko.sendMessage('tatatatata');");

		mesh->addComponent(Surface::create(
			assets->geometry("cubeGeometry"),
			//material::BasicMaterial::create()->diffuseMap(texture),
			material::BasicMaterial::create()->diffuseColor(0xFF0000FF),
			assets->effect("effect/Basic.effect")
		));
	});

	cef->messageReceived()->connect([=](std::string message)
	{
		std::cout << message << std::endl;
	});

	auto enterFrame = canvas->enterFrame()->connect([&](Canvas::Ptr canvas, uint time, uint deltaTime)
	{
		mesh->component<Transform>()->matrix()->appendRotationY(.01f);

		sceneManager->nextFrame();
		//sceneManager->nextFrame();
	});

	sceneManager->assets()->load();
	canvas->run();

	cef->unload();
	return 0;
}


