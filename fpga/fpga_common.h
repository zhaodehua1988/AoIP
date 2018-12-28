#ifndef _FPGA_COMMON_H_H_
#define _FPGA_COMMON_H_H_


#define FPGA_COMMON_DEBUG_MODE
#ifdef FPGA_COMMON_DEBUG_MODE
#define FPGA_printf(...)                            \
    do                                                  \
    {                                                   \
        printf(__VA_ARGS__);                            \
        fflush(stdout);                                 \
    } while (0)
#else
#define PCA9548A_printf(...)
#endif

#endif