/**********************************************************/
/*     University of California Extension, Santa Cruz     */
/*           Advanced C Programming                       */
/*    Instructor: Rajainder A. Yeldandi                   */
/*    Assignment Number:  16                              */
/*    Topic: Queues and sequencing                        */
/*    file name: a16_sequencing.c                         */
/*    Objective:    Implement a card game using queues    */
/*                  to represent cards that are removed   */
/*                  from the top of a deck and put on the */
/*                  bottom while trying to match a       */
/*                  specific sequence of cards.           */
/*                                                        */
/*    Comments:     The program is designed around the 13 */
/*                  card suit. No attempt is made to      */
/*                  handle a full size deck with >1 suits */
/*                  The game is very difficult to 'win',  */
/*                  since when the decksize is even, if   */
/*                  the next card to match is odd it will */
/*                  never be seen.                        */
/*                                                        */
/**********************************************************/

/*             preprocessor directives       */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#ifdef linux
#include <errno.h>
#endif
#define WRITEFILE
#ifdef WRITEFILE
#define OUTFILE "output16.txt"
#endif

#define QSZ 16 //queue size, must be power of two
#define DSZ 13 //deck size
#define ENQ 1
#define DEQ 0
/*                  Global Definitions                    */

#ifdef WRITEFILE
FILE *oFile=0; //file stream to allow dual printf output
#endif

typedef struct Q{
	int head;
	int tail;
	int q[QSZ];
} Q;


//Index 0 of FACE is a dummy, card values are 1-13
char *FACE[14]={" ","A","2","3","4","5","6","7","8","9","10","J","Q","K"};
enum queues{PATTERN,DECK,OPENED};
char * QNAMES[3]={"Pattern","Deck","Opened"};

/*                  Function Prototypes                   */

static int doPrint(char *,...);
static void doExit(int);

static int buildPattern(char *, int);
static void playGame(void); 
static void getShuffled(void);
static int whatCard(char *);
static void doQ(int *,int,int);
void showQueued(int head,int tail, int *q);

/*Exit program, hiding the call to getchar that is needed in Visual studio*/
static void doExit(int val){
#ifdef WRITEFILE
	if (oFile)
		fclose(oFile);
#endif
#ifndef linux
	doPrint("Enter any character to leave the program\n");
	getchar();
#endif
	exit(val);
}

/* Print and flush output string to stdout as a single operation. Also output
 * to a file if enabled and not null
 */
static int doPrint(char * str,...) {
	int i;
	va_list ap;
	va_start(ap,str);
	i=vprintf(str,ap); //save i for return to mimic what printf returns
	va_end(ap);
	fflush(stdin);
#ifdef WRITEFILE
	if (oFile) {
		va_start(ap,str);
		i=vfprintf(oFile,str,ap); //save i for return to mimic what printf returns
		va_end(ap);
	}
#endif
	return i;
}


/*Convert the card  from character to integer/index format*/
static int whatCard(char *buf) {
	int k;

	switch (*buf){
		case 'a':
		case 'A': 
			k=1; 
			break;
		case 'j':
		case 'J':
			k=11;
			break;
		case 'q':
		case 'Q':
			k=12;
			break;
		case 'k':
		case 'K':
			k=13;
			break;
		case '1': 
			if(*(buf+1) == '0') //must be followed by '0'
				k=10;
			else
				return 0;
			break;
		default:
			if ((*buf >= '2') && (*buf<='9')) 
				k=*buf-'0';
			else 
				return 0;
			break;
	}
	return k;
}


/*display the current contents of the specified queue*/
void showQueued(int head,int tail, int *q){
	while (head - tail) {
		doPrint("%-3s",FACE[q[head]]);
		head=(head+1)&(QSZ-1);
	}
	doPrint("\n");
}




/* Manages operations for 3 queues: pattern, deck, and opened. Operations are
 * enqueue/dequeue, controlled by the flag, and display/reset, indicated by
 * a null valp/q. The 'q' field is an enum that selects the queue. The queues
 * are circular with 1 empty space between tail and head to simplify 
 * differentiation between empty and full with no special case code. Queues are
 * asserted to be power of 2 in size.
 */
static void doQ(int *valp, int q, int EnQ) {
	static Q qs[3]; //Qs:pattern, deck, opened cards
	int i;
	if (!valp){
		if (q==-1){ //'empty' queues for a new game
			for (i=0;i<3;i++){
				(qs+i)->head=0;
				(qs+i)->tail=0;
			}
			return;
		}
		showQueued((qs+q)->head,(qs+q)->tail,(qs+q)->q);
		return;
	}

	if (EnQ){
		if (((((qs+q)->tail)+1)&(QSZ-1)) != (qs+q)->head) {
			(qs+q)->q[(qs+q)->tail] = *valp;
			(qs+q)->tail = ((qs+q)->tail+1)&(QSZ-1);
		} else {
			doPrint("Enqueue of %d failed, queue full: \n",*valp);
			showQueued((qs+q)->head,(qs+q)->tail,(qs+q)->q);
			doExit(2);
		}
	} else { //dequeue
		if ((qs+q)->head-(qs+q)->tail) /*not head==tail*/{
			*valp=(qs+q)->q[(qs+q)->head];
		(qs+q)->head=((qs+q)->head+1)&(QSZ-1);
		} else {
			doPrint("Dequeue failed on empty queue, exiting\n");
			doExit(3);
		}
	}
}


