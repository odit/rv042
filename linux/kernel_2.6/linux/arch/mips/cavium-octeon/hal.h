/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2004, 2005 Cavium Networks
 */
#ifndef __CAVIUM_OCTEON_HAL_H
#define __CAVIUM_OCTEON_HAL_H

extern void *octeon_bootmem_alloc(uint64_t size, uint64_t alignment);
extern void *octeon_bootmem_alloc_range(uint64_t size, uint64_t alignment, uint64_t min_addr, uint64_t max_addr);
extern int octeon_is_pci_host(void);
extern uint64_t octeon_get_clock_rate(void);
extern const char *octeon_board_type_string(void);
extern const char *octeon_get_pci_interrupts(void);
extern int octeon_get_southbridge_interrupt(void);
extern int octeon_get_boot_coremask(void);
extern int octeon_get_boot_num_arguments(void);
extern const char *octeon_get_boot_argument(int arg);
void octeon_hal_setup_reserved32(void);
int octeon_l2_lock_range(uint64_t addr, uint64_t len);
extern unsigned long octeon_crypto_enable(struct octeon_cop2_state *state);
extern void octeon_crypto_disable(struct octeon_cop2_state *state, unsigned long flags);

typedef union
{
    uint64_t u64;
    struct
    {
        uint64_t    tlbbist         : 1;    /**< RO 1 = BIST fail, 0 = BIST pass */
        uint64_t    l1cbist         : 1;    /**< RO 1 = BIST fail, 0 = BIST pass */
        uint64_t    l1dbist         : 1;    /**< RO 1 = BIST fail, 0 = BIST pass */
        uint64_t    dcmbist         : 1;    /**< RO 1 = BIST fail, 0 = BIST pass */
        uint64_t    ptgbist         : 1;    /**< RO 1 = BIST fail, 0 = BIST pass */
        uint64_t    wbfbist         : 1;    /**< RO 1 = BIST fail, 0 = BIST pass */
        uint64_t    reserved        : 22;   /**< Reserved */
        uint64_t    dismarkwblongto : 1;    /**< R/W If set, marked write-buffer entries time out the same as
                                                as other entries; if clear, marked write-buffer entries use the
                                                maximum timeout. */
        uint64_t    dismrgclrwbto   : 1;    /**< R/W If set, a merged store does not clear the write-buffer entry
                                                timeout state. */
        uint64_t    iobdmascrmsb    : 2;    /**< R/W Two bits that are the MSBs of the resultant CVMSEG LM word
                                                location for an IOBDMA. The other 8 bits come from the SCRADDR
                                                field of the IOBDMA. */
        uint64_t    syncwsmarked    : 1;    /**< R/W If set, SYNCWS and SYNCS only order marked stores; if clear,
                                                SYNCWS and SYNCS only order unmarked stores. SYNCWSMARKED has no
                                                effect when DISSYNCWS is set. */
        uint64_t    dissyncws       : 1;    /**< R/W If set, SYNCWS acts as SYNCW and SYNCS acts as SYNC. */
        uint64_t    diswbfst        : 1;    /**< R/W If set, no stall happens on write buffer full. */
        uint64_t    xkmemenas       : 1;    /**< R/W If set (and SX set), supervisor-level loads/stores can use
                                                XKPHYS addresses with VA<48>==0 */
        uint64_t    xkmemenau       : 1;    /**< R/W If set (and UX set), user-level loads/stores can use XKPHYS
                                                addresses with VA<48>==0 */
        uint64_t    xkioenas        : 1;    /**< R/W If set (and SX set), supervisor-level loads/stores can use
                                                XKPHYS addresses with VA<48>==1 */
        uint64_t    xkioenau        : 1;    /**< R/W If set (and UX set), user-level loads/stores can use XKPHYS
                                                addresses with VA<48>==1 */
        uint64_t    allsyncw        : 1;    /**< R/W If set, all stores act as SYNCW (NOMERGE must be set when
                                                this is set) RW, reset to 0. */
        uint64_t    nomerge         : 1;    /**< R/W If set, no stores merge, and all stores reach the coherent
                                                bus in order. */
        uint64_t    didtto          : 2;    /**< R/W Selects the bit in the counter used for DID time-outs
                                                0 = 231, 1 = 230, 2 = 229, 3 = 214. Actual time-out is between
                                                1× and 2× this interval. For example, with DIDTTO=3, expiration
                                                interval is between 16K and 32K. */
        uint64_t csrckalwys         : 1;    /**< R/W If set, the (mem) CSR clock never turns off. */
        uint64_t mclkalwys          : 1;    /**< R/W If set, mclk never turns off. */
        uint64_t wbfltime           : 3;    /**< R/W Selects the bit in the counter used for write buffer flush
                                                time-outs (WBFLT+11) is the bit position in an internal counter
                                                used to determine expiration. The write buffer expires between
                                                1× and 2× this interval. For example, with WBFLT = 0, a write
                                                buffer expires between 2K and 4K cycles after the write buffer
                                                entry is allocated. */
        uint64_t istrnol2           : 1;    /**< R/W If set, do not put Istream in the L2 cache. */
        uint64_t wbthresh           : 4;    /**< R/W The write buffer threshold. */
        uint64_t reserved2          : 2;    /**< Reserved */
        uint64_t cvmsegenak         : 1;    /**< R/W If set, CVMSEG is available for loads/stores in kernel/debug mode. */
        uint64_t cvmsegenas         : 1;    /**< R/W If set, CVMSEG is available for loads/stores in supervisor mode. */
        uint64_t cvmsegenau         : 1;    /**< R/W If set, CVMSEG is available for loads/stores in user mode. */
        uint64_t lmemsz             : 6;    /**< R/W Size of local memory in cache blocks, 54 (6912 bytes) is max legal value. */
    } s;
} octeon_cvmemctl_t;

#define OCTEON_CIU_INTX_SUM0(offset) (0x8001070000000000ull+((offset)*8))
#define OCTEON_CIU_INTX_EN0(offset) (0x8001070000000200ull+((offset)*16))
typedef union
{
    uint64_t u64;
    struct
    {
        uint64_t reserved                : 8;
        uint64_t timer                   : 4;
        uint64_t key_zero                : 1;
        uint64_t ipd_drp                 : 1;
        uint64_t gmx_drp                 : 2;
        uint64_t trace                   : 1;
        uint64_t rml                     : 1;
        uint64_t twsi                    : 1;
        uint64_t wdog_sum                : 1;
        uint64_t pci_msi                 : 4;
        uint64_t pci_int                 : 4;
        uint64_t uart                    : 2;
        uint64_t mbox                    : 2;
        uint64_t gpio                    : 16;
        uint64_t workq                   : 16;
    } s;
} octeon_ciu_intx0_t;

#define OCTEON_IOB_FAU_TIMEOUT (0x80011800F0000000ull)
typedef union
{
    uint64_t u64;
    struct
    {
        uint64_t reserved                : 51;
        uint64_t tout_enb                : 1;
        uint64_t tout_val                : 12;
    } s;
} octeon_iob_fau_timeout_t;

#define OCTEON_POW_NW_TIM (0x8001670000000210ull)
typedef union
{
    uint64_t u64;
    struct
    {
        uint64_t reserved                : 54;
        uint64_t nw_tim                  : 10;
    } s;
} octeon_pow_nw_tim_t;

#define OCTEON_LMC_FADR (0x8001180088000020ull)
typedef union
{
    uint64_t u64;
    struct
    {
        uint64_t reserved                : 32;
        uint64_t fdimm                   : 2;
        uint64_t fbunk                   : 1;
        uint64_t fbank                   : 3;
        uint64_t frow                    : 14;
        uint64_t fcol                    : 12;
    } s;
} octeon_lmc_fadr_t;

