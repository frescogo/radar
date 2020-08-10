void Serial_Pars (void) {
    //sprintf_P(STR, PSTR("(CONF: v%d.%d / %dcm / %ds / max=%d / equ=%d / cont=%d / max=%d)"),
    sprintf_P(STR, PSTR("(v%d%d%d/%dcm/%ds/maxs(%d,%d)/equ%d/cont%d/fim%d)"),
                MAJOR, MINOR, REVISION,
                S.distancia,
                (int)(S.timeout/1000),
                (int)S.maxima,
                (int)S.reves,
                (int)S.equilibrio,
                (int)CONT_PCT,
                (int)ABORT_FALLS);
    Serial.println(STR);
}

void Serial_Score (void) {
    Serial.println();
    Serial.println(F("-----------------------------------------------"));
    sprintf_P(STR, PSTR("%22s"), S.names[0]);
    Serial.print(STR);
    Serial.print(F(" / "));
    sprintf_P(STR, PSTR("%s"), S.names[1]);
    Serial.print(STR);
    Serial.println();

    Serial.println(F("-----------------------------------------------"));
    Serial.println();

    Serial.print(F("PONTOS ............. "));
    sprintf_P(STR, PSTR("%d"), G.pontos);
    Serial.print(STR);
    Serial.println(F(" pontos"));

    Serial.print(F("Tempo Restante ..... "));
    int restante = (G.time > S.timeout ? 0 : (int)ceil((S.timeout-G.time)/(float)1000));
    sprintf_P(STR, PSTR("%02d:%02d"), restante/60, restante%60);
    Serial.println(STR);

    Serial.print(F("Descanso ........... "));
    Serial.print(S.descanso/100);
    Serial.println("s");

    Serial.print(F("Quedas ............. "));
    Serial.println(Falls());

    Serial.print(F("Ataques ............ "));
    Serial.println(G.ataques);

    Serial.print(F("Ritmo .............. "));
    if (G.time > 5000) {
        Serial.print((int)G.ritmo);
        //Serial.print(F("/"));
        //Serial.print((int)G.pace[1]);
        Serial.println(F(" km/h"));
    } else {
        Serial.println(F("---"));
    }

    Serial.print(F("Juiz ............... "));
    Serial.println(S.juiz);

    for (int i=0; i<2; i++) {
        Serial.println();
        Serial.println(F("-----------------------------------------------"));
        Serial.println();

        sprintf_P(STR, PSTR("%10s: "), S.names[i]);
        Serial.print(STR);
        sprintf_P(STR, PSTR("%4d pontos "), G.jogs[i].pontos);
        Serial.println(STR);

        for (int j=0; j<LADO_NRM_REV; j++) {
            int pct, tot;
            Serial.print(F("   --> "));
            if (j == LADO_NRM) {
                tot = HITS_NRM;
                Serial.print(F("nrm"));
            } else {
                tot = HITS_REV;
                Serial.print(F("rev"));
            }
            Lado* lado = &G.jogs[i].lados[j];
            sprintf_P(STR, PSTR(": %4d pontos  (ata: %3d/%3d, med: %2d, min: %2d, max: %2d)"),
                lado->pontos, lado->golpes, tot, lado->media1, lado->minima, lado->maxima);
            Serial.println(STR);
        }
    }

    Serial.println();
    Serial.println(F("-----------------------------------------------"));
    Serial.println();
    Serial_Pars();
}

void Serial_Log (void) {
    Serial.println();
    Serial.println(F("-----------------------------------------------"));
    Serial.println();

    int ball  = 0;
    //u32 ps[2] = {0,0};
    for (int i=0 ; i<S.hit ; i++) {
        u8 dt  = S.hits[i].dt;
        u8 kmh = S.hits[i].kmh;
        int is_out = (kmh > 0);

        if (dt == 0) {
            ball = ball + 1;
            Serial.print(F("-- Sequencia "));
            sprintf_P(STR, PSTR("%02d"), ball);
            Serial.print(STR);
            Serial.println(F(" ----------------"));
        }

        Serial.print(F("  "));
        if (!is_out) {
            Serial.print(F("  "));
        } else {
            Serial.print(F("          "));
        }

        sprintf_P(STR, PSTR("%4d"), dt*100);
        Serial.print(STR);

        if (i==S.hit-1 || S.hits[i+1].dt==0) {
            Serial.println();
            //Serial.println(F("   -----   -----"));
            //sprintf_P(STR, PSTR("   %5ld   %5ld"), ps[0]/100, ps[1]/100);
            //Serial.println(STR);
        } else {
            //u16 pt = ((u16)kmh)*((u16)kmh);
            //ps[1 - i%2] += pt;

            Serial.print(F("  "));
            Serial.print(F("  "));
            if (!is_out) {
                Serial.print(F("        "));
            }
            sprintf_P(STR, PSTR("(%02d km/h)"), kmh); //, pt);
            Serial.print(STR);
        }
        Serial.println();
        delay(50);
    }
    //Serial.println();

    Serial.println(F("-----------------------------------------------"));
    Serial.println();

    Serial.println(F("    Atleta   |    Nrm    Rev   |     Total"));
    for (int i=0; i<2; i++) {
        sprintf_P(STR, PSTR("%10s   |   %4d   %4d   |   %04d pontos"),
            S.names[i],
            G.jogs[i].lados[LADO_NRM].pontos, G.jogs[i].lados[LADO_REV].pontos,
            G.jogs[i].pontos);
        Serial.println(STR);
    }

    Serial.println();
    Serial.println(F("-----------------------------------------------"));
    Serial.println();

    u16 avg, min_;
    PT_Equ(&avg,&min_);

    u16 equ = (!S.equilibrio ? 0 : avg - min_);
    u16 pct = Falls() * CONT_PCT;

    sprintf_P(STR, PSTR("Media ........... %02d.%02d"), avg/100, avg%100);
    Serial.print(STR);
    Serial.println(F("  pontos"));
    sprintf_P(STR, PSTR("Equilibrio ...... %02d.%02d  (-)"), equ/100, equ%100);
    Serial.println(STR);
    sprintf_P(STR, PSTR("Quedas (%02d) ..... %02d.%02d%% (-)"), Falls(), pct/100,pct%100);
    Serial.println(STR);
    sprintf_P(STR, PSTR("TOTAL ........... %04d"), G.pontos);
    Serial.print(STR);
    Serial.println(F("  pontos"));
}

