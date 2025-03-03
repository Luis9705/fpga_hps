#ifndef _ALTERA_HPS_0_H_
#define _ALTERA_HPS_0_H_

/*
 * This file was automatically generated by the swinfo2header utility.
 * 
 * Created from SOPC Builder system 'soc_system' in
 * file './soc_system.sopcinfo'.
 */

/*
 * This file contains macros for module 'hps_0' and devices
 * connected to the following masters:
 *   h2f_axi_master
 *   h2f_lw_axi_master
 * 
 * Do not include this header file and another header file created for a
 * different module or master group at the same time.
 * Doing so may result in duplicate macro names.
 * Instead, use the system header file which has macros with unique names.
 */

/*
 * Macros for device 'parallel_port_out_axi', class 'altera_up_avalon_parallel_port'
 * The macros are prefixed with 'PARALLEL_PORT_OUT_AXI_'.
 * The prefix is the slave descriptor.
 */
#define PARALLEL_PORT_OUT_AXI_COMPONENT_TYPE altera_up_avalon_parallel_port
#define PARALLEL_PORT_OUT_AXI_COMPONENT_NAME parallel_port_out_axi
#define PARALLEL_PORT_OUT_AXI_BASE 0x0
#define PARALLEL_PORT_OUT_AXI_SPAN 16
#define PARALLEL_PORT_OUT_AXI_END 0xf

/*
 * Macros for device 'parallel_port_in_axi', class 'altera_up_avalon_parallel_port'
 * The macros are prefixed with 'PARALLEL_PORT_IN_AXI_'.
 * The prefix is the slave descriptor.
 */
#define PARALLEL_PORT_IN_AXI_COMPONENT_TYPE altera_up_avalon_parallel_port
#define PARALLEL_PORT_IN_AXI_COMPONENT_NAME parallel_port_in_axi
#define PARALLEL_PORT_IN_AXI_BASE 0x10
#define PARALLEL_PORT_IN_AXI_SPAN 16
#define PARALLEL_PORT_IN_AXI_END 0x1f


#endif /* _ALTERA_HPS_0_H_ */
