/*
 * Garrett_paging.c
 * Simple Paging Address Translation
 * Author: Garrett
 *
 * Page size: 1024 bytes | Physical frames: 16
 * Page table: page 0→frame 5, page 1→frame 2, page 2→frame 9, page 3→frame 1
 */

#include <stdio.h>

#define PAGE_SIZE     1024
#define NUM_PAGES     4

/* Fixed page table: index = page number, value = frame number */
int page_table[NUM_PAGES] = {5, 2, 9, 1};

int main(void) {
    int n;

    printf("Enter number of logical addresses (N): ");
    scanf("%d", &n);

    printf("Enter logical address(es), one per line:\n");

    for (int i = 0; i < n; i++) {
        int logical;
        scanf("%d", &logical);

        int page   = logical / PAGE_SIZE;
        int offset = logical % PAGE_SIZE;

        if (page < 0 || page >= NUM_PAGES) {
            printf("Logical: %d | INVALID (page out of range)\n", logical);
        } else {
            int frame    = page_table[page];
            int physical = frame * PAGE_SIZE + offset;
            printf("Logical: %d | Page: %d | Offset: %d | Frame: %d | Physical: %d\n",
                   logical, page, offset, frame, physical);
        }
    }

    return 0;
}
