/*
 * Skeleton code for CSC 360, Spring 2021,  Assignment #4
 *
 * Prepared by: Michael Zastre (University of Victoria) 2021


 Edited to complete Assignment #4 by: Tim Rolston #V00920780
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/*
 * Some compile-time constants.
 */

#define REPLACE_NONE 0
#define REPLACE_FIFO 1
#define REPLACE_LRU  2
#define REPLACE_SECONDCHANCE 3
#define REPLACE_OPTIMAL 4


#define TRUE 1
#define FALSE 0
#define PROGRESS_BAR_WIDTH 60
#define MAX_LINE_LEN 100


/*
 * Some function prototypes to keep the compiler happy.
 */
int setup(void);
int teardown(void);
int output_report(void);
long resolve_address(long, int);
void error_resolve_address(long, int);

//added functions
void first_in_first_out(int memwrite, long page);
void least_recently_used(int memwrite, long page);
void second_chance(int memwrite, long page);

/*
 * Variables used to keep track of the number of memory-system events
 * that are simulated.
 */
int page_faults = 0;
int mem_refs    = 0;
int swap_outs   = 0;
int swap_ins    = 0;

/*added variables that need to retain their values as more calls
to resolve_address occur - fifo_tracker used for FIFO implementation
victim_frame used for 2ND CHANCE implementation and time_lru used for LRU
implementation*/
    long fifo_tracker = 0;
    long victim_frame = 0;
    long time_lru     = 0;


/*
 * Page-table information. You may want to modify this in order to
 * implement schemes such as SECONDCHANCE. However, you are not required
 * to do so.
 */
struct page_table_entry *page_table = NULL;
struct page_table_entry {
    long page_num;
    int dirty;
    int free;

//to be set to 0 or 1 for 2ND CHANCE
    int reference_bit;
/* a scalable number that represents the least recently used page number in table
ie. relates to when the page was put into the table (for LRU)*/
    long most_frequent;

};


/*
 * These global variables will be set in the main() function. The default
 * values here are non-sensical, but it is safer to zero out a variable
 * rather than trust to random data that might be stored in it -- this
 * helps with debugging (i.e., eliminates a possible source of randomness
 * in misbehaving programs).
 */

int size_of_frame = 0;  /* power of 2 */
int size_of_memory = 0; /* number of frames */
int page_replacement_scheme = REPLACE_NONE;


/*
 * Function to convert a logical address into its corresponding
 * physical address. The value returned by this function is the
 * physical address (or -1 if no physical address can exist for
 * the logical address given the current page-allocation state.
 */

