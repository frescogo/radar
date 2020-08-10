//#define XXX

typedef struct {
    char dir;
    char val[4];
} Speed;

typedef struct {
    //char  type;
    Speed peak;
    Speed live;
    char  size[3];
    char  ratio[3];
    char  status;
    char  cr;
} Radar_S;

void setup (void) {
    Serial.begin(9600);
    Serial1.begin(9600);
    Serial.println("=== RADAR ===");
}

int speed (Speed* s) {
    return
        (s->val[0] - '0') * 1000 +
        (s->val[1] - '0') *  100 +
        (s->val[2] - '0') *   10 +
        (s->val[3] - '0');
}

bool check_num (char c) {
    return (c>='0' && c<='9');
}

bool check (Radar_S* s) {
    return
        (s->peak.dir=='A' || s->peak.dir=='C') &&
        check_num(s->peak.val[0]) &&
        check_num(s->peak.val[1]) &&
        check_num(s->peak.val[2]) &&
        check_num(s->peak.val[3]) &&
        (s->live.dir=='A' || s->live.dir=='C') &&
        check_num(s->live.val[0]) &&
        check_num(s->live.val[1]) &&
        check_num(s->live.val[2]) &&
        check_num(s->live.val[3]) &&
        check_num(s->size[0]) &&
        check_num(s->size[1]) &&
        check_num(s->size[2]) &&
        check_num(s->ratio[0]) &&
        check_num(s->ratio[1]) &&
        check_num(s->ratio[2]) &&
        s->status == 0x40 &&
        s->cr == '\r';
}

void loop (void) {
#ifdef XXX
    while (1) {
        int n = Serial1.available();
        if (n >= sizeof(Radar_S)+1) {
            break;
        }
    }
    int c = Serial1.read();
    Serial.println(c);
#else
    while (1) {
        int n = Serial1.read();
        if (n == 0x83) {
            break;
        }
    }
    while (1) {
        int n = Serial1.available();
        if (n >= sizeof(Radar_S)) {
            break;
        }
    }
#endif

    Radar_S s;
    Serial1.readBytes((char*)&s, sizeof(Radar_S));
#ifdef XXX
    Serial.println((char*)&s);
#else

    static bool zold = false;
    int peak = speed(&s.peak);
    int live = speed(&s.live);

    bool znow = (peak==0 && live==0);

    if (zold && znow) {
        // was zero and is still zero
    } else if (!check(&s)) {
        // packet error
    } else if (znow) {
        Serial.println("========");
    } else {
        Serial.print(s.peak.dir);
        Serial.print("=");
        Serial.print(s.peak.val[1]); Serial.print(s.peak.val[2]); Serial.print('.'); Serial.print(s.peak.val[3]);
        Serial.print(" / ");
        Serial.print(s.live.dir);
        Serial.print("=");
        Serial.print(s.live.val[1]); Serial.print(s.live.val[2]); Serial.print('.'); Serial.print(s.live.val[3]);
        Serial.print(" / siz=");
        Serial.print(s.size[0]); Serial.print(s.size[1]); Serial.print(s.size[2]);
        Serial.print(" / rat=");
        Serial.print(s.ratio[0]); Serial.print(s.ratio[1]); Serial.print(s.ratio[2]);
        Serial.println();
    }
    zold = znow;
#endif
}
