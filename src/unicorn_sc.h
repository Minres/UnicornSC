/*******************************************************************************
 * Copyright 2022 MINRES Technologies GmbH
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/

#ifndef SRC_UNICORN_SC_H_
#define SRC_UNICORN_SC_H_

#include <tlm>
#include <systemc>
#include <memory>

class unicorn_sc: public sc_core::sc_module {
    struct Impl;
public:
    sc_core::sc_in<sc_core::sc_time> clk_i{"clk_i"};

    sc_core::sc_in<bool>  reset_i{"reset_i"};

    tlm::tlm_initiator_socket<32> isckt{"isckt"};

    sc_core::sc_attribute<std::string> arch{"arch", ""};

    sc_core::sc_attribute<std::string> mode{"mode", ""};

    sc_core::sc_attribute<int> model{"model", -1};

    sc_core::sc_vector<sc_core::sc_attribute<uint64_t>> internal_mem_start{"internal_mem_start"};

    sc_core::sc_vector<sc_core::sc_attribute<uint64_t>> internal_mem_size{"internal_mem_size"};

    sc_core::sc_vector<sc_core::sc_attribute<uint64_t>> external_mem_start{"external_mem_start"};

    sc_core::sc_vector<sc_core::sc_attribute<uint64_t>> external_mem_size{"external_mem_size"};

    sc_core::sc_attribute<uint64_t> start_address{"start_address", 0};

    unicorn_sc(sc_core::sc_module_name const& name, size_t num_internal_mems = 1, size_t num_external_mems = 1);

    virtual ~unicorn_sc();

    bool write_internal_memory(uint64_t addr, uint64_t size, unsigned char const* data);

private:
    void before_end_of_elaboration() override;
    void end_of_elaboration() override;
    void start_of_simulation() override;
    void end_of_simulation() override;
    void run();
    std::unique_ptr<Impl> impl;
};

#endif /* SRC_UNICORN_SC_H_ */
