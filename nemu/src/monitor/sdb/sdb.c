/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>
#include <cpu/cpu.h>
#include <memory/vaddr.h>
//#include <isa/riscv32/reg.c>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
  return -1;
}
static int cmd_si(char *args) {
  char *arg = strtok(NULL, " ");
  if(arg==NULL){
    cpu_exec(1);
    return 0;
  }
  int n = 1;
  if (sscanf(arg, "%d", &n) == 1 && n > 0)
  {
    cpu_exec(n);
  }
  else{
    printf("Bad number: \e[0;31m%s\e[0m\n", arg);
  }
  return 0;
}
void list_breakpoint();
static int cmd_info(char *args) {
  char *arg = strtok(NULL, " ");
	if (arg == NULL)
	{
		printf("undefined info args\n");
		return 0;
	}
  if (strcmp(arg, "r") == 0)
	{
    isa_reg_display();
  }
  else if (strcmp(arg, "w") == 0)
	{
		list_breakpoint();
	}
  else
	{
		printf("undefined info args\n");
	}
  return 0;
}

static int cmd_x(char *args) {
  char *arg = strtok(NULL, " ");
  if(arg){
    int n = 0;
    if (sscanf(arg, "%d", &n) == 1 && n > 0)
    {
      char *addr = strtok(NULL, " ");
      if(addr){
        if(strlen(addr)>=2 && addr[0] == '0' && addr[1] == 'x'){
          int32_t address;
          sscanf(addr+2, "%x", &address);
          printf("0x%08x\n",address);
          printf("Address\t\tDword block\tByte sequence\n");
          for(int i=0; i<n;i++){
            printf("0x%08x\t0x%08x\t", address, vaddr_read(address, 4));
		        for(int j=0;j<4;j++) {
			        printf("%02x ", vaddr_read(address+j, 1));/*read 1 byte once*/
		        }
		       printf("\n");
		       address += 4; /*address add 4 bytes to the next Dword block*/
            //DWORD 代表 unsigned long  
          }
        }
      }
    } 
  }
  else{
    printf("undefined behavior\n");
  }
  return 0;
}
static int cmd_p(char *args) {
  if(args == NULL) {
    printf("No Arguments\n");
    return 0;
  }
  bool success = false;
  uint32_t res = expr(args, &success);
  if(success == false) printf("Invalid Expression\n");
  else printf("> %u\n", res);
  return 0;
}
uint32_t new_wp(char* e, uint32_t res);
static int cmd_w(char *args)
{
  if(args == NULL) {
    printf("No Arguments\n");
    return 0;
  }
  bool success = false;
  uint32_t res = expr(args, &success);
  if(success == false) printf("Invalid Expression\n");
  else{
    uint32_t flag = new_wp(args, res);
    if(!flag)
      printf("Failed to add a watchpoint");
    else
      printf("Set watchpoint %ud", flag);
  }
  return 0;
}
void free_wp();
static int cmd_d(char *args)
{
  char *arg = strtok(NULL, " ");
  if(arg){
    int n = 0;
    if (sscanf(arg, "%d", &n) == 1 && n > 0)
      free_wp(n);
  }
  else{
    printf("undefined behavior\n");
  }
  return 0;
}
static int cmd_help(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "One Step", cmd_si },
  { "info", "Display informations about all regisiters", cmd_info },
  { "x", "Scan Memory", cmd_x },
  { "p", "expression evaluation", cmd_p },
  { "w", "Set Watchpoint", cmd_w },
  { "d", "Delete Watchpoint", cmd_d },
};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}




// #include <isa.h>
// #include <cpu/cpu.h>
// #include <memory/vaddr.h>
// #include <readline/readline.h>
// #include <readline/history.h>
// #include "sdb.h"
// #include <isa.h>

// static int is_batch_mode = false;

// void init_regex();
// void init_wp_pool();

// void isa_reg_display();

// // word_t expr(char *e, bool *success);

// /* We use the `readline' library to provide more flexibility to read from stdin. */
// static char* rl_gets() {
//   static char *line_read = NULL;

//   if (line_read) {
//     free(line_read);
//     line_read = NULL;
//   }

//   line_read = readline("(nemu) ");

//   if (line_read && *line_read) {
//     add_history(line_read);
//   }

//   return line_read;
// }

// static int cmd_c(char *args) {
//   cpu_exec(-1);
//   return 0;
// }


// static int cmd_q(char *args) {
  
//   return -1;
// }

// static int cmd_si(char *args){
//   if(args != NULL) cpu_exec(atoi(args));
//   else cpu_exec(1);

//   return 0;
// }

// void traverse_wp();
// static int cmd_info(char *args){
//   //in case, input w will case segmentation fault
//   //strcmp will dereference args, *NULL cause segmentation fault
//   if(args == NULL) return 0;

