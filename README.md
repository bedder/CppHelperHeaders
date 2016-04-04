# CppHelperHeaders

### What is this?

Over the course of my work I've written a handful of C++ helper classes for different tasks. I thought they might be useful for someone, so here they are.

As of the time of updating this README.md file, here are the classes I've included so far:

| File | Namespace | Class(es) | Description |
| ---- | --------- | --------- | ----------- |
| [`Optional.h`](https://github.com/bedder/CppHelperHeaders/blob/master/include/Optional.h) | `helper::optional` | `Copyable`, `NonCopyable`, (`Optional`), (`ConstructFromParameterList`) | Provides functionality like [`std::optional`](http://en.cppreference.com/w/cpp/utility/optional) but with controls on whether the underlying data type is copy-constructable. | 
| [`ThreadPool.h`](https://github.com/bedder/CppHelperHeaders/blob/master/include/ThreadPool.h) | `helper` | `ThreadPool`, (`ThreadPoolWorker`) | Provides a basic thread pooling system. |

### How do I use these?

Both `Optional` and `ThreadPool` are header-only, and are self contained, so you can just drop them into a project and immediately start using them. But if you're using them for any extended period of time, it might be worth seperating the implementations into `.cpp` files.

As well as the header-only class definitions, I've also tried to include examples of how they can be used in the [`source` directory](https://github.com/bedder/CppHelperHeaders/tree/master/source).

### Am I allowed to use these?

Sure! I've put them under the [MIT license](https://tldrlegal.com/license/mit-license), so as long as you abide by those rules (don't try to hold me liable if they break, and keep in the copyright/license information) then you're good to go.
