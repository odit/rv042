#include <linux/pci.h>
#include <asm/io.h>

#define PIO_RESERVED	0x100000UL
#define DEBUGPRINTK(format, ...) do {} while (0)
//#define DEBUGPRINTK(format, ...) printk(format, ##__VA_ARGS__)

static inline int is_pio(void __iomem *addr)
{
    return (unsigned long)addr < PIO_RESERVED;
}

unsigned int fastcall ioread8(void __iomem *addr)
{
    unsigned int result;
    DEBUGPRINTK("%s(%p, PIO=%d)", __FUNCTION__, addr, is_pio(addr));
    if (is_pio(addr))
        result = inb((unsigned long)addr);
    else
        result = readb(addr);
    DEBUGPRINTK("=%x\n", result);
    return result;
}

unsigned int fastcall ioread16(void __iomem *addr)
{
    unsigned int result;
    DEBUGPRINTK("%s(%p, PIO=%d)", __FUNCTION__, addr, is_pio(addr));
    if (is_pio(addr))
        result = inw((unsigned long)addr);
    else
        result = readw(addr);
    DEBUGPRINTK("=%x\n", result);
    return result;
}

#if 0
unsigned int fastcall ioread16be(void __iomem *addr)
{
}
#endif

unsigned int fastcall ioread32(void __iomem *addr)
{
    unsigned int result;
    DEBUGPRINTK("%s(%p, PIO=%d)", __FUNCTION__, addr, is_pio(addr));
    if (is_pio(addr))
        result = inl((unsigned long)addr);
    else
        result = readl(addr);
    DEBUGPRINTK("=%x\n", result);
    return result;
}

#if 0
unsigned int fastcall ioread32be(void __iomem *addr)
{
}
#endif

void fastcall iowrite8(u8 val, void __iomem *addr)
{
    DEBUGPRINTK("%s(%p, PIO=%d)=%x\n", __FUNCTION__, addr, is_pio(addr), (u32)val);
    if (is_pio(addr))
        outb(val, (unsigned long)addr);
    else
        writeb(val, addr);
}

void fastcall iowrite16(u16 val, void __iomem *addr)
{
    DEBUGPRINTK("%s(%p, PIO=%d)=%x\n", __FUNCTION__, addr, is_pio(addr), (u32)val);
    if (is_pio(addr))
        outw(val, (unsigned long)addr);
    else
        writew(val, addr);
}

#if 0
void fastcall iowrite16be(u16 val, void __iomem *addr)
{
}
#endif

void fastcall iowrite32(u32 val, void __iomem *addr)
{
    DEBUGPRINTK("%s(%p, PIO=%d)=%x\n", __FUNCTION__, addr, is_pio(addr), (u32)val);
    if (is_pio(addr))
        outl(val, (unsigned long)addr);
    else
        writel(val, addr);
}

#if 0
void fastcall iowrite32be(u32 val, void __iomem *addr)
{
}
#endif

void fastcall ioread8_rep(void __iomem *port, void *buf, unsigned long count)
{
    u8 *ptr = (u8*)buf;
    while (count--)
        *ptr++ = ioread8(port);
}

void fastcall ioread16_rep(void __iomem *port, void *buf, unsigned long count)
{
    u16 *ptr = (u16*)buf;
    while (count--)
        *ptr++ = ioread16(port);
}

void fastcall ioread32_rep(void __iomem *port, void *buf, unsigned long count)
{
    u32 *ptr = (u32*)buf;
    while (count--)
        *ptr++ = ioread32(port);
}

void fastcall iowrite8_rep(void __iomem *port, const void *buf, unsigned long count)
{
    const u8 *ptr = (const u8*)buf;
    while (count--)
        iowrite8(*ptr++, port);
}

void fastcall iowrite16_rep(void __iomem *port, const void *buf, unsigned long count)
{
    const u16 *ptr = (const u16*)buf;
    while (count--)
        iowrite16(*ptr++, port);
}

void fastcall iowrite32_rep(void __iomem *port, const void *buf, unsigned long count)
{
    const u32 *ptr = (const u32*)buf;
    while (count--)
        iowrite32(*ptr++, port);
}

/* Create a virtual mapping cookie for an IO port range */
void __iomem *ioport_map(unsigned long port, unsigned int nr)
{
    DEBUGPRINTK("%s(port=0x%lx, nr=0x%x)\n", __FUNCTION__, port, nr);
    return (void*)port;
}

void ioport_unmap(void __iomem *addr)
{
    DEBUGPRINTK("%s(%p)\n", __FUNCTION__, addr);
}

/* Create a virtual mapping cookie for a PCI BAR (memory or IO) */
struct pci_dev;
void __iomem *pci_iomap(struct pci_dev *dev, int bar, unsigned long maxLen)
{
    unsigned long start = pci_resource_start(dev, bar);
    unsigned long len = pci_resource_len(dev, bar);
    unsigned long flags = pci_resource_flags(dev, bar);

    DEBUGPRINTK("%s(dev=%p, bar=%d, maxLen=0x%lx)\n", __FUNCTION__, dev, bar, maxLen);

    if (!len || !start)
        return NULL;
    if (flags & IORESOURCE_IO)
    {
        DEBUGPRINTK("    Using address %p\n", (void*)start);
        return(void*)start;
    }
    if (flags & IORESOURCE_MEM)
    {
        DEBUGPRINTK("    Using address %p\n", (void*)((1ull<<63) | start));
        return(void*)((1ull<<63) | start);
    }

    /* What? */
    return NULL;
}

void pci_iounmap(struct pci_dev *dev, void __iomem *addr)
{
    DEBUGPRINTK("%s(dev=%p, addr=%p)\n", __FUNCTION__, dev, addr);
    /* Nothing to do */
}

