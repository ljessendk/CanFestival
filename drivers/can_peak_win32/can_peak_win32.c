/*

This file is not part of CanFestival.
This is third party contributed file.

It is provided as-this and without any warranty

*/

//****************************************************************************
// Copyright (C) 2006  PEAK System-Technik GmbH
//
// linux@peak-system.com
// www.peak-system.com
//
// This part of software is proprietary. It is allowed to
// distribute it with CanFestival. 
//
// No warranty at all is given.
//
// Maintainer(s): Edouard TISSERANT (edouard.tisserant@lolitech.fr)
//****************************************************************************

/*
   Obfuscated by COBF (Version 1.06 2006-01-07 by BB) at Tue Aug  8 23:36:30 2006
*/
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<errno.h>
#include<stddef.h>
#include<sys/ioctl.h>
#include<fcntl.h>
#include<signal.h>
#include<sys/time.h>
#include<unistd.h>
#include<pthread.h>
#include<applicfg.h>
#include"timer.h"
#include"can_driver.h"
#include"timers_driver.h"
#include"cobf.h"
#ifndef extra_PCAN_init_params
#define extra_PCAN_init_params
#endif
#ifdef PCAN2_HEADER_
#define pcan_s 2
#else
#define pcan_s 1
#endif
pcan_34 pcan_51{pcan_9 pcan_r;pcan_39 pcan_1;pcan_24*pcan_o;pcan_16*
pcan_k;}pcan_c;pcan_c pcan_g[pcan_s]={{0,},};pcan_47 pcan_l=pcan_38;
pcan_12 pcan_x(pcan_n pcan_b){
#ifdef PCAN2_HEADER_
pcan_d(pcan_g!=((pcan_c* )pcan_b))pcan_52(((pcan_c* )pcan_b)->pcan_k
->pcan_20,pcan_v extra_PCAN_init_params);pcan_j
#endif
pcan_54(((pcan_c* )pcan_b)->pcan_k->pcan_20,pcan_v
extra_PCAN_init_params);}pcan_t pcan_17(pcan_n pcan_b,pcan_4*pcan_e){
pcan_t pcan_f;pcan_23 pcan_a;pcan_11 pcan_i;pcan_22(&pcan_l);
#ifdef PCAN2_HEADER_
pcan_d(pcan_g!=((pcan_c* )pcan_b))pcan_i=pcan_27(&pcan_a);pcan_j
#endif
pcan_i=pcan_48(&pcan_a);pcan_d(pcan_i==pcan_w){pcan_d(pcan_a.pcan_q&~
(pcan_14|pcan_10)){pcan_d(pcan_a.pcan_q==pcan_25){pcan_7("\x21\x21"
"\x21\x20\x50\x65\x61\x6b\x20\x62\x6f\x61\x72\x64\x20\x72\x65\x61\x64"
"\x20\x3a\x20\x72\x65\x2d\x69\x6e\x69\x74\n");pcan_x(pcan_b);pcan_y(
10000);}pcan_u(&pcan_l);pcan_p pcan_a.pcan_q==pcan_28?pcan_a.pcan_8[2
]:pcan_46;}pcan_e->pcan_13.pcan_15=pcan_a.pcan_18;pcan_d(pcan_a.
pcan_q==pcan_v)pcan_e->pcan_2=0;pcan_j pcan_e->pcan_2=1;pcan_e->
pcan_z=pcan_a.pcan_6;pcan_3(pcan_f=0;pcan_f<pcan_a.pcan_6;pcan_f++)pcan_e
->pcan_f[pcan_f]=pcan_a.pcan_8[pcan_f];}pcan_u(&pcan_l);pcan_p pcan_i
;}pcan_12 pcan_44(pcan_n pcan_b){pcan_24*pcan_o=((pcan_c* )pcan_b)->
pcan_o;pcan_4 pcan_e;pcan_19(((pcan_c* )pcan_b)->pcan_r){pcan_11
pcan_i;pcan_d((pcan_i=pcan_17(pcan_b,&pcan_e))==pcan_w){pcan_26();
pcan_50(pcan_o,&pcan_e);pcan_30();}pcan_j{pcan_d(!(pcan_i&pcan_31||
pcan_i&pcan_40||pcan_i&pcan_45)){pcan_7("\x63\x61\x6e\x52\x65\x63\x65"
"\x69\x76\x65\x20\x72\x65\x74\x75\x72\x6e\x65\x64\x20\x65\x72\x72\x6f"
"\x72\x20\x28\x25\x64\x29\n",pcan_i);}pcan_y(1000);}}}pcan_t pcan_41(
pcan_n pcan_b,pcan_4*pcan_e){pcan_t pcan_f;pcan_23 pcan_a;pcan_a.
pcan_18=pcan_e->pcan_13.pcan_15;pcan_d(pcan_e->pcan_2==0)pcan_a.
pcan_q=pcan_v;pcan_j{pcan_a.pcan_q=pcan_14|pcan_10;}pcan_a.pcan_6=
pcan_e->pcan_z;pcan_3(pcan_f=0;pcan_f<pcan_e->pcan_z;pcan_f++)pcan_a.
pcan_8[pcan_f]=pcan_e->pcan_f[pcan_f];pcan_m=pcan_w;pcan_32{pcan_22(&
pcan_l);
#ifdef PCAN2_HEADER_
pcan_d(pcan_g!=((pcan_c* )pcan_b))pcan_m=pcan_53(&pcan_a);pcan_j
#endif
pcan_m=pcan_43(&pcan_a);pcan_d(pcan_m){pcan_d(pcan_m==pcan_25){pcan_7
("\x21\x21\x21\x20\x50\x65\x61\x6b\x20\x62\x6f\x61\x72\x64\x20\x77"
"\x72\x69\x74\x65\x20\x3a\x20\x72\x65\x2d\x69\x6e\x69\x74\n");pcan_x(
pcan_b);pcan_y(10000);}pcan_u(&pcan_l);pcan_y(100);}pcan_j{pcan_u(&
pcan_l);}}pcan_19(pcan_m!=pcan_w&&((pcan_c* )pcan_b)->pcan_r);pcan_p 0
;}pcan_n pcan_42(pcan_16*pcan_k){pcan_9 pcan_55[64];pcan_9*pcan_36;
pcan_21 pcan_h;pcan_3(pcan_h=0;pcan_h<pcan_s;pcan_h++){pcan_d(!pcan_g
[pcan_h].pcan_r)pcan_56;}pcan_d(pcan_h==pcan_s){pcan_0(pcan_5,"\x4f"
"\x70\x65\x6e\x20\x66\x61\x69\x6c\x65\x64\x2e\n");pcan_0(pcan_5,"\x63"
"\x61\x6e\x5f\x70\x65\x61\x6b\x5f\x77\x69\x6e\x33\x32\x2e\x63\x3a\x20"
"\x6e\x6f\x20\x6d\x6f\x72\x65\x20\x63\x61\x6e\x20\x70\x6f\x72\x74\x20"
"\x61\x76\x61\x69\x6c\x61\x62\x6c\x65\x20\x77\x69\x74\x68\x20\x74\x68"
"\x69\x73\x20\x70\x63\x61\x6e\x20\x6c\x69\x62\x72\x61\x72\x79\n");
pcan_0(pcan_5,"\x63\x61\x6e\x5f\x70\x65\x61\x6b\x5f\x77\x69\x6e\x33"
"\x32\x2e\x63\x3a\x20\x70\x6c\x65\x61\x73\x65\x20\x6c\x69\x6e\x6b\x20"
"\x61\x6e\x6f\x74\x68\x65\x72\x20\x65\x78\x65\x63\x75\x74\x61\x62\x6c"
"\x65\x20\x77\x69\x74\x68\x20\x61\x6e\x6f\x74\x68\x65\x72\x20\x70\x63"
"\x61\x6e\x20\x6c\x69\x62\n");pcan_p pcan_37;}pcan_g[pcan_h].pcan_r=1
;pcan_g[pcan_h].pcan_k=pcan_k;pcan_g[pcan_h].pcan_o=pcan_k->pcan_o;
pcan_x((pcan_c* )&pcan_g[pcan_h]);pcan_29((pcan_c* )&pcan_g[pcan_h],&
pcan_g[pcan_h].pcan_1);pcan_p(pcan_c* )&pcan_g[pcan_h];}pcan_21
pcan_57(pcan_n pcan_b){((pcan_c* )pcan_b)->pcan_r=0;
#ifdef PCAN2_HEADER_
pcan_d(pcan_g!=((pcan_c* )pcan_b))pcan_49();pcan_j
#endif
pcan_35();pcan_33(&((pcan_c* )pcan_b)->pcan_1);pcan_p 0;}
