#ifndef __NOTES_H__
#define __NOTES_H__

typedef struct {
   unsigned frequency;
   unsigned ms;
} note_t;

class Notes {
public:
    Notes(unsigned gpio) : gpio(gpio) { }

    void play(note_t *notes, int n_notes = 1, int time_scaling = 1);

    static const unsigned B0  = 31;
    static const unsigned C1  = 33;
    static const unsigned CS1 = 35;
    static const unsigned D1  = 37;
    static const unsigned DS1 = 39;
    static const unsigned E1  = 41;
    static const unsigned F1  = 44;
    static const unsigned FS1 = 46;
    static const unsigned G1  = 49;
    static const unsigned GS1 = 52;
    static const unsigned A1  = 55;
    static const unsigned AS1 = 58;
    static const unsigned B1  = 62;
    static const unsigned C2  = 65;
    static const unsigned CS2 = 69;
    static const unsigned D2  = 73;
    static const unsigned DS2 = 78;
    static const unsigned E2  = 82;
    static const unsigned F2  = 87;
    static const unsigned FS2 = 93;
    static const unsigned G2  = 98;
    static const unsigned GS2 = 104;
    static const unsigned A2  = 110;
    static const unsigned AS2 = 117;
    static const unsigned B2  = 123;
    static const unsigned C3  = 131;
    static const unsigned CS3 = 139;
    static const unsigned D3  = 147;
    static const unsigned DS3 = 156;
    static const unsigned E3  = 165;
    static const unsigned F3  = 175;
    static const unsigned FS3 = 185;
    static const unsigned G3  = 196;
    static const unsigned GS3 = 208;
    static const unsigned A3  = 220;
    static const unsigned AS3 = 233;
    static const unsigned B3  = 247;
    static const unsigned C4  = 262;
    static const unsigned CS4 = 277;
    static const unsigned D4  = 294;
    static const unsigned DS4 = 311;
    static const unsigned E4  = 330;
    static const unsigned F4  = 349;
    static const unsigned FS4 = 370;
    static const unsigned G4  = 392;
    static const unsigned GS4 = 415;
    static const unsigned A4  = 440;
    static const unsigned AS4 = 466;
    static const unsigned B4  = 494;
    static const unsigned C5  = 523;
    static const unsigned CS5 = 554;
    static const unsigned D5  = 587;
    static const unsigned DS5 = 622;
    static const unsigned E5  = 659;
    static const unsigned F5  = 698;
    static const unsigned FS5 = 740;
    static const unsigned G5  = 784;
    static const unsigned GS5 = 831;
    static const unsigned A5  = 880;
    static const unsigned AS5 = 932;
    static const unsigned B5  = 988;
    static const unsigned C6  = 1047;
    static const unsigned CS6 = 1109;
    static const unsigned D6  = 1175;
    static const unsigned DS6 = 1245;
    static const unsigned E6  = 1319;
    static const unsigned F6  = 1397;
    static const unsigned FS6 = 1480;
    static const unsigned G6  = 1568;
    static const unsigned GS6 = 1661;
    static const unsigned A6  = 1760;
    static const unsigned AS6 = 1865;
    static const unsigned B6  = 1976;
    static const unsigned C7  = 2093;
    static const unsigned CS7 = 2217;
    static const unsigned D7  = 2349;
    static const unsigned DS7 = 2489;
    static const unsigned E7  = 2637;
    static const unsigned F7  = 2794;
    static const unsigned FS7 = 2960;
    static const unsigned G7  = 3136;
    static const unsigned GS7 = 3322;
    static const unsigned A7  = 3520;
    static const unsigned AS7 = 3729;
    static const unsigned B7  = 3951;
    static const unsigned C8  = 4186;
    static const unsigned CS8 = 4435;
    static const unsigned D8  = 4699;
    static const unsigned DS8 = 4978;
    static const unsigned REST = 0;

private:
    unsigned gpio;
};

#endif
