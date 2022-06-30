
#include "include/sys.h"
#include "include/comms.h"
#include "include/modes.h"
#include "include/util.h"
#include "include/number.h"

#undef LOGL
#define LOGL(X) 
#undef LOG
#define LOG(X) 

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

  #define S_END -2
  #define S_GRP -1
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
bool eng;
bool shift;
bool secondF;
int calc_new_bytes = 0;
double result;
token_t* parsed;


void print_token(token_t* r){
#ifdef DEBUG
    if(r == nullptr) {
        LOG("*dead*");
        return;
    }
    
    if(r->order == O_NONE)
        LOG("?");
    else if(r->order == O_NUM)
        LOG(computeNumber(r->value));
    else if(r->order == O_PM && r->symbol == S_ADD)
        LOG("-");
    else if(r->order == O_PM && r->symbol == S_SUB)
        LOG("+");
    else if(r->order == O_MD && r->symbol == S_MUL)
        LOG("*");
    else if(r->order == O_MD && r->symbol == S_DIV)
        LOG("/");
    else if(r->order == O_PWR && r->symbol == S_POW)
        LOG("^");
    else if(r->order == O_FN && r->symbol == S_COS)
        LOG("cos(");
    else if(r->order == O_FN && r->symbol == S_END)
        LOG("(");
    else if(r->order == O_FN_END)
        LOG(")");
#endif
}


void
print_tree (token_t * r, int l)
{
#ifdef DEBUG
  if(!r) return;

  auto indent =[](int l) {
    for (int i = 0; i < l; i++)
      LOG("\t");
  };

  indent (l);

  //std::cout << r;
  LOG("[");
  print_token(r);
  LOGL("]");
    

  if (r->o1){
    LOG("o1:"); 
    print_tree (r->o1, l + 1);
  }
  if (r->o2){
    LOG("o2:"); 
    print_tree (r->o2, l + 1);
  }
#endif
}


int ii;
token_t* trickle(token_t* cur, token_t* o2){
    if(cur->o2){
        LOGL("TR: have o2");
        cur->o2 = trickle(cur->o2,o2);
        LOGL("o2 done");
        return cur;
    }
    if(o2->order == O_PWR or (o2->order==O_FN && cur->order == O_NUM)) { // ^ or N sqrt
        LOGL("TR: have PWR or ROOT");
        o2->o1 = cur;
        cur = o2;
    }
    else{ // number
    LOGL("have num");
        cur->o2 = o2;
    }
    LOGL("TRICKLETRICKLETRICKLE");
    print_tree(cur,2);
    return cur;
}

token_t* parse(token_t* cur, int id, int& status){
    if(parsed) return parsed;

    if(expr.count == 0) {
        LOGL("parse: no expr");
        return nullptr;
    }
    
    if(ii >= expr.count) {
        LOGL("parse: end of expr");
        return cur;
    }
        
    LOGL("parse()=-----------------------------------------");
    LOGL(id);
    LOGL("===");
    
    auto self = &expr.buf[ii];
    LOG(">>>>>>>>>>>>>>>");
    print_token(self);
    LOG("\n>>>>>>>>>>>>>>>");
    print_tree(cur,0);
    LOG("\n>>>>>>>>>>>>>>>");
    
    if(id == 0){
        LOGL("\nNEW GRP GGGGGGGGGGGGGGGGGGGGGGGGGGGGGG");
    
        if(self->order == O_FN){
            ii++;
            self->o2 = parse(nullptr,0,status);
        }
        if(cur==nullptr) cur = self;
        ii++;
        return parse(self,id+1,status);
    }
    

    if(self->order == O_NUM or self->order == O_FN){
        LOGL("parse: TRICKLE");
        print_tree(cur,0);
        cur = trickle(cur,self);
        
        //start a new group if self is grouped
        if(self->order == O_FN){
            ii++;
            LOGL("new group");
            self->o2 = parse(nullptr,0,status);
            //self->o1 = self->o2;
        }
        
        LOGL("next after number");
        ii++;
        cur = parse(cur, id+1,status);
    }
    
    else{
        if(self->order == O_PM){ // + or -
            LOGL("have PM");
            self->o1 = cur;
            cur = self;
            ii++;
            cur = parse(self, id+1, status);
        }
        else if(self->order == O_MD){ // * or /
        
            if(cur->order == O_NUM){
                LOGL("have MD with NUM");
                self->o1 = cur;
                print_tree(self,0);
                ii++;
                cur = parse(self, id+1, status);
            }
            else{
                if(cur->order >= self->order){
                    self->o1 = cur;
                    cur = self;
                }else{
                    self->o1 = cur->o2;
                    cur->o2 = self;
                }
                LOGL("have MD else");
                print_tree(cur,0);
                ii++;
                cur = parse(cur, id+1, status);
            }
            
        }
        else if(self->order == O_PWR){ // exp ^
            LOGL("have exp");
            cur = trickle(cur,self);
            print_tree(cur,0);
            ii++;
            cur = parse(cur, id+1, status);
        }
        else if(self->order == O_FN_END){ //brac end
            LOGL("end brac ))))");
            return cur;
        }
        else{
            //parse error
            return nullptr;
        }
    }
    LOGL("WRAPWRAPWRAPWRAPWRAP");
    print_tree(cur,0);
    return cur;
}

