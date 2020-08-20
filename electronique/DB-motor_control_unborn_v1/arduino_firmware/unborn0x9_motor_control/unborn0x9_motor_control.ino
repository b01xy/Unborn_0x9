#define NOP __asm__ __volatile__ ("nop\n\t");

//GPIO definition
int Trig_In = 2;
int Step = 3;
int Trig_Out = 4;
int Pulse_Neg = 5;
int Pulse_Pos = 6;
int Enable_motor = 7;
int Direction_motor = 8;
int IN4=12, IN3=11, IN2=10, IN1=9;


int pas=0;
int en=0;

void pulse()
{
  PORTD=B01010000;
  NOP;
  NOP;
  PORTD=B00000000;
}

void setup() {
  // INut your setuIN code here, to run once:
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  //pinMode(Direction, INPUT_PULLUP);
  //pinMode(Enable, INPUT_PULLUP);
  pinMode(13, OUTPUT);
  digitalWrite(IN4,0);
  digitalWrite(IN3,0);
  digitalWrite(IN2,0);
  digitalWrite(IN1,0);
  Serial.begin(115200);
  
  DDRD = B01110000; //GPIO 4,5,6 as OUTPUT
                    //GPIO 0,1,2,3,7 as INPUT
  
  //pinMode(2,INPUT);
  attachInterrupt(digitalPinToInterrupt(Trig_In), pulse, FALLING);
  attachInterrupt(digitalPinToInterrupt(Step), apply_step, RISING);
  
  pinMode(Direction_motor, INPUT);
  pinMode(Enable_motor, INPUT);
}

int lpas=0; //true value of the step
void set_step(int astep)
{
  lpas=astep%8;
  if (lpas<0) {lpas+=8;}
  //pas=lpas;
  switch(lpas)
  {

    case 0:
      digitalWrite(IN4,1);
      digitalWrite(IN3,0);
      digitalWrite(IN2,0);
      digitalWrite(IN1,0);
      break;
    case 1:
      digitalWrite(IN4,1);
      digitalWrite(IN3,1);
      digitalWrite(IN2,0);
      digitalWrite(IN1,0);
      break;
    case 2:
      digitalWrite(IN4,0);
      digitalWrite(IN3,1);
      digitalWrite(IN2,0);
      digitalWrite(IN1,0);
      break;
    case 3:
      digitalWrite(IN4,0);
      digitalWrite(IN3,1);
      digitalWrite(IN2,1);
      digitalWrite(IN1,0);
      break;
    case 4:
      digitalWrite(IN4,0);
      digitalWrite(IN3,0);
      digitalWrite(IN2,1);
      digitalWrite(IN1,0);
      break;
    case 5:
      digitalWrite(IN4,0);
      digitalWrite(IN3,0);
      digitalWrite(IN2,1);
      digitalWrite(IN1,1);
      break;
    case 6:
      digitalWrite(IN4,0);
      digitalWrite(IN3,0);
      digitalWrite(IN2,0);
      digitalWrite(IN1,1);
      break;
    case 7:
      digitalWrite(IN4,1);
      digitalWrite(IN3,0);
      digitalWrite(IN2,0);
      digitalWrite(IN1,1);
      break;
    default:
      digitalWrite(IN4,0);
      digitalWrite(IN3,0);
      digitalWrite(IN2,0);
      digitalWrite(IN1,0);
      break;
  }
}

void set_motor_off()
{
  digitalWrite(IN4,0);
  digitalWrite(IN3,0);
  digitalWrite(IN2,0);
  digitalWrite(IN1,0);
}

void apply_step()
{
  int sens = digitalRead(Direction_motor);
  int en = digitalRead(Enable_motor);

  if (en)
  {
    if (sens==1) {pas++;}
    else {pas--;}
  }

  set_step(pas);
}

void loop() 
{
  en = digitalRead(Enable_motor);
  if (!en)
  {
    set_motor_off();
  }
  delay(100);
}
