#include <FixedPoints.h>
#include <FixedPointsCommon.h>
#define fs 16000

#define maxdelay_chorus 100
#define maxdelay_flanger 80
//#define maxdelay_flanger 221

#define D1   276    //  23  476
#define D2   594    //  297 594
#define D3   958    //  329 658
#define D4   1200    //  350 700
#define D5   1550   //  775 1550   
#define D6   526    //  263 526 


//functions
SQ15x16 pwr(SQ15x16 var1, short var2);
SQ15x16 sinefixed(SQ15x16 arg);
SQ15x16 tangentfixed(SQ15x16 argt);
SQ15x16 cosinefixed(SQ15x16 arg);
SQ15x16 sqrtfixed(SQ15x16 argsq);

//functions declarations:
SQ15x16 pwr(SQ15x16 var1, short var2){
  SQ15x16 power = 1; int i;
  for(i=1;i<=var2;i++){
    power = power * var1;}
    return power;
  }
  
SQ15x16 sinefixed(SQ15x16 arg){

    SQ15x16 z = arg - M_PI;
    SQ15x16 a1,a2,a3, a5,a7, factor3,factor5,factor7;
    factor3 = +1.0/6.0; factor5 = -1.0/120.0; factor7= 1.0/5040.0;
    a1 = -z; a3 = pwr(z,3); a2 = pwr(z,2); a5 = a2*a3; a7= a5*a2;
    return (-z + factor3*a3 + factor5*a5 + factor7*a7);
}

SQ15x16 tangentfixed(SQ15x16 argt){
  SQ15x16 factor3, factor5, factor7,factor9, a2, a3,a5,a7,a9;
  factor3 = 1.0/3.0; factor5 = 2.0/15.0; factor7 = 17.0/315.0; factor9 = 62.0/2835.0; 
  a3 = pwr(argt,3); a2=pwr(argt,2); a5 = a3*a2; a7 = a5*a2; a9 = a7*a2;
  return (argt + factor3*a3 + factor5*a5 + factor7*a7 + factor9*a9);
  }
// variables : 
// ARDUINO GENERAL:
short in_ADC0, out_DAC0, input;
int BTN0,BTN1,BTN2,BTN3,BTN4,BTN5,BTN6,BTN7, POT0, POT1, POT2, POT3;
int BT0,BT1,BT2,BT3,BT4,BT5,BT6,BT7;
int count = 0; int effect = 8;


/// effects: 
// ECHO: 
SQ15x16 echo_mix = 0.50;
//SQ15x16 echo_buffer[fs];
SQ15x16 echo_buffer[16000];
unsigned int Delay_Depth_echo, Delay_echo = 0, a_echo = 0;

/////
// FUZZ:
SQ15x16 threshold = 0.33333 ; SQ15x16 in_0 = 0.00; SQ15x16 fuzzmix;

/////
// CHORUS DATTORO:
SQ15x16 af = -0.7;
SQ15x16 a0 = 0.7;
SQ15x16 chorus_DELAY[maxdelay_chorus+2] = {0};
int   chorus_Delay2 = 0;
SQ15x16 chorus_delay_sr = 0;
int   chorus_delay_int = 0;
SQ15x16 frac = 0;
int n,j=0;


/////
// FLANGER:
SQ15x16   delay_flanger[maxdelay_flanger+2] = {0};
SQ15x16   Delay2_flanger = 0;
SQ15x16 delay_sr_flanger = 0;
int  delay_int_flanger = 0;
SQ15x16 frac_flanger = 0;

/////
// REVERB_SCHROEDER:
SQ15x16 a1 = 0.7079, a2 = 0.6496, a3 = 0.6200, a4 = 0.6016, a5 = 0.4334, a6 = 0.5664; 
SQ15x16 b1 = 0.99 , b2 = 0.95 , b3 = 0.90 , b4 = 0.80;
int index5 = 0, index6 = 0;
SQ15x16 reverb_mix;
SQ15x16 X1[D1] = {0};
SQ15x16 X2[D2] = {0};
SQ15x16 X3[D3] = {0};
SQ15x16 X4[D4] = {0};
SQ15x16 X5[D5] = {0};
SQ15x16 X6[D6] = {0};
SQ15x16 S1 = 0, S2 =  0, S3 =  0, S4 =  0, S5 = 0, S6 = 0, S7 = 0, S51 = 0;
int DC1 = 0, DC2 = 0, DC3 = 0, DC4 = 0, DC5 = 1, DC6 = 1;

/////
// TREMOLO:
int sample; SQ15x16 LFO, tremolo_mix;

//uint16_t tremolo_Seno_table[fs];
/////
// LOW-PASS/HIGH-PASS: 
short type;
//float tangents[2048];
int fc_hplp; 
SQ15x16 xh[3], out_hplp, y, sqr2, frac_hplp, K,K2;
SQ15x16 b0_hplp,b1_hplp,b2_hplp,a1_hplp,a2_hplp;
SQ15x16 Q_hplp = 0.707;
//float K,K2,fc_hplp;

