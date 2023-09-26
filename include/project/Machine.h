//
// Created by Lexi Allen on 9/25/2023.
//

#ifndef CHEESE_MACHINE_H
#define CHEESE_MACHINE_H

#include <string>
#include <llvm/Support/Host.h>
#include <llvm/Support/TargetParser.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Target/TargetOptions.h>

#include <llvm/ADT/Triple.h>
#include "util/llvm_utils.h"
#include "error.h"
#include "util/Endianness.h"

namespace cheese::curdle {
    // This defines information about the target machine
    struct Machine {
        std::string triple;
        const llvm::Target *target;
        llvm::TargetMachine *machine;
        util::Endianness endianness;
        llvm::Align stack_alignment;
        std::size_t function_pointer_size;
        std::size_t data_pointer_size;
        llvm::DataLayout layout = llvm::DataLayout{""};

        Machine(std::string in_triple = llvm::sys::getDefaultTargetTriple(), std::string cpu = "generic",
                std::string features = "") {
            // This could have side effects, but they will only be run once, and if we get to this point we need them
            cheese::util::llvm::initialize_llvm();
            triple = in_triple;
            // Now we set everything else up
            std::string error;
            target = llvm::TargetRegistry::lookupTarget(triple, error);
            if (!target) {
                std::string err = "LLVM Error: " + error;
                throw error::CompilerError(cheese::Coordinate{0, 0, 0}, error::ErrorCode::GeneralCompilerError, err);
            }
            llvm::TargetOptions opt;
            auto reloc = llvm::Optional<llvm::Reloc::Model>();
            machine = target->createTargetMachine(triple, cpu, features, opt, reloc);
            layout = machine->createDataLayout();
            stack_alignment = layout.getStackAlignment();
            auto programAddressSpace = layout.getProgramAddressSpace();
            function_pointer_size = layout.getPointerSize(programAddressSpace);
            auto globalAddressSpace = layout.getDefaultGlobalsAddressSpace();
            data_pointer_size = layout.getPointerSize(globalAddressSpace);
            endianness = layout.isBigEndian() ? util::Endianness::Big : util::Endianness::Little;
        }

    };
}
#endif //CHEESE_MACHINE_H
