// TODO:
// - ritmo nao faz sentido sem distancia
// - fazer tempo em tempo real?
// - golpe 0 a cada 5s?

#define MAJOR    3
#define MINOR    0
#define REVISION 2

//#define DEBUG

#ifdef DEBUG
#define assert(x) \
    if (!x) {                               \
        pinMode(LED_BUILTIN,OUTPUT);        \
        while (1) {                         \
            digitalWrite(LED_BUILTIN,HIGH); \
            delay(250);                     \
            digitalWrite(LED_BUILTIN,LOW);  \
            delay(250);                     \
        }                                   \
    }
#else
#define assert(x)
#endif

#include <EEPROM.h>
#include "pitches.h"

typedef char  s8;
typedef short s16;
typedef unsigned long u32;

#if 1
#define PIN_LEFT  4
#define PIN_RIGHT 2
#define PIN_CFG   3
#else
#define PIN_LEFT  3
#define PIN_RIGHT 4
#define PIN_CFG   2
#endif

#define PIN_TONE 11

#define DEF_TIMEOUT     240         // 4 mins
#define REF_TIMEOUT     300         // 5 mins

#define REF_HITS        150
#define REF_REV         (S.reves ?  15 :   0)
#define REF_NRM         (S.reves ? 135 : 150)

#define REF_CONT        160         // 1.6%
#define REF_ABORT       15          // 15s per fall

#define HITS_MAX        650
#define HITS_REF        (HITS_NRM + HITS_REV)
#define HITS_NRM        min(REF_NRM,  max(1, REF_NRM * S.timeout / REF_TIMEOUT / 1000))
#define HITS_REV        min(REF_REV,  max(1, REF_REV * S.timeout / REF_TIMEOUT / 1000))

#define HIT_KMH_MAX     99          // safe value to avoid errors
#define HIT_KMH_50      50

#define STATE_IDLE      0
#define STATE_PLAYING   1
#define STATE_TIMEOUT   2

#define NAME_MAX        15

#define DESC_MAX        65000
#define DESC_FOLGA      5000

#define REVES_MIN       180
#define REVES_MAX       220

#define POT_BONUS       2
#define POT_VEL         50

#define REV_PCT         11/10   // do not use parenthesis (multiply before division)
#define EQU_PCT         110/100 // do not use parenthesis (multiply before division)
//#define CONT_MAX        (REF_CONT * REF_TIMEOUT / REF_ABORT)    // max PCT to loose (40%)
//#define CONT_PCT(f,t)   min(CONT_MAX, f * (((u32)REF_TIMEOUT)*REF_CONT*1000/max(1,t)))
//#define CONT_PCT(f,t)   min(9999, f * (((u32)REF_TIMEOUT)*REF_CONT*1000/max(1,t)))
#define CONT_PCT        (((u32)REF_TIMEOUT*1000) * REF_CONT / S.timeout)
#define ABORT_FALLS     (S.timeout / REF_ABORT / 1000)

static int  STATE;
static char STR[64];

typedef enum {
    MOD_CEL = 0,
    MOD_PC
} TMOD;

TMOD MOD;

typedef struct {    // { 0,kmh } --> saque
    u8 dt;          // 10        --> 10*100ms --> 1s
    s8 kmh;
} Hit;

typedef struct {
    char juiz[NAME_MAX+1];      // = "Juiz"
    char names[2][NAME_MAX+1];  // = { "Atleta ESQ", "Atleta DIR" }
    u32  timeout;               // = 180 * ((u32)1000) ms
    u16  distancia;             // = 700 cm
    s8   equilibrio;            // = sim/nao
    u8   maxima;                // = 85 kmh
    u16  reves;                 // = 180  (tempo minimo de segurar para o back)

    u16  descanso;              // cs (ms*10) (até 650s de descanso)
    u16  hit;
    Hit  hits[HITS_MAX];
} Save;
static Save S;

enum {
    LADO_NRM = 0,
    LADO_REV,
    LADO_NRM_REV
};

typedef struct {
    u16 golpes;
    u8  minima;
    u8  maxima;
    u16 media1;     // 6156 = 61.56 km/h
    u16 pontos;
} Lado;

// TODO: avg2, total/reves/normal->tot/rev/nrm
typedef struct {
    u16  pontos;                    // reves+normal (x100)
    Lado lados[LADO_NRM_REV];
} Jog;

typedef struct {
    // calculated when required
    u32  time;                        // ms (total time)
    u8   saques;
    s8   ritmo;                       // kmh
    u16  ataques;

    u16  pontos;
    Jog  jogs[2];
} Game;
static Game G;

enum {
    IN_NONE,
    IN_GO_FALL,
    IN_TIMEOUT,
    IN_RESTART,
    IN_UNDO,
    IN_RESET,
    IN_RADAR
};

int Falls (void) {
    return G.saques - (STATE==STATE_IDLE ? 0 : 1);
                        // after fall
}

// tom dos golpes            50-      50-60    60-70    70-80    80+
static const int NOTES[] = { NOTE_E3, NOTE_E5, NOTE_G5, NOTE_B5, NOTE_D6 };