//   if(strcmp(args,"r") == 0); //isa_reg_display();
//   else if(strcmp(args,"w") == 0) traverse_wp();
//   else printf("Invalid command!\n");
//   return 0;
// }


// static int cmd_x(char *args){
//   char *cnt_str = strtok(NULL, " ");
//   if(cnt_str != NULL){
//     int cnt = atoi(cnt_str);

//     char *addr_str = strtok(NULL, " ");
//     if(addr_str != NULL){
//       if(strlen(addr_str)>=2 && addr_str[0] == '0' && addr_str[1] == 'x'){
//         int addr = (int)strtol(addr_str+2,NULL,16);
//         printf("%-14s%-28s%-s\n","Address","Hexadecimal","Decimal");
//         for(int i = 0; i < cnt; ++i){
//           printf("0x%-12x0x%02x  0x%02x  0x%02x  0x%02x",(addr),vaddr_read(addr,1),vaddr_read(addr+1,1),vaddr_read(addr+2,1),vaddr_read(addr+3,1));
//           printf("\t  %04d  %04d  %04d  %04d\n",vaddr_read(addr,1),vaddr_read(addr+1,1),vaddr_read(addr+2,1),vaddr_read(addr+3,1));
//           addr += 4;
//         }
//       }
//       else
//         printf("the result of the given expression is NOT hexadecimal!");
//     }
//   }
//   return 0;
// }

// static int cmd_p(char *args){
//   if(args == NULL) return 0;

//   bool success = false;
//   int res = expr(args,&success);
//   if(success == false) printf("Invalid Expression\n");
//   else printf(">  %u\n", res);
//   return 0;
// }

// bool new_wp(char *e);

// static int cmd_w(char *args){
//   if(args == NULL) return 0;

//   if(!new_wp(args)) printf("the number of watch point is out of bound!\n");

//   return 0;
// }

// void free_wp(int n);

// static int cmd_d(char *args){
//   if(args == NULL) return 0;

//   free_wp(atoi(args));
//   return 0;
// }



// static int cmd_help(char *args);

// static struct {
//   const char *name;
//   const char *description;
//   int (*handler) (char *);
// } cmd_table [] = {
//   { "help", "Display informations about all supported commands", cmd_help },
//   { "c", "Continue the execution of the program", cmd_c },
//   { "q", "Exit NEMU", cmd_q },
//   /* TODO: Add more commands */
//   { "si", "Let the program single step through N instructions and pause execution, When N is not given, it defaults to 1", cmd_si },
//   { "info", "print register status or watchpoint information", cmd_info},
//   { "x" , "Evaluate the expression EXPR, using the result as the starting memory address, output N consecutive 4-bytes in hexadecimal", cmd_x},
//   { "p", "Evaluate the expression EXPR", cmd_p},
//   { "w", "Suspend program execution when the value of the expression EXPR changes", cmd_w},
//   { "d", "delete the watchpoint with sequence number n", cmd_d}
// };

// #define NR_CMD ARRLEN(cmd_table)

// static int cmd_help(char *args) {
//   /* extract the first argument */
//   char *arg = strtok(NULL, " ");
//   int i;

//   if (arg == NULL) {
//     /* no argument given */
//     for (i = 0; i < NR_CMD; i ++) {
//       printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
//     }
//   }
//   else {
//     for (i = 0; i < NR_CMD; i ++) {
//       if (strcmp(arg, cmd_table[i].name) == 0) {
//         printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
//         return 0;
//       }
//     }
//     printf("Unknown command '%s'\n", arg);
//   }
//   return 0;
// }

// void sdb_set_batch_mode() {
//   is_batch_mode = true;
// }

// void sdb_mainloop() {
//   if (is_batch_mode) {
//     cmd_c(NULL);
//     return;
//   }

//   for (char *str; (str = rl_gets()) != NULL; ) {
//     char *str_end = str + strlen(str);

//     /* extract the first token as the command */
//     char *cmd = strtok(str, " ");
//     if (cmd == NULL) { continue; }

//     /* treat the remaining string as the arguments,
//      * which may need further parsing
//      */
//     char *args = cmd + strlen(cmd) + 1;
//     if (args >= str_end) {
//       args = NULL;
//     }

// #ifdef CONFIG_DEVICE
//     extern void sdl_clear_event_queue();
//     sdl_clear_event_queue();
// #endif

//     int i;
//     for (i = 0; i < NR_CMD; i ++) {
//       if (strcmp(cmd, cmd_table[i].name) == 0) {
//         if (cmd_table[i].handler(args) < 0) { return; }
//         break;
//       }
//     }
//     if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
//   }
// }

// void init_sdb() {
//   /* Compile the regular expressions. */
//   init_regex();

//   /* Initialize the watchpoint pool. */
//   init_wp_pool();
// }
