struct pario_handle;

struct pario_port {
    volatile UBYTE *data_port; /* +0 */
    volatile UBYTE *data_ddr;  /* +4 */
    volatile UBYTE *ctrl_port; /* +8 */
    volatile UBYTE *ctrl_ddr;  /* +12 */
    UBYTE busy_bit;            /* +16 */
    UBYTE pout_bit;            /* +17 */
    UBYTE sel_bit;             /* +18 */
    UBYTE dummy1;
    UBYTE busy_mask;           /* +20 */
    UBYTE pout_mask;           /* +21 */
    UBYTE sel_mask;            /* +22 */
    UBYTE dummy2;
};

extern struct pario_handle *pario_init(struct Library *SysBase);
extern void pario_exit(struct pario_handle *ph);

extern int pario_setup_ack_irq(struct pario_handle *ph, struct Task *sigTask, BYTE signal);
extern void pario_cleanup_ack_irq(struct pario_handle *ph);

extern struct pario_port *pario_get_port(struct pario_handle *ph);
