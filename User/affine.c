/*
 * affine.c
 *
 *  Created on: 2026年5月30日
 *      Author: Reed
 */

#include "conf.h"
#include "affine.h"

#include <stdio.h>
#include <math.h>

affine_matrix affine_trans[2][16];
_affine_map affine_map[2][16];
coord affine_max[2];
coord affine_min[2];
uint8_t affine_rdy;

// 计算 2x2 矩阵的逆，成功返回 1，失败返回 0
// 输入 A[4] = {a11, a12, a21, a22}
// 输出 invA[4]
int inv2x2(const double A[4], double invA[4]) {
    double det = A[0] * A[3] - A[1] * A[2];
    if (fabs(det) < 1e-12) return 0;  // 奇异矩阵
    invA[0] =  A[3] / det;
    invA[1] = -A[1] / det;
    invA[2] = -A[2] / det;
    invA[3] =  A[0] / det;
    return 1;
}

// 使用两个点对精确求解线性变换矩阵
// 输入：两个源点 (sx1,sy1), (sx2,sy2)
//      两个目标点 (dx1,dy1), (dx2,dy2)
// 输出：mat[4] = {a, b, c, d}  （按行优先）
// 返回值：0 成功， -1 失败（点对导致矩阵奇异）
int solve_linear_transform_2pts(double sx1, double sy1, double dx1, double dy1,
                                double sx2, double sy2, double dx2, double dy2,
                                float mat[4]) {
    // 系数矩阵 M = [sx1 sy1; sx2 sy2]
    double M[4] = {sx1, sy1, sx2, sy2};
    double invM[4];
    if (!inv2x2(M, invM)) {
        return -1;  // 两点不能构成可逆矩阵（例如共线或原点）
    }

    // 解 a, b : invM * [dx1; dx2]
    mat[0] = invM[0] * dx1 + invM[1] * dx2;
    mat[1] = invM[2] * dx1 + invM[3] * dx2;

    // 解 c, d : invM * [dy1; dy2]
    mat[2] = invM[0] * dy1 + invM[1] * dy2;
    mat[3] = invM[2] * dy1 + invM[3] * dy2;
    return 0;
}
//1448
uint8_t cal_affine_matrix(coord x1,coord x2,coord y1,coord y2,affine_matrix* m){
    if(!m)return 1;
    uint8_t res = solve_linear_transform_2pts(x1.x,x1.y,y1.x,y1.y,
                                          x2.x,x2.y,y2.x,y2.y,
                                          m);
    return res;
}
uint8_t _affine_init(uint8_t id){
    uint8_t res=0;
    uint8_t t = id?config.js.r_mode:config.js.l_mode;
    //affine_pack* p = &(config.affine[id]);
    if(t!=1)return 0;
    for(int i=0;i<config.affine[id].cnt;++i){
        uint8_t nxt = (i+1)%config.affine[id].cnt;
        /*res |= cal_affine_matrix(p->map[i].angle,p->map[nxt].angle,
                p->map[i].notch,p->map[nxt].notch,affine_trans[id]+i);*/
        res |= cal_affine_matrix(affine_map[id][i].angle,affine_map[id][nxt].angle,
                affine_map[id][i].notch,affine_map[id][nxt].notch,affine_trans[id]+i);
        if(res){
            affine_trans[id][i].mat[0][0]=affine_trans[id][i].mat[1][1]=1.0f;
            affine_trans[id][i].mat[0][1]=affine_trans[id][i].mat[1][0]=0.0f;
        }
    }
    return res;
}
void affine_init(){
    uint8_t res=0;
    res|=_affine_init(0);
    res|=_affine_init(1);
    /*if(res){
        affine_rdy=0;
    }else {
        affine_rdy=1;
    }*/
}
int cross_product(coord x,coord y){//<0,than x is on the left of y,>0 x on the right
    return x.x*y.y-x.y*y.x;
}
int cal_affine_indx(coord x,uint8_t id){//look for the last angle on the left
    int c=0,i=0,s=0,tx=0,ty=0;
    for(;i<config.affine[id].cnt;++i){
        c=cross_product(x, affine_map[id][i].angle);//config.affine[id].map[i].angle);
        //if(c==0)return i;
        if(c==0){
            tx=ty=1;
            tx=x.x*affine_map[id][i].angle.x;
            ty=x.y*affine_map[id][i].angle.y;
            if(tx&&ty)
                tx=tx*ty;
            else if(ty)
                tx=ty;
            if(tx>=0)
                return i;
        }
        if(c<0)
            s=1;
        else if(c>0 && s)
            return i-1;
    }
    return config.affine[id].cnt-1;//last
}
coord cal_affine(coord x,uint8_t id){
    coord y;
    int indx = cal_affine_indx(x,id);
    y.x = x.x*affine_trans[id][indx].a + x.y*affine_trans[id][indx].b;
    y.y = x.x*affine_trans[id][indx].c + x.y*affine_trans[id][indx].d;
    return y;
}
void joystick_affine_scale(int32_t *x,int32_t *y,uint8_t id){
//do nothing
    if(config.affine[id].cnt<2)
        return;//not enough
    coord c;
    c.x=i32_clamp(*x, affine_min[id].x, affine_max[id].x);
    c.y=i32_clamp(*y, affine_min[id].y, affine_max[id].y);
    c = cal_affine(c, id);
    *x=c.x;
    *y=c.y;
}
