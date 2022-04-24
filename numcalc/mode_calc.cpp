
#include "include/sys.h"
#include "include/comms.h"
#include "include/modes.h"
#include "include/util.h"

typedef struct Token token_t;

typedef struct Token
{
  #define O_INPUT -1
  #define O_NUM 0
  #define O_PM 1 //+ -
  #define O_MD 2 //* /
  #define O_PWR 3 //^ 
  #define O_FN 4 //func ( or (
  #define O_FN_END 5// )

  #define S_END -1
  #define S_EQU 0
  #define S_SUB 1
  #define S_ADD 2
  #define S_MUL 3
  #define S_DIV 4
  #define S_POW 5
  #define S_SQR 6
  #define S_SIN 7
  #define S_COS 8
  #define S_TAN 9
  #define S_ABS 10
  
  int order;
  union {
      float value;
      int symbol;
  };
  
  token_t* o1;
  token_t* o2;
} token_t;

// darray_t<token_t> expr;
// darray_t<char> number_literal;

sarray_t<token_t> expr;
sarray_t<char> number_literal;
token_t expr_buf[100];
char number_literal_buf[50];

int num_mode = 0;
int num_mode_old = 0;
int num_dot = 0;
token_t* num_input;
int shift;
int calc_new_bytes = 0;

void expr_print(){
    // for(int i=0; i<expr.count; i++){
    //   if(expr.buf[i]->order)
    //     printf("i:%d (%d)[%d]\n",i,expr.buf[i]->order,expr.buf[i]->symbol);
    //   else
    //     printf("i:%d (%d)vv[%f]\n",i,expr.buf[i]->order,expr.buf[i]->value);
    // }
}

void expr_push_input(){
    token_t nn;
    nn.order = O_INPUT;
    sarray_push(expr,nn);
}

void expr_push_number(float value){
    token_t nn;
    nn.order = O_NUM;
    nn.value = value;
    sarray_push(expr,nn);
}

void expr_push_symbol(int sym){
    token_t nn;
    nn.symbol = sym;
    switch(sym){
        case S_END:
            nn.order = O_FN_END;
        break;

        case S_SUB:
        case S_ADD:
            nn.order = O_PM;
        break;

        case S_MUL:
        case S_DIV:
            nn.order = O_MD;
        break;

        case S_POW:
            nn.order = O_PWR;
        break;

        default:
            nn.order = O_FN;
        break;
    }

    sarray_push(expr,nn);

    return;
}

float equate(token_t* r){
  float rl = 0.f;
  float rr = 0.f;

  if (r->o1)
    {
      rl = equate (r->o1);
    }
  if (r->o2)
    {
      rr = equate (r->o2);
    }
    
//   print_token(r);
//   LOG("");
//   LOG(rl);
//   LOG(rr);
//   LOG(">>>>>>>>>>");
    
  if (r->order == O_NUM)
    return r->value;
  if (r->symbol == S_SUB)
    return rl - rr;
  if (r->symbol == S_ADD)
    return rl + rr;
  if (r->symbol == S_MUL)
    return rl * rr;
  if (r->symbol == S_DIV)
    return rl / rr;
    
  if (r->symbol == S_POW) 
    return pow (rl, rr);
  if (r->symbol == S_SQR) {
    if(r->o1){
        float oo = pow (rr, (1.f/rl));
        // LOG("sqrt res");
        // LOG(oo);
        // LOG(rr);
        // LOG(rl);
        return oo;
    }
    else
        return sqrt (rl + rr);
  }
  
  if (r->symbol == S_COS) 
    return cos (rl + rr);
  if (r->symbol == S_SIN) 
    return sin (rl + rr);
  if (r->symbol == S_TAN) 
    return tan (rl + rr);
  if (r->symbol == S_ABS){
    float a = rl+rr;
    if(a < 0) a*= -1.f;
    return a;
  }
      
  return rl + rr;
}


void mode_calc_on_begin(){
  expr.lim = 100;
  expr.buf = &expr_buf[0];
  sarray_clear(expr);  

  number_literal.lim = 50;
  number_literal.buf = &number_literal_buf[0];
  sarray_clear(number_literal);

  float p = 0.000000000001f;
  float M = 4.f;
  float r = M / p;
  Serial.println(p);
  Serial.println(M);
  Serial.println(r);
}

void mode_calc_on_end(){

}


int mode_calc_on_press(int i){
    if(i == K_Y) {
        shift = 1;
        return 1;
    }
    return 1;
}

int mode_calc_on_release(int i){
  resetInactiveTime();
    calc_new_bytes = 1;
    if(i == K_Y) {
        shift = 0;
        return 1;
    } 
    
    if(i== K_R) {
      //equate(nullptr);
      Serial.println("expr count");
      Serial.println(expr.count);
      Serial.println(expr.lim);
      num_dot = 0;
      num_mode = 0;
      num_mode_old = 0;
      return 1;
    }

    if(i == K_P) {
      expr_push_symbol(S_ADD);
      num_mode = 0;
    }
    else if(i== K_N) {
      expr_push_symbol(S_SUB);
      num_mode = 0;
    }
    else if(i== K_D) 
    {
      expr_push_symbol(S_DIV);
      num_mode = 0;
    }
    else if(i== K_X) {
      expr_push_symbol(S_MUL);
      num_mode = 0;
    }

    else {
        if(!num_mode){
          num_mode_old = 1;
          num_mode = 1;
          num_dot = 0;
          expr_push_input();
          num_input = &expr.buf[expr.count-1];
        }

        if(i == K_DOT && !num_dot) {
          sarray_push(number_literal,'.');
          num_dot = 1;
        }
        else if(i == K_0) sarray_push(number_literal,'0');
        else if(i == K_1) sarray_push(number_literal,'1');
        else if(i == K_2) sarray_push(number_literal,'2');
        else if(i == K_3) sarray_push(number_literal,'3');
        else if(i == K_4) sarray_push(number_literal,'4');
        else if(i == K_5) sarray_push(number_literal,'5');
        else if(i == K_6) sarray_push(number_literal,'6');
        else if(i == K_7) sarray_push(number_literal,'7');
        else if(i == K_8) sarray_push(number_literal,'8');
        else if(i == K_9) sarray_push(number_literal,'9');
    }
    
    if(!num_mode && num_mode_old){
        float f = atof(number_literal.buf);

        sarray_clear(number_literal);
        for(int i=0; i<number_literal.lim; i++)
          number_literal.buf[i] = '\0';

        num_input->value = f;
        num_input->order = O_NUM;
        num_mode = 0;
        num_mode_old = 0;
    }
    
    return 1;
}

void mode_calc_on_gfx(){
  if(!calc_new_bytes) return;
  int x = 0;
  for(int i=0; i<expr.count; i++){
    
    u8g2.setCursor(x*8,48);
    auto r = &expr.buf[i];
    if(r->order == O_INPUT){
      u8g2.print(number_literal.buf);
      x+=number_literal.count;
      continue;
    }

    else if(r->order){
      if (r->symbol == S_SUB)
        u8g2.print('-');
      else if (r->symbol == S_ADD)
        u8g2.print('+');
      else if (r->symbol == S_MUL)
        u8g2.print('*');
      else if (r->symbol == S_DIV)
        u8g2.print('/');
      x+=1;
    }
    else {
      u8g2.print(r->value);
      x+=1;
    }
  }
}