#define OCTEON_NPI_MEM_ACCESS_SUBID3 (0x80011F0000000028ull)
#define OCTEON_NPI_MEM_ACCESS_SUBID4 (0x80011F0000000030ull)
#define OCTEON_NPI_MEM_ACCESS_SUBID5 (0x80011F0000000038ull)
#define OCTEON_NPI_MEM_ACCESS_SUBID6 (0x80011F0000000040ull)
typedef union
{
    uint64_t u64;
    struct
    {
        uint64_t reserved                : 28;      /**< Reserved */
        uint64_t esr                     : 2;       /**< Endian-Swap on read. */
        uint64_t esw                     : 2;       /**< Endian-Swap on write. */
        uint64_t nsr                     : 1;       /**< No-Snoop on read. */
        uint64_t nsw                     : 1;       /**< No-Snoop on write. */
        uint64_t ror                     : 1;       /**< Relax Read on read. */
        uint64_t row                     : 1;       /**< Relax Order on write. */
        uint64_t ba                      : 28;      /**< PCI Address bits [63:36]. */
    } s;
} octeon_npi_mem_access_subid_t;

#define OCTEON_NPI_PCI_CFG00    (0x80011F0000001800ull)
typedef union
{
    uint32_t u32;
    struct
    {
        uint64_t devid                   : 16;      /**< This is the device ID for N3 */
        uint64_t vendid                  : 16;      /**< This is the Cavium's vendor ID */
    } s;
} octeon_pci_cfg00_t;

#define OCTEON_NPI_PCI_CFG01    (0x80011F0000001804ull)
typedef union
{
    uint32_t u32;
    struct
    {
        uint64_t dpe                     : 1;       /**< Detected Parity Error */
        uint64_t sse                     : 1;       /**< Signaled System Error */
        uint64_t rma                     : 1;       /**< Received Master Abort */
        uint64_t rta                     : 1;       /**< Received Target Abort */
        uint64_t sta                     : 1;       /**< Signaled Target Abort */
        uint64_t devt                    : 2;       /**< DEVSEL# timing (for PCI only/for PCIX = don?t care) */
        uint64_t mdpe                    : 1;       /**< Master Data Parity Error */
        uint64_t fbb                     : 1;       /**< Fast Back-to-Back Transactions Capable
                                                         Mode               1 = PCI Mode     0 = PCIX Mode
                                                         Dependent */
        uint64_t reserved                : 1;       /**< Reserved */
        uint64_t m66                     : 1;       /**< 66MHz Capable */
        uint64_t cle                     : 1;       /**< Capabilities List Enable */
        uint64_t i_stat                  : 1;       /**< When INTx# is asserted by N3 this bit will be set.
                                                         When deasserted by N3 this bit will be cleared. */
        uint64_t reserved1               : 8;       /**< Reserved */
        uint64_t i_dis                   : 1;       /**< When asserted '1' disables the generation of INTx#
                                                         by N3. When disabled '0' allows assertion of INTx#
                                                         by N3. */
        uint64_t fbbe                    : 1;       /**< Fast Back to Back Transaction Enable */
        uint64_t see                     : 1;       /**< System Error Enable */
        uint64_t ads                     : 1;       /**< Address/Data Stepping */
        uint64_t pee                     : 1;       /**< PERR# Enable */
        uint64_t vps                     : 1;       /**< VGA Palette Snooping */
        uint64_t mwice                   : 1;       /**< Memory Write & Invalidate Command Enable */
        uint64_t scse                    : 1;       /**< Special Cycle Snooping Enable */
        uint64_t me                      : 1;       /**< Master Enable */
        uint64_t msae                    : 1;       /**< Memory Space Access Enable */
        uint64_t isae                    : 1;       /**< I/O Space Access Enable */
    } s;
} octeon_pci_cfg01_t;

#define OCTEON_NPI_PCI_CFG02    (0x80011F0000001808ull)
typedef union
{
    uint32_t u32;
    struct
    {
        uint64_t cc                      : 24;      /**< Class Code (Network Encryption/Decryption Class) */
        uint64_t rid                     : 8;       /**< Revision ID */
    } s;
} octeon_pci_cfg02_t;

#define OCTEON_NPI_PCI_CFG03    (0x80011F000000180Cull)
typedef union
{
    uint32_t u32;
    struct
    {
        uint64_t bcap                    : 1;       /**< BIST Capable */
        uint64_t brb                     : 1;       /**< BIST Request/busy bit
                                                         Note: N3 does not support PCI BIST, therefore
                                                         this bit should remain zero. */
        uint64_t reserved                : 2;       /**< Reserved */
        uint64_t bcod                    : 4;       /**< BIST Code */
        uint64_t ht                      : 8;       /**< Header Type (Type 0) */
        uint64_t lt                      : 8;       /**< Latency Timer
                                                         (0=PCI)                 (0=PCI)
                                                         (0x40=PCIX)             (0x40=PCIX) */
        uint64_t cls                     : 8;       /**< Cache Line Size */
    } s;
} octeon_pci_cfg03_t;

#define OCTEON_NPI_PCI_CFG04    (0x80011F0000001810ull)
typedef union
{
    uint32_t u32;
    struct
    {
        uint64_t lbase                   : 20;      /**< Base Address[31:12] */
        uint64_t lbasez                  : 8;       /**< Base Address[11:4] (Read as Zero) */
        uint64_t pf                      : 1;       /**< Prefetchable Space */
        uint64_t typ                     : 2;       /**< Type (00=32b/01=below 1MB/10=64b/11=RSV) */
        uint64_t mspc                    : 1;       /**< Memory Space Indicator */
    } s;
} octeon_pci_cfg04_t;

#define OCTEON_NPI_PCI_CFG05    (0x80011F0000001814ull)
typedef union
{
    uint32_t u32;
    struct
    {
        uint64_t hbase                   : 32;      /**< Base Address[63:32] */
    } s;
} octeon_pci_cfg05_t;

#define OCTEON_NPI_PCI_CFG06    (0x80011F0000001818ull)
typedef union
{
    uint32_t u32;
    struct
    {
        uint64_t lbase                   : 5;       /**< Base Address[31:27] */
        uint64_t lbasez                  : 23;      /**< Base Address[26:4] (Read as Zero) */
        uint64_t pf                      : 1;       /**< Prefetchable Space */
        uint64_t typ                     : 2;       /**< Type (00=32b/01=below 1MB/10=64b/11=RSV) */
        uint64_t mspc                    : 1;       /**< Memory Space Indicator */
    } s;
} octeon_pci_cfg06_t;

#define OCTEON_NPI_PCI_CFG07    (0x80011F000000181Cull)
typedef union
{
    uint32_t u32;
    struct
    {
        uint64_t hbase                   : 32;      /**< Base Address[63:32] */
    } s;
} octeon_pci_cfg07_t;

#define OCTEON_NPI_PCI_CFG08    (0x80011F0000001820ull)
typedef union
{
    uint32_t u32;
    struct
    {
        uint64_t lbasez                  : 28;      /**< Base Address[31:4] (Read as Zero) */
        uint64_t pf                      : 1;       /**< Prefetchable Space */
        uint64_t typ                     : 2;       /**< Type (00=32b/01=below 1MB/10=64b/11=RSV) */
        uint64_t mspc                    : 1;       /**< Memory Space Indicator */
    } s;
} octeon_pci_cfg08_t;

#define OCTEON_NPI_PCI_CFG09    (0x80011F0000001824ull)
typedef union
{
    uint32_t u32;
    struct
    {
        uint64_t hbase                   : 25;      /**< Base Address[63:39] */
        uint64_t hbasez                  : 7;       /**< Base Address[38:31] (Read as Zero) */
    } s;
} octeon_pci_cfg09_t;

