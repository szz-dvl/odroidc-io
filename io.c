/* 
  
   Writes values to memory or system busses, and some helpers 
   
   Compile with: gcc -std=c99 -o io io.c
   
*/

#define _BSD_SOURCE /* popen / pclose */ 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "io.h"

#define SYSFILE "/sys/class/amlogic/debug"
#define OUTPUT_CMD "dmesg | tail -n 1"
#define BUSNUM 8
#define OPSNUM 7

#define KNRM "\x1B[0m"
#define KRED "\x1B[31;1m"
#define KGRN "\x1B[32;1m"
#define KYEL "\x1B[33;1m"
#define KBLU "\x1B[34;1m"
#define KMAG "\x1B[35;1m"
#define KCYN "\x1B[36;1m"
#define KWHT "\x1B[37;1m"

struct dict {
  
  char * name; 
  int val;
  
};

typedef struct dict dict;

dict busses [BUSNUM] = {{"CBUS", 'c'},{"VCBUS", 'v'},{"AOBUS", 'a'},{"AXBUS", 'x'},{"DOS", 'd'},{"SECBUS", 's'},{"MMC", 'm'},{"MEM", ' '}};
dict ops [OPSNUM] = {{"write", 0},{"read", 1},{"set", 2},{"unset", 3},{"print", 4},{"shift", 5},{"get", 6}};

char * colors[8] = { KNRM, KRED, KYEL, KGRN, KBLU, KMAG, KCYN, KWHT };

/* binary for dummies. fixed to 32 bits */
void print_binary (unsigned int n){

  int count = 32;
  char res [count];
  res[count] = 0;
  count --;

  while (n || count >= 0) {
    if (n & 1) 
      res[count] = '1';
    else 
      res[count] = '0';
    
    n >>= 1;
    count --;
  }
 
  int col = 3;

  for (int i = 0; i<32; i++) {
    if (!(i % 4)) {
      
      if (col == 3)
	col = 6;
      else
	col = 3;
    } 
    
    printf("%s%c  ", colors[col], res[i]);
  }
  
  printf("%s\n", KYEL);

  for (int i = 31; i>=0; i--) { 
    if (i>9)
      printf("%d ", i);
    else
      printf("%d  ", i);
  }

  printf("%s\n", KNRM);

}

int get_bus_code (char * bus, char * code) {
  
  int i = 0;
  
  while (strcmp(bus, busses[i].name) && i < BUSNUM) 
    i++;
  
  *code = busses[i].val;
    
  return i == BUSNUM ? 0 : 1;

}

int get_op_code (char * op, int * code) {
  
  int i = 0;
  
  while (strcmp(op, ops[i].name) && i < OPSNUM) 
    i++;
  
  *code = ops[i].val;
    
  return i == OPSNUM ? 0 : 1;

}

int is_interesting (char * value) {

  return strcmp(value, "0xffffffff\n") && strcmp(value, "0x00000000\n");

}

//Get the last line from "dmesg".
void get_output (char * outval) {

  char aux [32], * output = (char*) malloc (128 * sizeof(char));

  FILE *fp = popen(OUTPUT_CMD, "r");

  while(fgets(aux, sizeof(aux), fp))
    strcat(output, aux);

  pclose(fp);
  
  //Get the value at the right side of '='
  for (output = strtok(output, "="); output;  output = strtok(NULL, "=")) 
    strcpy(outval, output);
  
}

//Write to the debug "API" of amlogic, populated via /sys/class/amlogic/debug || +INFO: 'cat /sys/class/amlogic/help' 
void do_cmd (char * cmd, char * outval) {
  
  //printf("CMD: %s\n", cmd);
  FILE *fp = fopen(SYSFILE, "w");
  fprintf(fp, "%s", cmd); 
  fclose(fp);
  get_output(outval);

}

