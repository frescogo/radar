typedef int Radar_S;

void radar_setup (void) {
}

int radar_read (Radar_S* s) {
    static u32 old = millis();                                                  
    u32 now = millis();                                                         
    u32 dt = now - old;                                                         
    if (dt > 500) {                                                             
        old = now;                                                              
        if (random(0,5) <= 2) {                                                 
            int vel = random(300,1000);                                           
            return (random(0,2)==0) ? vel : -vel;                               
        }                                                                       
    }                                                                           
    return 0;                                                                   
}
