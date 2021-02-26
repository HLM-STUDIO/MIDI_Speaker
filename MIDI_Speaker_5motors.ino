typedef struct {
  float during;
  int x_fre;
  int y_fre;
  int z_fre;
  int a_fre;
  int m_fre;
} MIDI;

#define NOTE_NUMBER 2014
const MIDI NOTE_INFO[NOTE_NUMBER] PROGMEM = {
  //数据存放地
};
int x_frequency = 0, y_frequency = 0, z_frequency = 0, a_frequency = 0, m_frequency = 0;
int x_number_counter = 0, y_number_counter = 0, z_number_counter = 0, a_number_counter = 0, m_number_counter = 0;
#define TIMER1_FREQUENCY (62200)
#define ENABLE PCINT0   //使能脚位，相当于DRV8825 1脚置于低电平，对应uno 8号脚
#define X_MOTOR_PULSE PCINT18  //X电机脉冲，对应uno 2号脚
#define Y_MOTOR_PULSE PCINT19  //Y电机脉冲，对应uno 3号脚
#define Z_MOTOR_PULSE PCINT20  //Z电机脉冲，对应uno 4号脚
#define A_MOTOR_PULSE PCINT4   //A电机脉冲，对应uno 12号脚
#define M_MOTOR_PULSE PCINT2   //M电机脉冲，对应uno 9号脚
#define X_MOTOR_DIRECTION PCINT21    //X电机方向，对应uno 5号脚
#define Y_MOTOR_DIRECTION PCINT22    //Y电机方向，对应uno 6号脚
#define Z_MOTOR_DIRECTION PCINT23    //Z电机方向，对应uno 7号脚
#define A_MOTOR_DIRECTION PCINT5     //A电机方向，对应uno 13号脚
#define M_MOTOR_DIRECTION PCINT3     //M电机方向，对应uno 10号脚

void setup() {
  TCCR1A = 0; //ATmega328p手册 p133左右
  TCCR1B |= (1 << WGM12); //  Clear Timer on Compare Match (CTC) Mode
  TCCR1B |= (1 << CS10) | (1 << CS11); // Prescaler == 64
  TCNT1 &= ~0xff;//定时器1计数器寄存器清空
  OCR1A = 1;//比较寄存器设置为1
  TIMSK1 |= _BV(OCIE1A);//开启定时器2中断
  DDRB |= _BV(ENABLE) | _BV(A_MOTOR_PULSE) | _BV(A_MOTOR_DIRECTION) | _BV(M_MOTOR_PULSE) | _BV(M_MOTOR_DIRECTION);
  DDRD |= _BV(X_MOTOR_PULSE) | _BV(Y_MOTOR_PULSE) | _BV(X_MOTOR_DIRECTION) | _BV(Y_MOTOR_DIRECTION) | _BV(Z_MOTOR_PULSE) | _BV(Z_MOTOR_DIRECTION) ;
  PORTD = 0;
}

void loop() {
  for (int i = 0; i < NOTE_NUMBER; i++) {
    x_frequency =  pgm_read_dword(&NOTE_INFO[i].x_fre);
    y_frequency = pgm_read_dword(&NOTE_INFO[i].y_fre);
    z_frequency = pgm_read_dword(&NOTE_INFO[i].z_fre);
    a_frequency = pgm_read_dword(&NOTE_INFO[i].a_fre);
    tone(10, pgm_read_dword(&NOTE_INFO[i].m_fre), pgm_read_float(&NOTE_INFO[i].during)) ;
    delay(pgm_read_float(&NOTE_INFO[i].during));
    noTone(10);
  }
}

ISR(TIMER1_COMPA_vect) { //中断请求 interrupt service require
  if (x_frequency != 0) {
    if (++x_number_counter >= (x_frequency)) {
      x_number_counter = 0;
      if (bitRead(PORTD, X_MOTOR_PULSE))
        bitClear(PORTD, X_MOTOR_PULSE);
      else
        bitSet(PORTD, X_MOTOR_PULSE);
    }
  }
  if (y_frequency != 0) {
    if (++y_number_counter >= (y_frequency)) {
      y_number_counter = 0;
      if (bitRead(PORTD, Y_MOTOR_PULSE))
        bitClear(PORTD, Y_MOTOR_PULSE);
      else
        bitSet(PORTD, Y_MOTOR_PULSE);
    }
  }
  if (z_frequency != 0) {
    if (++z_number_counter >= (z_frequency)) {
      z_number_counter = 0;
      if (bitRead(PORTD, Z_MOTOR_PULSE))
        bitClear(PORTD, Z_MOTOR_PULSE);
      else
        bitSet(PORTD, Z_MOTOR_PULSE);
    }
  }
  if (a_frequency != 0) {
    if (++a_number_counter >= (a_frequency)) {
      a_number_counter = 0;
      if (bitRead(PORTB, A_MOTOR_PULSE))
        bitClear(PORTB, A_MOTOR_PULSE);
      else
        bitSet(PORTB, A_MOTOR_PULSE);
    }
  }

}