long resolve_address(long logical, int memwrite)
{
    int i;
    long page, frame;
    long offset;
    long mask = 0;
    long effective;

    /* Get the page and offset */
    page = (logical >> size_of_frame);

    for (i=0; i<size_of_frame; i++) {
        mask = mask << 1;
        mask |= 1;
    }
    offset = logical & mask;

    /* Find page in the inverted page table. */
    frame = -1;
    for ( i = 0; i < size_of_memory; i++ ) {
        if (!page_table[i].free && page_table[i].page_num == page) {
            frame = i;
            break;
        }
    }

    /* If frame is not -1, then we can successfully resolve the
     * address and return the result. */
    if (frame != -1) {

    //check if wanting to write to page, if so set dirty bit
      if(memwrite == TRUE){
        page_table[frame].dirty = TRUE;
      }
      //set reference bit indicating slot is full to 1 (for 2nd chance)
        if(page_replacement_scheme == REPLACE_SECONDCHANCE){
          page_table[frame].reference_bit = 1;
        }
      //give page most frequent int to keep track of when placed in table (for LRU)
        if(page_replacement_scheme == REPLACE_LRU){
            page_table[frame].most_frequent = time_lru;
            time_lru++;
        }

        effective = (frame << size_of_frame) | offset;
        return effective;
    }


    /* If we reach this point, there was a page fault. Find
     * a free frame. */
    page_faults++;

    for ( i = 0; i < size_of_memory; i++) {
        if (page_table[i].free) {
            frame = i;
            break;
        }
    }

    /* If we found a free frame, then patch up the
     * page table entry and compute the effective
     * address. Otherwise return -1.
     */
    if (frame != -1) {
        page_table[frame].page_num = page;
        page_table[i].free = FALSE;
        swap_ins++;

      //set reference bit indicating slot is full to 1 (for 2nd chance)
        if(page_replacement_scheme == REPLACE_SECONDCHANCE){
            page_table[frame].reference_bit = 1;
        }
      //give page most frequent int to keep track of when placed in table (for LRU)
        if(page_replacement_scheme == REPLACE_LRU){
            page_table[frame].most_frequent = time_lru;
            time_lru++;
        }

      //check if wanting to write to page, if so set dirty bit
        if(memwrite == TRUE){
          page_table[frame].dirty = TRUE;
        }

        effective = (frame << size_of_frame) | offset;
        return effective;
    } else {

      if (page_replacement_scheme == REPLACE_NONE){
        return -1;
      }

//check if FIFO
      if (page_replacement_scheme == REPLACE_FIFO){
//check if memwrite and dirty bit is set
        /*if (page_table[fifo_tracker].dirty == TRUE){
          swap_outs++;
          page_table[fifo_tracker].dirty = FALSE;
        }
        page_table[fifo_tracker].page_num = page;
        if(memwrite == TRUE){
          page_table[fifo_tracker].dirty = TRUE;
        }
        fifo_tracker++;
        if (fifo_tracker >= size_of_memory){
          fifo_tracker = 0;
        }
        swap_ins++;*/
        first_in_first_out(memwrite, page);
        effective = (fifo_tracker << size_of_frame) | offset;
        return effective;
      }

//check if LRU
      if (page_replacement_scheme == REPLACE_LRU){
        /*int temp = page_table[0].most_frequent;
        for (i = 0; i < size_of_memory; i++){
          if (page_table[i].most_frequent < temp){
            temp = page_table[i].most_frequent;
          }
        }
        for(i = 0; i < size_of_memory; i++){
          if(page_table[i].most_frequent == temp){
            if (page_table[i].dirty == TRUE){
              swap_outs++;
              page_table[i].dirty = FALSE;
            }

            page_table[i].page_num = page;
            if (memwrite == TRUE){
              page_table[i].dirty = TRUE;
            }
            page_table[i].most_frequent = time_lru;
            time_lru++;
            swap_ins++;
            least_recently_used(memwrite, page);
            effective = (i << size_of_frame) | offset;
            return effective;
          }
        }*/
        least_recently_used(memwrite, page);
        effective = (i << size_of_frame) | offset;
        return effective;
      }

//check if 2ND CHANCE
      if (page_replacement_scheme == REPLACE_SECONDCHANCE){
/* loop through page table starting at  frame find any reference bit set to 0 and replace with new page
if reference bit is 1 set to 0*/
        /*while(victim_frame < size_of_memory + 1){
          if(victim_frame >= size_of_memory){
            victim_frame = 0;
          }
//check if reference bit is set to 0 - if so then swap page in
          if (page_table[victim_frame].reference_bit == 0){
//check if memwrite and dirty bit is set
            if (page_table[victim_frame].dirty == TRUE){
              swap_outs++;
              page_table[victim_frame].dirty = FALSE;
            }
            page_table[victim_frame].page_num = page;
            if (memwrite == TRUE){
              page_table[victim_frame].dirty = TRUE;
            }
            page_table[victim_frame].reference_bit = 1;
            swap_ins++;
            effective = (victim_frame << size_of_frame) | offset;
            victim_frame++;
            return effective;
          }
//check if current page reference bit is 1 - if so then set to 0 for later swapping
          if (page_table[victim_frame].reference_bit == 1){
            page_table[victim_frame].reference_bit = 0;
            victim_frame++;
          }
        }*/
        second_chance(memwrite, page);
        effective = (victim_frame << size_of_frame) | offset;
        victim_frame++;
        return effective;
      }
        return -1;
    }
}

