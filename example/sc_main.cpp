/*******************************************************************************
 * Copyright 2022 MINRES Technologies GmbH
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/
#include <scc.h>
#include "testbench.h"

using namespace std;
using namespace sc_core;

int sc_main(int argc, char* argv[]) {
    ///////////////////////////////////////////////////////////////////////////
    // configure logging
    ///////////////////////////////////////////////////////////////////////////
    scc::init_logging(scc::LogConfig()
                          .logLevel(scc::log::TRACE));
    ///////////////////////////////////////////////////////////////////////////
    // check CLI arguments
    ///////////////////////////////////////////////////////////////////////////
    if(argc!=2)
        SCCFATAL()<<"Missing commandline argument: "<<argv[0]<<" <elf file name>";
    ///////////////////////////////////////////////////////////////////////////
    // create modules/channels and trace
    ///////////////////////////////////////////////////////////////////////////
    testbench dut{"dut"};
    dut.elf_file.set_value(argv[1]);
    ///////////////////////////////////////////////////////////////////////////
    // run the simulation
    ///////////////////////////////////////////////////////////////////////////
    try {
        sc_start(1_sec);
        if(!sc_end_of_simulation_invoked())
            sc_stop();
    } catch(sc_report& rep) {
        sc_report_handler::get_handler()(rep, SC_DISPLAY | SC_STOP);
    }
    auto errcnt = sc_report_handler::get_count(SC_ERROR);
    auto warncnt = sc_report_handler::get_count(SC_WARNING);
    return errcnt + warncnt;
}
