/* Hey EMACS -*- linux-c -*- */
/* $Id$ */

 /*
  * UAE - The Un*x Amiga Emulator
  *
  * MC68000 emulation
  *
  * (c) 1995 Bernd Schmidt
  */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "sysdeps.h"
#include "uconfig.h"
#include "options.h"
#include "memory.h"
#include "newcpu.h"
#include "compiler.h"

// tiemu begin
#include "tilibs.h"
#include "ti68k_def.h"
#include "memory.h"
#include "../ti_hw/dbus.h"
#include "bkpts.h"
#define write_log printf
static const struct uae_prefs currprefs = {0, 1, 1};
// tiemu end

/* Opcode of faulting instruction */
uae_u16 last_op_for_exception_3;
/* PC at fault time */
uaecptr last_addr_for_exception_3;
/* Address that generated the exception */
uaecptr last_fault_for_exception_3;

int areg_byteinc[] = { 1,1,1,1,1,1,1,2 };
int imm8_table[] = { 8,1,2,3,4,5,6,7 };

int movem_index1[256];
int movem_index2[256];
int movem_next[256];

int fpp_movem_index1[256];
int fpp_movem_index2[256];
int fpp_movem_next[256];

cpuop_func *cpufunctbl[65536];

#define COUNT_INSTRS 0

#if COUNT_INSTRS
static unsigned long int instrcount[65536];
static uae_u16 opcodenums[65536];

static int compfn (const void *el1, const void *el2)
{
    return instrcount[*(const uae_u16 *)el1] < instrcount[*(const uae_u16 *)el2];
}

static char *icountfilename (void)
{
    char *name = getenv ("INSNCOUNT");
    if (name)
	return name;
    return COUNT_INSTRS == 2 ? "frequent.68k" : "insncount";
}

void dump_counts (void)
{
    FILE *f = fopen (icountfilename (), "w");
    unsigned long int total;
    int i;

    write_log ("Writing instruction count file...\n");
    for (i = 0; i < 65536; i++) {
	opcodenums[i] = i;
	total += instrcount[i];
    }
    qsort (opcodenums, 65536, sizeof(uae_u16), compfn);

    fprintf (f, "Total: %lu\n", total);
    for (i=0; i < 65536; i++) {
	unsigned long int cnt = instrcount[opcodenums[i]];
	struct instr *dp;
	struct mnemolookup *lookup;
	if (!cnt)
	    break;
	dp = table68k + opcodenums[i];
	for (lookup = lookuptab;lookup->mnemo != dp->mnemo; lookup++)
	    ;
	fprintf (f, "%04x: %lu %s\n", opcodenums[i], cnt, lookup->name);
    }
    fclose (f);
}
#else
void dump_counts (void)
{
}
#endif

int currIntLev = -1;
int broken_in;
int delayTime = 0;

static unsigned long op_illg_1 (uae_u32 opcode) REGPARAM;

static unsigned long REGPARAM2 op_illg_1 (uae_u32 opcode)
{
    op_illg (opcode);
    return 4;
}

static void build_cpufunctbl (void)
{
    int i;
    unsigned long opcode;
    struct cputbl *tbl = (currprefs.cpu_level == 4 ? op_smalltbl_0_ff
			  : currprefs.cpu_level == 3 ? op_smalltbl_1_ff
			  : currprefs.cpu_level == 2 ? op_smalltbl_2_ff
			  : currprefs.cpu_level == 1 ? op_smalltbl_3_ff
			  : ! currprefs.cpu_compatible ? op_smalltbl_4_ff
			  : op_smalltbl_5_ff);

    printf ("UAE: building CPU function table (%d %d %d).\n",
	    currprefs.cpu_level, currprefs.cpu_compatible, currprefs.address_space_24);

    for (opcode = 0; opcode < 65536; opcode++)
	cpufunctbl[opcode] = op_illg_1;
    for (i = 0; tbl[i].handler != NULL; i++) {
	if (! tbl[i].specific)
	    cpufunctbl[tbl[i].opcode] = tbl[i].handler;
    }
    for (opcode = 0; opcode < 65536; opcode++) {
	cpuop_func *f;

	if (table68k[opcode].mnemo == i_ILLG || table68k[opcode].clev > currprefs.cpu_level)
	    continue;

	if (table68k[opcode].handler != -1) {
	    f = cpufunctbl[table68k[opcode].handler];
	    if (f == op_illg_1)
		abort();
	    cpufunctbl[opcode] = f;
	}
    }
    for (i = 0; tbl[i].handler != NULL; i++) {
	if (tbl[i].specific)
	    cpufunctbl[tbl[i].opcode] = tbl[i].handler;
    }
}

#if 0
unsigned long cycles_mask, cycles_val;

static void update_68k_cycles (void)
{
    cycles_mask = 0;
    cycles_val = currprefs.m68k_speed;
    if (currprefs.m68k_speed < 1) {
	cycles_mask = 0xFFFFFFFF;
	cycles_val = 0;
    }
}

void check_prefs_changed_cpu (void)
{
    if (currprefs.cpu_level != changed_prefs.cpu_level
	|| currprefs.cpu_compatible != changed_prefs.cpu_compatible) {
	currprefs.cpu_level = changed_prefs.cpu_level;
	currprefs.cpu_compatible = changed_prefs.cpu_compatible;
	build_cpufunctbl ();
    }
    if (currprefs.m68k_speed != changed_prefs.m68k_speed) {
	currprefs.m68k_speed = changed_prefs.m68k_speed;
	reset_frame_rate_hack ();
	update_68k_cycles ();
    }
}
#endif

void init_m68k (void)
{
    int i;

	printf ("UAE: version 0.8.15\n");
//    update_68k_cycles ();

    for (i = 0 ; i < 256 ; i++) {
	int j;
	for (j = 0 ; j < 8 ; j++) {
		if (i & (1 << j)) break;
	}
	movem_index1[i] = j;
	movem_index2[i] = 7-j;
	movem_next[i] = i & (~(1 << j));
    }
    for (i = 0 ; i < 256 ; i++) {
	int j;
	for (j = 7 ; j >= 0 ; j--) {
		if (i & (1 << j)) break;
	}
	fpp_movem_index1[i] = 7-j;
	fpp_movem_index2[i] = j;
	fpp_movem_next[i] = i & (~(1 << j));
    }
#if COUNT_INSTRS
    {
	FILE *f = fopen (icountfilename (), "r");
	memset (instrcount, 0, sizeof instrcount);
	if (f) {
	    uae_u32 opcode, count, total;
	    char name[20];
	    write_log ("Reading instruction count file...\n");
	    fscanf (f, "Total: %lu\n", &total);
	    while (fscanf (f, "%lx: %lu %s\n", &opcode, &count, name) == 3) {
		instrcount[opcode] = count;
	    }
	    fclose(f);
	}
    }
#endif
    write_log ("UAE: building CPU table for configuration: 68");
    if (currprefs.address_space_24 && currprefs.cpu_level > 1)
        write_log ("EC");
    switch (currprefs.cpu_level) {
    case 1:
        write_log ("010");
        break;
    case 2:
        write_log ("020");
        break;
    case 3:
        write_log ("020/881");
        break;
    case 4:
        /* Who is going to miss the MMU anyway...? :-)  */
        write_log ("040");
        break;
    default:
        write_log ("000");
        break;
    }
    if (currprefs.cpu_compatible)
        write_log (" (compatible mode)");
    write_log ("\n");
    
    read_table68k ();
    do_merges ();

    write_log ("UAE: %d CPU functions\n", nr_cpuop_funcs);

    build_cpufunctbl ();
}

