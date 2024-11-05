
 * This is a C implementation of malloc( ) and free( ), based on the buddy
 * memory allocation algorithm.
 *
 * Rename this file to heap.c before adding your comments
 *
 * Compile: gcc heap.c driver_cpg.c
 * Execute: ./a.out
 *
 */
#include <assert.h>
#include <stdio.h> // printf
/*
 * The following global variables are used to simulate memory allocation
 * Cortex-M's SRAM space.
 */
// Heap
char array[0x8000];          // simulate SRAM: 0x2000.0000 - 0x2000.7FFF
int heap_top = 0x20001000;   // the top of heap space
int heap_bot = 0x20004FE0;   // the address of the last 32B in heap
short max_size = 0x00004000; // maximum allocation: 16KB = 2^14
short min_size = 0x00000020; // minimum allocation: 32B = 2^5

// Memory Control Block: 2^10B = 1KB space
int mcb_top = 0x20006800;    // the top of MCB
int mcb_bot = 0x20006BFE;    // the address of the last MCB entry
int mcb_ent_sz = 0x00000002; // 2B per MCB entry
int mcb_total = 512;         // # MCB entries: 2^9 = 512 entries

/*
 * Convert a Cortex SRAM address to the corresponding array index.
 * @param  sram_addr address of Cortex-M's SRAM space starting at 0x20000000.
 * @return array index.
 */
int m2a(int sram_addr) { //stands for memory address to array_index 
  // TODO(student): add comment to each of the following line of code
  int index = sram_addr - 0x20000000;   //calculate index by subtracting the base SRAM address (0x20000000) 
  return index; //returning the calculated index 
}//closing of m2a(int sram_addr) method

/*
 * Reverse an array index back to the corresponding Cortex SRAM address.
 * @param  array index.
 * @return the corresponding Cortex-M's SRAM address in an integer.
 */
int a2m(int array_index) { // stands for array_index to memory address 
  //  TODO(student): add comment to each of the following line of code
  return array_index + 0x20000000; //Convert index of array back to memory address by adding base SRAM address (0x20000000)
}//closing of a2m(int array_index) method 

/*
 * In case if you want to print out, all array elements that correspond
 * to MCB: 0x2006800 - 0x20006C00.
 */
void printArray() {
  printf("memory ............................\n");
  // TODO(student): add comment to each of the following line of code
  for (int i = 0; i < 0x8000; i += 4) { //Loops over entire array every 4 bytes 
    if (a2m(i) >= 0x20006800) { // checking the first half of the four half segment 
    // index converted into memory address is greater than the mcb_top or top of memory control block 
      printf("%x = %x(%d)\n", a2m(i), *(short *)&array[i], *(short *)&array[i]); //if so, then print out the memory address (converted from index of array) as a hexadecimal, 
                                                                                 //contents of two consecutive bytes in the array at index i as a hexadecimal, and as a decimal 
    }  //closing the if-block 
    // checking the second half of the four half segment
    if (a2m(i + 2) >= 0x20006800) { // checking if the memory address of index + 2 is within the memory block 
      printf("%x = %x(%d)\n", a2m(i + 2), *(short *)&array[i + 2], *(short *)&array[i + 2]); //if so, then print out memory address (converted from index of array) as a hexadecimal,
                                                                                             //contents of two consecutive bytes in the array at index i as a hexadecimal, and as a decimal
    }//closing of if block 
  } // closing of for loop
} // closing of printArray() method 

/*
 * Print out the memory managed by mcb, showing used segments
 * and the value of the mcb's
 */