#define OCTEON_NPI_PCI_CFG10    (0x80011F0000001828ull)
typedef union
{
    uint32_t u32;
    struct
    {
        uint64_t cisp                    : 32;      /**< CardBus CIS Pointer (UNUSED) */
    } s;
} octeon_pci_cfg10_t;

#define OCTEON_NPI_PCI_CFG11    (0x80011F000000182Cull)
typedef union
{
    uint32_t u32;
    struct
    {
        uint64_t ssid                    : 16;      /**< SubSystem ID */
        uint64_t ssvid                   : 16;      /**< Subsystem Vendor ID */
    } s;
} octeon_pci_cfg11_t;

#define OCTEON_NPI_PCI_CFG12    (0x80011F0000001830ull)
typedef union
{
    uint32_t u32;
    struct
    {
        uint64_t erbar                   : 16;      /**< Expansion ROM Base Address[31:16] 64KB in size */
        uint64_t erbarz                  : 5;       /**< Expansion ROM Base Base Address (Read as Zero) */
        uint64_t reserved                : 10;      /**< Reserved */
        uint64_t erbar_en                : 1;       /**< Expansion ROM Address Decode Enable */
    } s;
} octeon_pci_cfg12_t;

#define OCTEON_NPI_PCI_CFG13    (0x80011F0000001834ull)
typedef union
{
    uint32_t u32;
    struct
    {
        uint64_t reserved                : 24;      /**< Reserved */
        uint64_t cp                      : 8;       /**< Capabilities Pointer */
    } s;
} octeon_pci_cfg13_t;

#define OCTEON_NPI_PCI_CFG15    (0x80011F000000183Cull)
typedef union
{
    uint32_t u32;
    struct
    {
        uint64_t ml                      : 8;       /**< Maximum Latency */
        uint64_t mg                      : 8;       /**< Minimum Grant */
        uint64_t inta                    : 8;       /**< Interrupt Pin (INTA#) */
        uint64_t il                      : 8;       /**< Interrupt Line */
    } s;
} octeon_pci_cfg15_t;

#define OCTEON_NPI_PCI_CFG16    (0x80011F0000001840ull)
typedef union
{
    uint32_t u32;
    struct
    {
        uint64_t trdnpr                  : 1;       /**< Target Read Delayed Transaction for I/O and
                                                         non-prefetchable regions discarded. */
        uint64_t trdard                  : 1;       /**< Target Read Delayed Transaction for all regions
                                                         discarded. */
        uint64_t rdsati                  : 1;       /**< Target(I/O and Memory) Read Delayed/Split at
                                                         timeout/immediately (default timeout). */
        uint64_t trdrs                   : 1;       /**< Target(I/O and Memory) Read Delayed/Split or Retry
                                                         select (of the application interface is not ready)
                                                         0 = Delayed Split Transaction
                                                         1 = Retry Transaction (always Immediate Retry, no
                                                         AT_REQ to application). */
        uint64_t trtae                   : 1;       /**< Target(I/O and Memory) Read Target Abort Enable
                                                         (if application interface is not ready at the
                                                         latency timeout).
                                                         Note: N3 as target will never target-abort,
                                                         therefore this bit should never be set. */
        uint64_t twsei                   : 1;       /**< Target(I/O) Write Split Enable (at timeout /
                                                         iately; default timeout) */
        uint64_t twsen                   : 1;       /**< T(I/O) write split Enable (if the application
                                                         interface is not ready) */
        uint64_t twtae                   : 1;       /**< Target(I/O and Memory) Write Target Abort Enable
                                                         (if the application interface is not ready at the
                                                         start of the cycle).
                                                         Note: N3 as target will never target-abort,
                                                         therefore this bit should never be set. */
        uint64_t tmae                    : 1;       /**< Target(Read/Write) Master Abort Enable; check
                                                         at the start of each transaction.
                                                         Note: This bit can be used to force a Master
                                                         Abort when N3 is acting as the intended target
                                                         device. */
        uint64_t tslte                   : 3;       /**< Target Subsequent(2nd-last) Latency Timeout Enable
                                                         Valid range: [1..7] and 0=8. */
        uint64_t tilt                    : 4;       /**< Target Initial(1st data) Latency Timeout in PCI
                                                         ModeValid range: [8..15] and 0=16. */
        uint64_t pbe                     : 12;      /**< Programmable Boundary Enable to disconnect/prefetch
                                                         for target burst read cycles to prefetchable
                                                         region in PCI. A value of 1 indicates end of
                                                         boundary (64 KB down to 16 Bytes). */
        uint64_t dppmr                   : 1;       /**< Disconnect/Prefetch to prefetchable memory
                                                         regions Enable. Prefetchable memory regions
                                                         are always disconnected on a region boundary.
                                                         Non-prefetchable regions for PCI are always
                                                         disconnected on the first transfer.
                                                         Note: N3 as target will never target-disconnect,
                                                         therefore this bit should never be set. */
        uint64_t reserved                : 1;       /**< Reserved */
        uint64_t tswc                    : 1;       /**< Target Split Write Control
                                                         0 = Blocks all requests except PMW
                                                         1 = Blocks all requests including PMW until
                                                         split completion occurs. */
        uint64_t mltd                    : 1;       /**< Master Latency Timer Disable */
    } s;
} octeon_pci_cfg16_t;

#define OCTEON_NPI_PCI_CFG17    (0x80011F0000001844ull)
typedef union
{
    uint32_t u32;
    struct
    {
        uint64_t tscme                   : 32;      /**< Target Split Completion Message Enable */
    } s;
} octeon_pci_cfg17_t;

#define OCTEON_NPI_PCI_CFG18    (0x80011F0000001848ull)
typedef union
{
    uint32_t u32;
    struct
    {
        uint64_t tdsrps                  : 32;      /**< Target Delayed/Split Request Pending Sequences */
    } s;
} octeon_pci_cfg18_t;