struct regstruct regs, lastint_regs;
static struct regstruct regs_backup[16];
static int backup_pointer = 0;
static long int m68kpc_offset;
int lastint_no;

#define get_ibyte_1(o) get_byte(regs.pc + (regs.pc_p - regs.pc_oldp) + (o) + 1)
#define get_iword_1(o) get_word(regs.pc + (regs.pc_p - regs.pc_oldp) + (o))
#define get_ilong_1(o) get_long(regs.pc + (regs.pc_p - regs.pc_oldp) + (o))

#if 0
uae_s32 ShowEA (FILE *f, int reg, amodes mode, wordsizes size, char *buf)
{
    uae_u16 dp;
    uae_s8 disp8;
    uae_s16 disp16;
    int r;
    uae_u32 dispreg;
    uaecptr addr;
    uae_s32 offset = 0;
    char buffer[80];

    switch (mode){
     case Dreg:
	sprintf (buffer,"D%d", reg);
	break;
     case Areg:
	sprintf (buffer,"A%d", reg);
	break;
     case Aind:
	sprintf (buffer,"(A%d)", reg);
	break;
     case Aipi:
	sprintf (buffer,"(A%d)+", reg);
	break;
     case Apdi:
	sprintf (buffer,"-(A%d)", reg);
	break;
     case Ad16:
	disp16 = get_iword_1 (m68kpc_offset); m68kpc_offset += 2;
	addr = m68k_areg(regs,reg) + (uae_s16)disp16;
	sprintf (buffer,"(A%d,$%04x) == $%08lx", reg, disp16 & 0xffff,
					(unsigned long)addr);
	break;
     case Ad8r:
	dp = get_iword_1 (m68kpc_offset); m68kpc_offset += 2;
	disp8 = dp & 0xFF;
	r = (dp & 0x7000) >> 12;
	dispreg = dp & 0x8000 ? m68k_areg(regs,r) : m68k_dreg(regs,r);
	if (!(dp & 0x800)) dispreg = (uae_s32)(uae_s16)(dispreg);
	dispreg <<= (dp >> 9) & 3;

	if (dp & 0x100) {
	    uae_s32 outer = 0, disp = 0;
	    uae_s32 base = m68k_areg(regs,reg);
	    char name[10];
	    sprintf (name,"A%d, ",reg);
	    if (dp & 0x80) { base = 0; name[0] = 0; }
	    if (dp & 0x40) dispreg = 0;
	    if ((dp & 0x30) == 0x20) { disp = (uae_s32)(uae_s16)get_iword_1 (m68kpc_offset); m68kpc_offset += 2; }
	    if ((dp & 0x30) == 0x30) { disp = get_ilong_1 (m68kpc_offset); m68kpc_offset += 4; }
	    base += disp;

	    if ((dp & 0x3) == 0x2) { outer = (uae_s32)(uae_s16)get_iword_1 (m68kpc_offset); m68kpc_offset += 2; }
	    if ((dp & 0x3) == 0x3) { outer = get_ilong_1 (m68kpc_offset); m68kpc_offset += 4; }

	    if (!(dp & 4)) base += dispreg;
	    if (dp & 3) base = get_long (base);
	    if (dp & 4) base += dispreg;

	    addr = base + outer;
	    sprintf (buffer,"(%s%c%d.%c*%d+%ld)+%ld == $%08lx", name,
		    dp & 0x8000 ? 'A' : 'D', (int)r, dp & 0x800 ? 'L' : 'W',
		    1 << ((dp >> 9) & 3),
		    disp,outer,
		    (unsigned long)addr);
	} else {
	  addr = m68k_areg(regs,reg) + (uae_s32)((uae_s8)disp8) + dispreg;
	  sprintf (buffer,"(A%d, %c%d.%c*%d, $%02x) == $%08lx", reg,
	       dp & 0x8000 ? 'A' : 'D', (int)r, dp & 0x800 ? 'L' : 'W',
	       1 << ((dp >> 9) & 3), disp8,
	       (unsigned long)addr);
	}
	break;
     case PC16:
	addr = m68k_getpc () + m68kpc_offset;
	disp16 = get_iword_1 (m68kpc_offset); m68kpc_offset += 2;
	addr += (uae_s16)disp16;
	sprintf (buffer,"(PC,$%04x) == $%08lx", disp16 & 0xffff,(unsigned long)addr);
	break;
     case PC8r:
	addr = m68k_getpc () + m68kpc_offset;
	dp = get_iword_1 (m68kpc_offset); m68kpc_offset += 2;
	disp8 = dp & 0xFF;
	r = (dp & 0x7000) >> 12;
	dispreg = dp & 0x8000 ? m68k_areg(regs,r) : m68k_dreg(regs,r);
	if (!(dp & 0x800)) dispreg = (uae_s32)(uae_s16)(dispreg);
	dispreg <<= (dp >> 9) & 3;

	if (dp & 0x100) {
	    uae_s32 outer = 0,disp = 0;
	    uae_s32 base = addr;
	    char name[10];
	    sprintf (name,"PC, ");
	    if (dp & 0x80) { base = 0; name[0] = 0; }
	    if (dp & 0x40) dispreg = 0;
	    if ((dp & 0x30) == 0x20) { disp = (uae_s32)(uae_s16)get_iword_1 (m68kpc_offset); m68kpc_offset += 2; }
	    if ((dp & 0x30) == 0x30) { disp = get_ilong_1 (m68kpc_offset); m68kpc_offset += 4; }
	    base += disp;

	    if ((dp & 0x3) == 0x2) { outer = (uae_s32)(uae_s16)get_iword_1 (m68kpc_offset); m68kpc_offset += 2; }
	    if ((dp & 0x3) == 0x3) { outer = get_ilong_1 (m68kpc_offset); m68kpc_offset += 4; }

	    if (!(dp & 4)) base += dispreg;
	    if (dp & 3) base = get_long (base);
	    if (dp & 4) base += dispreg;

	    addr = base + outer;
	    sprintf (buffer,"(%s%c%d.%c*%d+%ld)+%ld == $%08lx", name,
		    dp & 0x8000 ? 'A' : 'D', (int)r, dp & 0x800 ? 'L' : 'W',
		    1 << ((dp >> 9) & 3),
		    disp,outer,
		    (unsigned long)addr);
	} else {
	  addr += (uae_s32)((uae_s8)disp8) + dispreg;
	  sprintf (buffer,"(PC, %c%d.%c*%d, $%02x) == $%08lx", dp & 0x8000 ? 'A' : 'D',
		(int)r, dp & 0x800 ? 'L' : 'W',  1 << ((dp >> 9) & 3),
		disp8, (unsigned long)addr);
	}
	break;
     case absw:
	sprintf (buffer,"$%08lx", (unsigned long)(uae_s32)(uae_s16)get_iword_1 (m68kpc_offset));
	m68kpc_offset += 2;
	break;
     case absl:
	sprintf (buffer,"$%08lx", (unsigned long)get_ilong_1 (m68kpc_offset));
	m68kpc_offset += 4;
	break;
     case imm:
	switch (size){
	 case sz_byte:
	    sprintf (buffer,"#$%02x", (unsigned int)(get_iword_1 (m68kpc_offset) & 0xff));
	    m68kpc_offset += 2;
	    break;
	 case sz_word:
	    sprintf (buffer,"#$%04x", (unsigned int)(get_iword_1 (m68kpc_offset) & 0xffff));
	    m68kpc_offset += 2;
	    break;
	 case sz_long:
	    sprintf (buffer,"#$%08lx", (unsigned long)(get_ilong_1 (m68kpc_offset)));
	    m68kpc_offset += 4;
	    break;
	 default:
	    break;
	}
	break;
     case imm0:
	offset = (uae_s32)(uae_s8)get_iword_1 (m68kpc_offset);
	m68kpc_offset += 2;
	sprintf (buffer,"#$%02x", (unsigned int)(offset & 0xff));
	break;
     case imm1:
	offset = (uae_s32)(uae_s16)get_iword_1 (m68kpc_offset);
	m68kpc_offset += 2;
	sprintf (buffer,"#$%04x", (unsigned int)(offset & 0xffff));
	break;
     case imm2:
	offset = (uae_s32)get_ilong_1 (m68kpc_offset);
	m68kpc_offset += 4;
	sprintf (buffer,"#$%08lx", (unsigned long)offset);
	break;
     case immi:
	offset = (uae_s32)(uae_s8)(reg & 0xff);
	sprintf (buffer,"#$%08lx", (unsigned long)offset);
	break;
     default:
	break;
    }
    if (buf == 0)
	fprintf (f, "%s", buffer);
    else
	strcat (buf, buffer);
    return offset;
}
#endif /* 0 */

