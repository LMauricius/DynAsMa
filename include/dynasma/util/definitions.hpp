#pragma once
#ifndef INCLUDED_DYNASMA_DEFINITIONS_H
#define INCLUDED_DYNASMA_DEFINITIONS_H

namespace dynasma {

/*
Depends on the compiler
*/
#ifdef _MSC_VER
#define DYNASMA_NO_UNIQUE_ADDRESS msvc::no_unique_address
#else
#define DYNASMA_NO_UNIQUE_ADDRESS no_unique_address
#endif

} // namespace dynasma

#endif // INCLUDED_DYNASMA_DEFINITIONS_H