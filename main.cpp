#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>

using namespace std;

// big shout out to wikipedia!! kings.
#define HI_NIBBLE(b) (((b) >> 4) & 0x0F)
#define LO_NIBBLE(b) ((b) & 0x0F)

uint8_t fontset[80] {
   0xf0, 0x90, 0x90, 0x90, 0xf0, // 0
   0x20, 0x60, 0x20, 0x20, 0x70, // 1
   0xf0, 0x10, 0xf0, 0x80, 0xf0, // 2
   0xf0, 0x10, 0xf0, 0x10, 0xf0, // 3
   0x90, 0x90, 0xf0, 0x10, 0x10, // 4
   0xf0, 0x80, 0xf0, 0x10, 0xf0, // 5
   0xf0, 0x80, 0xf0, 0x90, 0xf0, // 6
   0xf0, 0x10, 0x20, 0x40, 0x40, // 7
   0xf0, 0x90, 0xf0, 0x90, 0xf0, // 8
   0xf0, 0x90, 0xf0, 0x10, 0xe0, // 9
   0xf0, 0x80, 0x80, 0x80, 0xf0, // A
   0xe0, 0x90, 0xe0, 0x90, 0xe0, // B
   0xf0, 0x80, 0x80, 0x80, 0xf0, // C
   0xe0, 0x90, 0x90, 0x90, 0xe0, // D
   0xf0, 0x80, 0xf0, 0x80, 0xf0, // E
   0xf0, 0x80, 0xf0, 0x80, 0x80
};

class processor {
   private:
      uint8_t memory[4096];
      uint8_t registers[16];
      bool screen[32][64];
      bool debugState = false;
      bool keyboard[16];
      uint16_t pc = 0;
      uint8_t sp = 0;
      uint16_t stack[16];
      uint16_t I = 0;
      uint8_t delay = 0;
      int cycle = 0;
   public:
      processor();
      void showMemory(uint8_t);
      void showDebug();
      void showScreen();
      void clearMemory();
      void clearScreen();
      void clearStack();
      void clearKeyboard();
      void debugStatus(bool);
      void openProgram();
      void loadFontset();
      uint8_t setMemory(int, uint8_t);
      uint8_t readMemory(int);
      uint8_t setRegister(int,uint8_t);
      uint8_t readRegister(int);
      uint16_t convertnnn(uint8_t, uint8_t);
      void tick();
};

processor::processor() {
   clearMemory();
   clearScreen();
   clearStack();
   loadFontset();
   pc = 0x200;
}

void processor::debugStatus(bool debugOn) {
   debugState = debugOn;
}

uint16_t processor::convertnnn(uint8_t small, uint8_t big) {
   uint16_t nnn = small;
   nnn = nnn << 8;
   nnn+=big;
   return nnn;
}