/* The plan is that this will take over the job of exception 3 handling -
 * the CPU emulation functions will just do a longjmp to m68k_go whenever
 * they hit an odd address. */
static int verify_ea (int reg, amodes mode, wordsizes size, uae_u32 *val)
{
    uae_u16 dp;
    uae_s8 disp8;
    uae_s16 disp16;
    int r;
    uae_u32 dispreg;
    uaecptr addr;
    uae_s32 offset = 0;

    switch (mode){
     case Dreg:
	*val = m68k_dreg (regs, reg);
	return 1;
     case Areg:
	*val = m68k_areg (regs, reg);
	return 1;

     case Aind:
     case Aipi:
	addr = m68k_areg (regs, reg);
	break;
     case Apdi:
	addr = m68k_areg (regs, reg);
	break;
     case Ad16:
	disp16 = get_iword_1 (m68kpc_offset); m68kpc_offset += 2;
	addr = m68k_areg(regs,reg) + (uae_s16)disp16;
	break;
     case Ad8r:
	addr = m68k_areg (regs, reg);
     d8r_common:
	dp = get_iword_1 (m68kpc_offset); m68kpc_offset += 2;
	disp8 = dp & 0xFF;
	r = (dp & 0x7000) >> 12;
	dispreg = dp & 0x8000 ? m68k_areg(regs,r) : m68k_dreg(regs,r);
	if (!(dp & 0x800)) dispreg = (uae_s32)(uae_s16)(dispreg);
	dispreg <<= (dp >> 9) & 3;

	if (dp & 0x100) {
	    uae_s32 outer = 0, disp = 0;
	    uae_s32 base = addr;
	    if (dp & 0x80) base = 0;
	    if (dp & 0x40) dispreg = 0;
	    if ((dp & 0x30) == 0x20) { disp = (uae_s32)(uae_s16)get_iword_1 (m68kpc_offset); m68kpc_offset += 2; }
	    if ((dp & 0x30) == 0x30) { disp = get_ilong_1 (m68kpc_offset); m68kpc_offset += 4; }
	    base += disp;

	    if ((dp & 0x3) == 0x2) { outer = (uae_s32)(uae_s16)get_iword_1 (m68kpc_offset); m68kpc_offset += 2; }
	    if ((dp & 0x3) == 0x3) { outer = get_ilong_1 (m68kpc_offset); m68kpc_offset += 4; }

	    if (!(dp & 4)) base += dispreg;
	    if (dp & 3) base = get_long (base);
	    if (dp & 4) base += dispreg;

	    addr = base + outer;
	} else {
	  addr += (uae_s32)((uae_s8)disp8) + dispreg;
	}
	break;
     case PC16:
	addr = m68k_getpc () + m68kpc_offset;
	disp16 = get_iword_1 (m68kpc_offset); m68kpc_offset += 2;
	addr += (uae_s16)disp16;
	break;
     case PC8r:
	addr = m68k_getpc () + m68kpc_offset;
	goto d8r_common;
     case absw:
	addr = (uae_s32)(uae_s16)get_iword_1 (m68kpc_offset);
	m68kpc_offset += 2;
	break;
     case absl:
	addr = get_ilong_1 (m68kpc_offset);
	m68kpc_offset += 4;
	break;
     case imm:
	switch (size){
	 case sz_byte:
	    *val = get_iword_1 (m68kpc_offset) & 0xff;
	    m68kpc_offset += 2;
	    break;
	 case sz_word:
	    *val = get_iword_1 (m68kpc_offset) & 0xffff;
	    m68kpc_offset += 2;
	    break;
	 case sz_long:
	    *val = get_ilong_1 (m68kpc_offset);
	    m68kpc_offset += 4;
	    break;
	 default:
	    break;
	}
	return 1;
     case imm0:
	*val = (uae_s32)(uae_s8)get_iword_1 (m68kpc_offset);
	m68kpc_offset += 2;
	return 1;
     case imm1:
	*val = (uae_s32)(uae_s16)get_iword_1 (m68kpc_offset);
	m68kpc_offset += 2;
	return 1;
     case imm2:
	*val = get_ilong_1 (m68kpc_offset);
	m68kpc_offset += 4;
	return 1;
     case immi:
	*val = (uae_s32)(uae_s8)(reg & 0xff);
	return 1;
     default:
	addr = 0;
	break;
    }
    if ((addr & 1) == 0)
	return 1;

    last_addr_for_exception_3 = m68k_getpc () + m68kpc_offset;
    last_fault_for_exception_3 = addr;
    return 0;
}

uae_u32 get_disp_ea_020 (uae_u32 base, uae_u32 dp)
{
    int reg = (dp >> 12) & 15;
    uae_s32 regd = regs.regs[reg];
    if ((dp & 0x800) == 0)
	regd = (uae_s32)(uae_s16)regd;
    regd <<= (dp >> 9) & 3;
    if (dp & 0x100) {
	uae_s32 outer = 0;
	if (dp & 0x80) base = 0;
	if (dp & 0x40) regd = 0;

	if ((dp & 0x30) == 0x20) base += (uae_s32)(uae_s16)next_iword();
	if ((dp & 0x30) == 0x30) base += next_ilong();

	if ((dp & 0x3) == 0x2) outer = (uae_s32)(uae_s16)next_iword();
	if ((dp & 0x3) == 0x3) outer = next_ilong();

	if ((dp & 0x4) == 0) base += regd;
	if (dp & 0x3) base = get_long (base);
	if (dp & 0x4) base += regd;

	return base + outer;
    } else {
	return base + (uae_s32)((uae_s8)dp) + regd;
    }
}

uae_u32 get_disp_ea_000 (uae_u32 base, uae_u32 dp)
{
    int reg = (dp >> 12) & 15;
    uae_s32 regd = regs.regs[reg];
#if 1
    if ((dp & 0x800) == 0)
	regd = (uae_s32)(uae_s16)regd;
    return base + (uae_s8)dp + regd;
#else
    /* Branch-free code... benchmark this again now that
     * things are no longer inline.  */
    uae_s32 regd16;
    uae_u32 mask;
    mask = ((dp & 0x800) >> 11) - 1;
    regd16 = (uae_s32)(uae_s16)regd;
    regd16 &= mask;
    mask = ~mask;
    base += (uae_s8)dp;
    regd &= mask;
    regd |= regd16;
    return base + regd;
#endif
}