#define OCTEON_NPI_PCI_CFG19    (0x80011F000000184Cull)
typedef union
{
    uint32_t u32;
    struct
    {
        uint64_t mrbcm                   : 1;       /**< Master Request (Memory Read) Byte Count/Byte
                                                         Enable select.
                                                         0 = Byte Enables valid. In PCI mode, a burst
                                                         transaction cannot be performed using
                                                         Memory Read command=4?h6.
                                                         1 = DWORD Byte Count valid (default). In PCI
                                                         Mode, the memory read byte enables are
                                                         automatically generated by the core.
                                                         Note: N3 Master Request transaction sizes are
                                                         always determined through the
                                                         am_attr[<35:32>|<7:0>] field. */
        uint64_t mrbci                   : 1;       /**< Master Request (I/O and CR cycles) byte count/byte
                                                         enable select.
                                                         0 = Byte Enables valid (default)
                                                         1 = DWORD byte count valid
                                                         Note: For N3K, This bit must always be zero
                                                         for proper operation (in support of
                                                         Type0/1 Cfg Space accesses which require byte
                                                         enable generation directly from a read mask). */
        uint64_t mdwe                    : 1;       /**< Master (Retry) Deferred Write Enable (allow
                                                         read requests to pass).
                                                         NOTE: Applicable to PCI Mode I/O and memory
                                                         transactions only.
                                                         0 = New read requests are NOT accepted until
                                                         the current write cycle completes. [Reads
                                                         cannot pass writes]
                                                         1 = New read requests are accepted, even when
                                                         there is a write cycle pending [Reads can
                                                         pass writes]. */
        uint64_t mdre                    : 1;       /**< Master (Retry) Deferred Read Enable (Allows
                                                         read/write requests to pass).
                                                         NOTE: Applicable to PCI mode I/O and memory
                                                         transactions only.
                                                         0 = New read/write requests are NOT accepted
                                                         until the current read cycle completes.
                                                         [Read/write requests CANNOT pass reads]
                                                         1 = New read/write requests are accepted, even
                                                         when there is a read cycle pending.
                                                         [Read/write requests CAN pass reads] */
        uint64_t mdrimc                  : 1;       /**< Master I/O Deferred/Split Request Outstanding
                                                         Maximum Count
                                                         0 = 4Ch[26:24]
                                                         1 = 1 */
        uint64_t mdrrmc                  : 3;       /**< Master Deferred Read Request Outstanding Max
                                                         Count (PCI only).
                                                         CR4C[26:24]  Max SAC cycles   MAX DAC cycles
                                                         000              8                4
                                                         001              1                0
                                                         010              2                1
                                                         011              3                1
                                                         100              4                2
                                                         101              5                2
                                                         110              6                3
                                                         111              7                3
                                                         For example, if these bits are programmed to
                                                         100, the core can support 2 DAC cycles, 4 SAC
                                                         cycles or a combination of 1 DAC and 2 SAC cycles.
                                                         NOTE: For the PCI-X maximum outstanding split
                                                         transactions, refer to CRE0[22:20] */
        uint64_t tmes                    : 8;       /**< Target/Master Error Sequence # */
        uint64_t teci                    : 1;       /**< Target Error Command Indication
                                                         0 = Delayed/Split
                                                         1 = Others */
        uint64_t tmei                    : 1;       /**< Target/Master Error Indication
                                                         0 = Target
                                                         1 = Master */
        uint64_t tmse                    : 1;       /**< Target/Master System Error. This bit is set
                                                         whenever ATM_SERR_O is active. */
        uint64_t tmdpes                  : 1;       /**< Target/Master Data PERR# error status. This
                                                         bit is set whenever ATM_DATA_PERR_O is active. */
        uint64_t tmapes                  : 1;       /**< Target/Master Address PERR# error status. This
                                                         bit is set whenever ATM_ADDR_PERR_O is active. */
        uint64_t reserved                : 2;       /**< Reserved */
        uint64_t tibcd                   : 1;       /**< Target Illegal I/O DWORD byte combinations detected. */
        uint64_t tibde                   : 1;       /**< Target Illegal I/O DWORD byte detection enable */
        uint64_t reserved1               : 1;       /**< Reserved */
        uint64_t tidomc                  : 1;       /**< Target I/O Delayed/Split request outstanding
                                                         maximum count.
                                                         0 = 4Ch[4:0],
                                                         1 = 1 */
        uint64_t tdomc                   : 5;       /**< Target Delayed/Split request outstanding maximum
                                                         count. [1..31] and 0=32.
                                                         NOTE: If the user programs these bits beyond the
                                                         Designed Maximum outstanding count, then the
                                                         designed maximum table depth will be used instead.
                                                         No additional Deferred/Split transactions will be
                                                         accepted if this outstanding maximum count
                                                         is reached. Furthermore, no additional
                                                         deferred/split transactions will be accepted if
                                                         the I/O delay/ I/O Split Request outstanding
                                                         maximum is reached. */
    } s;
} octeon_pci_cfg19_t;

#define OCTEON_NPI_PCI_CFG20    (0x80011F0000001850ull)
typedef union
{
    uint32_t u32;
    struct
    {
        uint64_t mdsp                    : 32;      /**< Master Deferred/Split sequence Pending */
    } s;
} octeon_pci_cfg20_t;

#define OCTEON_NPI_PCI_CFG21    (0x80011F0000001854ull)
typedef union
{
    uint32_t u32;
    struct
    {
        uint64_t scmre                   : 32;      /**< Master Split Completion message received with
                                                         error message. */
    } s;
} octeon_pci_cfg21_t;

#define OCTEON_NPI_PCI_CFG22    (0x80011F0000001858ull)
typedef union
{
    uint32_t u32;
    struct
    {
        uint64_t mac                     : 7;       /**< Master Arbiter Control
                                                         [31:26]: Used only in Fixed Priority mode
                                                         (when [25]=1)
                                                         [31:30]: MSI Request
                                                         00 = Highest Priority
                                                         01 = Medium Priority
                                                         10 = Lowest Priority
                                                         11 = RESERVED
                                                         [29:28]: Target Split Completion
                                                         00 = Highest Priority
                                                         01 = Medium Priority
                                                         10 = Lowest Priority
                                                         11 = RESERVED
                                                         [27:26]: New Request; Deferred Read,Deferred Write
                                                         00 = Highest Priority
                                                         01 = Medium Priority
                                                         10 = Lowest Priority
                                                         11 = RESERVED
                                                         [25]: Fixed/Round Robin Priority Selector
                                                         0 = Round Robin
                                                         1 = Fixed */
        uint64_t reserved                : 6;       /**< Reserved */
        uint64_t flush                   : 1;       /**< AM_DO_FLUSH_I control
                                                         NOTE: This bit MUST BE ONE for proper N3K operation */
        uint64_t mra                     : 1;       /**< Master Retry Aborted */
        uint64_t mtta                    : 1;       /**< Master TRDY timeout aborted */
        uint64_t mrv                     : 8;       /**< Master Retry Value [1..255] and 0=infinite */
        uint64_t mttv                    : 8;       /**< Master TRDY timeout value [1..255] and 0=disabled
                                                         Note: N3 does not support master TRDY timeout
                                                         (target is expected to be well behaved). */
    } s;
} octeon_pci_cfg22_t;

#define OCTEON_NPI_PCI_CFG56    (0x80011F00000018E0ull)
typedef union
{
    uint32_t u32;
    struct
    {
        uint64_t reserved                : 9;       /**< RESERVED */
        uint64_t most                    : 3;       /**< Maximum outstanding Split transactions
                                                         [000b=1..111b=32] */
        uint64_t mmbc                    : 2;       /**< Maximum Memory Byte Count
                                                         [0=512B,1=1024B,2=2048B,3=4096B] */
        uint64_t roe                     : 1;       /**< Relaxed Ordering Enable */
        uint64_t dpere                   : 1;       /**< Data Parity Error Recovery Enable */
        uint64_t ncp                     : 8;       /**< Next Capability Pointer */
        uint64_t pxcid                   : 8;       /**< PCI-X Capability ID */
    } s;
} octeon_pci_cfg56_t;

#define OCTEON_NPI_PCI_CFG57    (0x80011F00000018E4ull)
typedef union
{
    uint32_t u32;
    struct
    {
        uint64_t reserved                : 2;       /**< RESERVED */
        uint64_t scemr                   : 1;       /**< Split Completion Error Message Received */
        uint64_t mcrsd                   : 3;       /**< Maximum Cumulative Read Size designed */
        uint64_t mostd                   : 3;       /**< Maximum Outstanding Split transaction designed */
        uint64_t mmrbcd                  : 2;       /**< Maximum Memory Read byte count designed */
        uint64_t dc                      : 1;       /**< Device Complexity
                                                         0 = Simple Device
                                                         1 = Bridge Device */
        uint64_t usc                     : 1;       /**< Unexpected Split Completion */
        uint64_t scd                     : 1;       /**< Split Completion Discarded */
        uint64_t m133                    : 1;       /**< 133MHz Capable */
        uint64_t w64                     : 1;       /**< Indicates a 32b(=0) or 64b(=1) device */
        uint64_t bn                      : 8;       /**< Bus Number. Updated on all configuration write
                                                         (0x11=PCI)             cycles. It?s value is dependent upon the PCI/X
                                                         (0xFF=PCIX)            mode. */
        uint64_t dn                      : 5;       /**< Device Number. Updated on all configuration
                                                         write cycles. */
        uint64_t fn                      : 3;       /**< Function Number */
    } s;
} octeon_pci_cfg57_t;