void processor::tick() {

   while(delay>0) {
      delay--;
   }
   
   uint8_t in1 = readMemory(pc);
   uint8_t in2 = readMemory(pc+1);
   uint8_t opcode = HI_NIBBLE(in1);
   uint16_t nnn = convertnnn(LO_NIBBLE(in1), in2);

   if(debugState) {
      cout.width(2);
      cout 
         << endl 
         << "   --- pc: " << (int)pc << " --- " << endl << endl
         << " instruction: " << hex << setfill('0') << setw(2) << (int)in1 << setfill('0') << setw(2) << (int)in2 << endl
         << "  it is split into: " << (int)opcode << " " << LO_NIBBLE(in1) << " " << HI_NIBBLE(in2) << " " << LO_NIBBLE(in2)
         << endl << dec << endl
      ;
   }
   
   // to be continued
   //

   if(opcode == 0x0) {
      if(in2 == 0xe0) {
         clearScreen();
      }
      else if(in2 == 0xee) {
         pc = stack[sp];
         sp--;
      }
      else {
         pc = nnn;     
      }
   }
   
   else if(opcode == 0x1) {
      pc = nnn;     
   }

   else if(opcode == 0x2) {
      sp++;
      stack[sp] = pc;
      pc = nnn;
   }

   else if(opcode == 0x3) {
      uint8_t x = LO_NIBBLE(in1);
      if(in2==readRegister(x)) {
         pc+=2;
      }
   }

   else if(opcode == 0x4) {
      uint8_t x = LO_NIBBLE(in1);
      if(in2!=readRegister(x)) {
         pc+=2;
      }
   }

   else if(opcode == 0x5) {
      uint8_t x = readRegister(LO_NIBBLE(in1));
      uint8_t y = readRegister(HI_NIBBLE(in2));

      if(x==y) {
         pc+=2;
      }
   }
   
   // set register x to kk
   else if(opcode == 0x6) {
      uint8_t x = LO_NIBBLE(in1);
      uint8_t kk = in2;
      setRegister(x, kk);
   }

   else if(opcode == 7) {
      uint8_t x = LO_NIBBLE(in1);
      setRegister(x, readRegister(x) + in2);
   }

   else if(opcode == 8) {
      if(LO_NIBBLE(in2) == 0) {
         uint8_t x = LO_NIBBLE(in1);
         uint8_t y = HI_NIBBLE(in2);

         setRegister(x, readRegister(y));
      }
      
      else if(LO_NIBBLE(in2) == 1) {
         uint8_t x = LO_NIBBLE(in1);
         uint8_t y = HI_NIBBLE(in2);
         setRegister(x, readRegister(x) | readRegister(y));
      }

      else if(LO_NIBBLE(in2) == 2) {
         uint8_t x = LO_NIBBLE(in1);
         uint8_t y = HI_NIBBLE(in2);
         setRegister(x, readRegister(x) & readRegister(y));
      }

      else if(LO_NIBBLE(in2) == 3) {
         uint8_t x = LO_NIBBLE(in1);
         uint8_t y = HI_NIBBLE(in2);
         setRegister(x, readRegister(x) ^ readRegister(y));
      }

      else if(LO_NIBBLE(in2) == 4) {
         uint8_t x = LO_NIBBLE(in1);
         uint8_t y = HI_NIBBLE(in2);
         if(readRegister(x)+readRegister(y)>255) {
            setRegister(0xf, 1);
         }
         else {
            setRegister(0xf, 0);
         }
         setRegister(x, readRegister(x) + readRegister(y));
         // -------------------------------------------------------------------------- problem might occur here
      }

      else if(LO_NIBBLE(in2) == 5) {
         uint8_t x = LO_NIBBLE(in1);
         uint8_t y = HI_NIBBLE(in2);
         if(readRegister(x)>readRegister(y))
            setRegister(0xf, 1);
         else
            setRegister(0xf, 0);

         setRegister(x, readRegister(x)-readRegister(y));
         // -------------------------------------------------------------------------- problem might occur here
      }

      else if(LO_NIBBLE(in2) == 6) {
         uint8_t x = LO_NIBBLE(in1);
         uint8_t y = HI_NIBBLE(in2);
         if(readRegister(x)%2 == 0) {
            setRegister(15, 0);
         }
         else {
            setRegister(15, 1);
         }
         setRegister(x, readRegister(x)/2);
      }

      else if(LO_NIBBLE(in2) == 7) {
         uint8_t x = LO_NIBBLE(in1);
         uint8_t y = HI_NIBBLE(in2);
         if(readRegister(y)>readRegister(x)) {
            setRegister(15, 1);
         }
         else {
            setRegister(15, 0);
         }
         setRegister(x, readRegister(y) - readRegister(x));
      }

      else if(LO_NIBBLE(in2) == 0xe) {
         uint8_t x = LO_NIBBLE(in1);
         uint8_t y = HI_NIBBLE(in2);
         if(readRegister(x)>127) { setRegister(15, 1);
         }
         else {
            setRegister(15, 0);
         }
         setRegister(x, readRegister(x)*2);
      }


   }

   else if(opcode == 9) {
      uint8_t x = LO_NIBBLE(in1);
      uint8_t y = HI_NIBBLE(in2);
      if(readRegister(x)!=readRegister(y)) {
         pc+=2;
      }
   }

   else if(opcode == 0xa) {
      I = nnn;
   }

   else if(opcode == 0xb) {
      pc = nnn + readRegister(0);
   }

   else if(opcode == 0xc) {
      uint8_t x = LO_NIBBLE(in1);
      uint8_t kk = in2;
      setRegister(x, (rand()%255) & kk);
      cout << endl << "random: " << (int)((rand()%255) & kk) << endl;
   }
   
   // Display   

   else if(opcode == 0xd) {
      uint8_t x = LO_NIBBLE(in1);
      uint8_t y = HI_NIBBLE(in2);
      uint8_t n = LO_NIBBLE(in2);
      uint8_t buffer[n];
      
      if(debugState) {
         cout << endl << "  --- Display ---";
         cout << endl << "     x: " << (int)readRegister(x) << "  y: " << (int)readRegister(y) << endl;
      }
      for(int i = 0; i < n; i++) {
         buffer[i] = readMemory(I+i);
         if(debugState)
            cout << "      Byte: 0x" << hex <<(int)readMemory(I+i) << dec << endl;
      }
      
      uint8_t xc = readRegister(x);
      uint8_t yc = readRegister(y);

      for(int i = 0; i < n; i++) {
         for(int j = 0; j < 8; j++) {
               if((buffer[i] >> j) & 0x80) {
                  if(xc+j>64) {
                     if(!screen[yc+i][xc+j-1] || readRegister(0xf) == 1) {
                        setRegister(0xf, 1);
                     }
                     else {
                        setRegister(0xf, 0);
                     }
                     screen[yc+i][xc+j] = screen[yc+i][xc+j] ^ 0x01;
                  }
                  else {
                     if(screen[yc+i][64-xc+j] == false || readRegister(0xf) == 1) {
                        setRegister(0xf, 1);
                     }
                     else {
                        setRegister(0xf, 0);
                     }
                     screen[yc+i][64-xc+j] = screen[yc+i][64-xc+j] ^ 0x01;
                     
                  }
               }
               else {
                  screen[yc+i][xc+j] = screen[yc+i][xc+j] ^ 0x00;
               }
            } 
         } 
      }

   else if(opcode == 0xe) {
      if(in2 == 0x9e) {
         uint8_t x = LO_NIBBLE(in1);
         if(keyboard[readRegister(x)]) {
            pc+=2;
         }
      }
      else if(in2 == 0xa1) {
         uint8_t x = LO_NIBBLE(in1);
         if(!keyboard[readRegister(x)]) {
            pc+=2;
         }
      }
   }
else if(opcode == 0xf) {
      uint8_t x = LO_NIBBLE(in1);
      if(in2 == 0x07) {
         setRegister(x, delay);
      }
      else if(in2 == 0x0a) {
         bool run = true;
         while(run) {
            for(int i = 0; i < 16; i++) {
               if(keyboard[i]==true) {
                  run = false;
                  setRegister(x, i);
                  break;
               }
            }
         }
      }
      else if(in2 == 0x15) {
         delay = readRegister(x);
      }

      else if(in2 == 0x18) {
      }

      else if(in2 == 0x1e) {
         I = readRegister(x) + I;
      }

      else if(in2 == 0x29) {
         uint8_t x = LO_NIBBLE(in1);
         I = x * 0x05;
         cout << "Character for " << (int)x << " @ " << (int)I << endl;
      }

      else if(in2 == 0x33) {
         uint8_t num = readRegister(x);
         setRegister(I  , num/100 );       // 100 digit 
         setRegister(I+1, (num%100)/10 );  //  10 digit 
         setRegister(I+2, num%10 );        //   1 digit 
      }

      else if(in2 == 0x55) {
         for(int i = 0; i < readRegister(x); i++) {
            setMemory(I+i, readRegister(i));  
         }
      }

      else if(in2 == 0x65) {
         for(int i = 0; i < readRegister(x); i++) {
            setRegister(i, readMemory(I+i));  
         }
      }

   }

   pc+=2;
   cycle++;
}

