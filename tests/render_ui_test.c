#define main calculator_main
#include "../calculator/src/main.c"
#undef main

#include <assert.h>
#include <stdio.h>
#include <sys/stat.h>

static color_t pixels[SCREEN_W*SCREEN_H];
static int now_ticks=640;

void *GetVRAMAddress(void) { return pixels; }
void Bdisp_Fill_VRAM(int color,int mode) { (void)mode; for(int i=0;i<SCREEN_W*SCREEN_H;i++) pixels[i]=(color_t)color; }
void Bdisp_PutDisp_DD(void) {}
int RTC_GetTicks(void) { return now_ticks; }
int RTC_Elapsed_ms(int start,int duration) { return now_ticks-start>=duration; }
void CMT_Delay_100micros(int delay) { (void)delay; }

static void write_ppm(const char *dir,const char *name)
{
    char path[1024];snprintf(path,sizeof(path),"%s/%s.ppm",dir,name);
    FILE *out=fopen(path,"wb"); assert(out);
    fprintf(out,"P6\n%d %d\n255\n",SCREEN_W,SCREEN_H);
    for(int i=0;i<SCREEN_W*SCREEN_H;i++) {
        unsigned c=pixels[i];
        unsigned char rgb[3]={
            (unsigned char)((((c>>11)&31)*255+15)/31),
            (unsigned char)((((c>>5)&63)*255+31)/63),
            (unsigned char)(((c&31)*255+15)/31)
        };
        fwrite(rgb,1,3,out);
    }
    fclose(out);
}

static void reset_preview(void)
{
    memset(messages,0,sizeof(messages));memset(input,0,sizeof(input));
    message_count=0;assistant_index=-1;scroll_px=0;wifi_ok=esp_ok=1;upper_case=0;
    input_len=input_cursor=0;app_error[0]=0;
}

static void set_pair(const char *user,const char *assistant,int streaming)
{
    reset_preview();message_count=2;
    messages[0].user=1;str_copy(messages[0].text,MESSAGE_CAP,user);
    messages[1].user=0;str_copy(messages[1].text,MESSAGE_CAP,assistant);
    assistant_index=streaming?1:-1;
}

int main(int argc,char **argv)
{
    assert(argc==2);mkdir(argv[1],0775);

    memcpy(pixels,casiogpt_splash_pixels,sizeof(casiogpt_splash_pixels));
    write_ppm(argv[1],"01-splash");

    reset_preview();verify_key_state=2;verify_esp_state=2;verify_net_state=1;
    verify_esp_attempt=3;verify_wifi_attempt=0;verify_net_attempt=7;verify_net_max=10;verify_new_attempt=0;
    render_verify();write_ppm(argv[1],"02-verification");

    set_pair("Resuelve 2(x - 3) + 4 = 3x - 5.",
             "1. Distribuye: 2x - 6 + 4 = 3x - 5.\n2. Simplifica: 2x - 2 = 3x - 5.\n3. Despeja: x = 3.",0);
    render_chat();write_ppm(argv[1],"03-algebra");

    set_pair("Por que la sal se disuelve en agua pero no en aceite?",
             "La sal y el agua son polares. El agua atrae y separa los iones del cristal. El aceite es no polar, asi que no existe esa afinidad.",0);
    render_chat();write_ppm(argv[1],"04-science");

    set_pair("Estoy nervioso por el examen de manana.",
             "Es normal sentirse asi. Repasa los conceptos clave, descansa y confia en lo que ya sabes",1);
    str_copy(input,sizeof(input),"Gracias");input_len=input_cursor=7;
    render_chat();write_ppm(argv[1],"05-streaming");

    puts("PASS CasioGPT UI: splash, verification, algebra, science and streaming states.");
    return 0;
}
