enum {
    PC_RESTART = 0,
    PC_SEQ     = 1,
    PC_HIT     = 2,
    PC_TICK    = 3,
    PC_FALL    = 4,
    PC_END     = 5,
    PC_DESC    = 6
};

void PC_Desc (void) {
    Serial.print(PC_DESC);              // codigo de nova sequencia
    Serial.print(F(";"));
    Serial.print(S.descanso/100);       // tempo de descanso em s
    Serial.print(F(";"));
    Serial.println();
}

void PC_Seq (void) {
    Serial.print(PC_SEQ);               // codigo de nova sequencia
    Serial.print(F(";"));
    Serial.print(G.time/1000);          // tempo jogado em ms
    Serial.print(F(";"));
    Serial.print(S.descanso/100);       // tempo de descanso em s
    Serial.print(F(";"));
    Serial.print(Falls());              // total de quedas
    Serial.print(F(";"));
    Serial.print(S.names[0]);           // atleta a esquerda
    Serial.print(F(";"));
    Serial.print(S.names[1]);           // atleta a direita
    Serial.print(F(";"));
    Serial.print(S.juiz);               // juiz
    Serial.print(F(";"));
    Serial.println();
}

void PC_Player (int I) {
    Serial.print(PT_Behind() == I ? 1 : 0);   // 1=atras | 0=ok
    Serial.print(F(";"));
    Serial.print(G.jogs[I].pontos);
    Serial.print(F(";"));
    Serial.print(G.jogs[I].golpes);
    Serial.print(F(";"));
    Serial.print(G.jogs[I].media1);
    Serial.print(F(";"));
}

void PC_Hit (int player, bool is_back, int kmh) {
    Serial.print(PC_HIT);               // codigo de golpe
    Serial.print(F(";"));
    Serial.print(player);               // 0=esquerda | 1=direita
    Serial.print(F(";"));
    Serial.print(is_back);              // 0=normal   | 1=revez
    Serial.print(F(";"));
    Serial.print(kmh);                  // velocidade
    Serial.print(F(";"));
    Serial.println();
}

void PC_Tick (void) {
    Serial.print(PC_TICK);              // codigo de tick
    Serial.print(F(";"));
    Serial.print(G.time/1000);          // tempo jogado em ms
    Serial.print(F(";"));
    Serial.print(G.pontos);             // total da dupla
    Serial.print(F(";"));
    Serial.print(S.hit);                // total de golpes
    Serial.print(F(";"));
    PC_Player(0);
    PC_Player(1);
    Serial.println();
}

void PC_Fall (void) {
    Serial.print(PC_FALL);              // codigo de queda
    Serial.print(F(";"));
    Serial.print(Falls());              // total de quedas
    Serial.print(F(";"));
    PC_Player(0);
    PC_Player(1);
    Serial.println();
}

void PC_End (void) {
    Serial.print(PC_END);               // codigo de fim
    Serial.print(F(";"));
    PC_Player(0);
    PC_Player(1);
    Serial.println();
}

void PC_Nop (void) {
}

void PC_Atualiza (void) {
    PC_Seq();
    PC_Hit(0,0,0);
    PC_Hit(1,0,0);
    PC_Tick();
    PC_Fall();
}

void PC_Restart (void) {
    Serial.print(PC_RESTART);           // codigo de reinicio
    Serial.print(F(";"));
    Serial.print(S.timeout/1000);       // tempo total de jogo
    Serial.print(F(";"));
    Serial.print((int)S.equilibrio);    // equilibrio ligado?
    Serial.print(F(";"));
    Serial.print(S.names[0]);           // atleta a esquerda
    Serial.print(F(";"));
    Serial.print(S.names[1]);           // atleta a direita
    Serial.print(F(";"));
    Serial.print(S.juiz);               // juiz
    Serial.print(F(";"));
    Serial_Pars();
    PC_Atualiza();
}