uint8_t processor::setRegister(int index, uint8_t value) {
   uint8_t oldval = registers[index];
   registers[index] = value;
   return oldval;
}

uint8_t processor::readRegister(int index) {
   return registers[index];
}

void processor::showDebug() {
   cout << endl << " --- Registers ---         ---  Stack  ---         --- General ---" << endl << endl;
   for(int j = 0; j < 4; j++) {
      cout << "    ";
      for(int i = 0; i < 4; i++) {
         cout << hex << setfill('0') << setw(2) << (int)registers[(j*4)+i] << " ";
      }
      cout << "         ";
      for(int i = 0; i < 4; i++) {
         cout << hex << setfill('0') << setw(4) << (int)stack[(j*4)+i] << " ";
      }
      if(j == 0)
         cout << "      " << "I = " << I << ", pc: " << pc;
      if(j == 1)
         cout << "      " << "sp = " << hex << (int)sp << dec;
      if(j == 2)
         cout << "      " << "cycle = " << dec << cycle;
      cout << endl;
   }
   cout << endl << endl;
}

void processor::showMemory(uint8_t location) {
   cout << " ---  Memory locations " << location*256 << " - " << (location+1)*256 << "  --- " << endl << endl;
   for(int i = 0; i < 8; i++) {
      for(int j = 0; j < 32; j++) {
         cout.width(2); cout << hex << setfill('0') << (int)memory[(location*256)+((i*32)+j)] << " ";
         // cout.width(2); cout << hex << 0 << " ";
      }
      cout << dec << endl;
   }
   cout << endl << endl;
}

