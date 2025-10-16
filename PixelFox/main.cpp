#include "Test.h"
#include "TestEngine.h"
#include "TestMath.h"
#include "TestPhysics.h"

int main()
{
	Test test;
	test.Display();
	TestEngine engine;
	engine.Display();
	TestMath math;
	math.DisplayMath();
	TestPhysics physics;
	physics.DisplayPhyscis();
	return 0;
}