//Function for FIFO procedure
void first_in_first_out(int memwrite, long page){

  //check for set dirty bit for swap_out increment
  if (page_table[fifo_tracker].dirty == TRUE){
    swap_outs++;
    page_table[fifo_tracker].dirty = FALSE;
  }
  //remove next page in the table
  page_table[fifo_tracker].page_num = page;

  //check if wanting to write to page, if so set dirty bit
  if (memwrite == TRUE){
    page_table[fifo_tracker].dirty = TRUE;
  }
  //add to the index tracker and ensure it restarts at 0 when equal to size of memory
  fifo_tracker++;
  if (fifo_tracker >= size_of_memory){
    fifo_tracker = 0;
  }
  swap_ins++;
  return;

}

//Function for LRU procedure
void least_recently_used(int memwrite, long page){

  int i;
  int temp = page_table[0].most_frequent;
  /*loop through page table to find lowest value for most_frequent, that value
  will be the least recently used*/
  for (i = 0; i < size_of_memory; i++){
    if (page_table[i].most_frequent < temp){
      temp = page_table[i].most_frequent;
    }
  }
  //find the least recently used page in the table and replace it
  for (i = 0; i < size_of_memory; i++){
    if (page_table[i].most_frequent == temp){

      //check for set dirty bit for swap_out increment
      if (page_table[i].dirty == TRUE){
        swap_outs++;
        page_table[i].dirty = FALSE;
      }
      page_table[i].page_num = page;
      //check if wanting to write to page, if so set dirty bit
      if (memwrite == TRUE){
        page_table[i].dirty = TRUE;
      }
      //updating the new page's time added and incrementing variable for next use
      page_table[i].most_frequent = time_lru;
      time_lru++;
      swap_ins++;
      return;
    }
  }
}

//Function for 2ND CHANCE procedure
void second_chance(int memwrite, long page){

  //loop to continuously traverse through page table
  while (victim_frame < size_of_memory + 1){
    if (victim_frame >= size_of_memory){
      victim_frame = 0;
    }
    //check if reference bit is set to 0 - if so then swap page in
    if (page_table[victim_frame].reference_bit == 0){

      //check for set dirty bit for swap_out increment
      if (page_table[victim_frame].dirty == TRUE){
        swap_outs++;
        page_table[victim_frame].dirty = FALSE;
      }
      page_table[victim_frame].page_num = page;
      //check if wanting to write to page, if so set dirty bit
      if (memwrite == TRUE){
        page_table[victim_frame].dirty = TRUE;
      }
      page_table[victim_frame].reference_bit = 1;
      swap_ins++;
      return;
    }
    //check if current page reference bit is 1 - if so then set to 0 for later swapping
    if (page_table[victim_frame].reference_bit == 1){
      page_table[victim_frame].reference_bit = 0;
      victim_frame++;
    }
  }
}


/*
 * Super-simple progress bar.
 */
void display_progress(int percent)
{
    int to_date = PROGRESS_BAR_WIDTH * percent / 100;
    static int last_to_date = 0;
    int i;

    if (last_to_date < to_date) {
        last_to_date = to_date;
    } else {
        return;
    }

    printf("Progress [");
    for (i=0; i<to_date; i++) {
        printf(".");
    }
    for (; i<PROGRESS_BAR_WIDTH; i++) {
        printf(" ");
    }
    printf("] %3d%%", percent);
    printf("\r");
    fflush(stdout);
}


int setup()
{
    int i;

    page_table = (struct page_table_entry *)malloc(
        sizeof(struct page_table_entry) * size_of_memory
    );

    if (page_table == NULL) {
        fprintf(stderr,
            "Simulator error: cannot allocate memory for page table.\n");
        exit(1);
    }

    for (i=0; i<size_of_memory; i++) {
        page_table[i].free = TRUE;
    }

    return -1;
}


