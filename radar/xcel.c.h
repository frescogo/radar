void CEL_Restart (void) {
    Serial.print(F("= FrescoGO! (versao "));
    Serial.print(AFERICAO);
    Serial.print(".");
    Serial.print(MAJOR);
    Serial.print(".");
    Serial.print(MINOR);
    Serial.print(".");
    Serial.print(REVISION);
    Serial.println(F(") ="));
    Serial_Score();
}

void CEL_Service (int player) {
    Serial.println();
    Serial.println(F("  ESQ    DIR"));
}

void CEL_Hit (int player, bool is_back, int kmh) {
    Serial.print(F("   "));
    if (player == 1) {
        Serial.print(F("      "));
    }
    Serial.print(kmh);
    if (is_back) {
        Serial.print(F(" !"));
    }
    Serial.println();
}

void CEL_Fall (void) {
    Serial.println(F("QUEDA"));
    Serial_Score();
}

void CEL_End (void) {
    Serial.println(F("= FIM ="));
    Serial_Score();
}

void CEL_Nop (void) {
}
