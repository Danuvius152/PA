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
#include <memory/vaddr.h>
/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
// 十进制整数
// +, -, *, /
// (, )
// 空格串(一个或多个空格)
enum
{
  TK_NOTYPE = 256, TK_EQ, TK_NEQ, TK_AND,

  /* TODO: Add more token types */
  TK_REG,
  TK_DEC,
  TK_HEX,
  TK_NEG,
  TK_REF

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces  这里的+表示1个或多个！！！
  {"==", TK_EQ},        // equal
  {"!=", TK_NEQ},        // not equal
  {"&&", TK_AND},        
  {"\\+", '+'},         
  {"\\-", '-'},         
  {"\\*", '*'},         
  {"\\/", '/'},         
  {"\\(", '('},         
  {"\\)", ')'},
  {"0[xX][0-9a-fA-F]+", TK_HEX},         
  {"\\$(a[0-7]|t[0-6p]|s[0-9p]|s1[0-1]|ra|gp|pc|0)", TK_REG},
  {"0|[1-9][0-9]*", TK_DEC},
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case TK_NOTYPE:
            break;
          case '-':
            if (nr_token != 0 && (tokens[nr_token - 1].type == TK_DEC || tokens[nr_token - 1].type == TK_HEX || tokens[nr_token - 1].type == TK_REG || tokens[nr_token - 1].type == ')')) {
				      tokens[nr_token++].type = '-';
				    }
				    else {
				      tokens[nr_token++].type = TK_NEG;
				    }
            break;
          case '*':
            if (nr_token != 0 && (tokens[nr_token - 1].type == TK_DEC || tokens[nr_token - 1].type == TK_HEX || tokens[nr_token - 1].type == TK_REG || tokens[nr_token - 1].type == ')')) {
				      tokens[nr_token++].type = '*';
				    }
				    else {
				      tokens[nr_token++].type = TK_REF;
				    }
            break;
          case TK_DEC:
          case TK_HEX:
          case TK_REG:
            Assert(substr_len < sizeof(tokens[nr_token].str)/sizeof(tokens[nr_token].str[0]), "The Token length is out of bound!");
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';// necessary!!!
            // tokens[nr_token++].type = rules[i].token_type;
            // break;
          default:
            tokens[nr_token++].type = rules[i].token_type;
        }
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }
  // Log("num is %d",nr_token);
  // for(int i = 0; i < nr_token; ++i)
  //   printf("[%d, %s]", tokens[i].type, tokens[i].str);
  return true;
}

static bool check_parentheses(int p, int q){
  if(p>q || tokens[p].type!='('|| tokens[q].type!=')')
    return false;
  int cnt = 1;
  for (int i = p + 1; i < q; i++){
    if(tokens[i].type == '(')
      cnt++;
    if(tokens[i].type == ')')
      cnt--;
    if(cnt <= 0)
      return false;
  }
  if(cnt == 1)
    return true;
  else
    return false;
}

static int get_priority(int type){
  switch(type){
    case TK_AND:
      return 0;
    case TK_EQ:
    case TK_NEQ :
      return 1;
    case '+':
    case '-':
      return 2;
    case '*':
    case '/':
      return 3;
    case TK_NEG:
      return 4;
    case TK_REF:
      return 5;
    default:
      return -1;
    }
}

static int dominant_operator(int p, int q){
  int split = -1;
  bool valid = true;
  for (int i = p; i <= q; i++)
  {
    if(tokens[i].type == TK_DEC||tokens[i].type == TK_HEX||tokens[i].type == TK_REG)
      continue;
    if (tokens[i].type == '(')
    {
      valid = false;
      continue;
    }
    if (tokens[i].type == ')')
    {
      valid = true;
      continue;
    }
    if (valid && get_priority(tokens[i].type) != -1 && split == -1){
      split = i;
      continue;
    }
    if (valid && get_priority(tokens[i].type) != -1)
    {
      if (get_priority(tokens[i].type) <= get_priority(tokens[split].type)){
        if(get_priority(tokens[i].type)==4 && get_priority(tokens[split].type)==4)
          continue;
        split = i;
      }
    }
  }
  return split;
}

uint32_t eval(int p, int q, bool *success){
  if (p > q) {
    *success = false; 
    printf("Bad Range In Fuc Eval\n");
    return 0;
  }
  else if (p == q) {
    uint32_t result;
    switch(tokens[p].type){
      case TK_DEC:
        sscanf(tokens[p].str,"%u",&result);
        break;
      case TK_HEX:
        sscanf(tokens[p].str, "%x", &result);
        break;
      case TK_REG:
        result = isa_reg_str2val(tokens[p].str, success);
        break;
      default: 
        result = 0; *success = false;
    }
    return result;
  }
  else if (check_parentheses(p, q) == true) {
    return eval(p + 1, q - 1, success);
  }
  else {
    int op = dominant_operator(p, q);
    if(op == -1){
      *success = false;
      return 0;
    }
    uint32_t val1 = 0;
    uint32_t val2 = 0;
    if (tokens[op].type == TK_NEG){
      val2 = - eval(op + 1, q, success);
    }
    else if(tokens[op].type == TK_REF){
      val2 = vaddr_read(eval(op + 1, q, success), 4);
    }
    else{
      val1 = eval(p, op - 1, success);
      val2 = eval(op + 1, q, success);
    }
    switch (tokens[op].type) {
      case TK_EQ: return val1 == val2;
      case TK_NEQ: return val1 != val2;
      case TK_AND: return val1 && val2;
      case '+': return val1 + val2;
      case '-': return val1 - val2;
      case '*': return val1 * val2;
      case '/': return val1 / val2;
      case TK_NEG: return val2;
      case TK_REF: return val2;
      default: Log("INvalid Operator!");*success = false; return 0;
    }
  }
  *success = false;
  return 0;
}

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  return eval(0, nr_token - 1, success); //NB!
}