/////
// WAH WAH:
int n_wahwah = 0;
int minf, maxf;
SQ15x16 omega0, omega2, q, f, Input, InputBuffer[2], Output, OutputBuffer[2], fracw;
SQ15x16 Input2, InputBuffer2[2], Output2, OutputBuffer2[2];
SQ15x16  fmed= 1900.00; SQ15x16 amp = 1600.00;
short t1,t2,cycle_time;
const SQ15x16 pi = 3.14159; const SQ15x16 fsfixed = 16000.00; const SQ15x16 hundred = 100; const SQ15x16 erromax = 0.001; 
// controle
SQ15x16 fosc, Q, mix; int fosc_i, Qi, mix_i;
const SQ15x16 one = 1.00; const SQ15x16 half = 0.50;
SQ15x16 a,b,c, arg; 

//////



////// FUNCTION DECLARATIONS : 

short ECHO(int POT0,int POT1, int POT2, SQ15x16 input){

  a_echo =  map(POT1,0,4095,1,4);
  echo_buffer[Delay_echo] =  input + (echo_buffer[Delay_echo])>>a_echo;
  Delay_Depth_echo = map(POT0>>2,0,2047,1,fs);
 
  Delay_echo++;
  if(Delay_echo >= Delay_Depth_echo) Delay_echo = 0; 
  out_DAC0 = static_cast<short>((echo_buffer[Delay_echo])<<1);

  out_DAC0 = static_cast<short>((1-echo_mix)*input + echo_mix*out_DAC0);
  out_DAC0=map(out_DAC0,0,4095,1,POT2);
  return out_DAC0;
 
}

short REVERB(short POT0, short input){

    X1[DC1] = (input + a1 * X1[DC1]);
    X2[DC2] = (input + a2 * X2[DC2]);
    X3[DC3] = (input + a3 * X3[DC3]);
    X4[DC4] = (input + a4 * X4[DC4]);
    S1 = b1 * X1[DC1];
    S2 = b2 * X2[DC2];
    S3 = b3 * X3[DC3];
    S4 = b4 * X4[DC4];
    DC1++; if (DC1 >= D1) DC1 = 0;
    DC2++; if (DC2 >= D2) DC2 = 0;
    DC3++; if (DC3 >= D3) DC3 = 0;
    DC4++; if (DC4 >= D4) DC4 = 0;
    S5 = (S1 + S2 + S3 + S4)/4;
    X5[index5] = (S5 + a5 * X5[DC5]);
    S6 = -a5 * X5[index5] + X5[DC5];
    DC5++; if (DC5 >= D5) DC5 = 0;
    index5++; if (index5 >= D5) index5 = 0;
    X6[index6] = (S6 + a6 * X6[DC6]);
    S7 = -a6 * X6[index6] + X6[DC6];
    DC6++; if (DC6 >= D6) DC6 = 0;
    index6++; if (index6 >= D6) index6 = 0;
    reverb_mix = map(POT0,0,4095,0,100);
    reverb_mix = reverb_mix/100;
    out_DAC0= static_cast<short>(input*(1-reverb_mix) + S7*reverb_mix);
 
  return out_DAC0;
  }

short FUZZ(int POT0, SQ15x16 input){

  in_0 = input/2047 - 1; 
  
  if (abs(in_0) <= threshold) 
  {
    in_0 = in_0 *2;
  }
  if (abs(in_0) > threshold   && abs(in_0) < 2*threshold)
  {
    if ( in_0 > 0 )
      {
        in_0 = 2 - 3*in_0;
        in_0 = in_0 * in_0;
        in_0 = 3 - in_0;
        in_0 = in_0 / 3;
      }
    if ( in_0 < 0 )
      {
        in_0 = 2 - 3*abs(in_0);
        in_0 = in_0 * in_0;
        in_0 = 3 - in_0;
        in_0 = - in_0 / 3;
      }
  }

  if (abs(in_0) >= 2*threshold)
  {
    if ( in_0 > 0 ) in_0 =  0.98;
    if ( in_0 < 0 ) in_0 = -0.98;
  }

  fuzzmix = map(POT0, 0, 4095,1, 100);
  fuzzmix = fuzzmix/100.0;
  out_DAC0 = (short)((in_0 + 1) * 2047);
  out_DAC0 = (short)((1-fuzzmix)*input + fuzzmix*out_DAC0);
  return out_DAC0;
  }

