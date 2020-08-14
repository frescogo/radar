#define DT_750 750
#define REP_10  10

typedef struct {
    char peak_dir;
    char peak_val[4];
    char live_dir;
    char live_val[4];
    char size[3];
    char ratio[3];
    char status;
    char cr;
} Radar_S;

int four (char* num) {
    return
        (num[0] - '0') * 1000 +
        (num[1] - '0') *  100 +
        (num[2] - '0') *   10 +
        (num[3] - '0') *    1;
}

int three (char* num) {
    return
        (num[0] - '0') * 100 +
        (num[1] - '0') *  10 +
        (num[2] - '0') *   1;
}

bool check_num (char c) {
    return (c>='0' && c<='9');
}

bool check (Radar_S* s) {
    return
        (s->peak_dir=='A' || s->peak_dir=='C') &&
        check_num(s->peak_val[0]) &&
        check_num(s->peak_val[1]) &&
        check_num(s->peak_val[2]) &&
        check_num(s->peak_val[3]) &&
        (s->live_dir=='A' || s->live_dir=='C') &&
        check_num(s->live_val[0]) &&
        check_num(s->live_val[1]) &&
        check_num(s->live_val[2]) &&
        check_num(s->live_val[3]) &&
        check_num(s->size[0]) &&
        check_num(s->size[1]) &&
        check_num(s->size[2]) &&
        check_num(s->ratio[0]) &&
        check_num(s->ratio[1]) &&
        check_num(s->ratio[2]) &&
        s->status == 0x40 &&
        s->cr == '\r';
}

void show (Radar_S* s) {
    {
        Serial.print(s->peak_dir);
        Serial.print("=");
        Serial.print(s->peak_val[1]);
        Serial.print(s->peak_val[2]);
        Serial.print('.');
        Serial.print(s->peak_val[3]);
    }
    Serial.print(" / ");
    {
        Serial.print(s->live_dir);
        Serial.print("=");
        Serial.print(s->live_val[1]);
        Serial.print(s->live_val[2]);
        Serial.print('.');
        Serial.print(s->live_val[3]);
    }
    Serial.print(" / siz=");
    Serial.print(s->size[0]); Serial.print(s->size[1]); Serial.print(s->size[2]);
    Serial.print(" / rat=");
    Serial.print(s->ratio[0]); Serial.print(s->ratio[1]); Serial.print(s->ratio[2]);
    Serial.println();
}

void radar_setup (void) {
    Serial1.begin(9600);
}

int radar_read (Radar_S* s) {
    static u32  onow = millis();
    static char odir = '\0';

    static int buf_i = 0;
    static int buf[REP_10][2] = { {0,0},{0,0},{0,0},{0,0},{0,0},
                                  {0,0},{0,0},{0,0},{0,0},{0,0} };

    while (1) {
        int n = Serial1.read();
        if (n == 0x83) {
            break;              // espera o primeiro byte do pacote
        }
    }
    while (1) {
        int n = Serial1.available();
        if (n >= sizeof(Radar_S)) {
            break;              // espera ter o tamanho do pacote
        }
    }

    Serial1.readBytes((char*)s, sizeof(Radar_S));

    char dir   = s->peak_dir;
    int  vel   = four(s->peak_val);
    int  size  = three(s->size);
    int  ratio = three(s->ratio);

    // ignora golpes consecutivos na mesma direcao em menos de 500ms
    u32 now = millis();
    u32 dt = now - onow;
    if (dir==odir && dt<DT_750) {
        return 0;
    }

    if (
        (!check(s))                     ||  // erro no pacote
        (vel == 0)                      ||  // bola nao detectada
// TODO: remover esse teste de tamanho
        (size<3 || size>5)              ||  // tamanho incompativel
        //(s->peak_dir != s->live_dir)    ||  // direcoes incompativeis
        //(vel>0 && ratio<20)             ||  // ratio muito baixo
        false
    ) {
        return 0;
    }

    buf[buf_i][0] = vel;
    buf[buf_i][1] = dir;
    buf_i = (buf_i + 1) % REP_10;
    for (int i=1; i<REP_10; i++) {
        if (buf[i][0]!=buf[0][0] || buf[i][1]!=buf[0][1]) {
            return 0;               // aceita somente 4 picos iguais
        }
    }
    for (int i=0; i<REP_10; i++) {
        buf[i][0] = buf[i][1] = 0;
    }

    onow = now;
    odir = dir;
    return (dir == 'A') ? vel : -vel;
}
