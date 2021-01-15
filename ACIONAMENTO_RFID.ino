/*
 * --------------------------------------------------------------------------------------------------------------------
 * Example sketch/program showing how to read new NUID from a PICC to serial.
 * --------------------------------------------------------------------------------------------------------------------
 * This is a MFRC522 library example; for further details and other examples see: https://github.com/miguelbalboa/rfid
 * 
 * Example sketch/program showing how to the read data from a PICC (that is: a RFID Tag or Card) using a MFRC522 based RFID
 * Reader on the Arduino SPI interface.
 * 
 * When the Arduino and the MFRC522 module are connected (see the pin layout below), load this sketch into Arduino IDE
 * then verify/compile and upload it. To see the output: use Tools, Serial Monitor of the IDE (hit Ctrl+Shft+M). When
 * you present a PICC (that is: a RFID Tag or Card) at reading distance of the MFRC522 Reader/PCD, the serial output
 * will show the type, and the NUID if a new card has been detected. Note: you may see "Timeout in communication" messages
 * when removing the PICC from reading distance too early.
 * 
 * @license Released into the public domain.
 * 
 * Typical pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 */
//AUTOR: FRANSÉRGIO MARTINS CARVALHO
//CURSO: SISTEMAS EMBARCADOS NO SENAI SP
//DATA: 05/05/2020
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9

#define cad_decad 8//PINO PARA CADASTRAR OU DECADASTRAR
#define abre 7//PINO QUE ENERGIZA E DESENERGIZA BOBINA 
#define trava 4//PINO DE SINALIZAÇÃO DE TRAVAMENTO

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

MFRC522::MIFARE_Key key; 
  

int registro[10];//BANCO DE DADOS DOS CARTÕES CADASTRADOS->ATÉ DEZ CADASTROS
unsigned int novo=0;//INCREMENTADO QUANDO UM NOVO CARTÃO É CADASTRADO
unsigned short int c_a;//CONTAGEM DOS VALORES REGISTRADOS PARA FINS DE INCLUSÃO
bool d=0;//VARIÁVEL DE DECISÃO: d=1->CARTÃO JÁ CADASTRADO /// d=0->CARTÃO NÃO CADASTRADO
unsigned int menos;//MARCA O ÍNDICE DO CADASTRO RETIRADO
void setup() {
   
  Serial.begin(9600);//CONFIGURA COMUNICAÇÃO SERIAL PARA VALIDAÇÃO
  SPI.begin(); // CONFIGURA COMUNICAÇÃO SPI
  rfid.PCD_Init(); // INICIA COMUNICAÇÃO COM DISPOSITIVO PCD

pinMode(abre,OUTPUT);//PINO PARA DESTRAVAR A PORTA
pinMode(trava,OUTPUT);//PINO PARA SINALIZAR PORTA TRAVADA
}
 
void loop() {
  digitalWrite(abre,HIGH);//BOBINA ENERGIZADA
  
//VERIFICA SE HÁ TAG PRESENTE, CASO NÃO REINICIA O LAÇO
  if ( ! rfid.PICC_IsNewCardPresent())
  {
    return;
    }

  // VERIFICA SE A NUID FOI LIDA, CASO NÃO REINICIA O LAÇO
  if ( ! rfid.PICC_ReadCardSerial())
  {
    return;
    }

  Serial.println();
  Serial.print(printDec(rfid.uid.uidByte));//IMPRIME NA SERIAL APENAS O VALOR DO PRIMEIRO BYTE DOS QUATRO PRIMEIROS 
  Serial.println();
  for(c_a=0;c_a<=novo;++c_a)
  {
	if(registro[c_a]==printDec(rfid.uid.uidByte))
        {
			d=1;menos=c_a;
		} //SEMPRE QUE RECEBE UM CARTÃO CADASTRADO FAZ d=1 E MARCA O ÍNDICE DO CARTÃO EM MENOS
  }
  
  if((d==0) && !(digitalRead(cad_decad)))
      {
          Serial.println(F("Cartão não cadastrado")); //SE O CARTÃO NÃO ESTÁ CADASTRADO E O BOTÃO NÃO ESTÁ PRESSIONADO
        //PISCA O LED TRES VEZES COMO SINALIZAÇÃO
          digitalWrite(trava,HIGH);
          delay(250);
          digitalWrite(trava,LOW);
          delay(250);

          digitalWrite(trava,HIGH);
          delay(250);
          digitalWrite(trava,LOW);
          delay(250);

          digitalWrite(trava,HIGH);
          delay(250);
          digitalWrite(trava,LOW);
          delay(250);
      }

      if((!d) && (digitalRead(cad_decad)))
        {
          //SE O CARTÃO NÃO ESTÁ CADASTRADO E O BOTÃO ESTÁ PRESSIONADO
          registro[novo]=printDec(rfid.uid.uidByte);++novo;  //CADASTRA CARTÃO
          Serial.println(F("Cadastrou cartão"));
        }

      if((d) && (digitalRead(cad_decad)))//SE O CARTÃO ESTÁ CADASTRADO E O BOTÃO ESTÁ PRESSIONADO:DESCADASTRAR
      {
			unsigned short int rast=menos;
			for(rast;rast<=novo;++rast)
        {
		//DESCADASTRA CARTÃO RETIRANDO O ÚLTIMO
          if(menos==novo)
		  {
			  registro[menos]=0;
		  }
		  else
		  {
		// OU FAZENDO UM LEFT SHIFT ACIMA DO RETIRADO
		registro[rast]=registro[rast+1];
		};
		}
		//DIMINUIU 1 CARTÃO TAG DO REGISTRO
        novo=novo-1;                                                                                           //DIMINUI O NUMERO DOS CADASTRADOS
        Serial.println(F("Descadastrou cartão"));
      }
      
      if((d) && !(digitalRead(cad_decad)))//SE O CARTÃO ESTÁ CADASTRADO E O BOTÃO NÃO ESTÁ PRESSIONADO
      {  
        Serial.println(F("Cartão Cadastrado"));
        digitalWrite(abre,LOW);//BOBINA DESENERGIZADA POR 3 SEGUNDOS
        delay(3000);
      }
        digitalWrite(abre,HIGH);//BOBINA VOLTA A FICAR ENERGIZADA
        Serial.println(novo);//VALIDA QUANTOS CARTÕES ESTÃO CADASTRADOS
        d=0;//TODO CARTÃO NÃO ESTÁ CADASTRADO ATÉ QUE SE PROVE O CONTRÁRIO
    
    // ABORTA COMUNICAÇÃO COM TAG
    rfid.PICC_HaltA();

    // PARA DECODIFICAÇÃO NO LEITOR DE TAG
    rfid.PCD_StopCrypto1();
  }

  //RECEBE O ARRAY QUE CONTÉM OS 4 PRIMEIROS BYTES E RETORNA APENAS O PRIMEIRO NA FORMA DECIMAL
  int printDec(byte *buffer) {
      return buffer[0];
  }
