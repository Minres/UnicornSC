/*******************************************************************************
 * Copyright 2022 MINRES Technologies GmbH
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/

#ifndef _UART_H_
#define _UART_H_

#include <scc/tlm_target.h>

class tlm_signal_uart_extension;
class uart_regs;
class WsHandler;

class uart : public sc_core::sc_module, public scc::tlm_target<> {
public:
    SC_HAS_PROCESS(uart);// NOLINT
    sc_core::sc_in<sc_core::sc_time> clk_i{"clk_i"};
    sc_core::sc_in<bool> rst_i{"rst_i"};

    sc_core::sc_out<bool> tx_o{"tx_o"};

    sc_core::sc_out<bool> irq_o{"irq_o"};

    uart(sc_core::sc_module_name nm);
    virtual ~uart() override;

protected:
    void clock_cb();
    void reset_cb();
    void transmit_data();
    void update_irq();
    sc_core::sc_time clk;
    std::unique_ptr<uart_regs> regs;
    sc_core::sc_fifo<uint8_t> rx_fifo{"rx_fifo", 8}, tx_fifo{"tx_fifo", 8};
    std::vector<uint8_t> queue;
    sc_core::sc_event reg_update{"reg_update"};
};

#endif /* _UART_H_ */