short CHORUS(short POT0,short POT1,short input){

    POT0=map(POT0>>2,0,1024,1,maxdelay_chorus); //empirical adjusts  
    chorus_Delay2 = POT0/2;
    chorus_DELAY[0] = a0*input + af*chorus_DELAY[chorus_Delay2];
  
    for ( int u = 0; u <= POT0; u++)
      chorus_DELAY[POT0+1-u] = chorus_DELAY[POT0-u];
    
    POT1=map(POT1>>2,0,1024,1,4); // empirical adjusts
    //POT1 = 1;

    chorus_delay_sr = chorus_Delay2 - chorus_Delay2*sinefixed(2*PI*j*POT1/fs);
    chorus_delay_int = int(chorus_delay_sr);
    frac = chorus_delay_sr - chorus_delay_int;
    if (frac <= 0) frac = 0.01;      // Ajusts
    if (frac >= 1) frac = 0.99;      //
    
 
    j++; if (j*POT1>=fs) j = 0;  

     out_DAC0 = static_cast<short>(chorus_DELAY[chorus_delay_int+1]*frac + chorus_DELAY[chorus_delay_int]*(1-frac));

     if (out_DAC0 <=0 ) out_DAC0 = 0.01;

     return out_DAC0;
 }

short TREMOLO(short POT0, short POT1, SQ15x16 input){

  LFO = map(POT0,0,4095,100,250);
  LFO = LFO/100.00;
  out_DAC0 = static_cast<short>(input*sinefixed(2*PI*LFO/fsfixed*sample));
  tremolo_mix =map(POT1,0,4095,0,100); tremolo_mix = tremolo_mix/100.00;
  out_DAC0 = static_cast<short>((1-tremolo_mix)*input + (tremolo_mix)*out_DAC0);
 //out_DAC0 = (short)((1-tremolo_mix)*input);
  sample++;
  if (LFO*sample >= fs){ sample = 0;}

  return out_DAC0;

}

short WAHWAH(short POT0, short POT1, short POT2, SQ15x16 input){
  Input = input;
  
    
    arg = (2*pi*fosc/fsfixed)*n_wahwah;
    if (arg>(2*pi)){arg = arg - 2*pi;}
    
   f = fmed + amp*sinefixed(arg);
   omega0 = 2 * pi * (f) / fs; omega2 = omega0 * omega0; q = (2.00/Q)*omega0; fracw = 4 + q + omega2;
   a = q*(Input - InputBuffer[n_wahwah%2]); b = (2*omega2 - 8)*OutputBuffer[(n_wahwah+1)%2]; 
   c = (4 - q + omega2)*OutputBuffer[n_wahwah%2];
    Output = (a-b-c)/frac;
    InputBuffer[n_wahwah%2] = Input; OutputBuffer[n_wahwah%2] = Output;
    
     n++;
    if((n_wahwah*fosc)>=fs){n_wahwah=0;}

 // saida:
    Output = Input*(1-mix) + mix*Output;
    out_DAC0= (short)Output;

    return out_DAC0;

}

short FLANGER(short POT0, short POT1, short input){
 
      POT0=map(POT0>>2,0,1024,1,maxdelay_flanger); //empirical adjusts
    Delay2_flanger = POT0/2;


    for ( int u = 0; u <= POT0; u++)
      delay_flanger[POT0+1-u] = delay_flanger[POT0-u];

    delay_flanger[0] = input;
    
    
    POT1=map(POT1>>2,0,1024,1,4); // empirical adjusts
   
    delay_sr_flanger = Delay2_flanger - Delay2_flanger*sinefixed(2*pi*POT1/fsfixed*j); 
    delay_int_flanger = int(delay_sr_flanger);
    frac_flanger = delay_sr_flanger - delay_int_flanger;
    if (frac_flanger == 0) frac_flanger  = 0.01;      // Ajusts
    if (frac_flanger == 1) frac_flanger  = 0.99;      //

    j++;
    if (j*POT1>=fs) j = 0;  

 
     out_DAC0 = static_cast<short>(delay_flanger[delay_int_flanger+2]*frac_flanger + delay_flanger[delay_int_flanger]*(1-frac_flanger));
     return out_DAC0;
}

short LPHP(short POT0,short input){

   // if (POT0>2048){
      fc_hplp = POT0;
      type = 0;
  //  }
//    else{
//      fc = map(POT0,0,2048,5000,80);
      //type = 1;
 //   }
 //K2 = tan(M_PI/fs*fc_hplp); K =  (K2-1)/(K2+1);
   //K = (tangentfixed(pi/fsfixed*fc_hplp) -1)/(tangentfixed(pi/fsfixed*fc_hplp) +1);
//   K = (tangents[fc_hplp]-1)/(tangents[fc_hplp] + 1);

   xh[1] = input - K*xh[0];
   y =  K*xh[1] + xh[0];
   if (type == 0) {y = 0.5*(input + y);}
   
   xh[0] = xh[1];

    out_DAC0 = (short)y;
    
    
    //out_DAC0= static_cast<short>(out_hplp); 
    return out_DAC0;
 }
