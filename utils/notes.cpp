#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pi.h"
#include "notes.h"

static note_t happy_birthday[] = {
    { Notes::C4, 1 },
    { Notes::C4, 1 },
    { Notes::D4, 2 },
    { Notes::C4, 2 },
    { Notes::F4, 2 },
    { Notes::E4, 4 },
    { Notes::C4, 1 },
    { Notes::C4, 1 },
    { Notes::D4, 2 },
    { Notes::C4, 2 },
    { Notes::G4, 2 },
    { Notes::F4, 4 },
    { Notes::C4, 1 },
    { Notes::C4, 1 },
    { Notes::C5, 2 },
    { Notes::A4, 2 },
    { Notes::F4, 2 },
    { Notes::E4, 2 },
    { Notes::D4, 2 },
    { Notes::AS4, 1 },
    { Notes::AS4, 1 },
    { Notes::A4, 2 },
    { Notes::F4, 2 },
    { Notes::G4, 2 },
    { Notes::F4, 4 },
};

static note_t harry_potter[] = {
    // Hedwig's theme fromn the Harry Potter Movies
    // Socre from https://musescore.com/user/3811306/scores/4906610

  { Notes::REST, 1}, { Notes::D4, 1},{ Notes::G4, 2}, { Notes::AS4, 1}, { Notes::A4, 1},
  { Notes::G4, 2}, { Notes::D5, 1},{ Notes::C5, 4}, { Notes::A4, 4},
  { Notes::G4, 2}, { Notes::AS4, 1}, { Notes::A4, 1},
  { Notes::F4, 2}, { Notes::GS4, 1},
  { Notes::D4, 8}, { Notes::D4, 1},

  { Notes::G4, 2}, { Notes::AS4, 1}, { Notes::A4, 1},
  { Notes::G4, 2}, { Notes::D5, 1},
  { Notes::F5, 2}, { Notes::E5, 1},
  { Notes::DS5, 2}, { Notes::B4, 1},
  { Notes::DS5, 2}, { Notes::D5, 1}, { Notes::CS5, 1},
  { Notes::CS4, 2}, { Notes::B4, 1},
  { Notes::G4, 8},
  { Notes::AS4, 1},

  { Notes::D5, 2}, { Notes::AS4, 1},
  { Notes::D5, 2}, { Notes::AS4, 1},
  { Notes::DS5, 2}, { Notes::D5, 1},
  { Notes::CS5, 2}, { Notes::A4, 1},
  { Notes::AS4, 2}, { Notes::D5, 1}, { Notes::CS5, 1},
  { Notes::CS4, 2}, { Notes::D4, 1},
  { Notes::D5, 8},
  {Notes::REST,4}, { Notes::AS4,4},

  { Notes::D5, 2}, { Notes::AS4, 1},
  { Notes::D5, 2}, { Notes::AS4, 1},
  { Notes::F5, 2}, { Notes::E5, 1},
  { Notes::DS5, 2}, { Notes::B4, 1},
  { Notes::DS5, 2}, { Notes::D5, 1}, { Notes::CS5, 1},
  { Notes::CS4, 2}, { Notes::AS4, 1},
  { Notes::G4, 8}};

static char buf[128];

int
main()
{
    pi_init();

    Notes *notes = new Notes(21);

    while (pi_readline(buf, sizeof(buf)) != NULL) {
	uint pin, freq, duty;
	if (strncmp(buf, "bd", 2) == 0) {
	    int scaling = atoi(&buf[2]);
	    if (scaling == 0) scaling = 225;
	    notes->play(happy_birthday, sizeof(happy_birthday) / sizeof(happy_birthday[0]), scaling);
	} else if (strncmp(buf, "hp", 2) == 0) {
	    int scaling = atoi(&buf[2]);
	    if (scaling == 0) scaling = 250;
	    notes->play(harry_potter, sizeof(harry_potter) / sizeof(harry_potter[0]), scaling);
        } else if (strcmp(buf, "bootsel") == 0) {
            pi_reboot_bootloader();
	} else if (buf[0] == '?') {
	    printf("bd [ scaling_ms ] - play happy birthday\n");
	    printf("hp [ scaling_ms ] - play harry potter\n");
	    printf("bootsel\n");
	} else if (buf[0] && buf[0] != '\n') {
	    printf("invalid command\n");
	}
    }
}