#include "pt.c.h"
#include "serial.c.h"
#include "xcel.c.h"
#include "pc.c.h"
#include "radar.c.h"

void Sound (s8 kmh) {
    int ton = NOTES[min(max(0,kmh/10-4), 4)];
    tone(PIN_TONE, ton, 50);
}

int Await_Input (bool serial, bool hold, s8* kmh) {
    static u32 old;
    static int pressed = 0;
    while (1) {
        if (serial) {
            int ret = Serial_Check();
            if (ret != IN_NONE) {
                return ret;
            }
        }

        if (kmh != NULL) {
            Radar_S s;
            int val = radar_read(&s);
            if (val != 0) {
                *kmh = val/10;
                return IN_RADAR;
            }
        }

        u32 now = millis();

        int pin_left  = digitalRead(PIN_LEFT);
        int pin_right = digitalRead(PIN_RIGHT);

        // CFG UNPRESSED
        if (digitalRead(PIN_CFG) == HIGH) {
            pressed = 0;
            old = now;
        }

        // CFG PRESSED
        else
        {
            if (!pressed) {
                tone(PIN_TONE, NOTE_C2, 50);
                pressed = 1;
            }

            // fall
            if        (now-old>= 750 && pin_left==HIGH && pin_right==HIGH) {
                old = now;
                return IN_GO_FALL;
            } else if (now-old>=3000 && pin_left==LOW  && pin_right==HIGH) {
                old = now;
                return IN_UNDO;
            } else if (now-old>=3000 && pin_left==HIGH && pin_right==LOW) {
                old = now;
                return IN_RESTART;
            } else if (now-old>=3000 && pin_left==LOW  && pin_right==LOW) {
                old = now;
                return IN_RESET;
            }
        }

        if (!hold) {
            return IN_NONE;
        }
    }
}

void EEPROM_Load (void) {
    for (int i=0; i<sizeof(Save); i++) {
        ((byte*)&S)[i] = EEPROM[i];
    }
    S.hit = min(S.hit, HITS_MAX);
    S.names[0][NAME_MAX] = '\0';
    S.names[1][NAME_MAX] = '\0';
}

void EEPROM_Save (void) {
    for (int i=0; i<sizeof(Save); i++) {
        EEPROM[i] = ((byte*)&S)[i];
    }
}

void EEPROM_Default (void) {
    strcpy(S.juiz,     "?");
    strcpy(S.names[0], "Atleta ESQ");
    strcpy(S.names[1], "Atleta DIR");
    S.distancia  = 750;
    S.timeout    = DEF_TIMEOUT * ((u32)1000);
    S.equilibrio = 1;
    S.maxima     = 85;
    S.reves      = 0;
}

void setup (void) {
    Serial.begin(9600);
    radar_setup();

    pinMode(PIN_CFG,   INPUT_PULLUP);
    pinMode(PIN_LEFT,  INPUT_PULLUP);
    pinMode(PIN_RIGHT, INPUT_PULLUP);

    delay(2000);
    if (Serial.available()) {
        MOD = (TMOD) Serial.read();
    } else {
        MOD = MOD_CEL;
    }

    EEPROM_Load();
}

u32 alarm (void) {
    u32 left = S.timeout - G.time;
    if (left < 5000) {
        return S.timeout - 0;
    } else if (left < 10000) {
        return S.timeout - 5000;
    } else if (left < 30000) {
        return S.timeout - 10000;
    } else if (left < 60000) {
        return S.timeout - 30000;
    } else {
        return (G.time/60000 + 1) * 60000;
    }
}

#define XMOD(xxx,yyy)   \
    switch (MOD) {      \
        case MOD_CEL:   \
            xxx;        \
            break;      \
        case MOD_PC:    \
            yyy;        \
            break;      \
    }

void Desc (u32 now, u32* desc0, bool desconto) {
    u32 diff = now - *desc0;
    *desc0 = now;

    if (!desconto || diff>DESC_FOLGA) {     // 5s de folga
        u32 temp = S.descanso + diff/10;
        S.descanso = min(DESC_MAX, temp);   // limita a 65000
        XMOD(CEL_Nop(), PC_Desc());
    }
}

