#include <iostream>

#include "pxl8_include.h"

int main(int argc, char** argv)
{
	Pixelate::Log::Init();

	auto renderer = std::make_shared<Pixelate::Renderer>();

	renderer->Init();

	renderer->Dispose();

	return 0;
}