#define OCTEON_NPI_PCI_CFG58    (0x80011F00000018E8ull)
typedef union
{
    uint32_t u32;
    struct
    {
        uint64_t pmes                    : 5;       /**< PME Support (D0 to D3cold) */
        uint64_t d2s                     : 1;       /**< D2_Support */
        uint64_t d1s                     : 1;       /**< D1_Support */
        uint64_t auxc                    : 3;       /**< AUX_Current (0..375mA) */
        uint64_t dsi                     : 1;       /**< Device Specific Initialization */
        uint64_t reserved                : 1;       /**< Reserved */
        uint64_t pmec                    : 1;       /**< PME Clock */
        uint64_t pcimiv                  : 3;       /**< Indicates the version of the PCI
                                                         Management
                                                         Interface Specification with which the core
                                                         complies.
                                                         010b = Complies with PCI Management Interface
                                                         Specification Revision 1.1 */
        uint64_t ncp                     : 8;       /**< Next Capability Pointer */
        uint64_t pmcid                   : 8;       /**< Power Management Capability ID */
    } s;
} octeon_pci_cfg58_t;

#define OCTEON_NPI_PCI_CFG59    (0x80011F00000018ECull)
typedef union
{
    uint32_t u32;
    struct
    {
        uint64_t pmdia                   : 8;       /**< Power Management data input from application
                                                         (PME_DATA) */
        uint64_t bpccen                  : 1;       /**< BPCC_En (bus power/clock control) enable */
        uint64_t bd3h                    : 1;       /**< B2_B3#, B2/B3 Support for D3hot */
        uint64_t reserved                : 6;       /**< Reserved */
        uint64_t pmess                   : 1;       /**< PME_Status sticky bit */
        uint64_t pmedsia                 : 2;       /**< PME_Data_Scale input from application
                                                         Device                  (PME_DATA_SCALE[1:0])
                                                         Specific */
        uint64_t pmds                    : 4;       /**< Power Management Data_select */
        uint64_t pmeens                  : 1;       /**< PME_En sticky bit */
        uint64_t reserved1               : 6;       /**< RESERVED */
        uint64_t ps                      : 2;       /**< Power State (D0 to D3)
                                                         The N2 DOES NOT support D1/D2 Power Management
                                                         states, therefore writing to this register has
                                                         no effect (please refer to the PCI Power
                                                         Management
                                                         Specification v1.1 for further details about
                                                         it?s R/W nature. This is not a conventional
                                                         R/W style register. */
    } s;
} octeon_pci_cfg59_t;

#define OCTEON_NPI_PCI_CFG60    (0x80011F00000018F0ull)
typedef union
{
    uint32_t u32;
    struct
    {
        uint64_t reserved                : 8;       /**< RESERVED */
        uint64_t m64                     : 1;       /**< 32/64 b message */
        uint64_t mme                     : 3;       /**< Multiple Message Enable(1,2,4,8,16,32) */
        uint64_t mmc                     : 3;       /**< Multiple Message Capable(0=1,1=2,2=4,3=8,4=16,5=32) */
        uint64_t msien                   : 1;       /**< MSI Enable */
        uint64_t ncp                     : 8;       /**< Next Capability Pointer */
        uint64_t msicid                  : 8;       /**< MSI Capability ID */
    } s;
} octeon_pci_cfg60_t;

#define OCTEON_NPI_PCI_CFG61    (0x80011F00000018F4ull)
typedef union
{
    uint32_t u32;
    struct
    {
        uint64_t msi31t2                 : 30;      /**< App Specific MSI Address [31:2] */
        uint64_t reserved                : 2;       /**< RESERVED */
    } s;
} octeon_pci_cfg61_t;

#define OCTEON_NPI_PCI_CFG62    (0x80011F00000018F8ull)
typedef union
{
    uint32_t u32;
    struct
    {
        uint64_t msi                     : 32;      /**< MSI Address [63:32] */
    } s;
} octeon_pci_cfg62_t;

#define OCTEON_NPI_PCI_CFG63    (0x80011F00000018FCull)
typedef union
{
    uint32_t u32;
    struct
    {
        uint64_t reserved                : 16;      /**< RESERVED */
        uint64_t msimd                   : 16;      /**< MSI Message Data */
    } s;
} octeon_pci_cfg63_t;

#define OCTEON_NPI_CTL_STATUS   (0x80011F0000000010ull)
typedef union
{
    uint64_t u64;
    struct
    {
        uint64_t reserved                : 1;       /**< Reserved */
        uint64_t chip_rev                : 8;       /**< The revision of the N3.
                                                         register will be disabled. */
        uint64_t dis_pniw                : 1;       /**< When asserted '1' access from the PNI Window
                                                         Registers are disabled. */
        uint64_t out3_enb                : 1;       /**< When asserted '1' the output3 engine is enabled.
                                                         After enabling the values of the associated
                                                         Address and Size Register should not be changed. */
        uint64_t out2_enb                : 1;       /**< When asserted '1' the output2 engine is enabled.
                                                         After enabling the values of the associated
                                                         Address and Size Register should not be changed. */
        uint64_t out1_enb                : 1;       /**< When asserted '1' the output1 engine is enabled.
                                                         After enabling the values of the associated
                                                         Address and Size Register should not be changed. */
        uint64_t out0_enb                : 1;       /**< When asserted '1' the output0 engine is enabled.
                                                         After enabling the values of the associated
                                                         Address and Size Register should not be changed. */
        uint64_t ins3_enb                : 1;       /**< When asserted '1' the gather3 engine is enabled.
                                                         After enabling the values of the associated
                                                         Address and Size Register should not be changed.
                                                         write operation. */
        uint64_t ins2_enb                : 1;       /**< When asserted '1' the gather2 engine is enabled.
                                                         After enabling the values of the associated
                                                         Address and Size Register should not be changed. */
        uint64_t ins1_enb                : 1;       /**< When asserted '1' the gather1 engine is enabled.
                                                         After enabling the values of the associated
                                                         Address and Size Register should not be changed. */
        uint64_t ins0_enb                : 1;       /**< When asserted '1' the gather0 engine is enabled.
                                                         After enabling the values of the associated
                                                         Address and Size Register should not be changed. */
        uint64_t ins3_64b                : 1;       /**< When asserted '1' the instructions read by the
                                                         gather3 engine are 64-Byte instructions, when
                                                         de-asserted '0' instructions are 32-byte. */
        uint64_t ins2_64b                : 1;       /**< When asserted '1' the instructions read by the
                                                         gather2 engine are 64-Byte instructions, when
                                                         de-asserted '0' instructions are 32-byte. */
        uint64_t ins1_64b                : 1;       /**< When asserted '1' the instructions read by the
                                                         gather1 engine are 64-Byte instructions, when
                                                         de-asserted '0' instructions are 32-byte. */
        uint64_t ins0_64b                : 1;       /**< When asserted '1' the instructions read by the
                                                         gather0 engine are 64-Byte instructions, when
                                                         de-asserted '0' instructions are 32-byte. */
        uint64_t pci_wdis                : 1;       /**< When set '1' disables access to registers in
                                                         PNI address range 0x1000 - 0x17FF from the PCI. */
        uint64_t wait_com                : 1;       /**< When set '1' casues the NPI to wait for a commit
                                                         from the L2C before sending additional access to
                                                         the L2C from the PCI. */
        uint64_t spares1                 : 3;       /**< These bits are reserved and should be set to 0. */
        uint64_t max_word                : 5;       /**< The maximum number of words to merge into a single
                                                         write operation from the PPs to the PCI. Legal
                                                         values are 1 to 32, where a '0' is treated as 32. */
        uint64_t spares0                 : 22;      /**< These bits are reserved and should be set to 0. */
        uint64_t timer                   : 10;      /**< When the NPI starts a PP to PCI write it will wait
                                                         no longer than the value of TIMER in eclks to
                                                         merge additional writes from the PPs into 1
                                                         large write. The values for this field is 1 to
                                                         1024 where a value of '0' is treated as 1024. */
    } s;
} octeon_npi_ctl_status_t;

