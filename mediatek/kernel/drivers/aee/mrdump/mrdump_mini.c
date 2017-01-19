#include <linux/init.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/memblock.h>
#include <linux/elf.h>
#include <linux/kdebug.h>
#include <linux/module.h>
#include <linux/vmalloc.h>
#include <mach/smp.h>
#include <asm/pgtable.h>
#include <asm-generic/percpu.h>
#include <asm-generic/sections.h>
#include <linux/mrdump.h>
#include <linux/aee.h>
#include "../../../../kernel/sched/sched.h"

#define LOG_DEBUG(fmt, ...)			\
	if (aee_in_nested_panic())			\
		aee_nested_printf(fmt, ##__VA_ARGS__);	\
	else						\
		pr_debug(fmt, ##__VA_ARGS__)

#define LOG_ERROR(fmt, ...)			\
	if (aee_in_nested_panic())			\
		aee_nested_printf(fmt, ##__VA_ARGS__);	\
	else						\
		pr_err(fmt, ##__VA_ARGS__)

#define LOGV(fmt, msg...)
#define LOGD LOGV
#define LOGI LOG_DEBUG
#define LOGW LOG_ERROR
#define LOGE LOG_ERROR

extern void get_kernel_log_buffer(unsigned long *addr, unsigned long *size, unsigned long *start);
extern void get_android_log_buffer(unsigned long *addr, unsigned long *size, unsigned long *start, int type);
extern struct ram_console_buffer *ram_console_buffer;
static struct mrdump_mini_elf_header *mrdump_mini_ehdr;

__weak struct vm_struct *find_vm_area(const void *addr)
{
	return NULL;
}

/* copy from fs/binfmt_elf.c */
static void fill_elf_header(struct elfhdr *elf, int segs)
{
	memcpy(elf->e_ident, ELFMAG, SELFMAG);
	elf->e_ident[EI_CLASS] = ELF_CLASS;
	elf->e_ident[EI_DATA] = ELF_DATA;
	elf->e_ident[EI_VERSION] = EV_CURRENT;
	elf->e_ident[EI_OSABI] = ELF_OSABI;

	elf->e_type = ET_CORE;
	elf->e_machine = ELF_ARCH;
	elf->e_version = EV_CURRENT;
	elf->e_phoff = sizeof(struct elfhdr);
#ifndef ELF_CORE_EFLAGS
#define ELF_CORE_EFLAGS	0
#endif	
	elf->e_flags = ELF_CORE_EFLAGS;
	elf->e_ehsize = sizeof(struct elfhdr);
	elf->e_phentsize = sizeof(struct elf_phdr);
	elf->e_phnum = segs;

	return;
}

static void fill_elf_note_phdr(struct elf_phdr *phdr, int sz, loff_t offset)
{
	phdr->p_type = PT_NOTE;
	phdr->p_offset = offset;
	phdr->p_vaddr = 0;
	phdr->p_paddr = 0;
	phdr->p_filesz = sz;
	phdr->p_memsz = 0;
	phdr->p_flags = 0;
	phdr->p_align = 0;
	return;
}

static void fill_elf_load_phdr(struct elf_phdr *phdr, int sz,
			       unsigned long vaddr, unsigned long paddr)
{
	phdr->p_type = PT_LOAD;
	phdr->p_vaddr = vaddr;
	phdr->p_paddr = paddr;
	phdr->p_filesz = sz;
	phdr->p_memsz = 0;
	phdr->p_flags = 0;
	phdr->p_align = 0;
	return;
}

static void fill_note(struct elf_note *note, const char *name, int type, 
		      unsigned int sz)
{
	char *n_name = (char*)note + sizeof(struct elf_note);
	strncpy(n_name, name, 8);
	note->n_namesz = 8;
	note->n_type = type;
	note->n_descsz = sz;
	return;
}

static void fill_note16(struct elf_note *note, const char *name, int type, 
			unsigned int sz)
{
	char *n_name = (char*)note + sizeof(struct elf_note);
	strncpy(n_name, name, 16);
	note->n_namesz = 16;
	note->n_type = type;
	note->n_descsz = sz;
	return;
}
/*
 * fill up all the fields in prstatus from the given task struct, except
 * registers which need to be filled up separately.
 */
static void fill_prstatus(struct elf_prstatus *prstatus, struct pt_regs *regs,
			  struct task_struct *p, unsigned long pid)
{
	elf_core_copy_regs(&prstatus->pr_reg, regs);
	prstatus->pr_pid = pid;
	prstatus->pr_ppid = NR_CPUS;
	return;
}

static int fill_psinfo(struct elf_prpsinfo *psinfo)
{
	unsigned int i;
	strncpy(psinfo->pr_psargs, saved_command_line, ELF_PRARGSZ-1);
	for(i = 0; i < ELF_PRARGSZ - 1; i++)
		if (psinfo->pr_psargs[i] == 0)
			psinfo->pr_psargs[i] = ' ';
	psinfo->pr_psargs[ELF_PRARGSZ - 1] = 0;
	strncpy(psinfo->pr_fname, "vmlinux", sizeof(psinfo->pr_fname));
	return 0;
}

void mrdump_mini_add_misc(unsigned long addr, unsigned long size, unsigned long start, char *name)
{
	int i;
	struct elf_note *note;
	if (!virt_addr_valid((void *)addr))
		return;
	for (i = 0; i < MRDUMP_MINI_NR_MISC; i++) {
		note = &mrdump_mini_ehdr->misc[i].note;
		if (note->n_type == NT_IPANIC_MISC) {
			if (strncmp(mrdump_mini_ehdr->misc[i].name, name, 16) != 0)
				continue;
		}
		mrdump_mini_ehdr->misc[i].data.vaddr = addr;
		mrdump_mini_ehdr->misc[i].data.paddr = __pa(addr);
		mrdump_mini_ehdr->misc[i].data.size = size;
		mrdump_mini_ehdr->misc[i].data.start = virt_addr_valid((void *)start) ? __pa(start) : 0;
		fill_note16(note, name, NT_IPANIC_MISC, sizeof(struct mrdump_mini_elf_misc));
		break;
	}
}

int kernel_addr_valid(unsigned long addr)
{
	pgd_t *pgd;
	pud_t *pud;
	pmd_t *pmd;
	pte_t *pte;

	if (addr < PAGE_OFFSET)
		return 0;

	pgd = pgd_offset_k(addr);
	if (pgd_none(*pgd))
		return 0;
	pr_err("[%08lx] *pgd=%08llx", addr, (long long)pgd_val(*pgd));

	pud = pud_offset(pgd, addr);
	if (pud_none(*pud))
		return 0;
	pr_err("*pud=%08llx", (long long)pud_val(*pud));

	pmd = pmd_offset(pud, addr);
	if (pmd_none(*pmd))
		return 0;
	pr_err("*pmd=%08llx", (long long)pmd_val(*pmd));

	pte = pte_offset_kernel(pmd, addr);
	if (pte_none(*pte))
		return 0;
	pr_err("*pte=%08llx", (long long)pte_val(*pte));

	return pfn_valid(pte_pfn(*pte));
}

void mrdump_mini_add_entry(unsigned long addr, unsigned long size)
{
	struct elf_phdr *phdr;
	//struct vm_area_struct *vma;
	struct vm_struct *vm;
	unsigned long laddr, haddr, lnew, hnew;
	unsigned long paddr;
	int i;
	if (addr < PAGE_OFFSET)
		return;
	hnew = ALIGN(addr + size / 2, PAGE_SIZE);
	lnew = hnew - ALIGN(size, PAGE_SIZE);
	if (!virt_addr_valid(addr)) {
		/* vma = find_vma(&init_mm, addr); */
		/* pr_err("mirdump: add: %p, vma: %x", addr, vma); */
		/* if (!vma) */
		/* 	return; */
		/* pr_err("mirdump: (%p, %p), (%p, %p)", vma->vm_start, vma->vm_end, lnew, hnew);		 */
		/* hnew = min(vma->vm_end, hnew); */
		/* lnew = max(vma->vm_start, lnew); */
		vm = find_vm_area((void*)addr);
		if (!vm)
			return;
		/* lnew = max((unsigned long)vm->addr, lnew); */
		/* hnew = min((unsigned long)vm->addr + vm->size - PAGE_SIZE, hnew); */
		/* only dump 1 page */
		lnew = max((unsigned long)vm->addr, PAGE_ALIGN(addr) - PAGE_SIZE);
		hnew = lnew + PAGE_SIZE;
		paddr = __pfn_to_phys(vmalloc_to_pfn((void*)lnew));
	} else {
		lnew = max(lnew, PAGE_OFFSET);
		hnew = min(hnew, (unsigned long)high_memory);
		paddr = __pa(lnew);
	}
	for (i = 0; i < MRDUMP_MINI_NR_SECTION; i++) {
		phdr = &mrdump_mini_ehdr->phdrs[i];
		if (phdr->p_type == PT_NULL)
			break;
		if (phdr->p_type != PT_LOAD)
			continue;
		laddr = phdr->p_vaddr;
		haddr = laddr + phdr->p_filesz;
		/* full overlap with exist */
		if (lnew >= laddr && hnew <= haddr)
			return;
		/* no overlap, new */
		if (lnew >= haddr || hnew <= laddr)
			continue;
		/* partial overlap with exist, joining */
		lnew = lnew < laddr ? lnew : laddr;
		hnew = hnew > haddr ? hnew : haddr;
		paddr = __pa(lnew);
		break;
	}
	if (i < MRDUMP_MINI_NR_SECTION)
		fill_elf_load_phdr(phdr, hnew - lnew, lnew, paddr);
}

static void mrdump_mini_add_tsk_ti(int cpu, struct pt_regs *regs, int stack)
{
	struct task_struct *tsk = NULL;
	struct thread_info *ti = NULL;
	unsigned long *bottom = NULL;
	unsigned long *top = NULL;
	unsigned long *p;
	if (virt_addr_valid(regs->ARM_sp)) {
		ti = (struct thread_info *)(regs->ARM_sp & ~(THREAD_SIZE - 1));
		tsk = ti->task;
		bottom = (unsigned long *)regs->ARM_sp;
	}
	if (!(virt_addr_valid(tsk) && ti == (struct thread_info *)tsk->stack) && virt_addr_valid(regs->ARM_fp)) {
		ti = (struct thread_info *)(regs->ARM_fp & ~(THREAD_SIZE - 1));
		tsk = ti->task;
		bottom = (unsigned long *)regs->ARM_fp;
	}
	if (!virt_addr_valid(tsk) || ti != (struct thread_info *)tsk->stack) {
		tsk = cpu_curr(cpu);
		if (virt_addr_valid(tsk)) {
			ti = (struct thread_info *)tsk->stack;
			bottom = (unsigned long*)((void*)ti + sizeof(struct thread_info));
		}
	}

	mrdump_mini_add_entry(regs->ARM_sp, MRDUMP_MINI_SECTION_SIZE);
	mrdump_mini_add_entry((unsigned long)ti, MRDUMP_MINI_SECTION_SIZE);
	mrdump_mini_add_entry((unsigned long)tsk, MRDUMP_MINI_SECTION_SIZE);
	LOGE("mrdump: cpu[%d] tsk:%p ti:%p\n", cpu,  tsk, ti);
	if (!stack)
		return;
	top = (unsigned long *)((void*)ti + THREAD_SIZE);
	if (!virt_addr_valid(ti) || ! virt_addr_valid(top) || bottom < (unsigned long *)ti || bottom > top)
		return;

	for (p = (unsigned long *)ALIGN((unsigned long)bottom, sizeof(unsigned long)); p < top; p++) {
		if (!virt_addr_valid(*p))
			continue;
		if (*p >= (unsigned long)ti && *p <= (unsigned long)top)
			continue;
		if (*p >= (unsigned long)_stext && *p <= (unsigned long)_etext)
			continue;
		mrdump_mini_add_entry(*p, MRDUMP_MINI_SECTION_SIZE);
	}
}

static int mrdump_mini_cpu_regs(int cpu, struct pt_regs *regs, int main)
{
	char name[8];
	int id;
	if (cpu >= NR_CPUS)
		return -1;
	id = main ? 0 : cpu + 1;
	if (strncmp(mrdump_mini_ehdr->prstatus[id].name, "NA", 2))
		return -1;
	snprintf(name, 8, main ? "ke%d" : "core%d", cpu);
	fill_prstatus(&mrdump_mini_ehdr->prstatus[id].data, regs, 0, id ? id : (100 + cpu));
	fill_note(&mrdump_mini_ehdr->prstatus[id].note, name, NT_PRSTATUS, sizeof(struct elf_prstatus));
	return 0;
}

void mrdump_mini_per_cpu_regs(int cpu, struct pt_regs *regs)
{
	mrdump_mini_cpu_regs(cpu, regs, 0);
}
EXPORT_SYMBOL(mrdump_mini_per_cpu_regs);

static void mrdump_mini_add_loads(void);
void mrdump_mini_ke_cpu_regs(struct pt_regs *regs)
{
	int cpu;
	cpu = get_HW_cpuid();
	mrdump_mini_cpu_regs(cpu, regs, 1);
	mrdump_mini_add_loads();
}
EXPORT_SYMBOL(mrdump_mini_ke_cpu_regs);

static void mrdump_mini_build_elf_misc(void)
{
	int i;
	struct mrdump_mini_elf_misc misc;
	char log_type[][16] = {"_MAIN_LOG_", "_EVENTS_LOG_", "_RADIO_LOG_", "_SYSTEM_LOG_"};
	memset(&misc, 0, sizeof(struct mrdump_mini_elf_misc));
	get_kernel_log_buffer(&misc.vaddr, &misc.size, &misc.start);
	mrdump_mini_add_misc(misc.vaddr, misc.size, misc.start, "_KERNEL_LOG_");
	for (i = 0; i < 4; i++) {
		memset(&misc, 0, sizeof(struct mrdump_mini_elf_misc));
		get_android_log_buffer(&misc.vaddr, &misc.size, &misc.start, i + 1);
		mrdump_mini_add_misc(misc.vaddr, misc.size, misc.start, log_type[i]);
	}
}

static void mrdump_mini_add_loads(void)
{
	int cpu, i, id;
	struct pt_regs regs;
	struct elf_prstatus *prstatus;
	for (id = 0; id < NR_CPUS + 1; id++) {
		if (!strncmp(mrdump_mini_ehdr->prstatus[id].name, "NA", 2))
			continue;
		prstatus = &mrdump_mini_ehdr->prstatus[id].data;
		memcpy(&regs, &prstatus->pr_reg, sizeof(prstatus->pr_reg));
		if (prstatus->pr_pid >= 100) {
			for (i = 0; i < ELF_NGREG; i++)
				mrdump_mini_add_entry(((unsigned long*)&regs)[i], MRDUMP_MINI_SECTION_SIZE);
			cpu = prstatus->pr_pid - 100;
			mrdump_mini_add_tsk_ti(cpu, &regs, 1);
			mrdump_mini_add_entry((unsigned long)cpu_rq(cpu), MRDUMP_MINI_SECTION_SIZE);
		} else {
			cpu= prstatus->pr_pid - 1;
			mrdump_mini_add_tsk_ti(cpu, &regs, 0);
		}
	}
	
	mrdump_mini_add_entry((unsigned long)__per_cpu_offset, MRDUMP_MINI_SECTION_SIZE);
	mrdump_mini_add_entry((unsigned long)&mem_map, MRDUMP_MINI_SECTION_SIZE);
	mrdump_mini_add_entry((unsigned long)mem_map, MRDUMP_MINI_SECTION_SIZE);
}

static void mrdump_mini_dump_loads(loff_t offset, mrdump_write write)
{
	int errno;
	unsigned long start, size;
	int i;
	struct elf_phdr *phdr;
	loff_t pos = MRDUMP_MINI_HEADER_SIZE;
	for (i = 0; i < MRDUMP_MINI_NR_SECTION; i++) {
		phdr = &mrdump_mini_ehdr->phdrs[i];
		if (phdr->p_type == PT_NULL)
			break;
		if (phdr->p_type == PT_LOAD) {
			//mrdump_mini_dump_phdr(phdr, &pos);
			start = phdr->p_vaddr;
			size = ALIGN(phdr->p_filesz, SZ_512);
			phdr->p_offset = pos;
			errno = write((void*)start, pos + offset, size, 1);
			pos += size;
			if (IS_ERR(ERR_PTR(errno))) {
				LOGD("mirdump: write fail");
			}
		}
	}
}

int mrdump_mini_create_oops_dump(AEE_REBOOT_MODE reboot_mode, mrdump_write write,
				 loff_t sd_offset, const char *msg, va_list ap)
{
	mrdump_mini_dump_loads(sd_offset, write);
	write((void*)mrdump_mini_ehdr, sd_offset, MRDUMP_MINI_HEADER_SIZE, 1);
	return MRDUMP_MINI_BUF_SIZE;
}
EXPORT_SYMBOL(mrdump_mini_create_oops_dump);

void mrdump_mini_ipanic_done(void)
{
	mrdump_mini_ehdr->ehdr.e_ident[0] = 0;
}

static void __init *remap_lowmem(phys_addr_t start, phys_addr_t size)
{
	struct page **pages;
	phys_addr_t page_start;
	unsigned int page_count;
	pgprot_t prot;
	unsigned int i;
	void *vaddr;

	page_start = start - offset_in_page(start);
	page_count = DIV_ROUND_UP(size + offset_in_page(start), PAGE_SIZE);

	prot = pgprot_noncached(PAGE_KERNEL);

	pages = kmalloc(sizeof(struct page *) * page_count, GFP_KERNEL);
	if (!pages) {
		LOGE("%s: Failed to allocate array for %u pages\n", __func__, page_count);
		return NULL;
	}

	for (i = 0; i < page_count; i++) {
		phys_addr_t addr = page_start + i * PAGE_SIZE;
		pages[i] = pfn_to_page(addr >> PAGE_SHIFT);
	}
	vaddr = vmap(pages, page_count, VM_MAP, prot);
	kfree(pages);
	if (!vaddr) {
		LOGE("%s: Failed to map %u pages\n", __func__, page_count);
		return NULL;
	}

	return vaddr + offset_in_page(start);
}

static void __init mrdump_mini_elf_header_init(void)
{
	mrdump_mini_ehdr = remap_lowmem(MRDUMP_MINI_BUF_PADDR, MRDUMP_MINI_HEADER_SIZE);
	if (mrdump_mini_ehdr == NULL) {
		LOGE("mrdump mini reserve buffer fail");
		return;
	}
	LOGE("mirdump: reserved %x+%lx->%p", MRDUMP_MINI_BUF_PADDR, (unsigned long)MRDUMP_MINI_HEADER_SIZE, mrdump_mini_ehdr);
	memset(mrdump_mini_ehdr, 0, MRDUMP_MINI_HEADER_SIZE);
	fill_elf_header(&mrdump_mini_ehdr->ehdr, MRDUMP_MINI_NR_SECTION);
}

int __init mrdump_mini_init(void)
{
	int i;
	unsigned long size, offset;
	struct pt_regs regs;

	mrdump_mini_elf_header_init();

	fill_psinfo(&mrdump_mini_ehdr->psinfo.data);
	fill_note(&mrdump_mini_ehdr->psinfo.note, "vmlinux", NT_PRPSINFO, sizeof(struct elf_prpsinfo));

	memset(&regs, 0, sizeof(struct pt_regs));
	for (i = 0; i < NR_CPUS + 1; i++) {
		fill_prstatus(&mrdump_mini_ehdr->prstatus[i].data, &regs, 0, i);
		fill_note(&mrdump_mini_ehdr->prstatus[i].note, "NA", NT_PRSTATUS, sizeof(struct elf_prstatus));
	}

	offset = offsetof(struct mrdump_mini_elf_header, psinfo);
	size = sizeof(mrdump_mini_ehdr->psinfo) + sizeof(mrdump_mini_ehdr->prstatus);
	fill_elf_note_phdr(&mrdump_mini_ehdr->phdrs[0], size, offset);

	for (i = 0; i < MRDUMP_MINI_NR_MISC; i++)
		fill_note16(&mrdump_mini_ehdr->misc[i].note, "NA", 0, sizeof(struct mrdump_mini_elf_misc));
	mrdump_mini_build_elf_misc();
	fill_elf_note_phdr(&mrdump_mini_ehdr->phdrs[1], sizeof(mrdump_mini_ehdr->misc),
			   offsetof(struct mrdump_mini_elf_header, misc));
	
	return 0;
}
module_init(mrdump_mini_init);

void mrdump_mini_reserve_memory(void)
{
	memblock_reserve(MRDUMP_MINI_BUF_PADDR, MRDUMP_MINI_HEADER_SIZE);
}
