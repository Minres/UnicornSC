/*******************************************************************************
 * Copyright 2022 MINRES Technologies GmbH
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/

#define SC_INCLUDE_DYNAMIC_PROCESSES

#include "unicorn_sc.h"
#include <unicorn/unicorn.h>
#include <unordered_map>
#include <sstream>

namespace {
const std::unordered_map<std::string, uc_arch> arch_lookup = {
        {"ARM", UC_ARCH_ARM},    // ARM architecture (including Thumb, Thumb-2)
        {"ARM64",UC_ARCH_ARM64}, // ARM-64, also called AArch64
        {"MIPS",UC_ARCH_MIPS},   // Mips architecture
        {"X86",UC_ARCH_X86},     // X86 architecture (including x86 & x86-64)
        {"PPC", UC_ARCH_PPC},    // PowerPC architecture
        {"SPARC",UC_ARCH_SPARC}, // Sparc architecture
        {"M68K",UC_ARCH_M68K},   // M68K architecture
        {"RISCV",UC_ARCH_RISCV}, // RISCV architecture
        {"S390X",UC_ARCH_S390X}  // S390X architecture
};

const std::unordered_map<std::string, uc_mode> mode_lookup = {
        {"LITTLE_ENDIAN", UC_MODE_LITTLE_ENDIAN}    , // little-endian mode (default mode)
        {"BIG_ENDIAN"   , UC_MODE_BIG_ENDIAN}       , // big-endian mode
        // arm / arm64
        {"ARM"          , UC_MODE_ARM}              ,        // ARM mode
        {"THUMB"        , UC_MODE_THUMB}            , // THUMB mode (including Thumb-2)
        // mips
        {"MICRO"        , UC_MODE_MICRO}            ,    // MicroMips mode (currently unsupported)
        {"MIPS3"        , UC_MODE_MIPS3}            ,    // Mips III ISA (currently unsupported)
        {"MIPS32R6"     , UC_MODE_MIPS32R6}         , // Mips32r6 ISA (currently unsupported)
        {"MIPS32"       , UC_MODE_MIPS32}           ,   // Mips32 ISA
        {"MIPS64"       , UC_MODE_MIPS64}           ,   // Mips64 ISA
        // x86 / x64
        {"X86-16"       , UC_MODE_16}               , // 16-bit mode
        {"X86-32"       , UC_MODE_32}               , // 32-bit mode
        {"X86-64"       , UC_MODE_64}               , // 64-bit mode
        // ppc
        {"PPC32"        , UC_MODE_PPC32}            , // 32-bit mode
        {"PPC64"        , UC_MODE_PPC64}            , // 64-bit mode (currently unsupported)
        {"QPX"          , UC_MODE_QPX}              , // Quad Processing eXtensions mode (currently unsupported)
        // sparc
        {"SPARC32"      , UC_MODE_SPARC32}          , // 32-bit mode
        {"SPARC64"      , UC_MODE_SPARC64}          , // 64-bit mode
        {"V9"           , UC_MODE_V9}               ,      // SparcV9 mode (currently unsupported)
        // riscv
        {"RISCV32"      , UC_MODE_RISCV32}          , // 32-bit mode
        {"RISCV64"      , UC_MODE_RISCV64}            // 64-bit mode
};
}

struct unicorn_sc::Impl: public tlm::tlm_bw_transport_if<tlm::tlm_base_protocol_types> {
    unicorn_sc& owner;
    uc_engine *uc{nullptr};
    uc_hook count_hndl{0};
    uc_hook mem_alloc{0};
    uc_hook mem_read{0};
    uint64_t limit{0};
    uint64_t count{0};

    std::vector<std::pair<unicorn_sc::Impl*, uint64_t>> handler;

    explicit Impl(unicorn_sc& owner): owner(owner) { }

    ~Impl(){
        if(uc) uc_close(uc);
    }

    tlm::tlm_sync_enum nb_transport_bw(tlm::tlm_generic_payload& trans,
            tlm::tlm_phase& phase,
            sc_core::sc_time& t) override;

    void invalidate_direct_mem_ptr(sc_dt::uint64 start_range,
            sc_dt::uint64 end_range) override;

    static void hook_count_cb(struct uc_struct *uc, uint64_t address, uint32_t size, void *user_data) {
        Impl* impl= reinterpret_cast<Impl*>(user_data);
        impl->count++;
        if (impl->count >= impl->limit)
            uc_emu_stop(uc);
    }

    static bool hook_memalloc(uc_engine *uc, uc_mem_type type, uint64_t address, int size, int64_t value, void *user_data) {
        uint64_t algined_address = address & 0xFFFFFFFFFFFFF000ULL;
        int aligned_size = ((int)(size / 0x1000) + 1) * 0x1000;

        printf(">>> Allocating block at 0x%" PRIx64 " (0x%" PRIx64
               "), block size = 0x%x (0x%x)\n",
               address, algined_address, size, aligned_size);

        uc_mem_map(uc, algined_address, aligned_size, UC_PROT_ALL);
        // this recovers from missing memory, so we return true
        return true;
    }

    static bool hook_memread(uc_engine *uc, uc_mem_type type, uint64_t address, int size, int64_t value, void *user_data) {
        return true;
    }

    static uint64_t read_cb(uc_engine *uc, uint64_t offset, unsigned size, void *user_data){
        std::pair<unicorn_sc::Impl*, uint64_t>* me = reinterpret_cast<std::pair<unicorn_sc::Impl*, uint64_t>*>(user_data);
        uint64_t ret{0};
        sc_core::sc_time delay= sc_core::SC_ZERO_TIME;
        tlm::tlm_generic_payload gp;
        gp.set_command(tlm::TLM_READ_COMMAND);
        gp.set_address(std::get<1>(*me)+offset);
        gp.set_streaming_width(size);
        gp.set_data_length(size);
        gp.set_data_ptr(reinterpret_cast<uint8_t*>(&ret));
        std::get<0>(*me)->owner.isckt->b_transport(gp, delay);
        sc_assert(gp.get_response_status()==tlm::TLM_OK_RESPONSE);
        return ret;
    }

    static void write_cb(uc_engine *uc, uint64_t offset,  unsigned size, uint64_t value, void *user_data){
        std::pair<unicorn_sc::Impl*, uint64_t>* me = reinterpret_cast<std::pair<unicorn_sc::Impl*, uint64_t>*>(user_data);
        sc_core::sc_time delay= sc_core::SC_ZERO_TIME;
        tlm::tlm_generic_payload gp;
        gp.set_command(tlm::TLM_WRITE_COMMAND);
        gp.set_address(std::get<1>(*me)+offset);
        gp.set_streaming_width(size);
        gp.set_data_length(size);
        gp.set_data_ptr(reinterpret_cast<uint8_t*>(&value));
        std::get<0>(*me)->owner.isckt->b_transport(gp, delay);
        sc_assert(gp.get_response_status()==tlm::TLM_OK_RESPONSE);
    }

    void beoe(){
        auto it_arch = arch_lookup.find(owner.arch.value);
        if(it_arch==std::end(arch_lookup)) {
            SC_REPORT_FATAL(owner.name(),
                    "Illegal core type specification, allowed values are ARM, ARM64, MIPS, X86, PPC, SPARC, M68K, RISCV, S390X");
            return;
        }
        auto arch = it_arch->second;
        auto it_mode = mode_lookup.find(owner.mode.value);
        if(it_mode==std::end(mode_lookup)) {
            SC_REPORT_FATAL(owner.name(), "Illegal core mode specification");
            return;
        }
        auto mode = it_mode->second;
        auto model = owner.model.value;
        switch(arch) {
        case UC_ARCH_ARM:
            if(model>=UC_CPU_ARM_ENDING){
                SC_REPORT_FATAL(owner.name(), "Illegal core model specification");
                return;
            }
            break;
        case UC_ARCH_ARM64:
            if(model>=UC_CPU_ARM64_ENDING){
                SC_REPORT_FATAL(owner.name(), "Illegal core model specification");
                return;
            }
            break;
        case UC_ARCH_MIPS:
            if(mode == UC_MODE_MIPS32){
                if(model>=UC_CPU_MIPS32_ENDING){
                    SC_REPORT_FATAL(owner.name(), "Illegal core model specification");
                    return;
                }
            } else if(mode == UC_MODE_MIPS64){
                if( model>=UC_CPU_MIPS64_ENDING){
                    SC_REPORT_FATAL(owner.name(), "Illegal core model specification");
                    return;
                }
            } else
                model = -1;
            break;
        case UC_ARCH_X86:
            if(model>=UC_CPU_X86_ENDING){
                SC_REPORT_FATAL(owner.name(), "Illegal core model specification");
                return;
            }
            break;
        case UC_ARCH_PPC:
            if(mode == UC_MODE_PPC32){
                if(model>=UC_CPU_PPC32_ENDING){
                    SC_REPORT_FATAL(owner.name(), "Illegal core model specification");
                    return;
                }
            } else if(mode == UC_MODE_PPC64){
                if( model>=UC_CPU_PPC64_ENDING){
                    SC_REPORT_FATAL(owner.name(), "Illegal core model specification");
                    return;
                }
            } else
                model = -1;
            break;
        case UC_ARCH_SPARC:
            if(mode == UC_MODE_SPARC32){
                if(model>=UC_CPU_SPARC32_ENDING){
                    SC_REPORT_FATAL(owner.name(), "Illegal core model specification");
                    return;
                }
            } else if(mode == UC_MODE_SPARC64){
                if( model>=UC_CPU_SPARC64_ENDING){
                    SC_REPORT_FATAL(owner.name(), "Illegal core model specification");
                    return;
                }
            } else
                model = -1;
            break;
        case UC_ARCH_M68K:
            if(model>=UC_CPU_M68K_ENDING){
                SC_REPORT_FATAL(owner.name(), "Illegal core model specification");
                return;
            }
            break;
        case UC_ARCH_RISCV:
            if(mode == UC_MODE_RISCV32){
                if(model>=UC_CPU_RISCV32_ENDING){
                    SC_REPORT_FATAL(owner.name(), "Illegal core model specification");
                    return;
                }
            } else if(mode == UC_MODE_RISCV64){
                if( model>=UC_CPU_RISCV64_ENDING){
                    SC_REPORT_FATAL(owner.name(), "Illegal core model specification");
                    return;
                }
            } else
                model = -1;
            break;
        case UC_ARCH_S390X:
            if(model>=UC_CPU_S390X_ENDING){
                SC_REPORT_FATAL(owner.name(), "Illegal core model specification");
                return;
            }
            break;
        }
        auto err = uc_open(arch, mode, &uc);
        if (err) {
            std::stringstream ss;
            ss<<"Failed on uc_open() with error returned: "<<err<<" ("<<uc_strerror(err)<<")";
            SC_REPORT_FATAL(owner.name(), ss.str().c_str());
            return;
        }
        if(model>=0) {
            err = uc_ctl_set_cpu_model(uc, model);
            if (err) {
                std::stringstream ss;
                ss<<"Failed to set the cpu model with error returned: "<<err<<" ("<<uc_strerror(err)<<")";
                SC_REPORT_FATAL(owner.name(), ss.str().c_str());
                return;
            }
        }
        uc_hook_add(uc, &count_hndl, UC_HOOK_CODE, reinterpret_cast<void*>(hook_count_cb), this, 1, 0);
        // auto-allocate memory on access
//        uc_hook_add(uc, &mem_alloc, UC_HOOK_MEM_UNMAPPED, reinterpret_cast<void*>(hook_memalloc), NULL, 1, 0);
//        uc_hook_add(uc, &mem_read, UC_HOOK_MEM_READ, reinterpret_cast<void*>(hook_memread), NULL, 1, 0);

        for(auto i=0U; i< owner.internal_mem_size.size(); ++i)
            if(owner.internal_mem_size[i].value) {
                uc_mem_map(uc, owner.internal_mem_start[i].value, owner.internal_mem_size[i].value, UC_PROT_ALL);
            }
        if(owner.external_mem_size.value) {
            handler.emplace_back(this, owner.external_mem_start.value);
            auto* entry = &handler.back();
            uc_mmio_map(uc,owner.external_mem_start.value,owner.external_mem_size.value, read_cb, entry, write_cb, entry);
        }
    }

    inline uint64_t get_pc(){
        uc_arch arch;
        uc_ctl_get_arch(uc, &arch);
        uint64_t pc{0};
        switch (arch) {
        default:
            break;
        case UC_ARCH_M68K:
            uc_reg_read(uc, UC_M68K_REG_PC, &pc);
            break;
        case UC_ARCH_X86:
            uc_mode mode;
            uc_ctl_get_mode(uc, &mode);
            switch (mode) {
            default:
                break;
            case UC_MODE_16: {
                uint64_t ip;
                uint16_t cs;
                uc_reg_read(uc, UC_X86_REG_CS, &cs);
                uc_reg_read(uc, UC_X86_REG_IP, &ip);
                pc = ip + cs * 16;
                break;
            }
            case UC_MODE_32:
                uc_reg_read(uc, UC_X86_REG_EIP, &pc);
                break;
            case UC_MODE_64:
                uc_reg_read(uc, UC_X86_REG_RIP, &pc);
                break;
            }
            break;
            case UC_ARCH_ARM:
                uc_reg_read(uc, UC_ARM_REG_R15, &pc);
                break;
            case UC_ARCH_ARM64:
                uc_reg_read(uc, UC_ARM64_REG_PC, &pc);
                break;
            case UC_ARCH_MIPS:
                // TODO: MIPS32/MIPS64/BIGENDIAN etc
                uc_reg_read(uc, UC_MIPS_REG_PC, &pc);
                break;
            case UC_ARCH_SPARC:
                // TODO: Sparc/Sparc64
                uc_reg_read(uc, UC_SPARC_REG_PC, &pc);
                break;
            case UC_ARCH_PPC:
                uc_reg_read(uc, UC_PPC_REG_PC, &pc);
                break;
            case UC_ARCH_RISCV:
                uc_reg_read(uc, UC_RISCV_REG_PC, &pc);
                break;
            case UC_ARCH_S390X:
                uc_reg_write(uc, UC_S390X_REG_PC, &pc);
                break;
        }
        return pc;
    }

};

void unicorn_sc::run() {
    wait(sc_core::SC_ZERO_TIME);
    auto addr=start_address.value;
    while(true){
        impl->limit+=1000;
        if(reset_i.read()) {
            addr=start_address.value;
            wait(reset_i.negedge_event());
        }
        auto err = uc_emu_start(impl->uc, addr, std::numeric_limits<uint64_t>::max(), 0, 0);
        if (err) {
            std::stringstream ss;
            ss<<"Failed on uc_emu_start() with error returned: "<<err<<" ("<<uc_strerror(err)<<") at pc=0x"<<std::hex<<impl->get_pc();
            SC_REPORT_FATAL(name(), ss.str().c_str());
        }
        addr=impl->get_pc();
        wait(100*clk_i.read());
    }
}

unicorn_sc::unicorn_sc(const sc_core::sc_module_name &name, size_t num_internal_mems)
: sc_core::sc_module(name)
, internal_mem_start("internal_mem_start", num_internal_mems)
, internal_mem_size("internal_mem_size", num_internal_mems)
, impl(new unicorn_sc::Impl(*this))
{
    add_attribute(arch);
    add_attribute(mode);
    add_attribute(model);
    for(auto&a:internal_mem_start) add_attribute(a);
    for(auto&a:internal_mem_size) add_attribute(a);
    add_attribute(external_mem_start);
    add_attribute(external_mem_size);
    add_attribute(start_address);

    sc_core::sc_spawn([this](){run();});
    isckt.bind(*impl);
}

unicorn_sc::~unicorn_sc() {
}

tlm::tlm_sync_enum unicorn_sc::Impl::nb_transport_bw(tlm::tlm_generic_payload &trans, tlm::tlm_phase &phase, sc_core::sc_time &t) {
    return tlm::TLM_ACCEPTED;
}

void unicorn_sc::Impl::invalidate_direct_mem_ptr(sc_dt::uint64 start_range, sc_dt::uint64 end_range) {
}

void unicorn_sc::before_end_of_elaboration() { impl->beoe();}

void unicorn_sc::end_of_elaboration() {}

void unicorn_sc::start_of_simulation() {}

bool unicorn_sc::write_internal_memory(uint64_t addr, uint64_t size, const unsigned char *data) {
    return uc_mem_write(impl->uc, addr, data, size-1) == UC_ERR_OK;
}

void unicorn_sc::end_of_simulation() {
    uc_close(impl->uc);
    impl->uc=nullptr;

}

