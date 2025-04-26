//Version 1.1 of ABEX (Automatic BadUpdate Executor for XBOX360) by IvnLS

#define BUTTON_RB 27 // Button RB
#define BUTTON_A 26 // Button A
#define POWER_CONT 25 // Controller power
#define POWER_XBOX 32 // Xbox power
#define AUDIO_RCA 35 // RCA audio capture
#define BUZZER 33 // Buzzer to indicate stages
#define LED 2 // led
#define LED_CONT 34 // Controller LED, to check connection

unsigned long ultimaVerificacao = 0;
const unsigned long INTERVALO = 10000; // 10 seconds

unsigned long inicioAmostragem = millis();
int valorMin = 4095;
int valorMax = 0;

bool freezed = false;
bool sucess = false;

void setup() {
  Serial.begin(115200);
  
  pinMode(BUTTON_RB, OUTPUT);
  pinMode(BUTTON_A, OUTPUT);
  pinMode(POWER_CONT, OUTPUT);
  pinMode(POWER_XBOX, OUTPUT);
  pinMode(LED, OUTPUT);
  
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);

  // Start with transistors turned off except the control
  digitalWrite(BUTTON_RB, LOW);
  digitalWrite(BUTTON_A, LOW);
  digitalWrite(POWER_CONT, HIGH);
  digitalWrite(POWER_XBOX, LOW);
}

void loop() {

  digitalWrite(POWER_CONT, HIGH);
  
  while(verifyCont(LED_CONT) < 1 || verifyCont(LED_CONT) > 2.7) {
    digitalWrite(LED, HIGH);
    delay(200);
    digitalWrite(LED, LOW);
    delay(1000);
  }
  
  while(!sucess) {
    freezed = false;
    delay(22000);

    if (verifyCont(LED_CONT) < 1 || verifyCont(LED_CONT) > 2.7) {
      Serial.println("The console did not start!");
      break;  
    }
    
    //2xRB 3xA
    Serial.println("Entering BadUpdate...");

    beep(250);
    
    //2x RB
    digitalWrite(BUTTON_RB, HIGH);
    delay(500);
    digitalWrite(BUTTON_RB, LOW);
    delay(1500);

    digitalWrite(BUTTON_A, HIGH);
    delay(250);
    digitalWrite(BUTTON_A, LOW);

    delay(8000); // Sometimes games take a long time to load on the dashboard

    //Open RBB
    digitalWrite(BUTTON_A, HIGH);
    delay(250);
    digitalWrite(BUTTON_A, LOW);
  
    // RBB loading
    delay(35000);
  
    // Execute exploit
    digitalWrite(BUTTON_A, HIGH);
    delay(250);
    digitalWrite(BUTTON_A, LOW);

    beep(250);

    digitalWrite(POWER_CONT, LOW);
    
    //Verification loop
    while(!freezed) {
      inicioAmostragem = millis();
      valorMin = 4095;
      valorMax = 0;

      while (millis() - inicioAmostragem < INTERVALO) {
        int leitura = analogRead(AUDIO_RCA);
        if (leitura < valorMin) valorMin = leitura;
        if (leitura > valorMax) valorMax = leitura;
        delay(10); // collect every 10ms
      }
    
      int variacao = valorMax - valorMin;
      Serial.print("Variation after 10s: ");
      Serial.println(variacao);

      if (variacao > 200) {
        freezed = false;
      } else {
        delay(4000);

        // Turn the controller on
        digitalWrite(POWER_CONT, HIGH);
        delay(8000);

        //If the controller was connected
        if (verifyCont(LED_CONT) > 1.0 && verifyCont(LED_CONT) < 2.7) {

          //turn off controller
          digitalWrite(POWER_CONT, LOW);
          
          Serial.println("Success! Enjoy your ABEX exploit");

          //Successful Buzzer
          for(int i=1; i <= 4; i++) {
            tone(BUZZER, i * 1000);
            delay(250);
          }
          noTone(BUZZER);
          
          delay(2000);
          sucess = true;

          sucessLoop();
        } else {
          Serial.println("It appears the ABEX exploit failed, restarting the process...");

          //Failure Buzzer
          tone(BUZZER, 500);
          delay(1000);
          noTone(BUZZER);

          digitalWrite(POWER_CONT, LOW);

          //Command to shut down
          xboxPower();

          //turn off controller
          digitalWrite(POWER_CONT, HIGH);

          delay(10000);
          
          //turn on
          xboxPower();

          freezed = true;
          break;
        }
    
        // reboot
        valorMin = 4095;
        valorMax = 0;
        ultimaVerificacao = millis();
      }
      
      delay(10); //Sampling every 10ms
    }
  }  
}

//check if the controller has been connected by voltage
float verifyCont(int pino) {
  int leitura = analogRead(pino);
  float tensao = (leitura / 4095.0) * 3.3;
  return tensao;
}

void beep(int milis) {
  tone(BUZZER, 1000);
  delay(milis);
  noTone(BUZZER);
}

void xboxPower() {
  digitalWrite(POWER_XBOX, HIGH);
  delay(250);
  digitalWrite(POWER_XBOX, LOW);
}

void sucessLoop() {
  while (true) {
    digitalWrite(LED, HIGH);
    delay(200);
    digitalWrite(LED, LOW);
    delay(200);
  }
}
