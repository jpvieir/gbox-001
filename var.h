#include <FixedPoints.h>
#include <FixedPointsCommon.h>
#define fs 16000

#define maxdelay_chorus 100
#define maxdelay_flanger 160

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
// variables : 
// ARDUINO GENERAL:
short in_ADC0, out_DAC0, input;
int BTN0,BTN1,BTN2,BTN3,BTN4,BTN5,BTN6,BTN7, POT0, POT1, POT2;
int count = 0; int effect = 8;


/// effects: 
// ECHO: 
SQ15x16 echo_mix = 0.50;
SQ15x16 echo_buffer[fs];
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
SQ15x16   delay_flanger[maxdelay_flanger] = {0};
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

/////
// WAH WAH:
int minf, maxf;
SQ15x16 omega0, omega2, q, f, Input, InputBuffer[2], Output, OutputBuffer[2], fracw;
SQ15x16 Input2, InputBuffer2[2], Output2, OutputBuffer2[2];
SQ15x16  fmed, amp;
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
 }

short TREMOLO(short POT0, short POT1, short input){

  LFO = map(POT0,0,4095,50,150);
  LFO = LFO/100.00;
  out_DAC0 = static_cast<short>(input*sinefixed(2*PI*LFO*sample/fs));
  tremolo_mix =map(POT1,1,4095,0,100); tremolo_mix = tremolo_mix/100.00;
  out_DAC0 = static_cast<short>((1-tremolo_mix)*input + (tremolo_mix)*out_DAC0);
  sample++;
  if (LFO*sample >= fs) sample = 0;

}

short WAHWAH(short POT0, short POT1, short POT2, short input){

    fosc_i = map(POT0,0,4095,100,250); fosc = fosc_i/100.0;
    mix_i = map(POT1,0,4095,0,100); mix = mix_i/100.0;
    Qi = map(POT2,0,4095,100,400); Q = Qi/100.0;
    
    arg = (2*pi*fosc/fsfixed)*n;
    if (arg>(2*pi)){arg = arg - 2*pi;}
    
   f = fmed + amp*sinefixed(arg);
   omega0 = 2 * pi * (f) / fs; omega2 = omega0 * omega0; q = (2/Q)*omega0; fracw = 4 + q + omega2;
   a = q*(Input - InputBuffer[n%2]); b = (2*omega2 - 8)*OutputBuffer[(n+1)%2]; 
   c = (4 - q + omega2)*OutputBuffer[n%2];
    Output = (a-b-c)/frac;
    InputBuffer[n%2] = Input; OutputBuffer[n%2] = Output;
     n++;
    if((n*fosc)>=fs){n=0;}

 // saida:
    Output = Input*(1-mix) + mix*Output;
    out_DAC0= (short)(Output);

}

short FLANGER(short POT0, short POT1, short input){
   POT0=map(POT0>>2,0,1024,1,maxdelay_flanger); //empirical adjusts
    Delay2_flanger = POT0/2;


    for ( int u = 0; u <= POT0; u++)
      delay_flanger[POT0+1-u] = delay_flanger[POT0-u];

    delay_flanger[0] = input;
    
    
    POT1=map(POT1>>2,0,1024,1,4); // empirical adjusts
   
    delay_sr_flanger = Delay2_flanger - Delay2_flanger*sin(2*M_PI*POT1*j/fs); 
    delay_int_flanger = int(delay_sr_flanger);
    frac_flanger = delay_sr_flanger - delay_int_flanger;
    if (frac_flanger == 0) frac = 0.01;      // Ajusts
    if (frac_flanger == 1) frac = 0.99;      //

    j++;
    if (j*POT1>=fs) j = 0;  

     out_DAC0 = static_cast<short>(delay_flanger[delay_int_flanger+2]*frac_flanger + delay_flanger[delay_int_flanger]*(1-frac_flanger));
}