#define OCTEON_NPI_PCI_CTL_STATUS_2 (0x80011F000000118Cull)
typedef union
{
    uint32_t u32;
    struct
    {
        uint64_t reserved                : 3;       /**< Reserved */
        uint64_t bb1_hole                : 3;       /**< Big BAR 1 Hole
                                                         When PCI_CTL_STATUS_2[BB1]=1, this field defines
                                                         an encoded size of the upper BAR1 region which
                                                         OCTEON will mask out (ie: not respond to).
                                                         (see definition of BB1_HOLE and BB1_SIZ encodings
                                                         in the PCI_CTL_STATUS_2[BB1] definition below). */
        uint64_t bb1_siz                 : 1;       /**< Big BAR 1 Size
                                                         When PCI_CTL_STATUS_2[BB1]=1, this field defines
                                                         the programmable SIZE of BAR 1.
                                                         0: 1GB / 1: 2GB */
        uint64_t bb_ca                   : 1;       /**< Set to '1' for Big Bar Mode to do STT/LDT L2C
                                                         operations. */
        uint64_t bb_es                   : 2;       /**< Big Bar Node Endian Swap Mode
                                                         0: No Swizzle
                                                         1: Byte Swizzle (per-QW)
                                                         2: Byte Swizzle (per-LW)
                                                         3: LongWord Swizzle */
        uint64_t bb1                     : 1;       /**< Big Bar 1 Enable
                                                         When PCI_CTL_STATUS_2[BB1] is set, the following differences
                                                         occur:
                                                         - OCTEON's BAR1 becomes somewhere in the range 512-2048 MB rather
                                                         than the default 128MB.
                                                         - The following table indicates the effective size of
                                                         BAR1 when BB1 is set:
                                                         BB1_SIZ   BB1_HOLE  Effective size    Comment */
        uint64_t bb0                     : 1;       /**< Big Bar 0 Enable
                                                         When PCI_CTL_STATUS_2[BB0] is set, the following
                                                         differences occur:
                                                         - OCTEON's BAR0 becomes 2GB rather than the default 4KB.
                                                         PCI_CFG04[LBASE<18:0>] reads as zero and is ignored on write.
                                                         - OCTEON's BAR0 becomes burstable. (When BB0 is clear, OCTEON
                                                         single-phase disconnects PCI BAR0 reads and PCI/PCI-X BAR0
                                                         writes, and splits (burstably) PCI-X BAR0 reads.)
                                                         - The first 4KB, i.e. addresses on the PCI bus in the range
                                                         BAR0+0      .. BAR0+0xFFF
                                                         access OCTEON's PCI-type CSR's as when BB0 is clear.
                                                         - The remaining address space, i.e. addresses on the PCI bus
                                                         in the range
                                                         BAR0+0x1000 .. BAR0+0x7FFFFFFF
                                                         are mapped to OCTEON physical DRAM addresses as follows:
                                                         PCI Address Range                  OCTEON Physical Address Range
                                                         ------------------------------------+------------------------------
                                                         BAR0+0x00001000 .. BAR0+0x0FFFFFFF | 0x000001000 .. 0x00FFFFFFF
                                                         BAR0+0x10000000 .. BAR0+0x1FFFFFFF | 0x410000000 .. 0x41FFFFFFF
                                                         BAR0+0x20000000 .. BAR0+0x7FFFFFFF | 0x020000000 .. 0x07FFFFFFF
                                                         and PCI_CTL_STATUS_2[BB_ES] is the endian-swap and
                                                         PCI_CTL_STATUS_2[BB_CA] is the L2 cache allocation bit
                                                         for these references.
                                                         The consequences of any burst that crosses the end of the PCI
                                                         Address Range for BAR0 are unpredicable.
                                                         - The consequences of any burst access that crosses the boundary
                                                         between BAR0+0xFFF and BAR0+0x1000 are unpredictable in PCI-X
                                                         mode. OCTEON may disconnect PCI references at this boundary.
                                                         - The results of any burst read that crosses the boundary
                                                         between BAR0+0x0FFFFFFF and BAR0+0x10000000 are unpredictable.
                                                         The consequences of any burst write that crosses this same
                                                         boundary are unpredictable.
                                                         - The results of any burst read that crosses the boundary
                                                         between BAR0+0x1FFFFFFF and BAR0+0x20000000 are unpredictable.
                                                         The consequences of any burst write that crosses this same
                                                         boundary are unpredictable. */
        uint64_t erst_n                  : 1;       /**< Reset active Low. */
        uint64_t bar2pres                : 1;       /**< From fuse block. When fuse(MIO_FUS_DAT3[BAR2_EN])
                                                         is NOT blown the value of this field is '0' after
                                                         reset and BAR2 is NOT present. When the fuse IS
                                                         blown the value of this field is '1' after reset
                                                         and BAR2 is present. Note that SW can change this
                                                         field after reset. */
        uint64_t scmtyp                  : 1;       /**< Split Completion Message CMD Type (0=RD/1=WR)
                                                         When SCM=1, SCMTYP specifies the CMD intent (R/W) */
        uint64_t scm                     : 1;       /**< Split Completion Message Detected (Read or Write) */
        uint64_t en_wfilt                : 1;       /**< When '1' the window-access filter is enabled.
                                                         Unfilter writes are:
                                                         MIO,  SubId0
                                                         MIO,  SubId7
                                                         NPI,  SubId0
                                                         NPI,  SubId7
                                                         POW,  SubId7
                                                         IPD,  SubId7
                                                         USBN, SubId7
                                                         Unfiltered Reads are:
                                                         MIO,  SubId0
                                                         MIO,  SubId7
                                                         NPI,  SubId0
                                                         NPI,  SubId7
                                                         POW,  SubId1
                                                         POW,  SubId2
                                                         POW,  SubId3
                                                         POW,  SubId7
                                                         IPD,  SubId7
                                                         USBN, SubId7 */
        uint64_t spare                   : 1;       /**< Spare Bit */
        uint64_t ap_pcix                 : 1;       /**< PCX Core Mode status (0=PCI Bus/1=PCIX) */
        uint64_t ap_64ad                 : 1;       /**< PCX Core Bus status (0=32b Bus/1=64b Bus) */
        uint64_t b12_bist                : 1;       /**< Bist Status For Memeory In B12 */
        uint64_t pmo_amod                : 1;       /**< PMO-ARB Mode (0=FP[HP=CMD1,LP=CMD0]/1=RR) */
        uint64_t pmo_fpc                 : 3;       /**< PMO-ARB Fixed Priority Counter
                                                         When PMO_AMOD=0 (FP mode), this field represents
                                                         the # of CMD1 requests that are issued (at higher
                                                         priority) before a single lower priority CMD0
                                                         is allowed to issue (to ensure foward progress).
                                                         0: 1 CMD1 Request issued before CMD0 allowed
                                                         ...
                                                         7: 8 CMD1 Requests issued before CMD0 allowed */
        uint64_t tsr_hwm                 : 3;       /**< Target Split-Read ADB(allowable disconnect boundary)
                                                         High Water Mark.
                                                         Specifies the number of ADBs(128 Byte aligned chunks)
                                                         that are accumulated(pending) BEFORE the Target Split
                                                         completion is attempted on the PCI bus.
                                                         0: RESERVED/ILLEGAL
                                                         1: 2 Pending ADBs (129B-256B)
                                                         2: 3 Pending ADBs (257B-384B)
                                                         3: 4 Pending ADBs (385B-512B)
                                                         4: 5 Pending ADBs (513B-640B)
                                                         5: 6 Pending ADBs (641B-768B)
                                                         6: 7 Pending ADBs (769B-896B)
                                                         7: 8 Pending ADBs (897B-1024B)
                                                         Example: Suppose a 1KB target memory request with
                                                         starting byte offset address[6:0]=0x7F is split by
                                                         the OCTEON and the TSR_HWM=1(2 ADBs).
                                                         The OCTEON will start the target split completion
                                                         on the PCI Bus after 1B(1st ADB)+128B(2nd ADB)=129B
                                                         of data have been received from memory (even though
                                                         the remaining 895B has not yet been received). The
                                                         OCTEON will continue the split completion until it
                                                         has consumed all of the pended split data. If the
                                                         full transaction length(1KB) of data was NOT entirely
                                                         transferred, then OCTEON will terminate the split
                                                         completion and again wait for another 2 ADB-aligned data
                                                         chunks(256B) of pended split data to be received from
                                                         memory before starting another split completion request.
                                                         This allows Octeon (as split completer), to send back
                                                         multiple split completions for a given large split
                                                         transaction without having to wait for the entire
                                                         transaction length to be received from memory.
                                                         NOTE: For split transaction sizes 'smaller' than the
                                                         specified TSR_HWM value, the split completion
                                                         is started when the last datum has been received from
                                                         memory.
                                                         NOTE: It is IMPERATIVE that this field NEVER BE
                                                         written to a ZERO value. A value of zero is
                                                         reserved/illegal and can result in PCIX bus hangs). */
        uint64_t bar2_enb                : 1;       /**< When set '1' BAR2 is enable and will respond when
                                                         clear '0' BAR2 access will be target-aborted. */
        uint64_t bar2_esx                : 2;       /**< Value will be XORed with pci-address[37:36] to
                                                         determine the endian swap mode. */
        uint64_t bar2_cax                : 1;       /**< Value will be XORed with pci-address[38] to
                                                         determine the L2 cache attribute. */
    } s;
} octeon_pci_ctl_status_2_t;

