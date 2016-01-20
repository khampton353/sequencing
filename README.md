# sequencing
Code from a simple c class assignment using queues

Program requirements included generation of an output file as well as console display, and demonstrating use of queue data structures and operations. 

The program only uses cards from one suit, reading in a pattern of 13 card face values from a file and attempting to match a randomly generated 'deck' to the pattern. Because 'shuffling' ~randomizes the deck, the same pattern can be used with different results, so the inpuyt files include some malformed patterns and a lot of duplicates. The program processes the lines in the file until the pattern can be matched (a 'win') or the file is exhausted. 

The code was implemented and tested on Visual Studio at the request of the Instructor. It currently runs on both Linux and Windows, but has a bug displaying some error messages when run on Fedora in a Virtual Box

#Todo
- [ ] Determine scope of Linux display bug(s) (test on a native Linux installation)
- [ ] Fix bug(s)

  
