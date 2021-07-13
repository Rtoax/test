#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/list.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <asm/pgtable_types.h>
#include <asm/pgtable.h>

static void cycle_all_tasks(void);
static void print_proc_stats(struct task_struct *);
static void get_phys_addr(void);
static unsigned long virt2phys(struct mm_struct *, unsigned long);

int proc_init (void) {
  printk(KERN_INFO "proc_report: kernel module initialized\n");
  printk(KERN_INFO "proc_id,proc_name,contig_pages,noncontig_pages,total_pages\n");
  cycle_all_tasks();
  //create_proc_entry("proc_report", &proc_root);
  
  return 0;
}

void proc_cleanup(void) {
  printk(KERN_INFO "proc_report: performing cleanup of module\n");
}

static void cycle_all_tasks() {
    struct task_struct *task;
    int curr_contig = 0, curr_ncontig = 0, curr_total = 0, contig = 0, ncontig = 0, total = 0;
    
    for_each_process(task) {
        if(task->pid > 649) {
            printk(KERN_INFO "%d,%s,%d,%d,%d\n", task->pid, task->comm, curr_contig, curr_ncontig, curr_total);
        }
    }
    printk(KERN_INFO "TOTALS,,%d,%d,%d", contig, ncontig, total);
}

static void print_proc_stats(struct task_struct *task) {
    printk(KERN_INFO "proc_report: %d,%s,contig,ncontig,tot\n", task->pid, task->comm);
}

static void get_phys_addr() {
    struct task_struct *task = current->parent;
    struct vm_area_struct *vma = 0;
    unsigned long vpage, physical_page_addr;
    if (task->mm && task->mm->mmap)
        for (vma = task->mm->mmap; vma; vma = vma->vm_next)
            for (vpage = vma->vm_start; vpage < vma->vm_end; vpage += PAGE_SIZE) {
                physical_page_addr = virt2phys(task->mm, vpage);
                printk(KERN_INFO "Addr: %ld\n", physical_page_addr);
            }
}

static unsigned long virt2phys(struct mm_struct *mm, unsigned long vpage) {
    pgd_t *pgd;
    p4d_t *p4d;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *pte;
    unsigned long physical_page_addr;
    struct page *page;
    pgd = pgd_offset(mm, vpage);
    if (pgd_none(*pgd) || pgd_bad(*pgd))
        return 0;
    p4d = p4d_offset(pgd, vpage);
    if (p4d_none(*p4d) || p4d_bad(*p4d))
        return 0;
    pud = pud_offset(p4d, vpage);
    if (pud_none(*pud) || pud_bad(*pud))
        return 0;
    pmd = pmd_offset(pud, vpage);
    if (pmd_none(*pmd) || pmd_bad(*pmd))
        return 0;
    if (!(pte = pte_offset_map(pmd, vpage)))
        return 0;
    if (!(page = pte_page(*pte)))
        return 0;
    physical_page_addr = page_to_phys(page);
    pte_unmap(pte);
    return physical_page_addr;
}

MODULE_LICENSE("GPL");
module_init(proc_init);
module_exit(proc_cleanup);
