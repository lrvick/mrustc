/*
 * MRustC - Rust Compiler
 * - By John Hodge (Mutabah/thePowersGang)
 *
 * mir/main_bindings.hpp
 * - main.cpp binding
 */
#pragma once
#include <iostream>

namespace HIR {
class Crate;
}

extern void HIR_GenerateMIR(::HIR::Crate& crate);
extern void MIR_Dump(::std::ostream& sink, const ::HIR::Crate& crate);
