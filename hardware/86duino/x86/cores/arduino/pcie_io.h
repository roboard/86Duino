#ifndef __PCIE_EX2IO_H
#define __PCIE_EX2IO_H

#include "io.h"

struct pcie_io {
    void    *fd;
    bool    mmio;
};

inline unsigned long pcie_io_in(struct pcie_io *io, unsigned long offset)
{
    if (io->mmio)
        return io_In32(io->fd, offset << 2);
    
    return io_In8(io->fd, offset);
}

inline void pcie_io_out(struct pcie_io *io, unsigned long offset, unsigned long value)
{
    if (io->mmio)
        io_Out32(io->fd, offset << 2, value);
    else
        io_Out8(io->fd, offset, value);
}

inline bool pcie_io_init(struct pcie_io *io, bool mmio, unsigned long base)
{
    if (mmio) {
        io->mmio = true;
        io->fd = io_Alloc(IO_USE_MMIO, base, 0x40);
    } else {
        io->mmio = false;
        io->fd = io_Alloc(IO_USE_PORTIO, base, 0x40);
    }
    
    if (io->fd == NULL)
        return false;
    
    return true;
}

inline void pcie_io_close(struct pcie_io *io)
{
    if (io->fd) {
        io_Free(io->fd);
        io->fd = NULL;
    }
}

/* PCI-e Host UART Fast Access */
inline unsigned int pcie_io_GetUartTxFIFOFree(struct pcie_io *io)
{
    return io_In32(io->fd, 0x2C) & 0xFF;
}

inline unsigned int pcie_io_GetUartRxFIFORcvd(struct pcie_io *io)
{
    return (io_In32(io->fd, 0x2C) >> 16) & 0xFF;
}

inline void pcie_io_UartFastWrite(struct pcie_io *io, unsigned long value)
{
    io_Out32(io->fd, 0x20, value);
}

inline unsigned long pcie_io_UartFastRead(struct pcie_io *io)
{
    return io_In32(io->fd, 0x28);
}

#endif
