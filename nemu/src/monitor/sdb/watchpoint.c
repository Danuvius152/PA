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
    return 0;
  WP *wp_to_add = free_;
  free_ = free_->next;

  strcpy(wp_to_add->expr, e);
  wp_to_add->res = 0;

  wp_to_add->next = head;
  head = wp_to_add;
  return wp_to_add->NO;
}

void free_wp(uint32_t n){
  WP *p, *prev = NULL;
  for (p = head; p != NULL; prev=p,p=p->next){
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
  printf("Successful delete No: %ud watchpoint\n", p->NO);
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

// #include "sdb.h"

// #define NR_WP 32

// typedef struct watchpoint {
//   int NO;
//   struct watchpoint *next;

//   /* TODO: Add more members if necessary */
//   char expr[128];
//   uint32_t res;
// } WP;

// static WP wp_pool[NR_WP] = {};
// static WP *head = NULL, *free_ = NULL;
// static int nr_wp_used = 0;

// /* TODO: Implement the functionality of watchpoint */
// bool new_wp(char *e){
//   if(nr_wp_used >= NR_WP) return false;
  
//   WP *wp_to_add = free_;
//   free_ = free_->next;

//   if(head == NULL){
//     head = wp_to_add;
//     head->next = NULL;
//     strcpy(head->expr, e);
//     wp_to_add->res = 0;
//   }
//   else{
    
//     WP * itr = head;
//     while(itr->next != NULL){itr=itr->next;}
//     itr->next = wp_to_add;
//     wp_to_add->next = NULL;
//     strcpy(wp_to_add->expr, e);
//     wp_to_add->res = 0;
//   }
//   nr_wp_used ++;
  
//   return true;
// };

// void free_wp(int n){
//   if(nr_wp_used <= 0) return;

//   WP* wp_to_free = NULL;
//   if(head->NO == n){
//     wp_to_free = head;
//     head = head->next;
//     wp_to_free->next = NULL;
//   }
//   else{
//     WP *itr = head;
//     while(itr->next != NULL){
//       if(itr->next->NO == n){
//         wp_to_free = itr->next;
//         itr->next = wp_to_free->next;
//         wp_to_free->next = NULL;
//         break;
//       }
//       itr = itr->next;
//     }
//     if(itr->next == NULL) return;
//   }
  
//   wp_to_free->next = free_;
//   free_ = wp_to_free;

//   nr_wp_used --;
// };

// void traverse_wp(){
//   WP *itr = head;
//   while(itr != NULL){
//     printf("%02d\t%10s\t%-10u\n",itr->NO,itr->expr,itr->res);
//     itr = itr->next;
//   }
// }



// void init_wp_pool() {
//   int i;
//   for (i = 0; i < NR_WP; i ++) {
//     wp_pool[i].NO = i;
//     wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
//     strcpy(wp_pool[i].expr, "\0");
//     wp_pool[i].res = 0;
//   }

//   head = NULL;
//   free_ = wp_pool;
// }

