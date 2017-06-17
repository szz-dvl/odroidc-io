//Memory split 3/1; font:  linux/arch/arm/mach-meson8b/include/mach/io.h

#define IO_REGS_BASE		0xFE000000
#define IO_CBUS_BASE		(IO_REGS_BASE + 0x100000) // 1M
#define IO_APB_BUS_BASE	        (IO_REGS_BASE + 0x400000) // 2M
#define IO_AOBUS_BASE		(IO_REGS_BASE + 0x600000) // 1M
#define IO_AXI_BUS_BASE		(IO_REGS_BASE + 0x200000) // 2M
#define IO_DOS_BUS_BASE	        (IO_APB_BUS_BASE +  0x50000)
#define IO_SECBUS_BASE		(IO_REGS_BASE + 0x0D0000) // 32k
#define IO_MMC_BUS_BASE	        (IO_REGS_BASE + 0x008000) // 32K


#define CBUS_REG_OFFSET(reg)     ((reg) << 2)
#define CBUS_REG_ADDR(reg)	 (IO_CBUS_BASE + CBUS_REG_OFFSET(reg))

#define VCBUS_REG_ADDR(reg)      (IO_APB_BUS_BASE + 0x100000 + CBUS_REG_OFFSET(reg))

#define AOBUS_REG_OFFSET(reg)    ((reg))
#define AOBUS_REG_ADDR(reg)	 (IO_AOBUS_BASE + AOBUS_REG_OFFSET(reg))

#define AXI_REG_OFFSET(reg)      ((reg) << 2)
#define AXI_REG_ADDR(reg)	 (IO_AXI_BUS_BASE + AXI_REG_OFFSET(reg))

#define DOS_REG_ADDR(reg)        (IO_DOS_BUS_BASE + CBUS_REG_OFFSET(reg))

#define SECBUS_REG_OFFSET(reg)   ((reg) << 2)
#define SECBUS_REG_ADDR(reg)     (IO_SECBUS_BASE + SECBUS_REG_OFFSET(reg))
#define SECBUS2_REG_ADDR(reg)    (IO_SECBUS_BASE + 0x4000 + SECBUS_REG_OFFSET(reg))
#define SECBUS3_REG_ADDR(reg)    (IO_SECBUS_BASE + 0x6000 + SECBUS_REG_OFFSET(reg))

#define MMC_REG_ADDR(reg)        (IO_MMC_BUS_BASE + (reg))


//Keseto?
#define IO_AOBUS_PHY_BASE           0xc8100000 ///1M
#define MESON_PERIPHS1_VIRT_BASE    (IO_AOBUS_BASE + 0x4c0)
#define MESON_PERIPHS1_PHYS_BASE    (IO_AOBUS_PHY_BASE + 0x4c0)
