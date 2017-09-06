#include <SPI.h>

#define PIN_SPI_MOSI 11 // Master Out Slave In 
#define PIN_SPI_MISO 12 // Master In Slave Out
#define PIN_SPI_SCK 13 //Serial Clock
#define PIN_SPI_SS 10 // Slave Serect pin
#define PIN_BUSY 8 // busy mode

void setup() {
 // put your setup code here, to run once:
        pinMode(PIN_SPI_MISO, INPUT);
        pinMode(PIN_SPI_MOSI, OUTPUT);
        pinMode(PIN_SPI_SCK, OUTPUT);
        pinMode(PIN_SPI_SS, OUTPUT);
        pinMode(PIN_BUSY, INPUT);  

        digitalWrite(PIN_SPI_SS, HIGH); 

        SPI.begin();
        // SPI通信のセットアップ
        SPI.setDataMode(SPI_MODE3); // L6470の仕様に合わせる
        SPI.setBitOrder(MSBFIRST); // L6470の仕様に合わせる

        int m_step = 400; // 使用するモータの一回転あたりのステップ数
        L6470_resetDevice();
        L6470_setup();

        //MAX_SPEEDの値を変更
        /*
        L6470_send(0x07);
        L6470_send(0x01);
        L6470_send(0x89);        
        */
        /*
        // 目標速度rps[step/s]を指定して回転
        L6470_rpsRun(0, 2, m_step);
        // ex)
        // L6470_send(0x51) or L6470_send(0x50)
        // L6470_send(0x0X);
        // L6470_send(0xXX);
        // L6470_send(0xXX);

        // 目標速度に達した後に回転時間を指定
        while(!digitalRead(PIN_BUSY)){}
        delay(5000);

        // モータの停止
        int stopCmd = 2;
        if (stopCmd == 1){
                // 減速あり、角度保持あり
                L6470_softStop();                
        }else if (stopCmd == 2){
                // 減速あり、角度保持なし
                L6470_softHiZ();                
        }else if (stopCmd == 3){
                // 減速なし、角度保持あり
                L6470_hardStop();                
        }else if (stopCmd == 4){
                // 減速なし、角度保持なし
                L6470_hardHiZ();                
        }
        */

        // MAX_SPEEDの指定
        //  L6470_rpsMaxSpeed(5, m_step); 上手く動かない
        /*
        L6470_send(0x07);
        L6470_send(0x00);
        L6470_send(0x83);       
        */
        
        // ステップ数指定で回転
        // 1/128ステップより，400*128 = 1回転
        // 10回転 = 512,000 ->  0x7D000
/*
        // ⓪
        L6470_stepMove(1, 4000); 上手く動かない
        // ➀
        L6470_send(0x41);
        L6470_send(0x07);
        L6470_send(0xD0);
        L6470_send(0x00);
        L6470_softStop();
*/
        // ➁
        long n_step = 51200/4;
        int data[3];
        for(int i = 0; i < 3; i++){
                data[i] = n_step & 0xff;        
                Serial.println(data[i]);
                n_step = n_step >> 8;
        }
        L6470_send(0x41);
        L6470_send(data[2]);
        L6470_send(data[1]);
        L6470_send(data[0]);        
        L6470_softHiZ();
 }

void loop() {
 // put your main code here, to run repeatedly:
 

}
 
void L6470_send( unsigned char add_or_val ){
        while( !digitalRead(PIN_BUSY) ){ // BUSY(LOW状態)が解除されるまで待機
        }
        digitalWrite( PIN_SPI_SS, LOW);
        SPI.transfer( add_or_val ); // 1バイト(8ビット)のデータを送信
        digitalWrite( PIN_SPI_SS, HIGH);
}

void L6470_transfer( int add, int bytes, long val ){
       int data[ 3 ];
        for( int i = 0; i < bytes; i++) {
                data[ i ] = val & 0xff;
                val = val >> 8;
        }
 
        if ( bytes == 3 ) {        
                L6470_send( add );
                L6470_send( data[2] ) ;
                L6470_send( data[1] ) ;
                L6470_send( data[0] ) ;
        }
        else if ( bytes == 2 ) {
                L6470_send( add );
                L6470_send( data[1] ) ;
                L6470_send( data[0] ) ;
        }
        else if ( bytes == 1 ) {
                L6470_send( add );
                L6470_send( data[0] ) ;
        }
}

