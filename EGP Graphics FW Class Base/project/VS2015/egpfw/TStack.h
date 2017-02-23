#pragma once
#include <initializer_list>
#include <stdexcept>
#include <memory>

template <typename T>
class Stack
{
	private:
		struct Node
		{
			T mData;
			std::shared_ptr<Node> mNext;

			Node() : mData(T()), mNext(nullptr) {}
			explicit Node(const T& d) : mData(d), mNext(nullptr) {}
			Node(const T& d, Node* n) : mData(d), mNext(n) {}
		};

		std::shared_ptr<Node> mHead;
		unsigned int mCount;

	public:
		Stack();
		Stack(std::initializer_list<T> elements);
		~Stack();

		void push(const T& data);
		T pop();
		T peek() const;

		T product();
		
		bool contains(const T& data) const;
		int depth(const T& data) const;

		bool empty() const { return mCount == 0; }
		unsigned int count() const { return mCount; }
};

template <typename T>
Stack<T>::Stack()
{
	mCount = 0;
	mHead = nullptr;
}

template <typename T>
Stack<T>::Stack(std::initializer_list<T> elements)
{
	mCount = 0;
	mHead = nullptr;

	for (auto iter = elements.begin(); iter != elements.end(); ++iter)
		push(*iter);
}

template <typename T>
Stack<T>::~Stack()
{
	while (count() > 0)
		pop();
}

template <typename T>
void Stack<T>::push(const T& data)
{
	auto newNode = std::make_shared<Node>();
	newNode->mData = data;
	newNode->mNext = mHead;

	mHead = newNode;
	mCount++;
}

template <typename T>
T Stack<T>::pop()
{
	if (empty())
		throw std::out_of_range("Tried to pop an empty stack!");

	auto node = mHead;

	mHead = mHead->mNext;
	mCount--;

	auto data = node->mData;
	return data;
}

template <typename T>
T Stack<T>::peek() const
{
	if (empty())
		throw std::out_of_range("Tried to peek an empty stack!");

	return mHead->mData;
}

template <typename T>
T Stack<T>::product()
{
	T result = T();

	auto walker = mHead;
	while (walker != nullptr)
	{
		result *= walker->mData;
		walker = walker->mNext;
	}

	return result;
}

template <typename T>
bool Stack<T>::contains(const T& data) const
{
	return depth(data) > -1;
}

template <typename T>
int Stack<T>::depth(const T& data) const
{
	auto currentNode = mHead;
	int count = 0;

	while (currentNode != nullptr)
	{
		if (currentNode->mData == data)
			return count;

		count++;
		currentNode = currentNode->mNext;
	}

	return -1;
}


void testStack();