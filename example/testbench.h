/*******************************************************************************
 * Copyright 2022 MINRES Technologies GmbH
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/
#ifndef TESTBENCH_H_
#define TESTBENCH_H_

#include "uart.h"
#include <unicorn_sc.h>
#include <scc/memory.h>
#include <scc/report.h>
#include <scc/router.h>
#include <tlm/scc/target_mixin.h>
#include <scc/traceable.h>
#include <cci_configuration>

class testbench : public sc_core::sc_module, public scc::traceable {
public:
    enum { WIDTH = 32 };

    unicorn_sc core{"core", 2};

    sc_core::sc_signal<sc_core::sc_time> clk_32kHz{"clk_32kHz"};

    sc_core::sc_signal<sc_core::sc_time> clk{"clk"};

    sc_core::sc_signal<bool> reset{"reset"};

    sc_core::sc_signal<bool> uart0_tx{"uart0_tx"};

    sc_core::sc_signal<bool> uart0_irq{"uart0_irq"};

    scc::router<WIDTH> router{"router", 2};

    scc::memory<4_GB, WIDTH> mem{"mem"};

    uart uart0{"uart0"};

    cci::cci_param<std::string> elf_file{"elf_file", ""};

    cci::cci_param<bool> enable_disass{"enable_disass", false};

    cci::cci_param<bool> heart_beat{"heart_beat", false};

    testbench(sc_core::sc_module_name nm);
    void run();
    void start_of_simulation();
    uint64_t load_file(std::string name);

    sc_core::sc_event from_host_evt, to_host_evt;
    uint64_t tohost{std::numeric_limits<uint64_t>::max()}, fromhost{std::numeric_limits<uint64_t>::max()};
    char buffer[128];
};
#endif /* TESTBENCH_H_ */
