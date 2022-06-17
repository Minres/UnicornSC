/*******************************************************************************
 * Copyright 2022 MINRES Technologies GmbH
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/
#include "testbench.h"
#include <util/strprintf.h>
#include <elfio/elfio.hpp>

using namespace std;
using namespace sc_core;
using namespace scc;

testbench::testbench(sc_module_name nm)
: sc_module(nm) {
    SC_HAS_PROCESS(testbench);
    SC_THREAD(run);
    clk = 100_ns;
    clk_32kHz = 0.000030517578125_sec;
    core.clk_i(clk);
    core.reset_i(reset);
    core.isckt(router.target[0]);
    core.internal_mem_start[0].value = 0x0;
    core.internal_mem_size[0].value = 1_MB;
    core.internal_mem_start[1].value = 0x10000000;
    core.internal_mem_size[1].value = 16_kB;
    core.external_mem_start[0].value = 0xffff0000ULL;
    core.external_mem_size[0].value = 64_kB;
    core.arch.value = "RISCV";
    core.mode.value = "RISCV32";
    core.model.value = 1; // UC_CPU_RISCV32_BASE32 = 0,

    uart0.clk_i(clk);
    uart0.rst_i(reset);
    uart0.tx_o(uart0_tx);
    uart0.irq_o(uart0_irq);
    router.initiator[0](mem.target);
    router.set_default_target(0);
    router.initiator[1](uart0.socket);
    router.set_target_range(1, 0xffff0000, 0x40);
}


void testbench::run() {
    reset.write(true);
    wait(5 * clk.read());
    reset.write(false);
    wait(to_host_evt);
    wait(2 * clk.read());
    sc_stop();
}

void testbench::start_of_simulation() {
    if(elf_file.get_value().size() > 0) {
        istringstream is(elf_file.get_value());
        string s;
        while(getline(is, s, ',')) {
            core.start_address.value=load_file(s);
        }
    }
    mem.set_operation_callback([this](memory<4_GB, WIDTH>& m, tlm::tlm_generic_payload& gp, sc_core::sc_time& d) -> int {
        if(gp.is_write() && gp.get_address() == tohost) {
            uint32_t res = *reinterpret_cast<uint32_t*>(gp.get_data_ptr());
            switch(res) {
            case 0: // initialization
                return m.handle_operation(gp, d);
            case 1:
                SCCINFO(SCMOD) << "Firmware signals execution finished";
                break;
            default:
                SCCERR(SCMOD) << "Firmware signals execution failed with exit code 0x" << hex << res;
            }
            to_host_evt.notify(SC_ZERO_TIME);
        } else if(gp.is_read() && gp.get_address() == fromhost) {
            SCCWARN(SCMOD) << "From host read not implemented!";
            to_host_evt.notify(SC_ZERO_TIME);
        }
        return m.handle_operation(gp, d);
    });
}

uint64_t testbench::load_file(std::string name) {
    FILE* fp = fopen(name.c_str(), "r");
    if(fp) {
        std::array<char, 5> buf;
        auto n = fread(buf.data(), 1, 4, fp);
        if(n != 4)
            throw std::runtime_error("input file has insufficient size");
        buf[4] = 0;
        if(strcmp(buf.data() + 1, "ELF") == 0) {
            fclose(fp);
            // Create elfio reader
            ELFIO::elfio reader;
            // Load ELF data
            if(!reader.load(name))
                throw std::runtime_error("could not process elf file");
            // check elf properties
            if(reader.get_class() != ELFCLASS32)
                if(WIDTH == 32)
                    throw std::runtime_error("wrong elf class in file");
            if(reader.get_class() != ELFCLASS64)
                if(WIDTH == 64)
                    throw std::runtime_error("wrong elf class in file");
            if(reader.get_type() != ET_EXEC)
                throw std::runtime_error("wrong elf type in file");
            if(reader.get_machine() != EM_RISCV)
                throw std::runtime_error("wrong elf machine in file");
            auto entry = reader.get_entry();
            for(const auto pseg : reader.segments) {
                const auto fsize = pseg->get_file_size(); // 0x42c/0x0
                const auto seg_data = pseg->get_data();
                if(fsize > 0) {
                    if(!core.write_internal_memory(pseg->get_physical_address(), fsize, reinterpret_cast<const uint8_t* const>(seg_data)))
                        SCCERR(SCMOD) << "problem writing " << fsize << "bytes to 0x" << std::hex
                        << pseg->get_physical_address();
                }
            }
            for(const auto sec : reader.sections) {
                if(sec->get_name() == ".symtab") {
                    if ( SHT_SYMTAB == sec->get_type() ||
                            SHT_DYNSYM == sec->get_type() ) {
                        ELFIO::symbol_section_accessor symbols( reader, sec );
                        auto sym_no = symbols.get_symbols_num();
                        std::string   name;
                        ELFIO::Elf64_Addr    value   = 0;
                        ELFIO::Elf_Xword     size    = 0;
                        unsigned char bind    = 0;
                        unsigned char type    = 0;
                        ELFIO::Elf_Half      section = 0;
                        unsigned char other   = 0;
                        for ( auto i = 0U; i < sym_no; ++i ) {
                            symbols.get_symbol( i, name, value, size, bind, type, section, other );
                            if(name=="tohost") {
                                tohost = value;
                            } else if(name=="fromhost") {
                                fromhost = value;
                            }
                        }
                    }
                }
            }
            return entry;
        }
        throw std::runtime_error("memory load file is not a valid elf file");
    }
    throw std::runtime_error("memory load file not found");
}

