#include "var.h"

int test = 0;

// variar um vetor seletor[8] de 0 a 7 continuamente em baixa frequencia. 
// a cada variação a variável POT será atualizada com o valor do respectivo potenciometro x[N-1].
// a cada variação a variavel not(BTN será atualizada com o valor do respectivo botão x[N-1].
// CASO digitalread(not(BTN) de x[N-1], o código ativa o efeito N; 


void setup() {
   Serial.begin(9600);
  pmc_set_writeprotect(false); // Disabilita a protecao de escrita do periferico.
  pmc_enable_periph_clk(ID_TC4); //Abilita o Timer 4 
  TC_Configure(TC1,1, TC_CMR_WAVE | TC_CMR_WAVSEL_UP_RC | TC_CMR_TCCLKS_TIMER_CLOCK2);
  TC_SetRC(TC1, 1, 656); // sets <> 44.1 Khz interrupt rate
  TC_Start(TC1, 1);
  TC1->TC_CHANNEL[1].TC_IER=TC_IER_CPCS;
  TC1->TC_CHANNEL[1].TC_IDR=~TC_IER_CPCS;
  NVIC_EnableIRQ(TC4_IRQn);
  ADC->ADC_MR |= 0x80;   // Mode Register ----------------------  Add[0x400C0004] 
  ADC->ADC_CR=2;         // ADC Control Register.---------------  Add[0x400C0000]                           
  ADC->ADC_CHER=0x00F1;  // ADC Channel Enable Register --------  Add [0x400C0010]

  analogWrite(DAC1,0);  // analogWrite(pin, value) --------- Ativa o pino DAC0 para escrita analogica
//---------------------------------------------------------------------------------------------------//

  //BT0 = 0; BT1 = 1; BT2= 2; BT3 = 3;
  BT4 = 18; BT5 = 19; BT6 = 20; BT7 = 21;
  
 // pinMode(BT0, INPUT_PULLUP);
 // pinMode(BT1, INPUT_PULLUP);
 // pinMode(BT2, INPUT_PULLUP);
 // pinMode(BT3, INPUT_PULLUP);
  pinMode(BT4, INPUT_PULLUP);
  pinMode(BT5, INPUT_PULLUP);
  pinMode(BT6, INPUT_PULLUP);
  pinMode(BT7, INPUT_PULLUP);
//---------------------------------------------------------------------------------------------------//
  xh[0]=0; xh[1] = 0; xh[2] = 0;

  for(int i=0;i<2048;i++){
    //tangents[i] = tan(M_PI*(20+2*i)/fs);
    }
}

void loop() {

   
    while((ADC->ADC_ISR & 0x00F1)!=0x00F1); 
    in_ADC0=ADC->ADC_CDR[0];               
    POT0=ADC->ADC_CDR[7];                    
    POT1=ADC->ADC_CDR[6];                
    POT2=ADC->ADC_CDR[5]; 
    POT3=ADC->ADC_CDR[4];                 

  //  BTN0 = digitalRead(BT0);  BTN1 = digitalRead(BT1); BTN2 = digitalRead(BT2); BTN3 = digitalRead(BT3);
    BTN4 = digitalRead(BT4); BTN5 = digitalRead(BT5); BTN6 = digitalRead(BT6); BTN7 = digitalRead(BT7);

  //  if(not(BTN0) & count >= 200){count=0; if(effect==0){effect=8;}else{effect=0;}}
  //  if (not(BTN1) & count < 200) {count++;}
  //  if(not(BTN1) & count >= 200){count=0; if(effect==1){effect=8;}else{effect=1;}}
  //  if (not(BTN2) & count < 200) {count++;}
  //  if(not(BTN2) & count >= 200){count=0; if(effect==2){effect=8;}else{effect=2;}}
  //  if (not(BTN3) & count < 200) {count++;}
  //  if(not(BTN3) & count >= 200){count=0; if(effect==3){effect=8;}else{effect=3;}}
   if (not(BTN4) & count < 500) {count++;}
     if(not(BTN4) & count >= 500){count=0; if(effect==1){effect=8;}else{effect=1;}}                                     
    if (not(BTN5) & count < 200) {count++;}
    if(not(BTN5) & count >= 200){count=0; if(effect==2){effect=8;}else{effect=2;}}
    if (not(BTN6) & count < 500) {count++;}
    if(not(BTN6) & count >= 500){count=0; if(effect==3){effect=8;}else{effect=3;}}
    if (not(BTN7) & count < 200) {count++;}
    if(not(BTN7) & count >= 200){count=0; if(effect==5){effect=8;}else{effect=5;}} 
       
  // only 4 buttons are working, so i will be using REVERB, CHORUS, FUZZ AND FLANGER (1,2,3,5)
  //effect = 7 ;
    fosc_i = map(POT0,0,4095,200,2200); fosc = fosc_i/200.0;
    mix_i = map(POT1,0,4095,0,10); mix = mix_i/10.0;
    Qi = map(POT2,0,4095,100,400); Q = Qi/100.0;
    Serial.println(effect);
}

void TC4_Handler() 
{
  TC_GetStatus(TC1, 1);
  
  if(effect==8){out_DAC0 = in_ADC0;} // no effects
  if(effect==0){out_DAC0 = ECHO(POT0,POT1,POT2,in_ADC0);}
  if(effect==1){out_DAC0 = REVERB(POT0, in_ADC0);}
  if(effect==2){out_DAC0 = FUZZ(POT0,in_ADC0);}
  if(effect==3){out_DAC0 = CHORUS(POT0, POT1, in_ADC0);}
  if(effect==4){out_DAC0 = TREMOLO(POT0, POT1, in_ADC0);}
  if(effect==5){out_DAC0 = FLANGER(POT0, POT1,  in_ADC0);}
  if(effect==6){out_DAC0 = WAHWAH(POT0, POT1, POT2, in_ADC0);}
  if(effect==7){out_DAC0 = LPHP(POT0, in_ADC0);}

  out_DAC0 = map(out_DAC0,0,4095,1,POT3);
 
  dacc_set_channel_selection(DACC_INTERFACE, 1);         
  dacc_write_conversion_data(DACC_INTERFACE, out_DAC0);
}
