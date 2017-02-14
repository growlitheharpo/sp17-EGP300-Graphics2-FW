#include "TStack.h"
#include <iostream>
#include "vector3.h"

void testStack()
{
	Stack<int> aStack({ 5, 4, 3, 2, 1 });

	while (!aStack.empty())
		std::cout << aStack.pop() << " ";

	std::cout << std::endl;

	aStack.push(5);
	aStack.push(75);

	std::cout << aStack.peek() << " ";
	aStack.pop();
	std::cout << aStack.peek() << " ";
	aStack.pop();

	std::cout << " empty now? " << (aStack.empty() ? "true" : "false") << std::endl;

	/*
	Stack<Vector3> vecStack;
	vecStack.push(ONE_VECTOR3);
	vecStack.push(UP_VECTOR3);
	vecStack.push(RIGHT_VECTOR3);

	int depth = vecStack.depth(ONE_VECTOR3);
	bool contains = vecStack.contains(UP_VECTOR3);
	std::cout << "Depth of one vector: " << depth << "  contains up vector " << (contains ? "true" : "false") << std::endl;

	while (!aStack.empty())
		printVector(&vecStack.pop(), '\n');*/

	std::cout << std::endl;
}
