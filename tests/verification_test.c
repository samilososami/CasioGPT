#define main calculator_main
#include "../calculator/src/main.c"
#undef main

#include <assert.h>
#include <stdio.h>

static int now_ticks=1000;
static int serial_open=1;
static int serial_opens;

int RTC_GetTicks(void) { return now_ticks; }
int RTC_Elapsed_ms(int start,int duration) { return now_ticks-start>=duration; }
int Serial_IsOpen(void) { return serial_open; }
int Serial_Open(unsigned char *mode) { (void)mode; serial_open=1; serial_opens++; return 0; }
int Serial_Close(int mode) { (void)mode; serial_open=0; return 0; }
int Serial_ClearRX(void) { return 0; }
int Serial_ClearTX(void) { return 0; }
int Serial_PollRX(void) { return 0; }
int Serial_ReadSingle(unsigned char *out) { (void)out; return -1; }
int Serial_PollTX(void) { return 256; }
int Serial_Write(const unsigned char *buf,int count) { (void)buf; (void)count; return 0; }

static void reset_verify(void)
{
    rpc_abort();now_ticks=1000;serial_open=1;serial_opens=0;previous_id=0;
    verify_stage=VERIFY_IDLE;verify_retry_pending=0;verify_retry_delay=0;
    verify_net_transport_failures=0;verify_wifi_recovery_used=0;
    verify_esp_attempt=verify_wifi_attempt=verify_net_attempt=verify_new_attempt=0;
    verify_net_max=10;verify_key_state=2;verify_esp_state=verify_net_state=0;
    esp_ok=wifi_ok=internet_ok=0;app_error[0]=0;
}

static void rpc_failure(void)
{
    rpc.active=rpc.queued=rpc.waiting=0;rpc.result=-1;
    process_verify();
}

static void rpc_success(const char *response)
{
    rpc.active=rpc.queued=rpc.waiting=0;rpc.result=1;
    str_copy(rpc.response,sizeof(rpc.response),response);
    process_verify();
}

static void run_retry(void)
{
    assert(verify_retry_pending);
    now_ticks+=verify_retry_delay;
    process_verify();
    assert(!verify_retry_pending);
}

static void test_ten_uart_attempts_fail_cleanly(void)
{
    reset_verify();start_link_attempt(VERIFY_ESP);
    for(int attempt=1;attempt<=VERIFY_MAX_ATTEMPTS;attempt++) {
        assert(verify_esp_attempt==attempt);
        rpc_failure();
        if(attempt<VERIFY_MAX_ATTEMPTS) run_retry();
    }
    assert(verify_stage==VERIFY_FAILED);
    assert(verify_esp_state==-1);
    assert(strstr(app_error,"10 intentos"));
    assert(serial_opens==VERIFY_MAX_ATTEMPTS);
}

static void test_tenth_uart_attempt_recovers(void)
{
    reset_verify();start_link_attempt(VERIFY_ESP);
    for(int attempt=1;attempt<VERIFY_MAX_ATTEMPTS;attempt++) { rpc_failure();run_retry(); }
    assert(verify_esp_attempt==VERIFY_MAX_ATTEMPTS);
    rpc_success("LINK:77:2:486F6D65:OK");
    assert(verify_stage==VERIFY_NET_BEGIN);
    assert(verify_esp_state==2 && esp_ok && wifi_ok);
}

static void test_wifi_waits_and_recovers(void)
{
    reset_verify();start_link_attempt(VERIFY_ESP);
    rpc_success("LINK:1:0::OK");
    assert(verify_stage==VERIFY_WIFI && verify_retry_pending);
    for(int attempt=1;attempt<VERIFY_MAX_ATTEMPTS;attempt++) {
        run_retry();
        assert(verify_wifi_attempt==attempt);
        rpc_success("LINK:2:0::OK");
        assert(verify_stage==VERIFY_WIFI && verify_retry_pending);
    }
    run_retry();
    assert(verify_wifi_attempt==VERIFY_MAX_ATTEMPTS);
    rpc_success("LINK:3:2:486F6D65:OK");
    assert(verify_stage==VERIFY_NET_BEGIN && wifi_ok);
}

static void test_network_progress_and_recovery(void)
{
    reset_verify();verify_stage=VERIFY_NET_BEGIN;verify_net_state=1;
    rpc_success("NET_WAIT:8:7:10");
    assert(verify_stage==VERIFY_NET_POLL);
    assert(verify_net_attempt==7 && verify_net_max==10);

    rpc_success("NET_ERROR:8:NO_WIFI");
    assert(verify_wifi_recovery_used==1);
    assert(verify_stage==VERIFY_WIFI && verify_retry_pending);
    run_retry();
    assert(verify_wifi_attempt==1);
    rpc_success("LINK:9:2:486F6D65:OK");
    assert(verify_stage==VERIFY_NET_BEGIN);

    rpc_success("NET_DONE:10");
    assert(internet_ok && verify_net_state==2 && verify_stage==VERIFY_NEW);
    rpc_success("GPT_ERROR:11:TRANSIENT");
    assert(verify_stage==VERIFY_NEW && verify_retry_pending);
    run_retry();
    rpc_success("GPT_ACK:11:N:0");
    assert(verify_stage==VERIFY_READY);
}

static void test_network_transport_has_ten_attempts(void)
{
    reset_verify();verify_stage=VERIFY_NET_BEGIN;verify_net_state=1;
    for(int attempt=1;attempt<=VERIFY_MAX_ATTEMPTS;attempt++) {
        rpc_failure();
        if(attempt<VERIFY_MAX_ATTEMPTS) run_retry();
    }
    assert(verify_stage==VERIFY_FAILED && verify_net_state==-1);
    assert(verify_net_transport_failures==VERIFY_MAX_ATTEMPTS);
}

int main(void)
{
    test_ten_uart_attempts_fail_cleanly();
    test_tenth_uart_attempt_recovers();
    test_wifi_waits_and_recovers();
    test_network_progress_and_recovery();
    test_network_transport_has_ten_attempts();
    puts("PASS CasioGPT verification: 10 UART/Wi-Fi/Internet/session retries, progress and recovery.");
    return 0;
}