void processor::showScreen() {
   cout << "   +----------------------------------------------------------------+" << endl;
   for(int j = 0; j < 32; j++) {
      cout.width(2); cout << setfill(' ') << j << " |";
      for(int i = 0; i < 64; i++) {
         if(screen[j][i])
            cout << "#";
         else
            cout << " ";
      }
      cout << "|";
      cout << endl;
   }
   cout << "   +----------------------------------------------------------------+" << endl << endl;
}

uint8_t processor::readMemory(int index) {
   return memory[index];
}

uint8_t processor::setMemory(int index, uint8_t value) {
   int old = memory[index];
   memory[index] = value;
   if(debugState == true) 
      cout << endl << "value @ index ["<< dec << (int)index <<"]: " << hex << (int)old << " -> " << (int)value << dec << endl;
   return old;
}



void processor::openProgram() {
   char buffer[246];

   ifstream myFile;
   myFile.open("Pong.ch8", ios::in | ios::binary);

   if(!myFile.is_open()) {
      cerr << "Error opening file." << endl;
      return;
   }
   
   myFile.read(buffer, 246);

   myFile.close();

   for(int i = 0; i < 246; i++) {
      memory[i+0x200] = buffer[i];
   }
}

void processor::loadFontset() {
   for(int i = 0; i < 80; i++) {
      memory[i] = fontset[i];
   }
}

void processor::clearMemory() {
   for(int i = 0; i < 4096; i++) {
      memory[i] = 0; 
   }
}

void processor::clearScreen() {
   for(int i = 0; i < 2048; i++) {
      screen[i/64][i%64] = 0;
   }
}

void processor::clearStack() {
   for(int i = 0; i < 16; i++) {
      stack[i] = 0;
   }
}

void processor::clearKeyboard() {
   for(int i = 0; i < 16; i++) {
      keyboard[i] = 0;
   }
}


int main() {

   cout 
      << " +-- CHIP-8 emulator --+ " << endl
      << " | made by memer       | " << endl
      << " +-- very cool imho  --+ " << endl << endl << endl;
   ;
   
   // init processor with debug on
   processor* cpu = new processor();
   cpu->debugStatus(false);
   
   //cpu->setMemory(500, 66);
   cpu->openProgram();
   
   for(int i = 0; i < 0; i++) {
      cpu->tick();
   }

   for(int i = 0; i < 0; i++) {
      cpu->showMemory(i);
   }
   
   while(true) {
      cpu->showMemory(2);
      cpu->showDebug();
      cpu->debugStatus(true);
      cpu->tick();
      cpu->showScreen();
      cin.get();
   }
   
   return 0;
}
