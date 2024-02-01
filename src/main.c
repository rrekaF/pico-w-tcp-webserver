#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "lwip/tcp.h"
#include "lwip/pbuf.h"

#define OBLED CYW43_WL_GPIO_LED_PIN
#define OBLEDTOGGLE cyw43_arch_gpio_put
#define BUF_SIZE 2048
#define TCP_PORT 2024

typedef struct TCP_SERVER_T_
{
	struct tcp_pcb *server_pcb;
	struct tcp_pcb *client_pcb;
	bool complete;
	uint8_t buffer_sent[BUF_SIZE];
	uint8_t buffer_recv[BUF_SIZE];
	int sent_len;
	int recv_len;
	int run_count;
} TCP_SERVER_T;

void run_tcp_server();
TCP_SERVER_T *init_server();
bool tcp_server_open(TCP_SERVER_T *server);
err_t tcp_server_accept(void *arg, struct tcp_pcb *client_pcb, err_t err);

int main()
{
	stdio_init_all();
	// Wait for minicom to connect before starting
	int unsigned i = 0;
	while (i < 10)
	{
		printf("%d: zzz\n", ++i);
		sleep_ms(1000);
	}
	while (cyw43_arch_init())
	{
		printf("Wifi init failed\n");
		sleep_ms(1000);
	}
	// Go into wifi client mode
	cyw43_arch_enable_sta_mode();

	printf("Wifi init success\n");
	printf("connecting to wifi with credentials\n");
	printf("SSID: %s\n", WIFI_SSID);
	printf("PASS: %s\n", WIFI_PASSWORD);
	unsigned int attempts = 0;
	while (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000))
	{
		printf("wifi connection failed\nretrying...\nattempts: %d\n", ++attempts);
		sleep_ms(1000);
	}
	printf("connected to wifi\n");
	run_tcp_server();
}

void run_tcp_server(void)
{
	TCP_SERVER_T *server = init_server();
	if (!server)
	{
		return;
	}
	if (!tcp_server_open(server))
	{
	}
	short unsigned int dots = 0;
	while (true)
	{
		sleep_ms(1000);
		printf("waiting");
		for (int i = 0; i < dots; i++)
		{
			printf(".");
		}
		printf("\n");
		dots++;
		dots = dots % 4;
	}
}
err_t tcp_server_accept(void *arg, struct tcp_pcb *client_pcb, err_t err)
{
	printf("Callback function called\n");
	// Blink
	OBLEDTOGGLE(OBLED, 1);
	// printf("Led ON\n");
	// sleep_ms(50);
	// OBLEDTOGGLE(OBLED, 0);
	printf("Led OFF\n");
}
bool tcp_server_open(TCP_SERVER_T *server)
{
	// TODO: read docs lines 95, 97, 105
	printf("Starting server at %s on port %u\n", ip4addr_ntoa(netif_ip4_addr(netif_list)), TCP_PORT);
	printf("Creating PCB\n");
	struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
	if (!pcb)
	{
		printf("(-) Failed to create PCB\n");
		return false;
	}
	printf("(+) Created PCB successfully\n");
	printf("Binding to port %u\n", TCP_PORT);
	err_t err = tcp_bind(pcb, NULL, TCP_PORT);
	if (err)
	{
		printf("(-) Failed to bind to port%u\n", TCP_PORT);
		return false;
	}
	printf("(+) Bound port %u successfully\n", TCP_PORT);
	printf("Listening...\n");
	server->server_pcb = tcp_listen_with_backlog(pcb, 1);
	if (!server->server_pcb)
	{
		printf("(-) Failed to listen, closing PCB\n");
		if (pcb)
		{
			err = tcp_close(pcb);
			if (err)
			{
				printf("(-) Failed at closing PCB\n");
			}
			else
			{
				printf("(+) Closed PCB\n");
			}
		}
		return false;
	}
	tcp_arg(server->server_pcb, server);
	tcp_accept(server->server_pcb, tcp_server_accept);
}
TCP_SERVER_T *init_server(void)
{
	TCP_SERVER_T *server = calloc(1, sizeof(TCP_SERVER_T));
	if (!server)
	{
		printf("(-) Failed to allocate memory for a server instance\n");
		return NULL;
	}
	else
	{
		printf("(+) Succesfully allocated memmory for a server instance\n");
	}
	return server;
}