void printMemory() {
  printf("memory map ............................\n");
  int segment_size = 512; //segment size for representing memory blocks 
  char memory[0x8000]; //memory array which stimulates a memory map and each element in this array referes to a byte in the simulated memory space.
  for (int i = 0; i < 0x8000; i++) { //traversing thorugh all the elements of the memory array  
    memory[i] = '.'; //setting all elements to . indicates that all the memory is free at this point of the code 
  }//closing of for loop 
  // for each mcb, mark corresponding memory as used
  for (int mcb_addr = mcb_top; mcb_addr <= mcb_bot; mcb_addr += 2) { //for loop traversing through the entire memory control block from the start (mcb_top) 
  //to bottom (mcb_bot). Each memory block entry occupies 2 bytes as indicated by int mcb_ent_sz = 0x00000002 and this loop iterates through each mcb entry. 
  //This loop is used to check if each memory block is "used" or "free".
    if ((*(short *)&array[m2a(mcb_addr)] & 0x01) != 0) {// checking if specific memory block entry of the for loop is used
        //dereferencingg short pointer assigned to element of memort array and does a bitvise AND operation which checks if the least significant bit (LSB) is set 
        //The least significant bit is used to store flags. So if LSB is 1 it means that the memory block is currently in use. 0 indicated free memory block. 
      int heap_val = *(short *)&array[m2a(mcb_addr)]; //extracts a short value from array and stores it in an int variable
      heap_val = (heap_val / 2) * 2; //the block value is stripped of LSB and used to ignore the used bit
      int heap_start = heap_top + (mcb_addr - mcb_top) * 16; //heap range calculated based on used memory blocks 
      int heap_end = heap_start + heap_val; //calculate the end of heap based on start and values of memory control blocks 
      for (int x = heap_start; x < heap_end; x += segment_size) { //iterates through calculated heap range  
        memory[m2a(x)] = 'X'; //sets each segment in memory to 'X' indicating it's in use
      }//end of for loop
    }//end of if statement 
  }//end of for loop 
  printf("mcb\t\tmcb Address\tHeap Address\tUsed?\tmcb Value\n");  //prints out header line for a table of memory control block (MCB) information
  for (int addr = heap_top; addr < heap_bot; addr += segment_size) { //iterates over a range of addresses in the heap  
    int mcb_addr = mcb_top + (addr - heap_top) / 16;
    int mcb_index = (mcb_addr - mcb_top) / mcb_ent_sz;
    int heap_value = *(short *)&array[m2a(mcb_addr)];
    printf("mcb[%3d]\t%x\t%x\t%3c\t%4d\n", mcb_index, mcb_addr, addr,
           memory[m2a(addr)], heap_value);
  }
}

/*
 * _ralloc is _kalloc's helper function that is recursively called to
 * allocate a requested space, using the buddy memory allocaiton algorithm.
 * Implement it by yourself in step 1.
 *
 * @param  size  the size of a requested memory space
 * @param  left_mcb_addr  the address of the left boundary of MCB entries to
 examine
 * @param  right_mcb_addr the address of the right boundary of MCB entries to
 examine
 * @return the address of Cortex-M's SRAM space. While the computation is
 *         made in integers, cast it to (void *). The gcc compiler gives
 *         a warning sign:
                cast to 'void *' from smaller integer type 'int'
 *         Simply ignore it.
 */
void *_ralloc(int size, int left_mcb_addr, int right_mcb_addr) {
  // initial parameter computation
  //  TODO(student): add comment to each of the following line of code
  int entire_mcb_addr_space = right_mcb_addr - left_mcb_addr + mcb_ent_sz;
  int half_mcb_addr_space = entire_mcb_addr_space / 2;
  int midpoint_mcb_addr = left_mcb_addr + half_mcb_addr_space;
  int heap_addr = 0;
  int act_entire_heap_size = entire_mcb_addr_space * 16;
  int act_half_heap_size = half_mcb_addr_space * 16;

  // base case
  //  TODO(student): add comment to each of the following line of code
  if (size <= act_half_heap_size) {
    void *heap_addr =
        _ralloc(size, left_mcb_addr, midpoint_mcb_addr - mcb_ent_sz);
    if (heap_addr == 0) {
      return _ralloc(size, midpoint_mcb_addr, right_mcb_addr);
    }
    if ((array[m2a(midpoint_mcb_addr)] & 0x01) == 0) {
      *(short *)&array[m2a(midpoint_mcb_addr)] = act_half_heap_size;
    }
    return heap_addr;
  }
  // (size > act_half_heap_size)
  if ((array[m2a(left_mcb_addr)] & 0x01) != 0) {
    return 0;
  }
  if (*(short *)&array[m2a(left_mcb_addr)] < act_entire_heap_size) {
    return 0;
  }
  *(short *)&array[m2a(left_mcb_addr)] = act_entire_heap_size | 0x01;
  return (void *)(heap_top + (left_mcb_addr - mcb_top) * 16);
}