/*generate and enqueue a shuffled deck by randomizing swaps at deck indices. */
static void getShuffled(void){
	int i,j,tmp;
	int deck[DSZ];

	for (i=0;i<13;i++)
		deck[i]=i+1;
	for (i=0;i<13;i++){
		j=rand()%13;
		tmp=deck[i];
		deck[i]=deck[j];
		deck[j]=tmp;
	}
	for (i=0;i<13;i++)
		doQ(&deck[i],DECK,1); //put shuffled deck in its queue 

	doPrint("Initial Deck is: ");
	doQ(0,DECK,ENQ);
	return;
}

/* Play the game. Get a shuffled deck. Move the first card to the 'bottom' 
 * (DeQ/EnQ), then see if the next card (DeQ) matches the first card of  
 * the pattern. If it does remove both from the deck. Otherwise put the card 
 * back in the deck (EnQ).Repeat this pattern until all cards in the pattern
 * are matched, or it is clear that the game can't be won.
 */
static void playGame(){
	int d=DSZ, o=0, p=DSZ, val,match,sentinal=0;

	getShuffled(); //adds a 'shuffled' deck to the pattern q
	for (;p;p--){
		doQ(&match,PATTERN,DEQ);
		doPrint("Match is '%s', remaining pattern is: ",FACE[match]); 
		doQ(0,PATTERN,0);
		while (d){
			doQ(&val,DECK,DEQ); //remove the first card...
			--d;
			if(!d){ //last card. assert==match
				doQ(&val,OPENED,ENQ);
				break; //assert !p 
			}
			doQ(&val,DECK,ENQ); //put back in deck unseen...
			++d;
			doQ(&val,DECK,DEQ); //look at next card
			--d;
			if (val==match){
				doQ(&val,OPENED,ENQ);
				++o;
				doPrint("Matched '%s'! Matched cards are: ",
					FACE[match]);
				doQ(0,OPENED,0);
				doPrint("Remaining deck is: "); 
				doQ(0,DECK,0);
				sentinal=0;
				break;
			} else {
				doQ(&val,DECK,ENQ); //put back in deck, doesn't match pattern
				d++;
				if (!sentinal){ //if seen more than once you are stuck in a loop
					sentinal=val;
				} else if (val==sentinal){
					doPrint("Can't win, pattern and deck are inverted\n"
						"Pattern: ");
					doQ(0,PATTERN,0);
					doPrint("   Deck: ");
					doQ(0,DECK,0);
					doPrint("   Opened: ");
					doQ(0,OPENED,0);
					return;
				}
			}
		}//while looking for a match
	} //for loop: while pattern queue not empty
	doPrint("Game over, you won!\nOpened: ");
	doQ(0,OPENED,0);
	doPrint("   Pattern: ");
	doQ(0,PATTERN,0);
	doPrint("\n   Deck: ");
	doQ(0,DECK,0);
	doExit(0);
}





static int buildPattern(char *buf, int sz){
	int arr[DSZ];
	char c;
	int i=0,j=0,k;

	for (i=0;i<DSZ;i++) //use to detect duplicates in pattern
		arr[i]=0;
	i=0;
	while ((c=buf[i]) && (j<DSZ)){
		if(c==' '){
			i++;
			continue;
		}
		k=whatCard(&buf[i]);
		if (!k){
			doPrint("Bad character %c read at %d, ignoring pattern %s\n",
				c,i,buf);
			return 0;
		}

		if (arr[k-1] != 0){
			doPrint("Duplicate character %c read at %d, ignoring pattern %s\n",
					c,i,buf);
			return 0;
		}
		arr[k-1]=j++;
		doQ(&k,PATTERN,1);  //put on the pattern queue
		i++;
		if(k==10)
			i++; //2 chars
	}
	if (j==(DSZ)) {
		doPrint("Initial Pattern is: ");
		doQ(0,PATTERN,ENQ);
		return 1;
	}
	else{
		doPrint("Only %d cards, ignoring pattern %s\n",j,buf);
		return 0;
	}
}


/* main() program body */
int main(){
	FILE *infile;
	char buf[64];

	if (QSZ&(QSZ-1)){
		doPrint("Programming error, queue size must be power of two\n");
		doExit(9);
	}

#ifdef WRITEFILE
	//Open file to keep track of output. If enabled, doPrint will write to the
	//specified file as well as to stdout. Otherwise the global oFile == 0
	oFile = fopen(OUTFILE,"w");
	if (!oFile)
		doPrint("Can't write output file: %s\n",strerror(errno));
#endif

/* Read patterns from an input file and play the sequencing 'game' */
	if (!(infile=fopen("a16input_neg.txt","r"))) {
		doPrint("Unable to open input file, exiting\n");
		doExit(2);
	}

	while (fgets(buf, sizeof(buf),infile)) {
                doPrint("BUF:%s",buf);
		if (buf[strlen(buf)-1] == '\n')
		    buf[strlen(buf)-1] = ' '; 
		srand((unsigned int)time(0));
		if(buildPattern(buf, strlen(buf)+1))
			playGame(); //makes deck then 'plays'
		doQ(0,-1,0);
	}
	doExit(0);
}

