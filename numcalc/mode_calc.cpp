
#include "include/sys.h"
#include "include/comms.h"
#include "include/modes.h"
#include "include/util.h"
#include "include/number.h"

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
      double value;
      int symbol;
  };

  int vrep_len;
  
  token_t* o1;
  token_t* o2;
} token_t;

// darray_t<token_t> expr;
// darray_t<char> number_literal;

sarray_t<token_t> expr;
token_t expr_buf[100];
int shift;
int calc_new_bytes = 0;

void expr_push_input(){
    token_t nn;
    nn.order = O_INPUT;
    sarray_push(expr,nn);
}

void expr_push_number(){
    token_t* nn = nullptr;
    for(int i=0; i<expr.count; i++){  
      if(expr.buf[i].order == O_INPUT) {
        nn = &expr.buf[i];
        break;
      }
    }
    nn->order = O_NUM;
    nn->value = getInputNumberResult();
    nn->vrep_len = keypad_num.dot ? keypad_num.rep.count : keypad_num.rep.count + 2;
}

void expr_push_symbol(int sym){
    token_t nn;
    nn.symbol = sym;
    switch(sym){
        case S_END:
            nn.order = O_FN_END;
            nn.vrep_len = 1;
        break;

        case S_SUB:
        case S_ADD:
            nn.order = O_PM;
            nn.vrep_len = 1;
        break;

        case S_MUL:
        case S_DIV:
            nn.order = O_MD;
            nn.vrep_len = 1;
        break;

        case S_POW:
            nn.order = O_PWR;
            nn.vrep_len = 1;
        break;

        default:
            nn.order = O_FN;
            nn.vrep_len = 4;
        break;
    }

    sarray_push(expr,nn);

    return;
}

double equate(token_t* r){
  double rl = 0.f;
  double rr = 0.f;

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
        double oo = pow (rr, (1.f/rl));
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
    double a = rl+rr;
    if(a < 0) a*= -1.f;
    return a;
  }
      
  return rl + rr;
}


void mode_calc_on_begin(){
  expr.lim = 100;
  expr.buf = expr_buf;
  sarray_clear(expr);  
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
      Serial.println("=");
      for(int i=0; i<expr.count; i++){
        auto t = &expr.buf[i];
        Serial.print(" o:");
        Serial.print(t->order);
        Serial.print('{');
        if(t->order == 0){
        Serial.print('v');
        Serial.print(t->value);
        }
        else{
        Serial.print('s');
        Serial.print(t->symbol);
        }
        Serial.print('}');
      }
      return 1;
    }

    bool finish_number_input = 1;
    if(i == K_P) {
      expr_push_symbol(S_ADD);
    }
    else if(i== K_N) {
      expr_push_symbol(S_SUB);
    }
    else if(i== K_D) 
    {
      expr_push_symbol(S_DIV);
    }
    else if(i== K_X) {
      expr_push_symbol(S_MUL);
    }

    else {
      if(!keypad_num.input) {
        expr_push_input();
        startInputNumber();
      }
        
      numberInputKey(i);
      finish_number_input = 0;
    }

    if(keypad_num.input && finish_number_input){
      expr_push_number();
      keypad_num.input = false;
    }
    
    return 1;
}


//19 char wide line
void mode_calc_on_gfx(){
  if(!calc_new_bytes) return;
  int x = 0;
  for(int i=0; i<expr.count; i++){
    
    auto r = &expr.buf[i];
    u8g2.setCursor(x*8,32);

    if(r->order == O_INPUT){
      for(int i=0; i<keypad_num.rep.count; i++){
        u8g2.setCursor(x*8 + i*8,32);
        u8g2.print(keypad_num.rep.buf[i]);
      }

      x += keypad_num.rep.count;
      continue;
    }
    
    if(r->order != O_NUM){
      if (r->symbol == S_SUB)
        u8g2.print('-');
      else if (r->symbol == S_ADD)
        u8g2.print('+');
      else if (r->symbol == S_MUL)
        u8g2.print('*');
      else if (r->symbol == S_DIV)
        u8g2.print('/');
    }
    else {
      u8g2.print(r->value);
    }
    
    x += r->vrep_len;
  }
}