void do_read (char bus_code, char * addr, char * outval) {
  
  char cmd [32], * format  = bus_code == ' ' ? "read%c%s" : "read %c %s";
  
  sprintf(cmd, format, bus_code, addr);
  do_cmd(cmd, outval);
  
}

void do_write (unsigned long int val, char bus_code, char * addr, char * outval) {
  
  char cmd [32], * format  = bus_code == ' ' ? "write 0x%08lX%c%s" : "write 0x%08lX %c %s"; 

  sprintf(cmd, format, val, bus_code, addr);
  do_cmd(cmd, outval);
    
}

//Get the memory address of the reg "char_reg" for the bus "bus".
unsigned long int get_bus_addr (char bus, char * char_reg) {
  
  unsigned long int reg = (unsigned long int) strtoul(char_reg, NULL, char_reg[1] == 'x' ? 0 : 16);

  switch (bus) {
    
  case 'c':
    return CBUS_REG_ADDR(reg);
  case 'v':
    return VCBUS_REG_ADDR(reg);
  case 'a':
    return AOBUS_REG_ADDR(reg);
  case 'x':
    return AXI_REG_ADDR(reg);
  case 'd':
    return DOS_REG_ADDR(reg);
  case 's':
    return SECBUS_REG_ADDR(reg);
  case 'm':
    return MMC_REG_ADDR(reg);
  default :
    return reg;
  }
  
}