void L6470_resetDevice(){
        // NOP command : Nothing is performed，何もしない
        // command structure:  00000000 -> 0
        // 4回nop命令を送る(4byte分)．これは，L6470に一度に送る情報が最大で4byteであるため
        // 4回nopを送ることで，確実に前のコマンドを終了させてからresetする
        L6470_send(0x00);
        L6470_send(0x00);        
        L6470_send(0x00);        
        L6470_send(0x00);
        
        // ResetDevice command : 起動時の状態(power-up conditions)にL6470をリセットする
        // command structure: 11000000 -> c0   
        L6470_send(0xc0);
}
/*
void L6470_maxSpeed(){
        long spd = ( (rps * m_step) * 250 * pow(10, -9) ) / pow(2, -18);
        L6470_transfer( 0x07, 2, spd );
        
        //      400×5 [step/s]ができる値にしたい
        //      a [step/s] = ( SPEED × 2^-18 ) / tick
        //      2000 [step/s] = ( SPEED × 2^-18 ) / ( 250 × 10^-9 )
        //      SPEED [step/tick] = ( 2000 × 250 × 10^-9 ) / 2^-18
        //      SPEED [step/tick] = 2000 × 250 × 10^-9 × 2^18
        //      SPEED [step/tick] = 131.07 = 131
        //      -> 0x83

        //      4000 [step/s] = ( SPEED × 2^-18 ) / ( 250 × 10^-9 )
        //      SPEED [step/tick] = ( 4000 × 250 × 10^-9 ) / 2^-18
        //      SPEED [step/tick] = 4000 × 250 × 10^-9 × 2^18
        //      SPEED [step/tick] = 262.144 = 262 
        //      -> 0x106

        //      6000 [step/s] = ( SPEED × 2^-18 ) / ( 250 × 10^-9 )
        //      SPEED [step/tick] = ( 6000 × 250 × 10^-9 ) / 2^-18
        //      SPEED [step/tick] = 6000 × 250 × 10^-9 × 2^18
        //      SPEED [step/tick] = 393.216 = 393
        //      -> 0x189
};
*/
void L6470_stepMove( boolean dir, int n_step ){
        // 1/128ステップより，一回転が400ステップのモータでは400*128 = 51200ステップ = 1回転
        // 10回転 = 512,000ステップ ->  0x7D000

        int step_mode  = 128; // 1/128マイクロステップ
        n_step = n_step * step_mode;       
        if ( dir == 1 ){
                L6470_transfer(0x41, 3, n_step);        
        }
        if ( dir == 0 ){
                L6470_transfer(0x40, 3, n_step);        
        }
 }

void L6470_rpsRun( boolean dir, float rps , int m_step ){
        // dir: 回転方向
        // rps: 1秒あたりの回転数[step/s]
        // m_step: 使用するステッピングモータの1回転あたりのステップ数
        
        // 定速回転Run(DIR, Spd)：回転方向DIRと目標速度Spdを指定して定速回転させる
        //      command structure: 0101000D
        //      DIRの設定：
        //              -> D = 1: 正回転 01010001 -> 51
        //              -> D = 0: 逆回転 01010000 -> 50
        //      Spdの設定
        //              定速回転コマンド（0x5D）の直後に20bitで指定する．
        //              （実際は24bit（3byte）分を送信することになるため，上位側の4bitは常に0．）
        //              ステップ/動作クロック[step/tick]で速度を指定する（SPEEDレジスタ（20bit, 現在の速度）と同じ形式）．
        //              MAX_SPEED，MIN_SPEEDレジスタの間の値を指定する（そうでない場合はMAX，MINの速度値が優先される）
        //      step/tickの値からstep/sの値を計算する式（データシートより引用．動作クロックtickの値は250ns）
        //              a [step/s] = ( Spd × 2^-28 ) / tick
        //              spd [step/tick] = a × tick / 2^-28      

        long spd = (rps * m_step) * (250 * pow(10, -9)) / pow(2, -28);
        
        if ( dir == 1 ){
                L6470_transfer(0x51, 3, spd);
        }
        if ( dir == 0 ){
                L6470_transfer(0x50, 3, spd);
        }
        // 計算の例
        // 毎秒0.5回転, 200 step/s
        // 200 [step/s] = ( SPEED × 2^-28 ) / ( 250 × 10^-9 ) 
        // SPEED [step/tick] = ( 200 × 250 × 10^-9 ) / 2^-28
        // SPEED [step/tick] = 13421.7728 = 13422 = 0x346E

        // 毎秒1回転, 400 step/s
        // 400 [step/s] = ( SPEED × 2^-28 ) / ( 250 × 10^-9 ) 
        // SPEED [step/tick] = ( 400 × 250 × 10^-9 ) / 2^-28
        // SPEED [step/tick] = 26843.5456 = 26844 = 0x68DC

        // 毎秒5回転, 400 * 5 step/s
        // 2000 [step/s] = ( SPEED × 2^-28 ) / ( 250 × 10^-9 ) 
        // SPEED [step/tick] = ( 2000 × 250 × 10^-9 ) / 2^-28
        // SPEED [step/tick] = 134217.728 = 134218 = 0x20C4A

        // 毎秒10回転, 400 * 10 step/s
        // 4000 [step/s] = ( SPEED × 2^-28 ) / ( 250 × 10^-9 ) 
        // SPEED [step/tick] = ( 4000 × 250 × 10^-9 ) / 2^-28
        // SPEED [step/tick] = 268435.456 = 268435 = 0x41893

        // 毎秒15回転, 400 * 15 step/s
        // 6000 [step/s] = ( SPEED × 2^-28 ) / ( 250 × 10^-9 ) 
        // SPEED [step/tick] = ( 6000 × 250 × 10^-9 ) / 2^-28
        // SPEED [step/tick] = 402653.184  = 402653 = 0x624DD
};

void L6470_softStop(){
        L6470_send(0xb0);
}
void L6470_hardStop(){
        L6470_send(0xb8);        
}
void L6470_softHiZ(){
        L6470_send(0xa0);
}
void L6470_hardHiZ(){
        L6470_send(0xa8);
}

// 初期設定
void L6470_setup(){

}