void MakeSR (void)
{
#if 0
    assert((regs.t1 & 1) == regs.t1);
    assert((regs.t0 & 1) == regs.t0);
    assert((regs.s & 1) == regs.s);
    assert((regs.m & 1) == regs.m);
    assert((XFLG & 1) == XFLG);
    assert((NFLG & 1) == NFLG);
    assert((ZFLG & 1) == ZFLG);
    assert((VFLG & 1) == VFLG);
    assert((CFLG & 1) == CFLG);
#endif
    regs.sr = ((regs.t1 << 15) /*| (regs.t0 << 14)*/
	       | (regs.s << 13) /*| (regs.m << 12)*/ | (regs.intmask << 8)
	       | (GET_XFLG << 4) | (GET_NFLG << 3) | (GET_ZFLG << 2) | (GET_VFLG << 1)
	       | GET_CFLG);
}

void MakeFromSR (void)
{
//    int oldm = regs.m;
    int olds = regs.s;

    regs.t1 = (regs.sr >> 15) & 1;
//    regs.t0 = (regs.sr >> 14) & 1;
    regs.s = (regs.sr >> 13) & 1;
//    regs.m = (regs.sr >> 12) & 1;
    regs.intmask = (regs.sr >> 8) & 7;
    SET_XFLG ((regs.sr >> 4) & 1);
    SET_NFLG ((regs.sr >> 3) & 1);
    SET_ZFLG ((regs.sr >> 2) & 1);
    SET_VFLG ((regs.sr >> 1) & 1);
    SET_CFLG (regs.sr & 1);
#if 0
    if (currprefs.cpu_level >= 2) {
	if (olds != regs.s) {
	    if (olds) {
		if (oldm)
		    regs.msp = m68k_areg(regs, 7);
		else
		    regs.isp = m68k_areg(regs, 7);
		m68k_areg(regs, 7) = regs.usp;
	    } else {
		regs.usp = m68k_areg(regs, 7);
		m68k_areg(regs, 7) = /*regs.m ? regs.msp :*/ regs.isp;
	    }
	} else if (olds && oldm != regs.m) {
	    if (oldm) {
		regs.msp = m68k_areg(regs, 7);
		m68k_areg(regs, 7) = regs.isp;
	    } else {
		regs.isp = m68k_areg(regs, 7);
		m68k_areg(regs, 7) = regs.msp;
	    }
	}
    } else {
#endif
	if (olds != regs.s) {
	    if (olds) {
		regs.isp = m68k_areg(regs, 7);
		m68k_areg(regs, 7) = regs.usp;
	    } else {
		regs.usp = m68k_areg(regs, 7);
		m68k_areg(regs, 7) = regs.isp;
	    }
	}
//    }

    set_special (SPCFLAG_INT);
    if (regs.t1 /*|| regs.t0*/)
	set_special (SPCFLAG_TRACE);
    else
	unset_special (SPCFLAG_TRACE | SPCFLAG_DOTRACE);
}

int intlev(void) 
{
  int rc = currIntLev;
  currIntLev = -1;
  return rc;
}
 
void Exception(int nr, uaecptr oldpc)
{
    GList *l;

    uae_u32 currpc = m68k_getpc ();

    compiler_flush_jsr_stack();
    MakeSR();

    if (!regs.s) {
	regs.usp = m68k_areg(regs, 7);
#if 0
	if (currprefs.cpu_level >= 2)
	    m68k_areg(regs, 7) = regs.m ? regs.msp : regs.isp;
	else
#endif
	    m68k_areg(regs, 7) = regs.isp;
	regs.s = 1;
    }
#if 0
    if (currprefs.cpu_level > 0) {
	if (nr == 2 || nr == 3) {
	    int i;
	    /* @@@ this is probably wrong (?) */
	    for (i = 0 ; i < 12 ; i++) {
		m68k_areg(regs, 7) -= 2;
		put_word (m68k_areg(regs, 7), 0);
	    }
	    m68k_areg(regs, 7) -= 2;
	    put_word (m68k_areg(regs, 7), 0xa000 + nr * 4);
	} else if (nr ==5 || nr == 6 || nr == 7 || nr == 9) {
	    m68k_areg(regs, 7) -= 4;
	    put_long (m68k_areg(regs, 7), oldpc);
	    m68k_areg(regs, 7) -= 2;
	    put_word (m68k_areg(regs, 7), 0x2000 + nr * 4);
	} else if (regs.m && nr >= 24 && nr < 32) {
	    m68k_areg(regs, 7) -= 2;
	    put_word (m68k_areg(regs, 7), nr * 4);
	    m68k_areg(regs, 7) -= 4;
	    put_long (m68k_areg(regs, 7), currpc);
	    m68k_areg(regs, 7) -= 2;
	    put_word (m68k_areg(regs, 7), regs.sr);
	    regs.sr |= (1 << 13);
	    regs.msp = m68k_areg(regs, 7);
	    m68k_areg(regs, 7) = regs.isp;
	    m68k_areg(regs, 7) -= 2;
	    put_word (m68k_areg(regs, 7), 0x1000 + nr * 4);
	} else {
	    m68k_areg(regs, 7) -= 2;
	    put_word (m68k_areg(regs, 7), nr * 4);
	}
    } else {
#endif
	if (nr == 2 || nr == 3) {
	    m68k_areg(regs, 7) -= 12;
	    /* ??????? */
	    if (nr == 3) {
		put_long (m68k_areg(regs, 7), last_fault_for_exception_3);
		put_word (m68k_areg(regs, 7)+4, last_op_for_exception_3);
		put_long (m68k_areg(regs, 7)+8, last_addr_for_exception_3);
	    }
	    //write_log ("Exception %i!\n", nr);
	    goto kludge_me_do;
	}
//    }
    m68k_areg(regs, 7) -= 4;
    put_long (m68k_areg(regs, 7), currpc);
kludge_me_do:
    m68k_areg(regs, 7) -= 2;
    put_word (m68k_areg(regs, 7), regs.sr);
    m68k_setpc (get_long (regs.vbr + 4*nr));
    fill_prefetch_0 ();
    regs.t1 /*= regs.t0 = regs.m*/ = 0;
    unset_special (SPCFLAG_TRACE | SPCFLAG_DOTRACE);

  // added for capturing the exception and next launch a debugger
  if ((l = bkpts.exception) && !(regs.spcflags & SPCFLAG_BRK))
    {
        bkpts.id = 0;
        while (l) 
	    {
	        if (GPOINTER_TO_INT(l->data) == nr) 
	        {
				bkpts.type = BK_CAUSE_EXCEPTION;
				bkpts.mode = nr;
	            regs.spcflags |= SPCFLAG_BRK;	            
	            break;
	        }
	        
			bkpts.id++;
	        l = l->next;
	    }
    }
}

int uscycle = 0;

void Interrupt(int nr)
{
    assert(nr < 8 && nr >= 0);
    lastint_regs = regs;
    lastint_no = nr;
    Exception(nr+24, 0);

    regs.intmask = nr;
    set_special (SPCFLAG_INT);
}

static uae_u32 caar, cacr, itt0, itt1, dtt0, dtt1, tc, mmusr;

int m68k_move2c (int regno, uae_u32 *regp)
{
    if ((currprefs.cpu_level == 1 && (regno & 0x7FF) > 1)
	|| (currprefs.cpu_level < 4 && (regno & 0x7FF) > 2)
	|| (currprefs.cpu_level == 4 && regno == 0x802))
    {
	op_illg (0x4E7B);
	return 0;
    } else {
	switch (regno) {
	case 0: regs.sfc = *regp & 7; break;
	case 1: regs.dfc = *regp & 7; break;
	case 2: cacr = *regp & (currprefs.cpu_level < 4 ? 0x3 : 0x80008000); break;
	case 3: tc = *regp & 0xc000; break;
	  /* Mask out fields that should be zero.  */
	case 4: itt0 = *regp & 0xffffe364; break;
	case 5: itt1 = *regp & 0xffffe364; break;
	case 6: dtt0 = *regp & 0xffffe364; break;
	case 7: dtt1 = *regp & 0xffffe364; break;
	  
	case 0x800: regs.usp = *regp; break;
	case 0x801: regs.vbr = *regp; break;
	case 0x802: caar = *regp & 0xfc; break;
//	case 0x803: regs.msp = *regp; if (regs.m == 1) m68k_areg(regs, 7) = regs.msp; break;
//	case 0x804: regs.isp = *regp; if (regs.m == 0) m68k_areg(regs, 7) = regs.isp; break;
	default:
	    op_illg (0x4E7B);
	    return 0;
	}
    }
    return 1;
}

int m68k_movec2 (int regno, uae_u32 *regp)
{
    if ((currprefs.cpu_level == 1 && (regno & 0x7FF) > 1)
	|| (currprefs.cpu_level < 4 && (regno & 0x7FF) > 2)
	|| (currprefs.cpu_level == 4 && regno == 0x802))
    {
	op_illg (0x4E7A);
	return 0;
    } else {
	switch (regno) {
	case 0: *regp = regs.sfc; break;
	case 1: *regp = regs.dfc; break;
	case 2: *regp = cacr; break;
	case 3: *regp = tc; break;
	case 4: *regp = itt0; break;
	case 5: *regp = itt1; break;
	case 6: *regp = dtt0; break;
	case 7: *regp = dtt1; break;
	case 0x800: *regp = regs.usp; break;
	case 0x801: *regp = regs.vbr; break;
	case 0x802: *regp = caar; break;
//	case 0x803: *regp = regs.m == 1 ? m68k_areg(regs, 7) : regs.msp; break;
//	case 0x804: *regp = regs.m == 0 ? m68k_areg(regs, 7) : regs.isp; break;
	case 0x805: *regp = mmusr; break;
	default:
	    op_illg (0x4E7A);
	    return 0;
	}
    }
    return 1;
}

STATIC_INLINE int
div_unsigned(uae_u32 src_hi, uae_u32 src_lo, uae_u32 div, uae_u32 *quot, uae_u32 *rem)
{
	uae_u32 q = 0, cbit = 0;
	int i;

	if (div <= src_hi) {
	    return 1;
	}
	for (i = 0 ; i < 32 ; i++) {
		cbit = src_hi & 0x80000000ul;
		src_hi <<= 1;
		if (src_lo & 0x80000000ul) src_hi++;
		src_lo <<= 1;
		q = q << 1;
		if (cbit || div <= src_hi) {
			q |= 1;
			src_hi -= div;
		}
	}
	*quot = q;
	*rem = src_hi;
	return 0;
}

void m68k_divl (uae_u32 opcode, uae_u32 src, uae_u16 extra, uaecptr oldpc)
{
#if defined(uae_s64)
    if (src == 0) {
	Exception (5, oldpc);
	return;
    }
    if (extra & 0x800) {
	/* signed variant */
	uae_s64 a = (uae_s64)(uae_s32)m68k_dreg(regs, (extra >> 12) & 7);
	uae_s64 quot, rem;

	if (extra & 0x400) {
	    a &= 0xffffffffu;
	    a |= (uae_s64)m68k_dreg(regs, extra & 7) << 32;
	}
	rem = a % (uae_s64)(uae_s32)src;
	quot = a / (uae_s64)(uae_s32)src;
	if ((quot & UVAL64(0xffffffff80000000)) != 0
	    && (quot & UVAL64(0xffffffff80000000)) != UVAL64(0xffffffff80000000))
	{
	    SET_VFLG (1);
	    SET_NFLG (1);
	    SET_CFLG (0);
	} else {
	    if (((uae_s32)rem < 0) != ((uae_s64)a < 0)) rem = -rem;
	    SET_VFLG (0);
	    SET_CFLG (0);
	    SET_ZFLG (((uae_s32)quot) == 0);
	    SET_NFLG (((uae_s32)quot) < 0);
	    m68k_dreg(regs, extra & 7) = rem;
	    m68k_dreg(regs, (extra >> 12) & 7) = quot;
	}
    } else {
	/* unsigned */
	uae_u64 a = (uae_u64)(uae_u32)m68k_dreg(regs, (extra >> 12) & 7);
	uae_u64 quot, rem;

	if (extra & 0x400) {
	    a &= 0xffffffffu;
	    a |= (uae_u64)m68k_dreg(regs, extra & 7) << 32;
	}
	rem = a % (uae_u64)src;
	quot = a / (uae_u64)src;
	if (quot > 0xffffffffu) {
	    SET_VFLG (1);
	    SET_NFLG (1);
	    SET_CFLG (0);
	} else {
	    SET_VFLG (0);
	    SET_CFLG (0);
	    SET_ZFLG (((uae_s32)quot) == 0);
	    SET_NFLG (((uae_s32)quot) < 0);
	    m68k_dreg(regs, extra & 7) = rem;
	    m68k_dreg(regs, (extra >> 12) & 7) = quot;
	}
    }
#else
    if (src == 0) {
	Exception (5, oldpc);
	return;
    }
    if (extra & 0x800) {
	/* signed variant */
	uae_s32 lo = (uae_s32)m68k_dreg(regs, (extra >> 12) & 7);
	uae_s32 hi = lo < 0 ? -1 : 0;
	uae_s32 save_high;
	uae_u32 quot, rem;
	uae_u32 sign;

	if (extra & 0x400) {
	    hi = (uae_s32)m68k_dreg(regs, extra & 7);
	}
	save_high = hi;
	sign = (hi ^ src);
	if (hi < 0) {
	    hi = ~hi;
	    lo = -lo;
	    if (lo == 0) hi++;
	}
	if ((uae_s32)src < 0) src = -src;
	if (div_unsigned(hi, lo, src, &quot, &rem) ||
	    (sign & 0x80000000) ? quot > 0x80000000 : quot > 0x7fffffff) {
	    SET_VFLG (1);
	    SET_NFLG (1);
	    SET_CFLG (0);
	} else {
	    if (sign & 0x80000000) quot = -quot;
	    if (((uae_s32)rem < 0) != (save_high < 0)) rem = -rem;
	    SET_VFLG (0);
	    SET_CFLG (0);
	    SET_ZFLG (((uae_s32)quot) == 0);
	    SET_NFLG (((uae_s32)quot) < 0);
	    m68k_dreg(regs, extra & 7) = rem;
	    m68k_dreg(regs, (extra >> 12) & 7) = quot;
	}
    } else {
	/* unsigned */
	uae_u32 lo = (uae_u32)m68k_dreg(regs, (extra >> 12) & 7);
	uae_u32 hi = 0;
	uae_u32 quot, rem;

	if (extra & 0x400) {
	    hi = (uae_u32)m68k_dreg(regs, extra & 7);
	}
	if (div_unsigned(hi, lo, src, &quot, &rem)) {
	    SET_VFLG (1);
	    SET_NFLG (1);
	    SET_CFLG (0);
	} else {
	    SET_VFLG (0);
	    SET_CFLG (0);
	    SET_ZFLG (((uae_s32)quot) == 0);
	    SET_NFLG (((uae_s32)quot) < 0);
	    m68k_dreg(regs, extra & 7) = rem;
	    m68k_dreg(regs, (extra >> 12) & 7) = quot;
	}
    }
#endif
}

STATIC_INLINE void
mul_unsigned(uae_u32 src1, uae_u32 src2, uae_u32 *dst_hi, uae_u32 *dst_lo)
{
	uae_u32 r0 = (src1 & 0xffff) * (src2 & 0xffff);
	uae_u32 r1 = ((src1 >> 16) & 0xffff) * (src2 & 0xffff);
	uae_u32 r2 = (src1 & 0xffff) * ((src2 >> 16) & 0xffff);
	uae_u32 r3 = ((src1 >> 16) & 0xffff) * ((src2 >> 16) & 0xffff);
	uae_u32 lo;

	lo = r0 + ((r1 << 16) & 0xffff0000ul);
	if (lo < r0) r3++;
	r0 = lo;
	lo = r0 + ((r2 << 16) & 0xffff0000ul);
	if (lo < r0) r3++;
	r3 += ((r1 >> 16) & 0xffff) + ((r2 >> 16) & 0xffff);
	*dst_lo = lo;
	*dst_hi = r3;
}

void m68k_mull (uae_u32 opcode, uae_u32 src, uae_u16 extra)
{
#if defined(uae_s64)
    if (extra & 0x800) {
	/* signed variant */
	uae_s64 a = (uae_s64)(uae_s32)m68k_dreg(regs, (extra >> 12) & 7);

	a *= (uae_s64)(uae_s32)src;
	SET_VFLG (0);
	SET_CFLG (0);
	SET_ZFLG (a == 0);
	SET_NFLG (a < 0);
	if (extra & 0x400)
	    m68k_dreg(regs, extra & 7) = a >> 32;
	else if ((a & UVAL64(0xffffffff80000000)) != 0
		 && (a & UVAL64(0xffffffff80000000)) != UVAL64(0xffffffff80000000))
	{
	    SET_VFLG (1);
	}
	m68k_dreg(regs, (extra >> 12) & 7) = (uae_u32)a;
    } else {
	/* unsigned */
	uae_u64 a = (uae_u64)(uae_u32)m68k_dreg(regs, (extra >> 12) & 7);

	a *= (uae_u64)src;
	SET_VFLG (0);
	SET_CFLG (0);
	SET_ZFLG (a == 0);
	SET_NFLG (((uae_s64)a) < 0);
	if (extra & 0x400)
	    m68k_dreg(regs, extra & 7) = a >> 32;
	else if ((a & UVAL64(0xffffffff00000000)) != 0) {
	    SET_VFLG (1);
	}
	m68k_dreg(regs, (extra >> 12) & 7) = (uae_u32)a;
    }
#else
    if (extra & 0x800) {
	/* signed variant */
	uae_s32 src1,src2;
	uae_u32 dst_lo,dst_hi;
	uae_u32 sign;

	src1 = (uae_s32)src;
	src2 = (uae_s32)m68k_dreg(regs, (extra >> 12) & 7);
	sign = (src1 ^ src2);
	if (src1 < 0) src1 = -src1;
	if (src2 < 0) src2 = -src2;
	mul_unsigned((uae_u32)src1,(uae_u32)src2,&dst_hi,&dst_lo);
	if (sign & 0x80000000) {
		dst_hi = ~dst_hi;
		dst_lo = -dst_lo;
		if (dst_lo == 0) dst_hi++;
	}
	SET_VFLG (0);
	SET_CFLG (0);
	SET_ZFLG (dst_hi == 0 && dst_lo == 0);
	SET_NFLG (((uae_s32)dst_hi) < 0);
	if (extra & 0x400)
	    m68k_dreg(regs, extra & 7) = dst_hi;
	else if ((dst_hi != 0 || (dst_lo & 0x80000000) != 0)
		 && ((dst_hi & 0xffffffff) != 0xffffffff
		     || (dst_lo & 0x80000000) != 0x80000000))
	{
	    SET_VFLG (1);
	}
	m68k_dreg(regs, (extra >> 12) & 7) = dst_lo;
    } else {
	/* unsigned */
	uae_u32 dst_lo,dst_hi;

	mul_unsigned(src,(uae_u32)m68k_dreg(regs, (extra >> 12) & 7),&dst_hi,&dst_lo);

	SET_VFLG (0);
	SET_CFLG (0);
	SET_ZFLG (dst_hi == 0 && dst_lo == 0);
	SET_NFLG (((uae_s32)dst_hi) < 0);
	if (extra & 0x400)
	    m68k_dreg(regs, extra & 7) = dst_hi;
	else if (dst_hi != 0) {
	    SET_VFLG (1);
	}
	m68k_dreg(regs, (extra >> 12) & 7) = dst_lo;
    }
#endif
}
static char* ccnames[] =
{ "T ","F ","HI","LS","CC","CS","NE","EQ",
  "VC","VS","PL","MI","GE","LT","GT","LE" };

void m68k_reset (void)
{
    regs.spcflags = 0;	// here; otherwise bkpt at $0 is not catched (roms)
    m68k_areg (regs, 7) = tihw.initial_ssp;
    m68k_setpc(tihw.initial_pc);
    fill_prefetch_0 ();
    regs.kick_mask = 0xF80000;
    regs.s = 1;
//    regs.m = 0;
    regs.stopped = 0;
    regs.t1 = 0;
//    regs.t0 = 0;
    SET_ZFLG (0);
    SET_XFLG (0);
    SET_CFLG (0);
    SET_VFLG (0);
    SET_NFLG (0);
    // regs.spcflags = 0;
    regs.intmask = 7;
    regs.vbr = regs.sfc = regs.dfc = 0;
    regs.fpcr = regs.fpsr = regs.fpiar = 0;
}

unsigned long REGPARAM2 op_illg (uae_u32 opcode)
{
    uaecptr pc = m68k_getpc ();
    
#if 0
    if (cloanto_rom && (opcode & 0xF100) == 0x7100) {
	m68k_dreg (regs, (opcode >> 9) & 7) = (uae_s8)(opcode & 0xFF);
	m68k_incpc (2);
	fill_prefetch_0 ();
	return 4;
    }
#endif

    compiler_flush_jsr_stack ();

#if 0
    if (opcode == 0x4E7B && get_long (0x10) == 0 && (pc & 0xF80000) == 0xF80000) {
	write_log ("Your Kickstart requires a 68020 CPU. Giving up.\n");
	broken_in = 1;
	set_special (SPCFLAG_BRK);
	quit_program = 1;
    }
    if (opcode == 0xFF0D) {
	if ((pc & 0xF80000) == 0xF80000) {
	    /* This is from the dummy Kickstart replacement */
	    uae_u16 arg = get_iword (2);
	    m68k_incpc (4);
	    ersatz_perform (arg);
	    fill_prefetch_0 ();
	    return 4;
	} else if ((pc & 0xF80000) == 0xF00000) {
	    /* User-mode STOP replacement */
	    m68k_setstopped (1);
	    return 4;
	}
    }

    if ((opcode & 0xF000) == 0xA000 && (pc & 0xF80000) == 0xF00000) {
	/* Calltrap. */
	m68k_incpc(2);
	call_calltrap (opcode & 0xFFF);
	fill_prefetch_0 ();
	return 4;
    }
#endif

    if ((opcode & 0xF000) == 0xF000) {
	Exception(0xB,0);
	return 4;
    }
    if ((opcode & 0xF000) == 0xA000) {
#if 0
	if ((pc & 0xF80000) == 0xF00000) {
	    /* Calltrap. */
	    call_calltrap (opcode & 0xFFF);
	}
#endif
	Exception(0xA,0);
	return 4;
    }
#if 1
    fprintf(stderr, "UAE: illegal instruction: %04x at %08x\n", 
            opcode, m68k_getpc());
#endif
    Exception (4,0);
    return 4;
}

void mmu_op(uae_u32 opcode, uae_u16 extra)
{
    if ((opcode & 0xFE0) == 0x0500) {
	/* PFLUSH */
	mmusr = 0;
	write_log ("PFLUSH\n");
    } else if ((opcode & 0x0FD8) == 0x548) {
	/* PTEST */
	write_log ("PTEST\n");
    } else
	op_illg (opcode);
}

static uaecptr last_trace_ad = 0;

static void do_trace (void)
{
    if (0/*regs.t0 && currprefs.cpu_level >= 2*/) {
	uae_u16 opcode;
	/* should also include TRAP, CHK, SR modification FPcc */
	/* probably never used so why bother */
	/* We can afford this to be inefficient... */
	m68k_setpc (m68k_getpc ());
	fill_prefetch_0 ();
	opcode = get_word (regs.pc);
	if (opcode == 0x4e72 		/* RTE */
	    || opcode == 0x4e74 		/* RTD */
	    || opcode == 0x4e75 		/* RTS */
	    || opcode == 0x4e77 		/* RTR */
	    || opcode == 0x4e76 		/* TRAPV */
	    || (opcode & 0xffc0) == 0x4e80 	/* JSR */
	    || (opcode & 0xffc0) == 0x4ec0 	/* JMP */
	    || (opcode & 0xff00) == 0x6100  /* BSR */
	    || ((opcode & 0xf000) == 0x6000	/* Bcc */
		&& cctrue((opcode >> 8) & 0xf))
	    || ((opcode & 0xf0f0) == 0x5050 /* DBcc */
		&& !cctrue((opcode >> 8) & 0xf)
		&& (uae_s16)m68k_dreg(regs, opcode & 7) != 0))
	{
	    last_trace_ad = m68k_getpc ();
	    unset_special (SPCFLAG_TRACE);
	    set_special (SPCFLAG_DOTRACE);
	}
    } else if (regs.t1) {
	last_trace_ad = m68k_getpc ();
	unset_special (SPCFLAG_TRACE);
	set_special (SPCFLAG_DOTRACE);
    }
}


#if 0
static int do_specialties (void)
{
    if (regs.spcflags & SPCFLAG_COPPER)
	do_copper ();

    /*n_spcinsns++;*/
    while (regs.spcflags & SPCFLAG_BLTNASTY) {
	do_cycles (4);
	if (regs.spcflags & SPCFLAG_COPPER)
	    do_copper ();
    }

    run_compiled_code();
    if(regs.spcflags & SPCFLAG_ADRERR) 
    {
        Exception(3,0);
        regs.spcflags &= ~SPCFLAG_ADRERR;
    }
    if (regs.spcflags & SPCFLAG_DOTRACE) {
	Exception (9,last_trace_ad);
    }
    while (regs.spcflags & SPCFLAG_STOP) {
	do_cycles (4);
	if (regs.spcflags & SPCFLAG_COPPER)
	    do_copper ();
	if (regs.spcflags & (SPCFLAG_INT | SPCFLAG_DOINT)){
	    int intr = intlev ();
	    unset_special (SPCFLAG_INT | SPCFLAG_DOINT);
	    if (intr != -1 && intr > regs.intmask) {
		Interrupt (intr);
		regs.stopped = 0;
		unset_special (SPCFLAG_STOP);
	    }
	}
    }
    if (regs.spcflags & SPCFLAG_TRACE)
	do_trace ();

    if (regs.spcflags & SPCFLAG_DOINT) {
	int intr = intlev ();
	unset_special (SPCFLAG_DOINT);
	if (intr != -1 && intr > regs.intmask) {
	    Interrupt (intr);
	    regs.stopped = 0;
	}
    }
    if (regs.spcflags & SPCFLAG_INT) {
	unset_special (SPCFLAG_INT);
	set_special (SPCFLAG_DOINT);
    }
    if (regs.spcflags & (SPCFLAG_BRK | SPCFLAG_MODE_CHANGE)) {
	unset_special (SPCFLAG_BRK | SPCFLAG_MODE_CHANGE);
	return 1;
    }
    if(regs.spcflags & SPCFLAG_DBTRACE) 
    {
      regs.spcflags &= ~SPCFLAG_DBTRACE;
      //cb_launch_debugger();
    }
    return 0;
}

/* It's really sad to have two almost identical functions for this, but we
   do it all for performance... :( */
static void m68k_run_1 (void)
{
#ifdef DEBUG_PREFETCH
    uae_u8 saved_bytes[20];
    uae_u16 *oldpcp;
#endif
    for (;;) {
	int cycles;
	uae_u32 opcode = get_iword_prefetch (0);
#ifdef DEBUG_PREFETCH
	if (get_ilong (0) != do_get_mem_long (&regs.prefetch)) {
	    fprintf (stderr, "Prefetch differs from memory.\n");
	    debugging = 1;
	    return;
	}
	oldpcp = regs.pc_p;
	memcpy (saved_bytes, regs.pc_p, 20);
#endif
	/* assert (!regs.stopped && !(regs.spcflags & SPCFLAG_STOP)); */
/*	regs_backup[backup_pointer = (backup_pointer + 1) % 16] = regs;*/
#if COUNT_INSTRS == 2
	if (table68k[opcode].handler != -1)
	    instrcount[table68k[opcode].handler]++;
#elif COUNT_INSTRS == 1
	instrcount[opcode]++;
#endif
#if defined X86_ASSEMBLY
	__asm__ __volatile__("\tcall *%%ebx"
			     : "=&a" (cycles) : "b" (cpufunctbl[opcode]), "0" (opcode)
			     : "%edx", "%ecx",
			     "%esi", "%edi", "%ebp", "memory", "cc");
#else
	cycles = (*cpufunctbl[opcode])(opcode);
#endif
#ifdef DEBUG_PREFETCH
	if (memcmp (saved_bytes, oldpcp, 20) != 0) {
	    fprintf (stderr, "Self-modifying code detected.\n");
	    set_special (SPCFLAG_BRK);
	    debugging = 1;
	}
#endif
	/*n_insns++;*/
	cycles &= cycles_mask;
	cycles |= cycles_val;
	do_cycles (cycles);
	if (regs.spcflags) {
	    if (do_specialties ())
		return;
	}
    }
}

#define DEBUG_PREFETCH

/* Same thing, but don't use prefetch to get opcode.  */
static void m68k_run_2 (void)
{
    for (;;) {
	int cycles;
	uae_u32 opcode = get_iword (0);

	/* assert (!regs.stopped && !(regs.spcflags & SPCFLAG_STOP)); */
/*	regs_backup[backup_pointer = (backup_pointer + 1) % 16] = regs;*/
#if COUNT_INSTRS == 2
	if (table68k[opcode].handler != -1)
	    instrcount[table68k[opcode].handler]++;
#elif COUNT_INSTRS == 1
	instrcount[opcode]++;
#endif
#if defined X86_ASSEMBLY
	__asm__ __volatile__("\tcall *%%ebx"
			     : "=&a" (cycles) : "b" (cpufunctbl[opcode]), "0" (opcode)
			     : "%edx", "%ecx",
			     "%esi", "%edi", "%ebp", "memory", "cc");
#else
	cycles = (*cpufunctbl[opcode])(opcode);
#endif

	/*n_insns++;*/
	cycles &= cycles_mask;
	cycles |= cycles_val;
	do_cycles (cycles);
	if (regs.spcflags) {
	    if (do_specialties ())
		return;
	}
    }
}

#ifdef X86_ASSEMBLY
STATIC_INLINE void m68k_run1 (void (*func)(void))
{
    /* Work around compiler bug: GCC doesn't push %ebp in m68k_run_1. */
    __asm__ __volatile__ ("pushl %%ebp\n\tcall *%0\n\tpopl %%ebp"
			  : : "r" (func) : "%eax", "%edx", "%ecx", "memory", "cc");
}
#else
#define m68k_run1(F) F()
#endif

int in_m68k_go = 0;

void m68k_go (int may_quit)
{
    if (in_m68k_go || !may_quit) {
	write_log ("Bug! m68k_go is not reentrant.\n");
	abort ();
    }

    reset_frame_rate_hack ();
    update_68k_cycles ();

    in_m68k_go++;
    for (;;) {
	if (quit_program > 0) {
	    if (quit_program == 1)
		break;
	    quit_program = 0;
	    m68k_reset ();
	    reset_all_systems ();
	    customreset ();
	}
	if (debugging)
	    debug ();
	m68k_run1 (currprefs.cpu_compatible ? m68k_run_1 : m68k_run_2);
    }
    in_m68k_go--;
}
#endif

static void m68k_verify (uaecptr addr, uaecptr *nextpc)
{
    uae_u32 opcode, val;
    struct instr *dp;

    opcode = get_iword_1(0);
    last_op_for_exception_3 = opcode;
    m68kpc_offset = 2;

    if (cpufunctbl[opcode] == op_illg_1) {
	opcode = 0x4AFC;
    }
    dp = table68k + opcode;

    if (dp->suse) {
	if (!verify_ea (dp->sreg, dp->smode, dp->size, &val)) {
	    Exception (3, 0);
	    return;
	}
    }
    if (dp->duse) {
	if (!verify_ea (dp->dreg, dp->dmode, dp->size, &val)) {
	    Exception (3, 0);
	    return;
	}
    }
}

#if 0

void m68k_disasm (uaecptr addr, uaecptr *nextpc, int cnt, char *output)
{
    char temp[40];
    uaecptr newpc = 0;
    m68kpc_offset = addr - m68k_getpc ();
    output[0]=0;
    while (cnt-- > 0) {
	char instrname[20],*ccpt;
	uae_u32 opcode;
	struct mnemolookup *lookup;
	struct instr *dp;
	sprintf(temp, "%08lx: ", m68k_getpc() + m68kpc_offset);
	strcat(output, temp);
	opcode = get_iword_1 (m68kpc_offset);
	m68kpc_offset += 2;
	if (cpufunctbl[opcode] == op_illg_1) {
	    opcode = 0x4AFC;
	}
	dp = table68k + opcode;
	for (lookup = lookuptab;lookup->mnemo != dp->mnemo; lookup++)
	    ;

	strcpy (instrname, lookup->name);
	ccpt = strstr (instrname, "cc");
	if (ccpt != 0) {
	    strncpy (ccpt, ccnames[dp->cc], 2);
	}
	strcat(output,instrname);
	switch(dp->size)
	{
	 case sz_byte: strcat(output, ".B "); break;
	 case sz_word: strcat(output, ".W "); break;
	 case sz_long: strcat(output, ".L "); break;
	 default: strcat (output, "   "); break;
	}

	if (dp->suse) {
	    newpc = m68k_getpc () + m68kpc_offset;
	    newpc += ShowEA (0, dp->sreg, dp->smode, dp->size, output);
	}
	if (dp->suse && dp->duse)
	    strcat (output, ",");
	if (dp->duse) {
	    newpc = m68k_getpc () + m68kpc_offset;
	    newpc += ShowEA (0, dp->dreg, dp->dmode, dp->size, output);
	}
	if (ccpt != 0) {
	    if (cctrue(dp->cc)) {
		sprintf (temp, " == %08lx (TRUE)", newpc);
		strcat (output, temp);
	    } else {
		sprintf (temp, " == %08lx (FALSE)", newpc);
		strcat (output, temp);
	    }
	} else if ((opcode & 0xff00) == 0x6100) { /* BSR */
	    sprintf (temp, " == %08lx", newpc);
	    strcat (output, temp);
	}
	strcat (output, "\n");
    }
    if (nextpc)
	*nextpc = m68k_getpc () + m68kpc_offset;
}

void m68k_dumpstate (uaecptr *nextpc)
{
    int i;
    char buffer[256];

    for (i = 0; i < 8; i++){
	printf ("D%d: %08lx ", i, m68k_dreg(regs, i));
	if ((i & 3) == 3) printf ("\n");
    }
    for (i = 0; i < 8; i++){
	printf ("A%d: %08lx ", i, m68k_areg(regs, i));
	if ((i & 3) == 3) printf ("\n");
    }
    if (regs.s == 0) regs.usp = m68k_areg(regs, 7);
//    if (regs.s && regs.m) regs.msp = m68k_areg(regs, 7);
    if (regs.s /*&& regs.m == 0*/) regs.isp = m68k_areg(regs, 7);
    printf ("USP=%08lx SSP=%08lx VBR=%08lx\n",
	     regs.usp,regs.isp/*,regs.msp*/,regs.vbr);
    printf ("T=%d S=%d X=%d N=%d Z=%d V=%d C=%d IMASK=%d\n",
	     regs.t1/*, regs.t0*/, regs.s, /*regs.m,*/
	     GET_XFLG, GET_NFLG, GET_ZFLG, GET_VFLG, GET_CFLG, regs.intmask);
    for (i = 0; i < 8; i++){
	printf ("FP%d: %g ", i, regs.fp[i]);
	if ((i & 3) == 3) printf ("\n");
    }
    printf ("N=%d Z=%d I=%d NAN=%d\n",
	     (regs.fpsr & 0x8000000) != 0,
	     (regs.fpsr & 0x4000000) != 0,
	     (regs.fpsr & 0x2000000) != 0,
	     (regs.fpsr & 0x1000000) != 0);
    if (currprefs.cpu_compatible)
	printf ("prefetch %08lx\n", (unsigned long)do_get_mem_long(&regs.prefetch));

    m68k_disasm (m68k_getpc (), nextpc, 1, buffer);
    printf("%s\n", buffer);
    if (nextpc)
	printf ("next PC: %08lx\n", *nextpc);
}
 
#endif /* 0 */