#define OCTEON_NPI_PCI_BAR1_INDEXX(offset) (0x80011F0000001100ull+((offset)*4))
typedef union
{
    uint32_t u32;
    struct
    {
        uint64_t reserved                : 14;      /**< Reserved */
        uint64_t addr_idx                : 14;      /**< Address bits [35:22] sent to L2C */
        uint64_t ca                      : 1;       /**< Set '1' when access is to be cached in L2. */
        uint64_t end_swp                 : 2;       /**< Endian Swap Mode */
        uint64_t addr_v                  : 1;       /**< Set '1' when the selected address range is valid. */
    } s;
} octeon_pci_bar1_indexx_t;

#define OCTEON_NPI_PCI_READ_CMD_6   (0x80011F0000001180ull)
#define OCTEON_NPI_PCI_READ_CMD_C   (0x80011F0000001184ull)
#define OCTEON_NPI_PCI_READ_CMD_E   (0x80011F0000001188ull)

#define OCTEON_NPI_PCI_INT_ARB_CFG  (0x80011F0000000130ull)
typedef union
{
    uint64_t u64;
    struct
    {
        uint64_t reserved                : 59;      /**< Reserved */
        uint64_t en                      : 1;       /**< Internal arbiter enable. */
        uint64_t park_mod                : 1;       /**< Bus park mode. 0=park on last, 1=park on device. */
        uint64_t park_dev                : 3;       /**< Bus park device. 0-3 External device, 4 = Octane. */
    } s;
} octeon_npi_pci_int_arb_cfg_t;

#define OCTEON_MIO_BOOT_REG_CFGX(offset) (0x8001180000000000ull+((offset)*8))
typedef union
{
    uint64_t u64;
    struct
    {
        uint64_t reserved                : 27;      /**< Reserved */
        uint64_t sam                     : 1;       /**< Region 0 SAM */
        uint64_t we_ext                  : 2;       /**< Region 0 write enable count extension */
        uint64_t oe_ext                  : 2;       /**< Region 0 output enable count extension */
        uint64_t en                      : 1;       /**< Region 0 enable */
        uint64_t orbit                   : 1;       /**< No function for region 0 */
        uint64_t ale                     : 1;       /**< Region 0 ALE mode */
        uint64_t width                   : 1;       /**< Region 0 bus width */
        uint64_t size                    : 12;      /**< Region 0 size */
        uint64_t base                    : 16;      /**< Region 0 base address */
    } s;
} octeon_mio_boot_reg_cfgx_t;

#define OCTEON_L2T_ERR (0x8001180080000008ull)
typedef union
{
    uint64_t u64;
    struct
    {
        uint64_t reserved_28_63          : 36;
        uint64_t lck_intena2             : 1;
        uint64_t lckerr2                 : 1;
        uint64_t lck_intena              : 1;
        uint64_t lckerr                  : 1;
        uint64_t fset                    : 3;
        uint64_t fadr                    : 10;
        uint64_t fsyn                    : 6;
        uint64_t ded_err                 : 1;
        uint64_t sec_err                 : 1;
        uint64_t ded_intena              : 1;
        uint64_t sec_intena              : 1;
        uint64_t ecc_ena                 : 1;
    } s;
} octeon_l2t_err_t;

#define OCTEON_L2C_DBG (0x8001180080000030ull)
typedef union
{
    uint64_t u64;
    struct
    {
        uint64_t reserved_15_63          : 49;
        uint64_t lfb_enum                : 4;
        uint64_t lfb_dmp                 : 1;
        uint64_t ppnum                   : 4;
        uint64_t set                     : 3;
        uint64_t finv                    : 1;
        uint64_t l2d                     : 1;
        uint64_t l2t                     : 1;
    } s;
} octeon_l2c_dbg_t;

#define OCTEON_L2C_LCKBASE (0x8001180080000058ull)
typedef union
{
    uint64_t u64;
    struct
    {
        uint64_t reserved_31_63          : 33;
        uint64_t lck_base                : 27;
        uint64_t reserved_1_3            : 3;
        uint64_t lck_ena                 : 1;
    } s;
} octeon_l2c_lckbase_t;

#define OCTEON_L2C_LCKOFF (0x8001180080000060ull)
typedef union
{
    uint64_t u64;
    struct
    {
        uint64_t reserved_10_63          : 54;
        uint64_t lck_offset              : 10;
    } s;
} octeon_l2c_lckoff_t;

#define OCTEON_L2C_CFG (0x8001180080000000ull)
typedef union
{
    uint64_t u64;
    struct
    {
        uint64_t reserved_14_63          : 50;
        uint64_t fpexp                   : 4;
        uint64_t fpempty                 : 1;
        uint64_t fpen                    : 1;
        uint64_t idxalias                : 1;
        uint64_t mwf_crd                 : 4;
        uint64_t rsp_arb_mode            : 1;
        uint64_t rfb_arb_mode            : 1;
        uint64_t lrf_arb_mode            : 1;
    } s;
} octeon_l2c_cfg_t;