/*
 * _rfree is _kfree's helper function that is recursively called to
 * deallocate a space, using the buddy memory allocaiton algorithm.
 * Implement it by yourself in step 1.
 *
 * @param  mcb_addr that corresponds to a SRAM space to deallocate
 * @return the same as the mcb_addr argument in success, otherwise 0.
 */
int _rfree(int mcb_addr) {
  //  TODO(student): add comment to each of the following line of code
  short mcb_contents = *(short *)&array[m2a(mcb_addr)];
  int mcb_index = mcb_addr - mcb_top;
  short mcb_disp = (mcb_contents /= 16);
  short my_size = (mcb_contents *= 16);

  // mcb_addr's used bit was cleared
  *(short *)&array[m2a(mcb_addr)] = mcb_contents;

  //  TODO(student): add comment to each of the following line of code
  if ((mcb_index / mcb_disp) % 2 == 0) {
    if (mcb_addr + mcb_disp >= mcb_bot) {
      return 0; // my buddy is beyond mcb_bot!
    }
    short mcb_buddy = *(short *)&array[m2a(mcb_addr + mcb_disp)];
    if ((mcb_buddy & 0x0001) == 0) {
      mcb_buddy = (mcb_buddy / 32) * 32;
      if (mcb_buddy == my_size) {
        *(short *)&array[m2a(mcb_addr + mcb_disp)] = 0;
        my_size *= 2;
        *(short *)&array[m2a(mcb_addr)] = my_size;
        return _rfree(mcb_addr);
      }
    }
  } else {
    if (mcb_addr - mcb_disp < mcb_top) {
      return 0; // my buddy is below mcb_top!
    }
    short mcb_buddy = *(short *)&array[m2a(mcb_addr - mcb_disp)];
    if ((mcb_buddy & 0x0001) == 0) {
      mcb_buddy = (mcb_buddy / 32) * 32;
      if (mcb_buddy == my_size) {
        *(short *)&array[m2a(mcb_addr)] = 0;
        my_size *= 2;
        *(short *)&array[m2a(mcb_addr - mcb_disp)] = my_size;
        return _rfree(mcb_addr - mcb_disp);
      }
    }
  }
  return mcb_addr;
}

/*
 * Initializes MCB entries. In step 2's assembly coding, this routine must
 * be called from Reset_Handler in startup_TM4C129.s before you invoke
 * driver.c's main( ).
 */
void _kinit() {
  //  TODO(student): add comment to each of the following line of code
  for (int i = 0x20001000; i < 0x20005000; i++) {
    array[m2a(i)] = 0;
  }

  *(short *)&array[m2a(mcb_top)] = max_size;
  for (int i = 0x20006802; i < 0x20006C00; i += 2) {
    array[m2a(i)] = 0;
    array[m2a(i + 1)] = 0;
  }
}

/*
 * Step 2 should call _kalloc from SVC_Handler.
 *
 * @param  the size of a requested memory space
 * @return a pointer to the allocated space
 */
void *_kalloc(int size) { return _ralloc(size, mcb_top, mcb_bot); }

/*
 * Step 2 should call _kfree from SVC_Handler.
 *
 * @param  a pointer to the memory space to be deallocated.
 * @return the address of this deallocated space.
 */
void *_kfree(void *ptr) {
  //  TODO(student): add comment to each of the following line of code
  int addr = (int)ptr;

  if (addr < heap_top || addr > heap_bot) {
    return NULL;
  }
  int mcb_addr = mcb_top + (addr - heap_top) / 16;

  if (_rfree(mcb_addr) == 0) {
    return NULL;
  }
  return ptr;
}

/*
 * _malloc should be implemented in stdlib.s in step 2.
 * _kalloc must be invoked through SVC in step 2.
 *
 * @param  the size of a requested memory space
 * @return a pointer to the allocated space
 */
void *_malloc(int size) {
  assert(size >= min_size);
  static int init = 0;
  if (init == 0) {
    init = 1;
    _kinit();
  }
  return _kalloc(size);
}

/*
 * _free should be implemented in stdlib.s in step 2.
 * _kfree must be invoked through SVC in step 2.
 *
 * @param  a pointer to the memory space to be deallocated.
 * @return the address of this deallocated space.
 */
void *_free(void *ptr) { return _kfree(ptr); }
