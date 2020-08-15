void PT_Bests (int n, u8* bests, Jog* jog) {
    u32 sum = 0;
    for (int i=0; i<n; i++) {
        u8 v = bests[i];
        if (v == 0) {
            break;
        }
        sum += v;
    }
    int golpes = min(n, jog->golpes);
    jog->minima = bests[n-1];
    jog->maxima = bests[0];
    jog->media1 = (golpes == 0) ? 0 : sum*100/golpes;
    jog->pontos = sum;
}

int PT_Equ (u16* avg, u16* min_) {
    u16 p0  = G.jogs[0].pontos;
    u16 p1  = G.jogs[1].pontos;
    *avg  = (p0 + p1) / 2;
    *min_ = min(*avg, ((u32)min(p0,p1))*EQU_PCT);
    return *avg == *min_;
}

int PT_Behind (void) {
    u16 avg, min_;
    if (!PT_Equ(&avg,&min_)) {
        if (G.jogs[0].pontos < G.jogs[1].pontos) {
            return 0;   // atleta a esquerda atras
        } else {
            return 1;   // atleta a direita  atras
        }
    } else {
        return -1;      // equilibrio
    }
}

void PT_All (void) {
    G.time    = 0;
    G.saques  = 0;
    G.quedas  = 0;
    G.ataques = 0;

    static u8 bests[2][REF_HITS]; // kmh
    memset(bests, 0, 2*REF_HITS*sizeof(u8));

    for (int i=0; i<2; i++) {
        G.jogs[i].golpes = 0;
    }

//Serial.println("---");
    for (int i=0 ; i<S.hit ; i++) {
        u8 dt   = S.hits[i].dt;
        s8 kmh_ = S.hits[i].kmh;
        u8 kmh  = abs(kmh_);
        int is_in = (kmh_ < 0);

        if (dt == 0) {
            if (kmh == 0) {
                G.quedas++;
            } else {
                G.saques++;
            }
        }

        G.time += dt;

        if (i==S.hit-1 || S.hits[i+1].dt==0) {
            continue; // conta dt, mas nao golpes/kmh do ultimo (nao sabemos se foi defendido)
        }

        if (kmh < HIT_KMH_50) {
            continue;
        }

        G.ataques++;
        G.jogs[is_in].golpes++;

        // bests
        u8* vec = bests[is_in];
        for (int j=0; j<HITS_CUR; j++) {
            if (kmh > vec[j]) {
                for (int k=HITS_CUR-1; k>j; k--) {
                    vec[k] = vec[k-1];
                }
                vec[j] = kmh;
                break;
            }
        }
    }
    G.time *= 100;
    G.time = min(G.time, S.timeout);

    for (int i=0; i<2; i++) {
        Jog* jog = &G.jogs[i];
        PT_Bests(HITS_CUR, bests[i], jog);
        G.jogs[i].pontos = (u32)jog->pontos;
    }

    u16 avg, min_;
    PT_Equ(&avg,&min_);

    int pct    = G.quedas * CONT_PCT;
    u32 pontos = (S.equilibrio ? min_ : avg);
    G.pontos   = pontos * (10000-pct) / 10000;
}
