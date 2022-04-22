
#include "include/sys.h"
#include "include/comms.h"
#include "include/modes.h"
#include "include/util.h"
// #include <string>

typedef struct Token token_t;
typedef struct Token
{
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
  
  int order;
  union {
      float value;
      int symbol;
  };
  
  token_t* o1;
  token_t* o2;
} token_t;

darray_t<token_t> expr;
darray_t<char> number_literal;

// void expr_print(){
//     for(int i=0; i<expr_count; i++)
//         printf("i:%d (%d)[%f]\n",i,expr[i]->order,expr[i]->value);
// }

void expr_push_number(float value){
    token_t nn;
    nn.order = 0;
    nn.value = value;
    darray_push(expr,nn);
    return;
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

    darray_push(expr,nn);

    return;
}

void equate(){

}

void mode_calc_on_begin(){
    darray_clear(expr);  
}

void mode_calc_on_end(){

}

int num_mode = 0;
int num_mode_old = 0;
int shift;

int mode_calc_on_press(int i){
    if(i == K_Y) {
        shift = 1;
        return 1;
    }
    return 1;
}

int mode_calc_on_release(int i){
    if(i == K_Y) {
        shift = 0;
        return 1;
    } 
    
    if(i== K_R) equate();

    if(i == K_P) expr_push_symbol(S_ADD);
    else if(i== K_N) expr_push_symbol(S_SUB);
    else if(i== K_D) expr_push_symbol(S_DIV);
    else if(i== K_X) expr_push_symbol(S_SUB);

    else {
        num_mode = 1;
        if(i == K_DOT) darray_push(number_literal,'.');
        else if(i == K_0) darray_push(number_literal,'0');
        else if(i == K_1) darray_push(number_literal,'1');
        else if(i == K_2) darray_push(number_literal,'2');
        else if(i == K_3) darray_push(number_literal,'3');
        else if(i == K_4) darray_push(number_literal,'4');
        else if(i == K_5) darray_push(number_literal,'5');
        else if(i == K_6) darray_push(number_literal,'6');
        else if(i == K_7) darray_push(number_literal,'7');
        else if(i == K_8) darray_push(number_literal,'8');
        else if(i == K_9) darray_push(number_literal,'9');
    }
    
    if(!num_mode && num_mode_old){
        float f = atof(number_literal.buf);
        darray_clear(number_literal);
        expr_push_number(f);
        num_mode = 0;
    }
    
    return 1;
}
