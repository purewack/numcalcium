
#include "include/sys.h"
#include "include/comms.h"
#include "include/modes.h"
#include "include/util.h"
#include "include/number.h"

typedef struct Token token_t;

typedef struct Token
{
  #define O_NONE -1
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
  int vlen;

  token_t* o1;
  token_t* o2;
} token_t;

// darray_t<token_t> expr;
// darray_t<char> number_literal;

sarray_t<token_t> expr;
token_t expr_buf[100];
int expr_cursor;
int expr_cursor_inter;
int wstart;
int ovlc;
int ocx;
int cpos, ocpos;
int shift;
int calc_new_bytes = 0;

int expr_insert_number(){
  if(expr_cursor+1 == 100) return 0 ;
  if(expr.buf[expr_cursor+1].order == O_NUM) {
    //enter edit mode number
    expr_cursor_inter = 0;
    expr_cursor++;
    return 1;
  }

  if(expr.buf[expr_cursor].order > O_NUM)
    expr_cursor++;

  token_t nn;
  nn.order = O_NUM;
  clearNumber(nn.value);
  sarray_insert(expr,nn,expr_cursor);
  return 1;
 }

int expr_insert_symbol(int sym){
  
    // if(keypad_num_inputting){
    //   expr_insert_number();
    //   LOGln("end input");
    // }
    if(expr.buf[expr_cursor].order != O_NONE)
      expr_cursor++;
    

    token_t nn;
    nn.symbol = sym;
    switch(sym){
        case S_END:
            nn.order = O_FN_END;
            nn.vlen = 1;
        break;

        case S_SUB:
        case S_ADD:
            nn.order = O_PM;
            nn.vlen = 1;
        break;

        case S_MUL:
        case S_DIV:
            nn.order = O_MD;
            nn.vlen = 1;
        break;

        case S_POW:
            nn.order = O_PWR;
            nn.vlen = 1;
        break;

        default:
            nn.order = O_FN;
            nn.vlen = 1;
        break;
    }

    sarray_insert(expr,nn,expr_cursor);

    return 1;
}

double equate(token_t* r){
  double rl = 0.f;
  double rr = 0.f;

  if (r->o1)
      rl = equate (r->o1);
  if (r->o2)
      rr = equate (r->o2);
    
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
  expr_cursor = 0;
  expr_cursor_inter = 0;
  wstart = 0;
  ocpos = 0;
  cpos = 0;
  token_t cv = {0};
  cv.order = O_NONE;
  sarray_clear(expr, cv); 
}

void mode_calc_on_end(){

}

void mode_calc_on_nav(int d){
  auto e = &expr.buf[expr_cursor];
  if(e->order == O_NUM && expr_cursor_inter != -1){
    if(expr_cursor_inter == 0)
      expr_cursor_inter = d > 0 ? 1 : e->vlen;
    else if(expr_cursor_inter != -1){
      expr_cursor_inter += d;
      if(expr_cursor_inter <= 0 || expr_cursor_inter > e->vlen){
        expr_cursor_inter = -1;
        calc_new_bytes = 1;
        return;
      } 
    }
  }
  else{
    expr_cursor_inter = -1;  
  }

  if(expr_cursor_inter <= 0){
    expr_cursor_inter = 0;
    expr_cursor += d;
    if(expr_cursor < 0) expr_cursor = expr.count-1;
    if(expr_cursor > expr.count-1) expr_cursor = 0;
  }
  calc_new_bytes = 1;
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

    if(i == K_P) {
      expr_insert_symbol(S_ADD);
    }
    else if(i== K_N) {
      expr_insert_symbol(S_SUB);
    }
    else if(i== K_D) 
    {
      expr_insert_symbol(S_DIV);
    }
    else if(i== K_X) {
      if(!shift)
        expr_insert_symbol(S_MUL);
      else{
        if(expr.buf[expr_cursor].order == O_NUM){
          auto del = numberInputBackspace(expr.buf[expr_cursor].value, expr_cursor_inter);
          if(del) {
            sarray_remove(expr,expr_cursor);
            expr_cursor--;
          }
          else{
            expr.buf[expr_cursor].vlen = numberLength(expr.buf[expr_cursor].value);
            if(expr_cursor_inter > expr_buf[expr_cursor].vlen)
              expr_cursor_inter--;
          }
        }
        else{
          expr.buf[expr_cursor].order = O_NONE;
          expr.buf[expr_cursor].vlen = 0;
          sarray_remove(expr,expr_cursor);
          expr_cursor--;
        }

        if(expr_cursor < 0) 
          expr_cursor = 0;
      }
    }
    else{
      if(expr.buf[expr_cursor].order != O_NUM){
        expr_insert_number();
        expr.buf[expr_cursor].vlen = 0;
      }

      numberInputKey(expr.buf[expr_cursor].value, i, expr_cursor_inter);
      expr.buf[expr_cursor].vlen = numberLength(expr.buf[expr_cursor].value);
    }

    
    return 1;
}


