#include <cstdio>
#include <iostream>
#include <thread>

#include <sys/mman.h>

extern "C" {
#include "libdune/dune.h"
}

using namespace std;

volatile bool swap_mappings = false;
bool tlb_shootdown = false;

static void pgflt_handler(uintptr_t addr, uint64_t fec, struct dune_tf *tf)
{
	ptent_t *pte;
	dune_vm_lookup(pgroot, (void *) addr, CREATE_NORMAL, &pte);
	if (addr >= mmap_base && addr < (mmap_base + 2 * PGSIZE)) {
		if (!swap_mappings) {
			*pte = PTE_P | PTE_W | PTE_ADDR(dune_va_to_pa((void *)addr));
		} else {
			uintptr_t va = (addr - mmap_base) < PGSIZE ?
				addr + PGSIZE : addr - PGSIZE;
			*pte = PTE_P | PTE_W | PTE_ADDR(dune_va_to_pa((void *)va));
			if (tlb_shootdown) {
				dune_tlb_shootdown();
			}
		}
	} else {
		*pte = PTE_P | PTE_W | PTE_ADDR(dune_va_to_pa((void *)addr));
	}
}

static void init_dune()
{
	int err = dune_init(0);
	if (err) {
		cerr << "Dune init error!" << endl;
		exit(err);
	}
	dune_register_pgflt_handler(pgflt_handler);
}

void print_thread_view(const char *thread)
{
    cout << thread << " thread view:" << endl;
	uintptr_t p = mmap_base;
	cout << "\t" << (void *)p << " ==> " << (char *)p << endl;
	p += PGSIZE;
	cout << "\t" << (void *)p << " ==> " << (char *)p << endl;
}

void thread_work()
{
	dune_enter();

	assert(!swap_mappings);
	print_thread_view("another");

	swap_mappings = true;
	while (swap_mappings) { }

	print_thread_view("another");
}

void main_work() {
	assert(!swap_mappings);
	print_thread_view("main");

	while (!swap_mappings) { }

	dune_vm_unmap(pgroot, (void *)mmap_base, 2 * PGSIZE);
	cout << "(the main thread swaps the mappings...)" << endl;
	print_thread_view("main");

	swap_mappings = false;
}

int main(int argc, char *argv[])
{
	init_dune();

	if (mmap((void *)PGADDR(mmap_base), 2 * PGSIZE,
		PROT_READ|PROT_WRITE, MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS,
		-1, 0) != (void *)mmap_base) {
		cerr << "mmap failed for " << (void *)mmap_base << endl;
		return errno;
	}

	uintptr_t p = mmap_base;
	sprintf((char *)p, "guest physical page %d", 0);
	p += PGSIZE;
	sprintf((char *)p, "guest physical page %d", 1);

	int err = dune_enter();
	if (err){
		cerr << "Dune enter error!" << endl;
		return err;
	}

	cout << "Without TLB shootdown:" << endl;
	thread tx = thread(thread_work);
	main_work();
	tx.join();

	dune_vm_unmap(pgroot, (void *)mmap_base, 2 * PGSIZE);

	cout << endl << "With TLB shootdown:" << endl;
	tlb_shootdown = true;
	tx = thread(thread_work);
	main_work();
	tx.join();

	return 0;
}

