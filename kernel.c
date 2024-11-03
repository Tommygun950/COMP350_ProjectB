void printString(char*);
void printChar(char*);
char* readString(char*);
char* readSector(char*, int);
void handleInterrupt21(int ax, char* bx, int cx, int dx);

//main function
void main(){
/* printString test script	
	printString("printString works!!!");
*/

/* readString test script.
	char line[80];
	printString("Enter a line: ");
	readString(line);
	printString(line);
*/

/* readSector test script.	
	char buffer[512];
	readSector(buffer, 30);
	printString(buffer);
*/

/* handInterrupt21 test script.
	makeInterrupt21();
	interrupt(0x21,0,0,0,0);
*/
	char line[80];
	makeInterrupt21();
	interrupt(0x21,1,line,0,0);
	interrupt(0x21,0,line,0,0);

	while(1);
}

//Prints string to terminal.
void printString(char* chars){
	int i=0;
	while (chars[i] != 0x0){ //0x0 signifies the last character in the string.
		char al = chars[i]; //AL register is the place holder for each character.
		char ah = 0xe; //AH register is always 0xe.
		int ax = ah*256 + al; //AX is always equal to AH*256+AL.
		interrupt(0x10,ax,0,0,0);//0,0,0 are registers BX,CX,DX but are not used.
		++i;
	}
}

//Prints a single character to the terminal.
void printChar(char* c){	
	char al = c;
	char ah = 0xe;
	int ax = ah*256 + al;
	interrupt(0x10,ax,0,0,0);
}

//Takes input fro mthe screen and sends it back out (like echo command).
char* readString(char* array_input)
{
	char keyboard_input = 0x0;

	int i = 0;
	while (i < 80) { //The character array is limited to 80 elements. 
		//0x16 is how BIOS reads from interrupt.
		keyboard_input = (char) interrupt(0x16,0,0,0,0); 

		//0xd is enter and notifies the system that's the end of the string.
		if (keyboard_input == 0xd)
			break;
		//Makes sure if backspace is pressed, it won't store it.
		if (keyboard_input == 0x8 && i == 0)
			continue;
		//0x8 is the backspace key, and will remove the last element.
		if (keyboard_input == 0x8){
			char space = ' ';
			interrupt(0x10, 0xe * 256 + keyboard_input,0,0,0);
			interrupt(0x10, 0xe * 256 + space,0,0,0);
			interrupt(0x10, 0xe * 256 + keyboard_input,0,0,0);
			//the 2nd backspace removes it from the screen.
			--i;
			continue;
		}

		array_input[i] = keyboard_input;
		interrupt(0x10, 0xe * 256 + keyboard_input,0,0,0);
		++i;
	}
	array_input[i] = 0xa; //0xa is for a line feed.
	array_input[i + 1] = 0x0; //0x0 signifies the end of the string.

	interrupt(0x10, 0xe * 256 + 0xd,0,0,0); //Outputs each entire element from the array.
	interrupt(0x10, 0xe * 256 + 0xa,0,0,0); //Outputs line feed character.

	return array_input;
}

//Reads from a file and outputs it to the screen.
char* readSector(char* buffer, int sector){
	int ah = 2; //Tells BIOS to read a sector rather than write.
	int al = 1; //Leave as 1, its the number of sectors to read.
	int ax = ah * 256 + al;

	char* bx = buffer; //address where the data should be stored to.
	
	int ch = 0; //track number.
	int cl = sector + 1; //relative sector number.
	int cx = ch * 256 + cl; 

	int dh = 0; //head number.
	int dx = dh * 256 + 0x80;

	interrupt(0x13, ax, bx, cx, dx);
	
	return buffer;
}

//Our own defined interrupt that can be called within kernel.c.
void handleInterrupt21(int ax, char* bx, int cx, int dx){
	switch(ax){ //AX is the # that determines which function to run.
		case 0: printString(bx); break; //if ax is 0, it'll printString.
		case 1: readString(bx); break; //if ax is 1, it'll readString.
		case 2: readSector(bx, cx); break; //if ax is 2, it'll read sector.
		
		default: printString("ERROR: AX is invalid."); break; //if ax isn't anything above, it'll print and error.
	}
}