void PC_Restart (void);     // assinatura de arquivo nao incluido ainda

int Serial_Check (void) {
    static char CMD[32];
    static int  i = 0;
    bool restart = false;

    char c;
    while (Serial.available()) {
        c = Serial.read();
        if (c=='\n' || c=='\r' || c=='$' ) {
            if (i == 0) {
                                // skip
            } else {
                CMD[i] = '\0';
                goto _COMPLETE;   // complete
            }
        } else {
            CMD[i++] = c;       // continue
        }
    }
    return IN_NONE;
_COMPLETE:
    i = 0;

    if (strncmp_P(CMD, PSTR("restaurar"), 9) == 0) {
        return IN_RESET;
    } else if (strncmp_P(CMD, PSTR("reiniciar"), 9) == 0) {
        return IN_RESTART;
    } else if (strncmp_P(CMD, PSTR("terminar"), 8) == 0) {
        return IN_TIMEOUT;
    } else if (strncmp_P(CMD, PSTR("desfazer"), 8) == 0) {
        return IN_UNDO;
    } else if (strncmp_P(CMD, PSTR("placar"), 6) == 0) {
        Serial_Score();
        return IN_NONE;
    } else if (strncmp_P(CMD, PSTR("relatorio"), 9) == 0) {
        Serial_Score();
        Serial_Log();
        return IN_NONE;
    } else if (strncmp_P(CMD, PSTR("tempo "), 6) == 0) {
        S.timeout = ((u32) max(10, atoi(&CMD[6]))) * 1000;
    } else if (strncmp_P(CMD, PSTR("distancia "), 5) == 0) {
        restart = true;
        S.distancia = max(100, min(1000, atoi(&CMD[10])));
    } else if (strncmp_P(CMD, PSTR("equilibrio sim"), 14) == 0) {
        S.equilibrio = 1;
    } else if (strncmp_P(CMD, PSTR("equilibrio nao"), 14) == 0) {
        S.equilibrio = 0;
    } else if (strncmp_P(CMD, PSTR("maxima "), 7) == 0) {
        S.maxima = max(50, min(HIT_KMH_MAX, atoi(&CMD[7])));
    } else if (strncmp_P(CMD, PSTR("reves "), 5) == 0) {
        S.reves = atoi(&CMD[5]);
        if (S.reves) {
            S.reves = max(REVES_MIN, min(REVES_MAX, S.reves));
        }
/*
    } else if (strncmp_P(CMD, PSTR("continuidade "), 13) == 0) {
        S.continuidade = atoi(&CMD[13]);
*/
    } else if (strncmp_P(CMD, PSTR("esquerda "), 9) == 0) {
        if (strlen(&CMD[9]) < 15) {
            strcpy(S.names[0], &CMD[9]);
        } else {
            goto ERR;
        }
    } else if (strncmp_P(CMD, PSTR("direita "), 8) == 0) {
        if (strlen(&CMD[8]) < 15) {
            strcpy(S.names[1], &CMD[8]);
        } else {
            goto ERR;
        }
    } else if (strncmp_P(CMD, PSTR("juiz "), 5) == 0) {
        if (strlen(&CMD[5]) < 15) {
            strcpy(S.juiz, &CMD[5]);
        } else {
            goto ERR;
        }
    } else {
        goto ERR;
    }

    if (0) {
ERR:;
        Serial.println(F("err"));
OK:;
    } else {
        Serial.println(F("ok"));
    }
    EEPROM_Save();
    PT_All();
    if (MOD==MOD_CEL) {
        Serial_Score();
    } else if (restart) {
        PC_Restart();
    }

    return IN_NONE;
}
