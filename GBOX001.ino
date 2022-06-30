#include "var.h"
//#include "effects.c"

// variar um vetor seletor[8] de 0 a 7 continuamente em baixa frequencia. 
// a cada variação a variável POT será atualizada com o valor do respectivo potenciometro x[N-1].
// a cada variação a variavel BTN será atualizada com o valor do respectivo botão x[N-1].
// CASO digitalread(BTN) de x[N-1], o código ativa o efeito N; 

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
  ADC->ADC_CHER=0x1CC0;  // ADC Channel Enable Register --------  Add [0x400C0010]
 
  analogWrite(DAC0,0);  // analogWrite(pin, value) --------- Ativa o pino DAC0 para escrita analogica

  BTN0 = 0; BTN1 = 1; BTN2= 2; BTN3 = 3; BTN4 = 4; BTN5 = BTN6; BTN7 = 7;
  pinMode(BTN0, INPUT_PULLUP);
  pinMode(BTN1, INPUT_PULLUP);
  pinMode(BTN2, INPUT_PULLUP);
  pinMode(BTN3, INPUT_PULLUP);
  pinMode(BTN4, INPUT_PULLUP);
  pinMode(BTN5, INPUT_PULLUP);
  pinMode(BTN6, INPUT_PULLUP);
  pinMode(BTN7, INPUT_PULLUP);
//---------------------------------------------------------------------------------------------------//

}

void loop() {
  while((ADC->ADC_ISR & 0x1CC0)!=0x1CC0); 
    in_ADC0=ADC->ADC_CDR[7];               // Faz a leitura do ADC e armazena na variavel correspondente
    POT0=ADC->ADC_CDR[10];                 // Faz a leitura do ADC e armazena na variavel correspondente        
    POT1=ADC->ADC_CDR[11];                 // Faz a leitura do ADC e armazena na variavel correspondente   
    POT2=ADC->ADC_CDR[12];                 // Faz a leitura do ADC e armazena na variavel correspondente 

  
    if (BTN0 & count < 100) {count++;}else{count=0; if(effect==0){effect=8;}else{effect=0;}}
    if (BTN1 & count < 100) {count++;}else{count=0; if(effect==1){effect=8;}else{effect=1;}}
    if (BTN2 & count < 100) {count++;}else{count=0; if(effect==2){effect=8;}else{effect=2;}}
    if (BTN3 & count < 100) {count++;}else{count=0; if(effect==3){effect=8;}else{effect=3;}}
    if (BTN4 & count < 100) {count++;}else{count=0; if(effect==4){effect=8;}else{effect=4;}}
    if (BTN5 & count < 100) {count++;}else{count=0; if(effect==5){effect=8;}else{effect=5;}}
    if (BTN6 & count < 100) {count++;}else{count=0; if(effect==6){effect=8;}else{effect=6;}}
    if (BTN7 & count < 100) {count++;}else{count=0; if(effect==7){effect=8;}else{effect=7;}} 
     

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
  if(effect==5){out_DAC0 = WAHWAH(POT0, POT1, POT2, in_ADC0);}
  if(effect==6){out_DAC0 = FLANGER(POT0, POT1,  in_ADC0);}
  if(effect==7){out_DAC0 = LPHP(POT0, POT1, in_ADC0);}
 
  dacc_set_channel_selection(DACC_INTERFACE, 0);         
  dacc_write_conversion_data(DACC_INTERFACE, out_DAC0);
}
