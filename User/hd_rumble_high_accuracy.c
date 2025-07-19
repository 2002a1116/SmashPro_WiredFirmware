#include "hd_rumble_high_accuracy.h"
#include "global_api.h"
void TIM3_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
int16_t ccr_lookup_tb[]={
        0,3,6,9,12,15,18,21,25,28,31,34,37,40,43,
        47,50,53,56,59,62,65,69,72,75,78,81,84,87,90,
        94,97,100,103,106,109,112,115,119,122,125,128,131,134,137,
        140,144,147,150,153,156,159,162,165,168,171,175,178,181,184,
        187,190,193,196,199,202,205,209,212,215,218,221,224,227,230,
        233,236,239,242,245,248,251,254,257,260,264,267,270,273,276,
        279,282,285,288,291,294,297,300,303,306,309,312,315,318,321,
        324,327,330,333,336,339,342,344,347,350,353,356,359,362,365,
        368,371,374,377,380,383,386,388,391,394,397,400,403,406,409,
        412,414,417,420,423,426,429,432,434,437,440,443,446,449,451,
        454,457,460,463,466,468,471,474,477,479,482,485,488,491,493,
        496,499,501,504,507,510,512,515,518,521,523,526,529,531,534,
        537,539,542,545,547,550,553,555,558,561,563,566,568,571,574,
        576,579,581,584,587,589,592,594,597,599,602,604,607,609,612,
        615,617,620,622,625,627,629,632,634,637,639,642,644,647,649,
        652,654,656,659,661,664,666,668,671,673,675,678,680,683,685,
        687,690,692,694,696,699,701,703,706,708,710,712,715,717,719,
        721,724,726,728,730,732,735,737,739,741,743,745,748,750,752,
        754,756,758,760,762,765,767,769,771,773,775,777,779,781,783,
        785,787,789,791,793,795,797,799,801,803,805,807,809,811,813,
        814,816,818,820,822,824,826,828,829,831,833,835,837,839,840,
        842,844,846,847,849,851,853,854,856,858,860,861,863,865,866,
        868,870,871,873,875,876,878,879,881,883,884,886,887,889,890,
        892,894,895,897,898,900,901,903,904,906,907,908,910,911,913,
        914,916,917,918,920,921,922,924,925,927,928,929,930,932,933,
        934,936,937,938,939,941,942,943,944,946,947,948,949,950,951,
        953,954,955,956,957,958,959,960,962,963,964,965,966,967,968,
        969,970,971,972,973,974,975,976,977,978,978,979,980,981,982,
        983,984,985,986,986,987,988,989,990,990,991,992,993,994,994,
        995,996,997,997,998,999,999,1000,1001,1001,1002,1003,1003,1004,1004,
        1005,1006,1006,1007,1007,1008,1008,1009,1009,1010,1010,1011,1011,1012,1012,
        1013,1013,1014,1014,1015,1015,1015,1016,1016,1017,1017,1017,1018,1018,1018,
        1019,1019,1019,1019,1020,1020,1020,1020,1021,1021,1021,1021,1022,1022,1022,
        1022,1022,1022,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,
        1023,1023,1024,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,
        1023,1023,1022,1022,1022,1022,1022,1022,1021,1021,1021,1021,1020,1020,1020,
        1020,1019,1019,1019,1019,1018,1018,1018,1017,1017,1017,1016,1016,1015,1015,
        1015,1014,1014,1013,1013,1012,1012,1011,1011,1010,1010,1009,1009,1008,1008,
        1007,1007,1006,1006,1005,1004,1004,1003,1003,1002,1001,1001,1000,999,999,
        998,997,997,996,995,994,994,993,992,991,990,990,989,988,987,
        986,986,985,984,983,982,981,980,979,978,978,977,976,975,974,
        973,972,971,970,969,968,967,966,965,964,963,962,960,959,958,
        957,956,955,954,953,951,950,949,948,947,946,944,943,942,941,
        939,938,937,936,934,933,932,930,929,928,927,925,924,922,921,
        920,918,917,916,914,913,911,910,908,907,906,904,903,901,900,
        898,897,895,894,892,890,889,887,886,884,883,881,879,878,876,
        875,873,871,870,868,866,865,863,861,860,858,856,854,853,851,
        849,847,846,844,842,840,839,837,835,833,831,829,828,826,824,
        822,820,818,816,814,813,811,809,807,805,803,801,799,797,795,
        793,791,789,787,785,783,781,779,777,775,773,771,769,767,765,
        762,760,758,756,754,752,750,748,745,743,741,739,737,735,732,
        730,728,726,724,721,719,717,715,712,710,708,706,703,701,699,
        696,694,692,690,687,685,683,680,678,675,673,671,668,666,664,
        661,659,656,654,652,649,647,644,642,639,637,634,632,629,627,
        625,622,620,617,615,612,609,607,604,602,599,597,594,592,589,
        587,584,581,579,576,574,571,568,566,563,561,558,555,553,550,
        547,545,542,539,537,534,531,529,526,523,521,518,515,512,510,
        507,504,501,499,496,493,491,488,485,482,479,477,474,471,468,
        466,463,460,457,454,451,449,446,443,440,437,434,432,429,426,
        423,420,417,414,412,409,406,403,400,397,394,391,388,386,383,
        380,377,374,371,368,365,362,359,356,353,350,347,344,342,339,
        336,333,330,327,324,321,318,315,312,309,306,303,300,297,294,
        291,288,285,282,279,276,273,270,267,264,260,257,254,251,248,
        245,242,239,236,233,230,227,224,221,218,215,212,209,205,202,
        199,196,193,190,187,184,181,178,175,171,168,165,162,159,156,
        153,150,147,144,140,137,134,131,128,125,122,119,115,112,109,
        106,103,100,97,94,90,87,84,81,78,75,72,69,65,62,
        59,56,53,50,47,43,40,37,34,31,28,25,21,18,15,
        12,9,6,3,0,-3,-6,-9,-12,-15,-18,-21,-25,-28,-31,
        -34,-37,-40,-43,-47,-50,-53,-56,-59,-62,-65,-69,-72,-75,-78,
        -81,-84,-87,-90,-94,-97,-100,-103,-106,-109,-112,-115,-119,-122,-125,
        -128,-131,-134,-137,-140,-144,-147,-150,-153,-156,-159,-162,-165,-168,-171,
        -175,-178,-181,-184,-187,-190,-193,-196,-199,-202,-205,-209,-212,-215,-218,
        -221,-224,-227,-230,-233,-236,-239,-242,-245,-248,-251,-254,-257,-260,-264,
        -267,-270,-273,-276,-279,-282,-285,-288,-291,-294,-297,-300,-303,-306,-309,
        -312,-315,-318,-321,-324,-327,-330,-333,-336,-339,-342,-344,-347,-350,-353,
        -356,-359,-362,-365,-368,-371,-374,-377,-380,-383,-386,-388,-391,-394,-397,
        -400,-403,-406,-409,-412,-414,-417,-420,-423,-426,-429,-432,-434,-437,-440,
        -443,-446,-449,-451,-454,-457,-460,-463,-466,-468,-471,-474,-477,-479,-482,
        -485,-488,-491,-493,-496,-499,-501,-504,-507,-510,-512,-515,-518,-521,-523,
        -526,-529,-531,-534,-537,-539,-542,-545,-547,-550,-553,-555,-558,-561,-563,
        -566,-568,-571,-574,-576,-579,-581,-584,-587,-589,-592,-594,-597,-599,-602,
        -604,-607,-609,-612,-615,-617,-620,-622,-625,-627,-629,-632,-634,-637,-639,
        -642,-644,-647,-649,-652,-654,-656,-659,-661,-664,-666,-668,-671,-673,-675,
        -678,-680,-683,-685,-687,-690,-692,-694,-696,-699,-701,-703,-706,-708,-710,
        -712,-715,-717,-719,-721,-724,-726,-728,-730,-732,-735,-737,-739,-741,-743,
        -745,-748,-750,-752,-754,-756,-758,-760,-762,-765,-767,-769,-771,-773,-775,
        -777,-779,-781,-783,-785,-787,-789,-791,-793,-795,-797,-799,-801,-803,-805,
        -807,-809,-811,-813,-814,-816,-818,-820,-822,-824,-826,-828,-829,-831,-833,
        -835,-837,-839,-840,-842,-844,-846,-847,-849,-851,-853,-854,-856,-858,-860,
        -861,-863,-865,-866,-868,-870,-871,-873,-875,-876,-878,-879,-881,-883,-884,
        -886,-887,-889,-890,-892,-894,-895,-897,-898,-900,-901,-903,-904,-906,-907,
        -908,-910,-911,-913,-914,-916,-917,-918,-920,-921,-922,-924,-925,-927,-928,
        -929,-930,-932,-933,-934,-936,-937,-938,-939,-941,-942,-943,-944,-946,-947,
        -948,-949,-950,-951,-953,-954,-955,-956,-957,-958,-959,-960,-962,-963,-964,
        -965,-966,-967,-968,-969,-970,-971,-972,-973,-974,-975,-976,-977,-978,-978,
        -979,-980,-981,-982,-983,-984,-985,-986,-986,-987,-988,-989,-990,-990,-991,
        -992,-993,-994,-994,-995,-996,-997,-997,-998,-999,-999,-1000,-1001,-1001,-1002,
        -1003,-1003,-1004,-1004,-1005,-1006,-1006,-1007,-1007,-1008,-1008,-1009,-1009,-1010,-1010,
        -1011,-1011,-1012,-1012,-1013,-1013,-1014,-1014,-1015,-1015,-1015,-1016,-1016,-1017,-1017,
        -1017,-1018,-1018,-1018,-1019,-1019,-1019,-1019,-1020,-1020,-1020,-1020,-1021,-1021,-1021,
        -1021,-1022,-1022,-1022,-1022,-1022,-1022,-1023,-1023,-1023,-1023,-1023,-1023,-1023,-1023,
        -1023,-1023,-1023,-1023,-1023,-1023,-1024,-1023,-1023,-1023,-1023,-1023,-1023,-1023,-1023,
        -1023,-1023,-1023,-1023,-1023,-1023,-1022,-1022,-1022,-1022,-1022,-1022,-1021,-1021,-1021,
        -1021,-1020,-1020,-1020,-1020,-1019,-1019,-1019,-1019,-1018,-1018,-1018,-1017,-1017,-1017,
        -1016,-1016,-1015,-1015,-1015,-1014,-1014,-1013,-1013,-1012,-1012,-1011,-1011,-1010,-1010,
        -1009,-1009,-1008,-1008,-1007,-1007,-1006,-1006,-1005,-1004,-1004,-1003,-1003,-1002,-1001,
        -1001,-1000,-999,-999,-998,-997,-997,-996,-995,-994,-994,-993,-992,-991,-990,
        -990,-989,-988,-987,-986,-986,-985,-984,-983,-982,-981,-980,-979,-978,-978,
        -977,-976,-975,-974,-973,-972,-971,-970,-969,-968,-967,-966,-965,-964,-963,
        -962,-960,-959,-958,-957,-956,-955,-954,-953,-951,-950,-949,-948,-947,-946,
        -944,-943,-942,-941,-939,-938,-937,-936,-934,-933,-932,-930,-929,-928,-927,
        -925,-924,-922,-921,-920,-918,-917,-916,-914,-913,-911,-910,-908,-907,-906,
        -904,-903,-901,-900,-898,-897,-895,-894,-892,-890,-889,-887,-886,-884,-883,
        -881,-879,-878,-876,-875,-873,-871,-870,-868,-866,-865,-863,-861,-860,-858,
        -856,-854,-853,-851,-849,-847,-846,-844,-842,-840,-839,-837,-835,-833,-831,
        -829,-828,-826,-824,-822,-820,-818,-816,-814,-813,-811,-809,-807,-805,-803,
        -801,-799,-797,-795,-793,-791,-789,-787,-785,-783,-781,-779,-777,-775,-773,
        -771,-769,-767,-765,-762,-760,-758,-756,-754,-752,-750,-748,-745,-743,-741,
        -739,-737,-735,-732,-730,-728,-726,-724,-721,-719,-717,-715,-712,-710,-708,
        -706,-703,-701,-699,-696,-694,-692,-690,-687,-685,-683,-680,-678,-675,-673,
        -671,-668,-666,-664,-661,-659,-656,-654,-652,-649,-647,-644,-642,-639,-637,
        -634,-632,-629,-627,-625,-622,-620,-617,-615,-612,-609,-607,-604,-602,-599,
        -597,-594,-592,-589,-587,-584,-581,-579,-576,-574,-571,-568,-566,-563,-561,
        -558,-555,-553,-550,-547,-545,-542,-539,-537,-534,-531,-529,-526,-523,-521,
        -518,-515,-512,-510,-507,-504,-501,-499,-496,-493,-491,-488,-485,-482,-479,
        -477,-474,-471,-468,-466,-463,-460,-457,-454,-451,-449,-446,-443,-440,-437,
        -434,-432,-429,-426,-423,-420,-417,-414,-412,-409,-406,-403,-400,-397,-394,
        -391,-388,-386,-383,-380,-377,-374,-371,-368,-365,-362,-359,-356,-353,-350,
        -347,-344,-342,-339,-336,-333,-330,-327,-324,-321,-318,-315,-312,-309,-306,
        -303,-300,-297,-294,-291,-288,-285,-282,-279,-276,-273,-270,-267,-264,-260,
        -257,-254,-251,-248,-245,-242,-239,-236,-233,-230,-227,-224,-221,-218,-215,
        -212,-209,-205,-202,-199,-196,-193,-190,-187,-184,-181,-178,-175,-171,-168,
        -165,-162,-159,-156,-153,-150,-147,-144,-140,-137,-134,-131,-128,-125,-122,
        -119,-115,-112,-109,-106,-103,-100,-97,-94,-90,-87,-84,-81,-78,-75,
        -72,-69,-65,-62,-59,-56,-53,-50,-47,-43,-40,-37,-34,-31,-28,
        -25,-21,-18,-15,-12,-9,-6,-3};//10 bit offset