void main (int argc, char ** argv) {
  
  
  if (argc < 3) {
    printf("Usage: ./io [operation (read/write/set/unset/shift/print)]\n"		\
	   "\t write: [address (hexa) REG: 16 bits, MEM: 32 bits] [bus (CBUS/VCBUS/AOBUS/AXBUS/DOS/SECBUS/MMC/MEM)] [(hexa) value] <bitmask> \n" \
	   "\t read:  [address (hexa) REG: 16 bits, MEM: 32 bits] [bus (CBUS/VCBUS/AOBUS/AXBUS/DOS/SECBUS/MMC/MEM)] \n"	\
	   "\t set:   [address (hexa) REG: 16 bits, MEM: 32 bits] [bus (CBUS/VCBUS/AOBUS/AXBUS/DOS/SECBUS/MMC/MEM)] [bitmask]\n"	\
	   "\t unset: [address (hexa) REG: 16 bits, MEM: 32 bits] [bus (CBUS/VCBUS/AOBUS/AXBUS/DOS/SECBUS/MMC/MEM)] [bitmask]\n"	\
	   "\t shift: [direction (left / right)] [value (hexa) up to 32 bits] [(dec) amount of bits to shift] \n" \
	   "\t print: [value (hexa) up to 32 bits] \n"			\
	   "\t get:   [value (binary) up to 32 bits] \n"			\
	   );
    exit(1);
  }
  
  char * op = argv[1], * addr, * bus_name;
  char bus_code, outval[128];
  int opcode;

  if (!get_op_code(op, &opcode)) {
    printf("OP \"%s\" not understood\n", op);
    exit(1);
  }
  
  if (getuid() && opcode < 4) {
  
    printf("Write / Read / Set / Unset operations requires superuser privileges, please run the later command with sudo, exiting now.\n");
    exit(1);
  
  } else if (opcode < 4) {

    addr = argv[2];
    bus_name = argv[3];
   
    if (!get_bus_code(bus_name, &bus_code)) {
	printf("BUS \"%s\" not understood\n", bus_name);
	exit(1);
    }
    
  }
 
  switch (opcode) {
    
  case 0 :
    /* performing a "write" operation */
    {
      
      char * token = (char*) malloc (2 * sizeof(char));
      unsigned long int val = (unsigned long int) strtoul(argv[4], NULL, argv[4][1] == 'x' ? 0 : 16);
      
      //add or "substract" bits from the given value
      if(argv[5]) {
	
	for (token = strtok(argv[5], ","); token;  token = strtok(NULL, ",")) {
	  
	  unsigned int bit = atoi(token);
	  
	  if (val & (1 << bit))
	    val &= ~(1 << bit);
	  else
	    val |= (1 << bit);
	}
	
      }
      
      do_write (val, bus_code, addr, outval);
      
      printf("Write 0x%08lX to %s (%s), result = %s", val, addr, bus_name, outval);  
      
    }
    break;;
    
  case 1 :
    /* performing a "read" operation */
    { 
      
      do_read (bus_code, addr, outval);
      
      printf("Read %s from %s, result = %s", addr, bus_name, outval);
      
    }
    break;;
    
  case 2 :
    //set bits for a memory address
    {
      
      char * token = (char*) malloc (2 * sizeof(char));
      
      do_read (bus_code, addr, outval);

      printf("Original value of %s from %s = %s", addr, bus_name, outval);
      
      unsigned long int result = (unsigned long int) strtoul(outval, NULL, 0);
      
      for (token = strtok(argv[4], ","); token;  token = strtok(NULL, ","))
	result |= (1 << atoi(token));
      
      do_write (result, bus_code, addr, outval);
      
      printf("Value after bitmask of %s from %s = %s", addr, bus_name, outval);
      
    }
    break;;
    
  case 3 :
    //unset bits for a memory address
    {
      
      char * token = (char*) malloc (2 * sizeof(char));
      
      do_read (bus_code, addr, outval);

      printf("Original value of %s from %s = %s", addr, bus_name, outval);
      
      unsigned long int result = (unsigned long int) strtoul(outval, NULL, 0);
      
      for (token = strtok(argv[4], ","); token;  token = strtok(NULL, ","))
	result &= ~(1 << atoi(token));
      
      do_write (result, bus_code, addr, outval);
      
      printf("Value after bitmask of %s from %s = %s", addr, bus_name, outval);

    }
    break;;
    
  case 4 :
    /* print value in binary format */
    {
   
      unsigned long int val = (unsigned long int) strtoul(argv[2], NULL, argv[2][1] == 'x' ? 0 : 16);
      
      print_binary(val);
      
    }
    break;;
  
  case 5 :
    /* shift value "shift" positions to the left or right */
    {
      
      unsigned int left = !strcmp(argv[2], "left") ? 1U : 0U;
      unsigned long int val = (unsigned long int) strtoul(argv[3], NULL, argv[3][1] == 'x' ? 0 : 16);
      unsigned int shift = argv[4] ? (unsigned int) atoi(argv[4]) : 0U;
      
      printf("Original value is: 0x%08lX, %s shifted value is: 0x%08lX\n", val, left ? "left" : "right", left ? val << shift : val >> shift);

    }
    break;;

  case 6 :
    /* get value for a string of bits */
    {
      
      unsigned long int val = (unsigned long int) strtoul(argv[2], NULL, 2);
      
      printf("%s = 0x%lX | %lu \n", argv[2], val, val);
      
    }
    break;;
     
  default:
    break;;  
  }
  
  /* If we performed a read / write operation on a bus, translate its reg address and perfom a read operation directly to memory (check). */
  
  if (bus_code != ' ' && opcode < 2) {
	  
	  char lastout [128], phy_char [32];
	  strcpy(lastout, outval);
	  
	  unsigned long int phys_addr = get_bus_addr(bus_code, addr);
	  sprintf(phy_char, "0x%08lX", phys_addr);
	  
	  do_read (' ', phy_char, outval);
	  
	  printf("Memory address 0x%08lX: %s", phys_addr, outval);
	  
	  if (is_interesting(outval) && !strcmp(outval, lastout)) {
		  unsigned long int result = (unsigned long int) strtoul(outval, NULL, 0);
		  print_binary(result);
	  }
	  
  } else if ((bus_code == ' ' && opcode < 2 && is_interesting(outval)) || (opcode > 1 && opcode < 4 && is_interesting(outval))) {
	  
	  unsigned long int result = (unsigned long int) strtoul(outval, NULL, 0);
	  print_binary(result);
    
  } 
  
}



