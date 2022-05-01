
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
  int symbol;
  vnum_t value;

  token_t* o1;
  token_t* o2;
} token_t;

// darray_t<token_t> expr;
// darray_t<char> number_literal;

sarray_t<token_t> expr;
token_t expr_buf[100];
unsigned int expr_edit_index;
unsigned int expr_cursor;
int shift;
int calc_new_bytes = 0;

int expr_push_input(){
  if(expr_edit_index != -1) return 0;
  
  token_t nn;
  nn.order = O_INPUT;
  sarray_push(expr,nn);
  expr_edit_index = expr.count-1;
  expr_cursor = expr.count-1;

  return 1;
}

int expr_push_number(){
  if(expr_edit_index < 0) return 0;
  
  token_t* nn = &expr.buf[expr_edit_index];
  nn->order = O_NUM;
  nn->value = keypad_num;
  expr_cursor = expr.count-1;
  expr_edit_index = -1;

  return 1;
 }

int expr_push_symbol(int sym){
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
    expr_cursor = expr.count-1;

    return 1;
}

double equate(token_t* r){
  double rl = 0.f;
  double rr = 0.f;

  if (r->o1)
      rl = equate (r->o1);
  if (r->o2)
      rr = equate (r->o2);
    
//   print_token(r);
//   LOG("");
//   LOG(rl);
//   LOG(rr);
//   LOG(">>>>>>>>>>");
    
  if (r->order == O_NUM)
    return r->value.result;
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
  expr_cursor = 0;
  expr_edit_index = -1;
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
      LOGL("expr count");
      LOGL(expr.count);
      LOGL(expr.lim);
      LOGL("=");
      for(int i=0; i<expr.count; i++){
        auto t = &expr.buf[i];
        LOG(" o:");
        LOG(t->order);
        LOG('{');
        if(t->order == O_NUM){
          LOG('v');
          computeNumber(t->value);
          LOG(t->value.result);
        }
        else{
          LOG('s');
          LOG(t->symbol);
        }
        LOG('}');
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
      if(shift){
        if(sarray_peek(expr).order <= O_NUM){
          finish_number_input = 0;
          auto d = numberInputBackspace(keypad_num);
          if(keypad_num.e_dc == 0 && keypad_num.m_dc == 0){
            sarray_pop(expr);
            keypad_num_inputting = 0;
            expr_cursor = expr.count-1;
          }
        }
        else {
          sarray_pop(expr);
          expr_cursor = expr.count-1;
          
          if(expr.buf[expr_cursor].order <= O_NUM){
            expr.buf[expr_cursor].order = O_INPUT;
            keypad_num = expr.buf[expr_cursor].value;
            keypad_num_inputting = 1;
            beginNumberInput(keypad_num);
            Serial.println("re enter input");
          }
        }
      }
      else
        expr_push_symbol(S_MUL);
    }
    else{
      if(!keypad_num_inputting){
        keypad_num = {0};
        keypad_num_inputting = 1;
        expr_push_input();
        beginNumberInput(keypad_num);
        Serial.println("enter input");
      }

      numberInputKey(keypad_num, i);
      finish_number_input = 0;
      int x = 0;
    }

    if(finish_number_input && keypad_num_inputting){
      endNumberInput(keypad_num);
      expr_push_number();
      keypad_num_inputting = 0;
      Serial.println("end input");
      int x = 0;
    }
    
    return 1;
}


//19 char wide line
void mode_calc_on_gfx(){
  if(!calc_new_bytes) return;
  int x = 0;
  for(int i=0; i<expr.count; i++){
    
    auto r = &expr.buf[i];

    Serial.print(i);
    Serial.print("::");
    Serial.println(r->order);

    if(i == expr_edit_index){
      if(i == expr_cursor){
        auto e = keypad_num.e_dc;
        auto m = keypad_num.m_dc;
        if(e < 0) e*=-1;
        if(m < 0) m*=-1;
        auto w = e+m;
        if(keypad_num.m_dc < 0) w += 1;
        w*=6;
        u8g2.drawHLine(x,32,w);
      }
      print_vnum(keypad_num,x,32);
    }
    else{
      if(r->order > O_NUM){
        u8g2.setCursor(x,32);
        if (r->symbol == S_SUB)
          u8g2.print('-');
        else if (r->symbol == S_ADD)
          u8g2.print('+');
        else if (r->symbol == S_MUL)
          u8g2.print('*');
        else if (r->symbol == S_DIV)
          u8g2.print('/');

        if(i == expr_cursor){
          u8g2.drawHLine(x,32,8);
        }
        x += 6;
      }
      else {
        if(i == expr_cursor){
          auto w = keypad_num.e_dc+keypad_num.m_dc;
          if(keypad_num.m_dc > 0) w += 1;
          w*=6;
          u8g2.drawHLine(x,32,w);
        }
        print_vnum(r->value,x,32);
      }
    }

    
  }
}