int teardown()
{

    return -1;
}


void error_resolve_address(long a, int l)
{
    fprintf(stderr, "\n");
    fprintf(stderr,
        "Simulator error: cannot resolve address 0x%lx at line %d\n",
        a, l
    );
    exit(1);
}


int output_report()
{
    printf("\n");
    printf("Memory references: %d\n", mem_refs);
    printf("Page faults: %d\n", page_faults);
    printf("Swap ins: %d\n", swap_ins);
    printf("Swap outs: %d\n", swap_outs);

    return -1;
}


int main(int argc, char **argv)
{
    /* For working with command-line arguments. */
    int i;
    char *s;

    /* For working with input file. */
    FILE *infile = NULL;
    char *infile_name = NULL;
    struct stat infile_stat;
    int  line_num = 0;
    int infile_size = 0;

    /* For processing each individual line in the input file. */
    char buffer[MAX_LINE_LEN];
    long addr;
    char addr_type;
    int  is_write;

    /* For making visible the work being done by the simulator. */
    int show_progress = FALSE;

    /* Process the command-line parameters. Note that the
     * REPLACE_OPTIMAL scheme is not required for A#3.
     */
    for (i=1; i < argc; i++) {
        if (strncmp(argv[i], "--replace=", 9) == 0) {
            s = strstr(argv[i], "=") + 1;
            if (strcmp(s, "fifo") == 0) {
                page_replacement_scheme = REPLACE_FIFO;
            } else if (strcmp(s, "lru") == 0) {
                page_replacement_scheme = REPLACE_LRU;
            } else if (strcmp(s, "secondchance") == 0) {
                page_replacement_scheme = REPLACE_SECONDCHANCE;
            } else if (strcmp(s, "optimal") == 0) {
                page_replacement_scheme = REPLACE_OPTIMAL;
            } else {
                page_replacement_scheme = REPLACE_NONE;
            }
        } else if (strncmp(argv[i], "--file=", 7) == 0) {
            infile_name = strstr(argv[i], "=") + 1;
        } else if (strncmp(argv[i], "--framesize=", 12) == 0) {
            s = strstr(argv[i], "=") + 1;
            size_of_frame = atoi(s);
        } else if (strncmp(argv[i], "--numframes=", 12) == 0) {
            s = strstr(argv[i], "=") + 1;
            size_of_memory = atoi(s);
        } else if (strcmp(argv[i], "--progress") == 0) {
            show_progress = TRUE;
        }
    }

    if (infile_name == NULL) {
        infile = stdin;
    } else if (stat(infile_name, &infile_stat) == 0) {
        infile_size = (int)(infile_stat.st_size);
        /* If this fails, infile will be null */
        infile = fopen(infile_name, "r");
    }


    if (page_replacement_scheme == REPLACE_NONE ||
        size_of_frame <= 0 ||
        size_of_memory <= 0 ||
        infile == NULL)
    {
        fprintf(stderr,
            "usage: %s --framesize=<m> --numframes=<n>", argv[0]);
        fprintf(stderr,
            " --replace={fifo|lru|optimal} [--file=<filename>]\n");
        exit(1);
    }


    setup();

    while (fgets(buffer, MAX_LINE_LEN-1, infile)) {
        line_num++;
        if (strstr(buffer, ":")) {
            sscanf(buffer, "%c: %lx", &addr_type, &addr);
            if (addr_type == 'W') {
                is_write = TRUE;
            } else {
                is_write = FALSE;
            }

            if (resolve_address(addr, is_write) == -1) {
                error_resolve_address(addr, line_num);
            }
            mem_refs++;
        }

        if (show_progress) {
            display_progress(ftell(infile) * 100 / infile_size);
        }
    }


    teardown();
    output_report();

    fclose(infile);

    exit(0);
}
