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

#include "sdb.h"
#include <stdint.h>
#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  char* expr;
  uint32_t res;
} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;
void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].expr = (char*)malloc(sizeof(char));
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */


void list_breakpoint(){
  WP *it = head;
  if (it == NULL)
	{
		printf("No breakpoints\n");
		return;
	}
  while(it != NULL){
    printf("%02d\t%10s\t%10u\n", it->NO, it->expr, it->res);
    it = it->next;
  }
}
//free_指向链表末尾的空
uint32_t new_wp(char* e, uint32_t res){
  if(free_->next==NULL)
    return -1;
  WP *wp_to_add = free_;
  free_ = free_->next;

  strcpy(wp_to_add->expr, e);
  wp_to_add->res = res;

  wp_to_add->next = head;
  head = wp_to_add;
  return wp_to_add->NO;
}

void free_wp(uint32_t n){
  WP *p, *prev = NULL;
  for (p = head; p != NULL; prev = p,p = p->next){
    if(p->NO == n)
      break;
  }
  if(p == NULL){
    printf("no watchpoint number\n");
    return;
  }
  if(prev == NULL){
    head = p->next;
  }
  else{
    prev->next = p->next;
  }
  printf("Successful delete No: %d watchpoint\n", p->NO);
  free(p->expr);

  // WP* cur = head;
  // while(cur->next != NULL && cur->next != free_ && cur->next->NO != n ) {
  //   cur = cur->next;
  // }
  // if(cur->next == NULL || cur == free_) {
  //   printf("no watchpoint number\n");
  //   return;
  // }
  // WP* a = cur->next;
  // cur->next = cur->next->next;
  // a->next = free_->next;
  // free_->next = a;
  // printf("Successful delete No: %d watchpoint\n", a->NO);
}

void diff_check_wp(){
  bool *success = false;
  bool has_print_head = false;
  WP *it = head;
  while(!it){
    uint32_t res = expr(it->expr, success);
    if(*success)
      printf("The expression of watch point %d is invalid!\n",it->NO);
    else if(it->res != res){
      if(!has_print_head){
        printf("%-6s\t%-10s\t%-8s\t%-8s\n", "Number", "Expression", "New Value", "Old Value"); 
        has_print_head = true;
      }
      printf("%-5d\t%-14s\t%-14u\t%-14u\n",it->NO,it->expr,res,it->res);
      it->res = res;
      nemu_state.state = NEMU_STOP;
    }
    it = it->next;
  }
}
