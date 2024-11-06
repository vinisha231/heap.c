/*
 * This is a C implementation of malloc( ) and free( ), based on the buddy
 * memory allocation algorithm.
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
  int index = sram_addr - 0x20000000;   //calculate index by subtracting the base SRAM address (0x20000000) 
  return index; //returning the calculated index 
}//closing of m2a(int sram_addr) method

/*
 * Reverse an array index back to the corresponding Cortex SRAM address.
 * @param  array index.
 * @return the corresponding Cortex-M's SRAM address in an integer.
 */
int a2m(int array_index) { // stands for array_index to memory address 
  return array_index + 0x20000000; //Convert index of array back to memory address by adding base SRAM address (0x20000000)
}//closing of a2m(int array_index) method 

/*
 * In case if you want to print out, all array elements that correspond
 * to MCB: 0x2006800 - 0x20006C00.
 */
void printArray() {
  printf("memory ............................\n");
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
    int mcb_addr = mcb_top + (addr - heap_top) / 16; //calculates the address of the memory block in heap
    int mcb_index = (mcb_addr - mcb_top) / mcb_ent_sz;//calculates the index of the memory block in heap 
    int heap_value = *(short *)&array[m2a(mcb_addr)];//calculates the value of the heap (not sure if this is unused) 
    printf("mcb[%3d]\t%x\t%x\t%3c\t%4d\n", mcb_index, mcb_addr, addr, memory[m2a(addr)], heap_value); //prints out the values of the memory address, memory index and value of the heap. 
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
  int entire_mcb_addr_space = right_mcb_addr - left_mcb_addr + mcb_ent_sz; // Calculates the total address space for MCBs from left to right boundaries, including one MCB entry size
  int half_mcb_addr_space = entire_mcb_addr_space / 2; //Calculate half of the total MCB address space for dividing the memory block in half
  int midpoint_mcb_addr = left_mcb_addr + half_mcb_addr_space; // Calculate the midpoint MCB address, representing the middle of the left and right MCB range
  int heap_addr = 0; //initializing the heap address to 0 here, this heap_ addr stores the allocated heap address if found
  int act_entire_heap_size = entire_mcb_addr_space * 16; //// Calculate the actual entire heap size corresponding to the MCB address space (multiply by 16 for byte size)
  int act_half_heap_size = half_mcb_addr_space * 16; //// Calculate the actual half heap size, which represents half of the entire heap size

  // base case
  if (size <= act_half_heap_size) { //checking if requested size fits in the half heap size
    void *heap_addr = _ralloc(size, left_mcb_addr, midpoint_mcb_addr - mcb_ent_sz); //// Recursively attempt to allocate in the left half (smaller partition)
    if (heap_addr == 0) { // checking if allocation failed in the left half
      return _ralloc(size, midpoint_mcb_addr, right_mcb_addr); //if so, trying to allocate in the right half
    }// closing of if-block
    if ((array[m2a(midpoint_mcb_addr)] & 0x01) == 0) { //If the right half's MCB at midpoint is unused (checked by & 0x01 == 0)
      *(short *)&array[m2a(midpoint_mcb_addr)] = act_half_heap_size; //Set the size of the MCB at midpoint to the half heap size
    }// closing of if block 
    return heap_addr;// Return the allocated heap address
  }
  // (size > act_half_heap_size)
  if ((array[m2a(left_mcb_addr)] & 0x01) != 0) { // Check if the left MCB is already marked as used (non-zero in the used flag)
    return 0;// If the MCB is used, return 0 indicating allocation failure
  }// closing of if block 
  if (*(short *)&array[m2a(left_mcb_addr)] < act_entire_heap_size) { // Checking if the size of the memory block at left MCB is less than the required size
    return 0; // Check if the size of the memory block at left MCB is less than the required size
  }// closing of if block 
  *(short *)&array[m2a(left_mcb_addr)] = act_entire_heap_size | 0x01; // Mark the MCB at left_mcb_addr as used by setting the size and the used flag (| 0x01)
  return (void *)(heap_top + (left_mcb_addr - mcb_top) * 16); // Return the pointer to the start of the allocated memory in the heap
} //closing of method.

/*
 * _rfree is _kfree's helper function that is recursively called to
 * deallocate a space, using the buddy memory allocaiton algorithm.
 * Implement it by yourself in step 1.
 *
 * @param  mcb_addr that corresponds to a SRAM space to deallocate
 * @return the same as the mcb_addr argument in success, otherwise 0.
 */