#define OCTEON_ASXX_INT_EN(block_id)    (0x80011800B0000018ull+((block_id)*0x8000000ull))
#define OCTEON_ASXX_INT_REG(block_id)   (0x80011800B0000010ull+((block_id)*0x8000000ull))
#define OCTEON_CIU_MBOX_SETX(offset)    (0x8001070000000600ull+((offset)*8))
#define OCTEON_CIU_MBOX_CLRX(offset)    (0x8001070000000680ull+((offset)*8))
#define OCTEON_CIU_SOFT_RST             (0x8001070000000740ull)
#define OCTEON_CIU_SOFT_PRST            (0x8001070000000748ull)
#define OCTEON_CIU_SOFT_BIST            (0x8001070000000738ull)
#define OCTEON_LMC_MEM_CFG0             (0x8001180088000000ull)
#define OCTEON_L2T_ERR                  (0x8001180080000008ull)
#define OCTEON_L2D_ERR                  (0x8001180080000010ull)
#define OCTEON_L2D_FUS3                 (0x80011800800007B8ull)
#define OCTEON_POW_ECC_ERR              (0x8001670000000218ull)
#define OCTEON_NPI_RSL_INT_BLOCKS       (0x80011F0000000000ull)
#define OCTEON_NPI_PCI_INT_ENB2         (0x80011F00000011A0ull)
#define OCTEON_NPI_PCI_INT_SUM2         (0x80011F0000001198ull)
#define OCTEON_NPI_INT_ENB              (0x80011F0000000020ull)
#define OCTEON_NPI_INT_SUM              (0x80011F0000000018ull)
#define OCTEON_MIO_UARTX_SRR(offset)    (0x8001180000000A10ull+((offset)*1024))
#define OCTEON_MIO_UARTX_LSR(offset)    (0x8001180000000828ull+((offset)*1024))
#define OCTEON_MIO_UARTX_RBR(offset)    (0x8001180000000800ull+((offset)*1024))
#define OCTEON_MIO_UARTX_IER(offset)    (0x8001180000000808ull+((offset)*1024))
#define OCTEON_MIO_UARTX_THR(offset)    (0x8001180000000840ull+((offset)*1024))
#define OCTEON_MIO_FUS_DAT3             (0x8001180000001418ull)
#define OCTEON_LED_EN                   (0x8001180000001A00ull)
#define OCTEON_LED_CLK_PHASE            (0x8001180000001A08ull)
#define OCTEON_LED_PRT                  (0x8001180000001A10ull)
#define OCTEON_LED_DBG                  (0x8001180000001A18ull)
#define OCTEON_LED_UDD_CNTX(offset)     (0x8001180000001A20ull+((offset)*8))
#define OCTEON_LED_PRT_FMT              (0x8001180000001A30ull)
#define OCTEON_LED_UDD_DATX(offset)     (0x8001180000001A38ull+((offset)*8))
#define OCTEON_LED_BLINK                (0x8001180000001A48ull)
#define OCTEON_LED_POLARITY             (0x8001180000001A50ull)
#define OCTEON_LED_PRT_STATUSX(offset)  (0x8001180000001A80ull+((offset)*8))
#define OCTEON_LED_CYLON                (0x8001180000001AF8ull)
#define OCTEON_LED_UDD_DAT_SETX(offset) (0x8001180000001AC0ull+((offset)*16))
#define OCTEON_LED_UDD_DAT_CLRX(offset) (0x8001180000001AC8ull+((offset)*16))

#define OCTEON_IOB_INT_SUM              (0x80011800F0000058ull)
#define OCTEON_IOB_PKT_ERR              (0x80011800F0000068ull)
#define OCTEON_IPD_INT_SUM              (0x80014F0000000168ull)
#define OCTEON_ZIP_ERROR                (0x8001180038000088ull)
#define OCTEON_PKO_REG_DEBUG0           (0x8001180050000098ull)
#define OCTEON_TIM_REG_ERROR            (0x8001180058000088ull)
#define OCTEON_FPA_INT_SUM              (0x8001180028000040ull)
#define OCTEON_IOB_INT_ENB              (0x80011800F0000060ull)
#define OCTEON_IPD_INT_ENB              (0x80014F0000000160ull)
#define OCTEON_ZIP_INT_MASK             (0x8001180038000090ull)
#define OCTEON_PKO_REG_INT_MASK         (0x8001180050000090ull)
#define OCTEON_TIM_REG_INT_MASK         (0x8001180058000090ull)
#define OCTEON_FPA_INT_ENB              (0x8001180028000048ull)
#define OCTEON_IPD_CLK_COUNT            (0x80014F0000000338ull)
#define OCTEON_GPIO_BIT_CFGX(offset)    (0x8001070000000800ull+((offset)*8))
#define OCTEON_PIP_INT_EN               (0x80011800A0000010ull)
#define OCTEON_PIP_INT_REG              (0x80011800A0000008ull)
#define OCTEON_PKO_REG_ERROR            (0x8001180050000088ull)
#define OCTEON_DFA_ERR                  (0x8001180030000028ull)
#define OCTEON_MIO_BOOT_ERR             (0x80011800000000A0ull)
#define OCTEON_MIO_BOOT_INT             (0x80011800000000A8ull)
#define OCTEON_KEY_INT_ENB              (0x8001180020000008ull)
#define OCTEON_KEY_INT_SUM              (0x8001180020000000ull)

static inline uint64_t octeon_ptr_to_phys(void *ptr)
{
    if (((uint64_t)ptr >> 62) == 3)
        return (uint64_t)ptr & 0x3ffffffful;
    else
        return (uint64_t)ptr & 0xfffffffffful;
}

static inline void *octeon_phys_to_ptr(uint64_t physical_address)
{
    return (void*)(physical_address | (1ul<<63));
}

static inline void octeon_write_csr(uint64_t csr_addr, uint64_t val)
{
    *(volatile uint64_t *)csr_addr = val;
}

static inline uint64_t octeon_read_csr(uint64_t csr_addr)
{
    return *(volatile uint64_t *)csr_addr;
}


/**
 * Write a 32bit value to the Octeon NPI register space
 *
 * @param address Address to write to
 * @param val     Value to write
 */
static inline void octeon_npi_write32(uint64_t address, uint32_t val)
{
    volatile uint32_t *ptr = (volatile uint32_t *)(address ^ 4);
    *ptr = val;
}


/**
 * Read a 32bit value from the Octeon NPI register space
 *
 * @param address Address to read
 * @return The result
 */
static inline uint32_t octeon_npi_read32(uint64_t address)
{
    volatile uint32_t *ptr = (volatile uint32_t *)(address ^ 4);
    return *ptr;
}


static inline uint64_t octeon_get_core_num(void)
{
    uint64_t core_num;
    asm ("rdhwr %0, $0" : "=r"(core_num));
    return core_num;
}

static inline void octeon_led_write(int bank, uint32_t data)
{
    octeon_write_csr(OCTEON_LED_UDD_DATX(bank), data);
}

static inline uint32_t octeon_led_read(int bank)
{
    return octeon_read_csr(OCTEON_LED_UDD_DATX(bank));
}

static inline void octeon_led_set(int bank, int bit)
{
    if (octeon_is_pass1())
    {
        extern spinlock_t octeon_led_lock;
        spin_lock(&octeon_led_lock);
        octeon_write_csr(OCTEON_LED_UDD_DATX(bank), octeon_read_csr(OCTEON_LED_UDD_DATX(bank)) | (1<<bit));
        spin_unlock(&octeon_led_lock);
    }
    else
    {
        octeon_write_csr(OCTEON_LED_UDD_DAT_SETX(bank), 1<<bit);
    }
}

static inline void octeon_led_clear(int bank, int bit)
{
    if (octeon_is_pass1())
    {
        extern spinlock_t octeon_led_lock;
        spin_lock(&octeon_led_lock);
        octeon_write_csr(OCTEON_LED_UDD_DATX(bank), octeon_read_csr(OCTEON_LED_UDD_DATX(bank)) & ~(1<<bit));
        spin_unlock(&octeon_led_lock);
    }
    else
    {
        octeon_write_csr(OCTEON_LED_UDD_DAT_CLRX(bank), 1<<bit);
    }
}

void octeon_write_lcd(const char *s);
void octeon_check_cpu_bist(void);
void octeon_hal_init(void);
int octeon_get_boot_uart(void);

#endif