void loop (void)
{
// RESTART
    STATE = STATE_IDLE;
    PT_All();

    XMOD(CEL_Restart(), PC_Restart());

    while (1)
    {
// GO
        PT_All();

        if (G.time >= S.timeout) {
            goto _TIMEOUT;          // if reset on ended game
        }
        if (Falls() >= ABORT_FALLS) {
            goto _TIMEOUT;
        }

        while (1) {
            int got = Await_Input(true,true,NULL);
            switch (got) {
                case IN_RESET:
                    EEPROM_Default();
                    goto _RESTART;
                case IN_RESTART:
                    goto _RESTART;
                case IN_UNDO:
                    if (S.hit > 0) {
_UNDO:
                        while (1) {
                            S.hit -= 1;
                            if (S.hit == 0) {
                                break;
                            } else if (S.hits[S.hit].dt == 0) {
                                S.hit -= 1;
                                break;
                            }
                        }
                        tone(PIN_TONE, NOTE_C2, 100);
                        delay(110);
                        tone(PIN_TONE, NOTE_C3, 100);
                        delay(110);
                        tone(PIN_TONE, NOTE_C4, 300);
                        delay(310);
                        EEPROM_Save();
                        PT_All();
                        XMOD(Serial_Score(), PC_Atualiza());
                    }
                case IN_GO_FALL:
                    goto _BREAK1;
            }
        }
_BREAK1:

        u32 desc0 = millis();               // comeca a contar o descanso
        XMOD(Serial_Score(), PC_Nop());

_SERVICE:
        tone(PIN_TONE, NOTE_C7, 500);

// SERVICE

        s8 kmh_ = 0;
        u8 kmh  = 0;
        int is_out;

        while (1) {
            int got = Await_Input(true,false,&kmh_);
            switch (got) {
                case IN_RESET:
                    EEPROM_Default();
                    goto _RESTART;
                case IN_RESTART:
                    goto _RESTART;
                case IN_GO_FALL:
                    Desc(millis(), &desc0, false);
                    goto _SERVICE;
                case IN_RADAR:
                    kmh = abs(kmh_);
                    is_out = (kmh_ > 0);
                    goto _BREAK2;

                case IN_NONE:
                    u32 now = millis();
                    if (now - desc0 >= 10000) {
                        Desc(now, &desc0, true);
                        tone(PIN_TONE, NOTE_C7, 500);
                    }
                    break;
            }
        }
_BREAK2:

        u32 t0 = millis();
        Desc(t0, &desc0, true);
        S.hits[S.hit++] = { 0, (s8)kmh_ };
        Sound(kmh);
        XMOD(CEL_Service(is_out), PC_Nop());
        XMOD(CEL_Hit(is_out,false,kmh), PC_Hit(is_out,false,kmh));
        STATE = STATE_PLAYING;
        PT_All();

        while (1)
        {
            while (1) {
                int got = Await_Input(true,true,&kmh_);
                if (got == IN_RESET) {
                    EEPROM_Default();
                    goto _RESTART;
                } else if (got == IN_RESTART) {
                    goto _RESTART;
                } else if (got == IN_TIMEOUT) {
                    goto _TIMEOUT;
                } else if (got == IN_GO_FALL) {
                    goto _FALL;
                } else if (got == IN_RADAR) {
                    kmh = abs(kmh_);
                    is_out = (kmh_ > 0);
                    break;
                }
            }

            u32 t1 = millis();
            u32 dt = (t1 - t0);
            t0 = t1;

            dt = min(dt/100, 255);              // maximo DT=25.500ms
            S.hits[S.hit++] = { (u8)dt, (s8)kmh_ };

            u8 al_now = 0;
            if (G.time+dt*100 > alarm()) {
                tone(PIN_TONE, NOTE_C7, 250);
                al_now = 1;
            } else {
                Sound(kmh);
            }

            PT_All();
            XMOD(CEL_Nop(), PC_Tick());

//Serial.println(dt);
//Serial.println(G.time);

            if (G.time >= S.timeout) {
                goto _TIMEOUT;
            }
            if (S.hit >= HITS_MAX-5) {
                goto _TIMEOUT;
            }

            if (!al_now && S.equilibrio && G.time>=30000 && PT_Behind()==is_out) {
                tone(PIN_TONE, NOTE_C2, 30);
            }
            XMOD(CEL_Hit(is_out,false,kmh), PC_Hit(is_out,false,kmh));
        }
_FALL:
        if (Falls() >= ABORT_FALLS) {
            S.hits[S.hit++] = { 0, 0 }; // emula um saque nulo pra contar essa queda
        }

        STATE = STATE_IDLE;

        tone(PIN_TONE, NOTE_C4, 100);
        delay(110);
        tone(PIN_TONE, NOTE_C3, 100);
        delay(110);
        tone(PIN_TONE, NOTE_C2, 300);
        delay(310);

        PT_All();
        XMOD(CEL_Fall(), PC_Fall());
        XMOD(CEL_Nop(),  PC_Tick());
        EEPROM_Save();
    }

_TIMEOUT:
    STATE = STATE_TIMEOUT;
    tone(PIN_TONE, NOTE_C2, 2000);
    PT_All();
    XMOD(CEL_Nop(), PC_Tick());
    XMOD(CEL_End(), PC_End());
    EEPROM_Save();

    while (1) {
        int got = Await_Input(true,true,NULL);
        if (got == IN_RESET) {
            EEPROM_Default();
            goto _RESTART;
        } else if (got == IN_RESTART) {
            goto _RESTART;
        } else if (got == IN_UNDO) {
            STATE = STATE_IDLE;
            if (Falls() >= ABORT_FALLS) {
                S.hit--;    // reverse above
            }
            goto _UNDO;
        }
    }

_RESTART:
    tone(PIN_TONE, NOTE_C5, 2000);
    S.hit = 0;
    S.descanso = 0;
    EEPROM_Save();
}