hd_rumble_high_accurary_pack left_high,left_low,right_high,right_low;
hd_rumble_high_accurary_pack left_high_buf,left_low_buf,right_high_buf,right_low_buf;
uint8_t left_high_buf_rdy,left_low_buf_rdy,right_high_buf_rdy,right_low_buf_rdy;
uint8_t left_high_buf_stdby,left_low_buf_stdby,right_high_buf_stdby,right_low_buf_stdby;
uint16_t left_high_pos,left_low_pos,right_high_pos,right_low_pos;
uint32_t left_high_sum,left_low_sum,right_high_sum,right_low_sum;
ring_buffer left_high_rb,left_low_rb,right_high_rb,right_low_rb;
uint8_t left_high_rb_buf[HD_RUMBLE_HIGH_ACC_RINGBUFFER_CAP*HD_RUMBLE_HIGH_ACC_PACK_SIZE];
uint8_t left_low_rb_buf[HD_RUMBLE_HIGH_ACC_RINGBUFFER_CAP*HD_RUMBLE_HIGH_ACC_PACK_SIZE];
uint8_t right_high_rb_buf[HD_RUMBLE_HIGH_ACC_RINGBUFFER_CAP*HD_RUMBLE_HIGH_ACC_PACK_SIZE];
uint8_t right_low_rb_buf[HD_RUMBLE_HIGH_ACC_RINGBUFFER_CAP*HD_RUMBLE_HIGH_ACC_PACK_SIZE];
uint8_t rumble_rb_len[4][HD_RUMBLE_HIGH_ACC_RINGBUFFER_CAP];
void hd_rumble_high_accurary_init(){
    ring_buffer_init(&left_high_rb, left_high_rb_buf, rumble_rb_len[0], HD_RUMBLE_HIGH_ACC_RINGBUFFER_CAP, HD_RUMBLE_HIGH_ACC_PACK_SIZE);
    ring_buffer_init(&left_low_rb, left_low_rb_buf, rumble_rb_len[1], HD_RUMBLE_HIGH_ACC_RINGBUFFER_CAP, HD_RUMBLE_HIGH_ACC_PACK_SIZE);
    ring_buffer_init(&right_high_rb, right_high_rb_buf, rumble_rb_len[2], HD_RUMBLE_HIGH_ACC_RINGBUFFER_CAP, HD_RUMBLE_HIGH_ACC_PACK_SIZE);
    ring_buffer_init(&right_low_rb, right_low_rb_buf, rumble_rb_len[3], HD_RUMBLE_HIGH_ACC_RINGBUFFER_CAP, HD_RUMBLE_HIGH_ACC_PACK_SIZE);
    left_high_buf_stdby=left_low_buf_stdby=right_high_buf_stdby=right_low_buf_stdby=1;
}
uint8_t rumble_rb_overflow=0;
void push_waveform(uint8_t channel,hd_rumble_high_accurary_pack* ptr)
{
    uint8_t res=0;
    if(!ptr)return;
    switch(channel)
    {
    case 0:
        res=ring_buffer_push(&left_high_rb, (uint8_t*)ptr, HD_RUMBLE_HIGH_ACC_PACK_SIZE);
        //if(ptr->amp)
        //    //printf("high amp:%d\r\n",ptr->amp);
        break;
    case 2:
        res=ring_buffer_push(&left_low_rb, (uint8_t*)ptr, HD_RUMBLE_HIGH_ACC_PACK_SIZE);
        //if(ptr->amp)
        //    //printf("low amp:%d\r\n",ptr->amp);
        break;
    case 1:
        res=ring_buffer_push(&right_high_rb, (uint8_t*)ptr, HD_RUMBLE_HIGH_ACC_PACK_SIZE);
        break;
    case 3:
        res=ring_buffer_push(&right_low_rb, (uint8_t*)ptr, HD_RUMBLE_HIGH_ACC_PACK_SIZE);
        break;
    default:
        break;
    }
    if(res){
        rumble_rb_overflow=1;
        flush_rgb(ENABLE);
    }
}
void push_waveform_into_buffer_task()
{
    if(left_high_buf_stdby&&left_high_rb.size){
        left_high_buf_stdby=0;
        memcpy(&left_high_buf,&left_high_rb_buf[left_high_rb.top*HD_RUMBLE_HIGH_ACC_PACK_SIZE],HD_RUMBLE_HIGH_ACC_PACK_SIZE);
        ring_buffer_pop(&left_high_rb);
        left_high_buf_rdy=1;
    }
    if(left_low_buf_stdby&&left_low_rb.size){
        left_low_buf_stdby=0;
        memcpy(&left_low_buf,&left_low_rb_buf[left_low_rb.top*HD_RUMBLE_HIGH_ACC_PACK_SIZE],HD_RUMBLE_HIGH_ACC_PACK_SIZE);
        ring_buffer_pop(&left_low_rb);
        left_low_buf_rdy=1;
    }
    if(right_high_buf_stdby&&right_high_rb.size){
        right_high_buf_stdby=0;
        memcpy(&right_high_buf,&right_high_rb_buf[right_high_rb.top*HD_RUMBLE_HIGH_ACC_PACK_SIZE],HD_RUMBLE_HIGH_ACC_PACK_SIZE);
        ring_buffer_pop(&right_high_rb);
        right_high_buf_rdy=1;
    }
    if(right_low_buf_stdby&&right_low_rb.size){
        right_low_buf_stdby=0;
        memcpy(&right_low_buf,&right_low_rb_buf[right_low_rb.top*HD_RUMBLE_HIGH_ACC_PACK_SIZE],HD_RUMBLE_HIGH_ACC_PACK_SIZE);
        ring_buffer_pop(&right_low_rb);
        right_low_buf_rdy=1;
    }
}
uint8_t step_forward(uint16_t* pos,uint32_t* sum,uint32_t step)//return if new cycle
{
    uint32_t step_tmp;
    if(!step)return 1;
    *sum+=HD_RUMBLE_STEP;
    if(*sum>=step){
        step_tmp=*sum/step;
        *pos+=step_tmp;
        *sum-=step_tmp*step;
        if((*pos)>HD_RUMBLE_SAMPLERATE){
            *pos&=0x7ff;
            //*pos&=0x3ff;
            return 1;
        }
    }
    return 0;
}
uint8_t ck_if_not_enough(uint16_t pos,uint32_t tick,uint32_t target){
    //if(celi(tick*percent)>=target),enough
    //return 1;
    return (((HD_RUMBLE_SAMPLERATE-pos)*tick+(HD_RUMBLE_SAMPLERATE-1))>>HD_RUMBLE_SAMPLERATE_SHIFT)<=target;
    //todo: this isnt actually accurate as sum is lost,but i cant be bothered.
}
static int32_t tim3_irq_tmp_l,tim3_irq_tmp_r;
static uint32_t left_high_tick,left_low_tick,right_high_tick,right_low_tick,tim3_counter;
#define HD_RUMBLE_HIGH_ACC_LAST_ATLEAST_CNT (50UL*HD_RUMBLE_FRAME_TIME_MS)
#define HD_RUMBLE_HIGH_ACC_TIMEOUT_CNT (50UL*HD_RUMBLE_FRAME_TIMEOUT_MS)
uint8_t rumble_pattern_check(hd_rumble_high_accurary_pack* p,uint16_t* pos,uint32_t* sum,uint32_t tick)
{
    if(user_config.rumble_pattern){
        return (!p->amp)||(step_forward(pos, sum, p->step)&&(tim3_counter-tick)>=HD_RUMBLE_HIGH_ACC_LAST_ATLEAST_CNT);
    }
    else {
        return (!p->amp||step_forward(pos, sum, p->step))&&(tim3_counter-tick)>=HD_RUMBLE_HIGH_ACC_LAST_ATLEAST_CNT;
    }
}
static uint8_t switch_flag=0xf;
static uint8_t switch_upd;
#define SET_RUMBLEFLAG_BIT(v,x) ((v)|=(1<<(x)))
#define GET_RUMBLEFLAG_BIT(v,x) ((v)&(1<<(x)))
//NOTICE:result of GET_SWITCH_FLAG is !! NOT !! a boolean,not do math with it without converting.
void TIM3_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM3,TIM_IT_Update)==1){
        ++tim3_counter;
        //we unroll the loop manually incase the compiler didnt
        if(user_config.legacy_rumble){
            if(rumble_pattern_check(&left_high,&left_high_pos,&left_high_sum,left_high_tick)){
                if(left_high_buf_rdy){
                    /*
                        switch_flag[0]=1;
                        left_high.amp=0;
                    */
                    //switch_upd[0]=1;
                    SET_RUMBLEFLAG_BIT(switch_upd,0);
                }else if(tim3_counter-left_high_tick>=HD_RUMBLE_HIGH_ACC_TIMEOUT_CNT){
                    left_high.amp=0;
                    //switch_flag[0]=1;
                    SET_RUMBLEFLAG_BIT(switch_flag,0);
                }
            }
            if(rumble_pattern_check(&left_low,&left_low_pos,&left_low_sum,left_low_tick)){
                if(left_low_buf_rdy){
                    /*
                        switch_flag[1]=1;
                        left_low.amp=0;
                    */
                    //switch_upd[1]=1;
                    SET_RUMBLEFLAG_BIT(switch_upd,1);
                }else if(tim3_counter-left_low_tick>=HD_RUMBLE_HIGH_ACC_TIMEOUT_CNT){
                    left_low.amp=0;
                    //switch_flag[1]=1;
                    SET_RUMBLEFLAG_BIT(switch_flag,1);
                }
            }
            if(rumble_pattern_check(&right_high,&right_high_pos,&right_high_sum,right_high_tick)){
                if(right_high_buf_rdy){
                    /*
                        switch_flag[2]=1;
                        right_high.amp=0;
                    */
                    //switch_upd[2]=1;
                    SET_RUMBLEFLAG_BIT(switch_upd,2);
                }else if(tim3_counter-right_high_tick>=HD_RUMBLE_HIGH_ACC_TIMEOUT_CNT){
                    right_high.amp=0;
                    //switch_flag[2]=1;
                    SET_RUMBLEFLAG_BIT(switch_flag,2);
                }
            }
            if(rumble_pattern_check(&right_low,&right_low_pos,&right_low_sum,right_low_tick)){
                if(right_low_buf_rdy){
                        /*
                        switch_flag[3]=1;
                        right_low.amp=0;
                        */
                    //switch_upd[3]=1;
                    SET_RUMBLEFLAG_BIT(switch_upd,3);
                }else if(tim3_counter-right_low_tick>=HD_RUMBLE_HIGH_ACC_TIMEOUT_CNT){
                    right_low.amp=0;
                    //switch_flag[3]=1;
                    SET_RUMBLEFLAG_BIT(switch_flag,3);
                }
            }
            //if oppo_switch=1 || not enough for more ,then end
            if(user_config.legacy_rumble){
                //if(switch_upd[0]){
                if(GET_RUMBLEFLAG_BIT(switch_upd,0)){
                    left_high_buf_rdy=0;
                    left_high=left_high_buf;
                    left_high_sum=left_high_pos=0;
                    left_high_buf_stdby=1;
                    left_high_tick=tim3_counter;
                }
                //if(switch_upd[1]){
                if(GET_RUMBLEFLAG_BIT(switch_upd,1)){
                    left_low_buf_rdy=0;
                    left_low=left_low_buf;
                    left_low_sum=left_low_pos=0;
                    left_low_buf_stdby=1;
                    left_low_tick=tim3_counter;
                }
                //if(switch_upd[2]){
                if(GET_RUMBLEFLAG_BIT(switch_upd,2)){
                    right_high_buf_rdy=0;
                    right_high=right_high_buf;
                    right_high_sum=right_high_pos=0;
                    right_high_buf_stdby=1;
                    right_high_tick=tim3_counter;
                }
                //if(switch_upd[3]){
                if(GET_RUMBLEFLAG_BIT(switch_upd,3)){
                    right_low_buf_rdy=0;
                    right_low=right_low_buf;
                    right_low_sum=right_low_pos=0;
                    right_low_buf_stdby=1;
                    right_low_tick=tim3_counter;
                }
            }
            switch_upd=0;
            tim3_irq_tmp_l=((HD_RUMBLE_TIM_PERIOD_MID*(((ccr_lookup_tb[left_high_pos]*left_high.amp)+
                    (ccr_lookup_tb[left_low_pos]*left_low.amp))>>HD_RUMBLE_AMP_SHIFT_1))>>HD_RUMBLE_AMP_SHIFT_2)
                            +HD_RUMBLE_TIM_PERIOD_MID;
            tim3_irq_tmp_r=((HD_RUMBLE_TIM_PERIOD_MID*(((ccr_lookup_tb[right_high_pos]*right_high.amp)+
                    (ccr_lookup_tb[right_low_pos]*right_low.amp))>>HD_RUMBLE_AMP_SHIFT_1))>>HD_RUMBLE_AMP_SHIFT_2)
                            +HD_RUMBLE_TIM_PERIOD_MID;
        }
        else {
            step_forward(&left_high_pos,&left_high_sum,left_high.step);
            step_forward(&left_low_pos,&left_low_sum,left_low.step);
            step_forward(&right_high_pos,&right_high_sum,right_high.step);
            step_forward(&right_low_pos,&right_low_sum,right_low.step);
            tim3_irq_tmp_l=((HD_RUMBLE_TIM_PERIOD_RANGE*(((ccr_lookup_tb[left_high_pos]*left_high.amp)+
                    (ccr_lookup_tb[left_low_pos]*left_low.amp))>>HD_RUMBLE_AMP_SHIFT_1))>>HD_RUMBLE_AMP_SHIFT_2)
                            +HD_RUMBLE_TIM_PERIOD_MID;
            tim3_irq_tmp_r=((HD_RUMBLE_TIM_PERIOD_RANGE*(((ccr_lookup_tb[right_high_pos]*right_high.amp)+
                    (ccr_lookup_tb[right_low_pos]*right_low.amp))>>HD_RUMBLE_AMP_SHIFT_1))>>HD_RUMBLE_AMP_SHIFT_2)
                            +HD_RUMBLE_TIM_PERIOD_MID;
            if(left_high_buf_rdy&&left_low_buf_rdy){
                    left_high_buf_rdy=left_low_buf_rdy=0;
                    left_low=left_low_buf;
                    left_high=left_high_buf;
                    left_high_buf_stdby=left_low_buf_stdby=1;
                    left_high_tick=left_low_tick=tim3_counter;
                    if(!left_high.amp||!left_high.step)
                        left_high.step=left_high_pos=left_high_sum=0;
                    if(!left_low.amp||!left_low.step)
                        left_low.step=left_low_pos=left_low_sum=0;
            }
            if(right_high_buf_rdy&&right_low_buf_rdy){
                    right_high_buf_rdy=right_low_buf_rdy=0;
                    right_low=right_low_buf;
                    right_high=right_high_buf;
                    right_high_buf_stdby=right_low_buf_stdby=1;
                    right_high_tick=right_low_tick=tim3_counter;
                    if(!right_high.amp||!right_high.step)
                        right_high.step=right_high_pos=right_high_sum=0;
                    if(!right_low.amp||!right_low.step)
                        right_low.step=right_low_pos=right_low_sum=0;
            }
            /*if(left_high_buf_rdy){
                left_high_buf_rdy=0;
                left_high=left_high_buf;
                left_high_buf_stdby=1;
            }
            if(left_low_buf_rdy){
                left_low_buf_rdy=0;
                left_low=left_low_buf;
                left_low_buf_stdby=1;
            }
            if(right_high_buf_rdy){
                right_high_buf_rdy=0;
                right_high=right_high_buf;
                right_high_buf_stdby=1;
            }
            if(right_low_buf_rdy){
                right_low_buf_rdy=0;
                right_low=right_low_buf;
                right_low_buf_stdby=1;
            }*/
        }
        //CHLCVR=200;
        CHLCVR=i32_clamp(tim3_irq_tmp_l,0,HD_RUMBLE_TIM_PERIOD);
        CHRCVR=i32_clamp(tim3_irq_tmp_r,0,HD_RUMBLE_TIM_PERIOD);
        /*CHLCVR=i32_clamp(tim3_irq_tmp_l,0,HD_RUMBLE_TIM_PERIOD);
        CHRCVR=i32_clamp(tim3_irq_tmp_r,0,HD_RUMBLE_TIM_PERIOD);*/
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    }
}