//19 char wide line
void mode_calc_on_gfx(){
  if(!calc_new_bytes) return;

  //vlla == 21 max
  int ll = 0;
  int vlc = 0;
  int cpx = 0;
  int cpl = 0;
  int sx = 0;
  int cx = 0;
  for(int i=0; i<expr.count; i++){
    auto r = &expr.buf[i];
    
    if(i == expr_cursor){
      cpx = vlc+1;
    }

    ll = r->vlen;
    vlc += ll;

    if(i == expr_cursor){
      cpl = ll;  
      if(expr_cursor_inter>0){
        cx = cpl-ll;
        cx += expr_cursor_inter;
      }
    }
  
  } 



  #define WINW 21
  // int win_start = vlc > WINW ? vlc-WINW : 0;
  // int win_end = win_start + (vlc > WINW ? WINW : vlc);
  // if(cx && cpx+cx-1 < ocx) wstart = cpx+cx-1;
  // else if(vlc > ovlc && vlc > WINW) wstart = vlc-WINW;
  // ovlc = vlc;
  // ocx = cx ? cpx+cx-1 : cpx;
  // wstart = wstart<0 ? 0 : wstart;


  ocpos = cpos;
  cpos = (cx < 1 ? cpx+cpl-1 : cpx+cx-1);
  if(cpos <= wstart) wstart = cpos-1;
  if(cpos > wstart+WINW) wstart = cpos-WINW;

  LOGL("======= ");
  LOG("ll - ");LOGL(ll);
  LOG("vcl - ");LOGL(vlc);
  LOG("cpx - "); LOGL(cpx);
  LOG("cpl - ");LOGL(cpl);
  LOG("cx - ");LOGL(cx);
  LOG("expr_cursor - "); LOGL(expr_cursor);
  LOG("expr inter - "); LOGL(expr_cursor_inter);
  LOG("cpos - "); LOGL(cpos);
  LOG("wstart "); LOGL(wstart);
  LOGL("======= ");

  int x = (wstart)*(-6);

  for(int i=0; i<expr.count; i++){

    auto r = &expr.buf[i];
    int xx = x;
    
    if(r->order > O_NUM){
      if(x<0){
        x+=6;
        continue;
      }
      if(x>122) continue;

      u8g2.setCursor(x,32);
      if (r->symbol == S_SUB)
        u8g2.print('-');
      else if (r->symbol == S_ADD)
        u8g2.print('+');
      else if (r->symbol == S_MUL)
        u8g2.print('*');
      else if (r->symbol == S_DIV)
        u8g2.print('/');

      
      x += 6;
    }
    else {
      printNumber(r->value,x,32);
    }
    
    if(i == expr_cursor){
      u8g2.drawHLine(xx,33,r->vlen*6);
      if(expr_cursor_inter > 0){
        int xxx = xx - 6 + expr_cursor_inter*6;
        u8g2.drawHLine(xxx,35,6);
        //u8g2.drawHLine(xxx+5,36,1);
      }
    }
  }

  if(wstart){
    u8g2.setDrawColor(2);
    u8g2.drawBox(0,24,6,8);
    u8g2.setDrawColor(1);
  }
}