double
compute_expr (token_t * r)
{
  double rl = 0.f;
  double rr = 0.f;

  if (r->o1)
    {
      rl = compute_expr (r->o1);
    }
  if (r->o2)
    {
      rr = compute_expr (r->o2);
    }
    
  print_token(r);
  LOG("");
  LOG(rl);
  LOG(rr);
  LOG(">>>>>>>>>>");
    
  if (r->order == O_NUM)
    return computeNumber(r->value);
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
  if (r->symbol == S_ABS) 
    return abs (rl + rr);
      
  return rl + rr;
}



int expr_insert_number(){
  if(expr_cursor+1 == 100) return 0 ;
  if(expr.buf[expr_cursor+1].order == O_NUM) {
    //enter edit mode number
    expr_cursor_inter = 0;
    expr_cursor++;
    return 0;
  }

  if(expr.buf[expr_cursor].order > O_NUM)
    expr_cursor++;

  token_t nn;
  nn.order = O_NUM;
  clearNumber(nn.value);
  sarray_insert(expr,nn,expr_cursor);
  return 0;
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
        
        case S_COS:
            nn.order = O_FN;
            nn.vlen = 4;
        break;

        case S_SIN:
            nn.order = O_FN;
            nn.vlen = 4;
        break;

        case S_TAN:
            nn.order = O_FN;
            nn.vlen = 4;
        break;

        case S_ABS:
            nn.order = O_FN;
            nn.vlen = 4;
        break;

        case S_SQR:
            nn.order = O_FN;
            nn.vlen = 5;
        break;

        default:
            nn.order = O_FN;
            nn.vlen = 1;
        break;
    }

    sarray_insert(expr,nn,expr_cursor);

    return 0;
}

void clearAll(){
  expr_cursor = 0;
  expr_cursor_inter = 0;
  wstart = 0;
  token_t cv = {0};
  cv.order = O_NONE;
  cv.symbol = S_END;
  sarray_clear(expr, cv); 
  result = 0.0;
  parsed = nullptr;
  secondF = false;
  calc_new_bytes = 1;
}

void mode_calc_on_begin(){
  eng = 1;
  expr.lim = 100;
  expr.buf = expr_buf; 
  clearAll();
  lcd_clear();
  drawTitle();
  lcd_update();
}

void mode_calc_on_end(){

}