int _rfree(int mcb_addr) {
  short mcb_contents = *(short *)&array[m2a(mcb_addr)]; // Retrieve the contents of the MCB (memory control block) at the given address
  int mcb_index = mcb_addr - mcb_top; // Calculate the MCB index relative to the starting MCB address
  short mcb_disp = (mcb_contents /= 16);// Calculate displacement (size in 16-byte units) of the MCB contents by dividing by 16
  short my_size = (mcb_contents *= 16);// Calculate the actual size of the memory block managed by this MCB (revert to original scale)

  // mcb_addr's used bit was cleared
  *(short *)&array[m2a(mcb_addr)] = mcb_contents; //this bit is now marked as free I think 

  if ((mcb_index / mcb_disp) % 2 == 0) { // Clear the used bit for this MCB, marking it as free
    if (mcb_addr + mcb_disp >= mcb_bot) { //// Check if the current MCB index and its displacement point to an even location
      return 0; // my buddy is beyond mcb_bot!
    }
    short mcb_buddy = *(short *)&array[m2a(mcb_addr + mcb_disp)]; // Retrieve contents of the "buddy" MCB to check its status
    if ((mcb_buddy & 0x0001) == 0) {// If the buddy MCB is free (not in use)
      mcb_buddy = (mcb_buddy / 32) * 32;// Clear the used bit of the buddy MCB 
      if (mcb_buddy == my_size) {//checking if it matches the size of this block
        *(short *)&array[m2a(mcb_addr + mcb_disp)] = 0; // Clear the buddy MCB and combine with this block
        my_size *= 2; //double the size  
        *(short *)&array[m2a(mcb_addr)] = my_size; //mark as free
        return _rfree(mcb_addr); // Recursively attempt to free the combined block
      }//closing of if-block
    }//closing of nested if-blcok 
  } else { 
    if (mcb_addr - mcb_disp < mcb_top) { // If the buddy MCB is in an odd position, check if it's within bounds on the lower side
      return 0; // my buddy is below mcb_top!
    } //closing of if-block 
    short mcb_buddy = *(short *)&array[m2a(mcb_addr - mcb_disp)]; // Retrieve the contents of the left-side buddy MCB
    if ((mcb_buddy & 0x0001) == 0) { // Check if the left-side buddy is free
      mcb_buddy = (mcb_buddy / 32) * 32; // Clear the used bit of the buddy and compare its size with the current block's size
      if (mcb_buddy == my_size) { //if it matches the size of this block 
        *(short *)&array[m2a(mcb_addr)] = 0; // Clear this MCB and combine with the buddy
        my_size *= 2; //double the size  
        *(short *)&array[m2a(mcb_addr - mcb_disp)] = my_size;//mark as free
        return _rfree(mcb_addr - mcb_disp); //// Recursively free the combined block
      }//closing of if block
    }//closing of nested if block 
  } //closing of else block 
  return mcb_addr; // Return the current MCB address after free operation
} //closing of method 

/*
 * Initializes MCB entries. In step 2's assembly coding, this routine must
 * be called from Reset_Handler in startup_TM4C129.s before you invoke
 * driver.c's main( ).
 */
void _kinit() {
  for (int i = 0x20001000; i < 0x20005000; i++) { // Initialize memory array from address 0x20001000 to 0x20005000 
    array[m2a(i)] = 0; //store all elements of array to zero
  }// closing of for loop 

  *(short *)&array[m2a(mcb_top)] = max_size; // Set the initial size of the MCB at the top address to the maximum size
  for (int i = 0x20006802; i < 0x20006C00; i += 2) {  // Initialize additional memory area from 0x20006802 to 0x20006C00 
    array[m2a(i)] = 0; //storing elements of array to zero in 2-byte increments
    array[m2a(i + 1)] = 0; //storing nighbour of array to zero in 2-byte increments
  }//closing of for loop 
}// closing of method 

/*
 * Step 2 should call _kalloc from SVC_Handler.
 *
 * @param  the size of a requested memory space
 * @return a pointer to the allocated space
 */
void *_kalloc(int size) { return _ralloc(size, mcb_top, mcb_bot); } // Calls the `_ralloc` function to allocate memory with the specified size

/*
 * Step 2 should call _kfree from SVC_Handler.
 *
 * @param  a pointer to the memory space to be deallocated.
 * @return the address of this deallocated space.
 */
void *_kfree(void *ptr) {
  int addr = (int)ptr; // Cast the pointer to an integer address for calculations

  if (addr < heap_top || addr > heap_bot) { //Check if the address is within the bounds of the heap
    return NULL; //return NULL if not
  }//closing of if-block
  int mcb_addr = mcb_top + (addr - heap_top) / 16; //Calculate the MCB address based on the address

  if (_rfree(mcb_addr) == 0) {// Attempt to free the memory
    return NULL;//return NULL if freeing fails
  }//closing of if-block
  return ptr; // Return the original pointer on successful free operation
}

/*
 * _malloc should be implemented in stdlib.s in step 2.
 * _kalloc must be invoked through SVC in step 2.
 *
 * @param  the size of a requested memory space
 * @return a pointer to the allocated space
 */
void *_malloc(int size) {
  assert(size >= min_size); // Assert to ensure the size is at least the minimum required size
  static int init = 0; // Static variable to ensure initialization happens only once
  if (init == 0) {// If not initialized
    init = 1; //changing variable to 1 
    _kinit(); //initialize the memory using _kinit
  }//closing of if-block
  return _kalloc(size); //Allocate memory with the given size and return the pointer
}

/*
 * _free should be implemented in stdlib.s in step 2.
 * _kfree must be invoked through SVC in step 2.
 *
 * @param  a pointer to the memory space to be deallocated.
 * @return the address of this deallocated space.
 */
void *_free(void *ptr) { return _kfree(ptr); } // Free the specified pointer by calling `_kfree` and return the result
