/*
 * mini_cpp.cpp
 *
 *  Created on: 22 mar 2016
 *      Author: scott
 */

#include <stdlib.h>

extern "C" void * malloc(size_t size)
{
	return (void *)0 ;
}

extern "C" void free(void * ptr)
{}

void * operator new (size_t size) noexcept
{
	return malloc(size);
}

void operator delete( void *p ) noexcept
{
	free(p);
}

extern "C" int __aeabi_atexit(	void *object,
								void (*destructor)(void *),
								void *dso_handle)
{
	return 0;
}


extern "C" void __cxa_pure_virtual()
{
	while (1);
}