void mode_calc_on_process(){
//void mode_calc_on_nav(int d){

    

    if(io.turns_left || io.turns_right){
        int d = 0;
        if(io.turns_left){
            d = -io.turns_left;
            io.turns_left = 0;
        }
        else if(io.turns_right){
            d = io.turns_right;
            io.turns_right = 0;
        }
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
        return;
    }


    //int mode_calc_on_press(int i){
    if(io.bscan_down & (1<<K_Y)){
        io.bscan_down = 0;
        shift = 1;
        return;
    }


//int mode_calc_on_release(int i){
    if(io.bscan_up){
        int i = io.bscan_up;
        io.bscan_up = 0;
        calc_new_bytes = 1;
        if(i == (1<<K_Y)) {
            shift = 0;
            return;
        } 

        if(shift && i==(1<<K_F3)){
            clearAll();
            return;
        }

        if(i== (1<<K_R)) {
            if(shift){
                expr_insert_symbol(S_END);
                return;
            }

            if(!parsed){
                LOGL("clear expr childs");
                for(int i=0; i<expr.count; i++){
                expr.buf[i].o1 = nullptr;
                expr.buf[i].o2 = nullptr;
                }
            }

            LOGL("start parse");
            int status;
            ii = 0;
            // hud.drawBox(0,0,128,8);
            // hud.sendBuffer();
            parsed = parse(nullptr,0,status);
            LOGL("end parse");
            
            if(parsed) LOGL("have tree");
            result = compute_expr(parsed);
            LOGL(result);
            LOGL("result");
            return;
        }

        parsed = nullptr;

        if(i == (1<<K_P)) {
            expr_insert_symbol(shift ? S_POW : S_ADD);
        }
        else if(i== (1<<K_N)) {
            expr_insert_symbol(S_SUB);
        }
        else if(i== (1<<K_D)) 
        {
            expr_insert_symbol(S_DIV);
        }
        else if(i== (1<<K_X)) {
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
        if(shift){
            if(i==(1<<K_DOT)) expr_insert_symbol(S_GRP);
            if(i==(1<<K_2)) secondF = true;
            return;
        }else if(secondF){
            if(i==(1<<K_1)) expr_insert_symbol(S_SIN);
            if(i==(1<<K_2)) expr_insert_symbol(S_COS);
            if(i==(1<<K_3)) expr_insert_symbol(S_TAN);
            if(i==(1<<K_4)) expr_insert_symbol(S_SQR);
            if(i==(1<<K_5)) expr_insert_symbol(S_ABS);
            secondF = false;
            return;
        }

        if(expr.buf[expr_cursor].order != O_NUM){
            expr_insert_number();
            expr.buf[expr_cursor].vlen = 0;
        }

        numberInputKey(expr.buf[expr_cursor].value, i, expr_cursor_inter);
        expr.buf[expr_cursor].vlen = numberLength(expr.buf[expr_cursor].value);
        }

        
        return;
    }


//19 char wide line
//void mode_calc_on_gfx(){
    if(!calc_new_bytes) return;
    calc_new_bytes = 0;
    clearProgGFX();

// if(secondF){
//     hud.setCursor(0,18);
//     hud.print("1=sin 2=cos 3=tan");
//     hud.setCursor(0,18+9*1);
//     hud.print("4=sqrt 5=abs P=pow");
//     return;
// }

    //vlla == 21 max
    const int y = 32-8;
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
    int cpos = (cx < 1 ? cpx+cpl-1 : cpx+cx-1);
    if(cpos <= wstart) wstart = cpos-1;
    if(cpos > wstart+WINW) wstart = cpos-WINW;
    if(wstart < 0) wstart = 0;

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

            if (r->symbol == S_SUB)
                lcd_drawChar(x,y,sys_font,'-');
            else if (r->symbol == S_ADD)
                lcd_drawChar(x,y,sys_font,'+');
            else if (r->symbol == S_MUL)
                lcd_drawChar(x,y,sys_font,'*');
            else if (r->symbol == S_DIV)
                lcd_drawChar(x,y,sys_font,'/');
            else if (r->symbol == S_POW)
                lcd_drawChar(x,y,sys_font,'^');
            else if (r->symbol == S_COS)
                lcd_drawString(x,y,sys_font,"cos(");
            else if (r->symbol == S_SIN)
                lcd_drawString(x,y,sys_font,"sin(");
            else if (r->symbol == S_COS)
                lcd_drawString(x,y,sys_font,"cos(");
            else if (r->symbol == S_TAN)
                lcd_drawString(x,y,sys_font,"tan(");
            else if (r->symbol == S_ABS)
                lcd_drawString(x,y,sys_font,"abs(");
            else if (r->symbol == S_SQR)
                lcd_drawString(x,y,sys_font,"sqrt(");
            else if (r->symbol == S_SQR)
                lcd_drawString(x,y,sys_font,"sqrt(");
            else if (r->symbol == S_GRP)
                lcd_drawChar(x,y,sys_font,'(');
            else if (r->symbol == S_END)
                lcd_drawChar(x,y,sys_font,')');

            x += 6*r->vlen;
        }
        else {
            printNumber(r->value,x,y);
        }
        
        if(i == expr_cursor){
            lcd_drawHline(xx,y+9,r->vlen*6);
            if(expr_cursor_inter > 0){
                int xxx = xx - 6 + expr_cursor_inter*6;
                lcd_drawHline(xxx,y+11,6);
                //hud.drawHLine(xxx+5,36,1);
            }
        }
    }

// if(wstart){
//     hud.setDrawColor(2);
//     hud.drawBox(0,24,6,8);
//     hud.setDrawColor(1);
// }

    
    char str[64];
    const char* sign = (result < 0) ? "-" : " ";
    double val = (result < 0) ? -result : result;
    long exp = long(val);
    double frac_d = val-double(exp);
    int cc = snprintf(str,64,"%ld",exp);
    if(frac_d < 0.000001){
    	long frac = long(frac_d * pow(10,12));
    	snprintf(str,64,"%s%d.%012d",sign,exp,frac);
    }else{
    	long frac = long(frac_d * pow(10,6));
    	snprintf(str,64,"%s%d.%06d...",sign,exp,frac);
    }
    for(int i=0;i<(frac_d < 0.000001 ? 4 : 2);i++)
    	lcd_drawHline(10+6*(cc+2+(3*i)),63,(3*6)-3);

    lcd_drawChar(0,64-10,sys_font,'=');
    lcd_drawString(8,64-10,sys_font,str);

    if(eng){//engineering notation
    	
	if(abs(result) < 1){
    	lcd_drawChar(0,64-10-8,sys_font,'=');
	      	if(result < 0.000000001) {
    			long frac = long(frac_d * pow(10,12));
	       		snprintf(str,64,"%dp",frac);	       
		}
		else if(result < 0.000001) {
    			long frac = long(frac_d * pow(10,9));
	       		snprintf(str,64,"%dn",frac);	       
		}
		else if(result < 0.001) {
    			long frac = long(frac_d * pow(10,6));
	       		snprintf(str,64,"%du",frac);	       
		}
		else{
    			long frac = long(frac_d * pow(10,3));
	       		snprintf(str,64,"%dm",frac);	       
		}
		lcd_drawString(16,64-10-8,sys_font,str);
	}
    }
    else{ //SI notation
    
    }

    updateProgGFX();
}
