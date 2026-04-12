#ifndef DEBUG_HPP
#define DEBUG_HPP

#ifdef DEBUG
	#include <iostream>
	#include <string>
	#define DEBUG_LOG(x) do { std::cerr << (x) << '\n'; } while(0)
#else
	#define DEBUG_LOG(x) do { (void)(x); } while(0)
#endif

#endif // DEBUG_HPP
