#define READ_PIN    D0
#define WRITE_PIN   D1
#define CLK_PIN     D2

#define PING_TIME   400

#define BLINK_MESSAGE           0b10110001110001110000000000000000 // (ALL) (KEEP ALIVE=7) (NID) (BLINK) (no data)
#define PULSE_MESSAGE           0b10010000010100110000000000000000 // (DOWNSTREAM) (KEEP ALIVE=1) (UPSTREAM) (PULSE) (no data)
#define DATA_MESSAGE            0b10000001111001000000000000000000 // (NID) (KEEP ALIVE=7) (CHANNEL 1) (DATA) (no data)
#define DEND_PING               0b10010000010100010000000000000000 // (DOWNSTREAM) (KEEP ALIVE=1) (UPSTREAM) (PING) (no data)
#define NID_PING                0b10111000000000010000000000000000 // (ALL) (KEEP ALIVE=32) (NID) (PING) (no data)
#define NID_IDENTIFY_REQUEST    0b10110001110001010000000000000100 // (ALL) (KEEP ALIVE=7) (NID) (IDENTIFY) (data=0b100 : channel 1)

#define LEAD_BIT_MASK           0b10000000000000000000000000000000
#define RECIPIENT_MASK          0b01110000000000000000000000000000
#define KEEP_ALIVE_MASK         0b00001111110000000000000000000000
#define SENDER_MASK             0b00000000001110000000000000000000
#define HEADER_MASK             0b00000000000001110000000000000000
#define DATA_MASK               0b00000000000000001111111111111111


typedef enum{
    NID         =   0b000,
    DOWNSTREAM  =   0b001,
    UPSTREAM    =   0b010,
    ALL         =   0b011,
    SELECTED1   =   0b100,
    SELECTED2   =   0b101,
    SELECTED3   =   0b110,
    SELECTED4   =   0b111
} device_identifiers;

typedef enum{
    PING        =   0b001,
    PULSE       =   0b011,
    BLINK       =   0b111,
    DATA        =   0b100,
    IDENTIFY    =   0b101
} message_identifiers;

int incoming_serial_byte            =   0;

unsigned int read_buffer       =   0;
int read_buffer_count               =   0;
unsigned int write_buffer[3]   =   {0,0,0};
int write_buffer_count              =   0;
int write_buffer_ready_count        =   0;

int i;


Timer ping_timer(PING_TIME, writePing);

void setup() {
    pinMode(READ_PIN, INPUT_PULLDOWN);
    pinMode(WRITE_PIN, OUTPUT);

    Serial.begin();
    attachInterrupt(CLK_PIN, clk_isr, CHANGE);
    attachInterrupt(READ_PIN, read_isr, CHANGE);

    ping_timer.start();
}

void loop() {
    unsigned int recipient_id;
    unsigned int keep_alive;
    unsigned int sender_id;
    unsigned int header;
    unsigned int data_frame;
    unsigned int message_buffer;
    if (Serial.available() > 0){
        incoming_serial_byte = Serial.read();
        Serial.print("received: ");
        Serial.println(incoming_serial_byte, DEC);
    } else{
        incoming_serial_byte = 0;
    }
    if (incoming_serial_byte == 49){
        write_buffer[write_buffer_ready_count] = BLINK_MESSAGE;
        write_buffer_ready_count += 1;
        incoming_serial_byte = 0;
    } else if (incoming_serial_byte == 50){
        write_buffer[write_buffer_ready_count] = NID_IDENTIFY_REQUEST;
        write_buffer_ready_count += 1;
    }
    if (read_buffer_count >= 33){
        recipient_id = read_buffer & RECIPIENT_MASK; // 3-bit recipient id
        keep_alive = read_buffer & KEEP_ALIVE_MASK; // 6-bit keep alive
        sender_id = read_buffer & SENDER_MASK; // 3-bit sender id
        header = read_buffer & HEADER_MASK; // 3-bit message id
        data_frame = read_buffer & DATA_MASK; // 16-bit data frame
        if (data_frame != 0){
            Serial.print('data from channel ');
            Serial.print((device_identifiers) sender_id, DEC);
            Serial.print(" : ");
            Serial.println(data_frame, DEC);
        }
        //Serial.print(read_buffer, DEC);
        read_buffer = 0;
    }
}

void clk_isr(){
    int value = 0;
    if (read_buffer_count < 34){
        read_buffer <<= 1;
        value = digitalRead(READ_PIN);
        if (value != 0) read_buffer |= 1;
        read_buffer_count += 1;
    }
    if (write_buffer_ready_count >= 1){
        if (write_buffer_count < 33){
            value = write_buffer[0];
            value >>= 31;
            if (value != 0){
                pinSetFast(WRITE_PIN);
            } else{
                pinResetFast(WRITE_PIN);
            }
            write_buffer[0] <<= 1;
            write_buffer_count += 1;
        } else{
            write_buffer_count = 0;
            for(i=0; i<write_buffer_ready_count; i++){
                write_buffer[i] = write_buffer[i+1];
            }
            write_buffer_ready_count -= 1;
        }
    }
}

void read_isr(){
    if (read_buffer_count > 34){
        read_buffer_count = 1;
    }
}

void writePing(){
    write_buffer[write_buffer_ready_count] = NID_PING;
    write_buffer_ready_count += 1;
    write_buffer[write_buffer_ready_count] = DEND_PING;
    write_buffer_ready_count += 1;
}