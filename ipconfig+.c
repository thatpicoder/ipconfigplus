#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

#define BUFFER_SIZE 16384

void get_local_ip(char *ip, char *host) {
    #ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);
    #endif
    struct hostent *h;
    gethostname(host, 256);
    h = gethostbyname(host);
    strcpy(ip, inet_ntoa(*((struct in_addr*)h->h_addr_list[0])));
}

void fetch_api(char *response) {
    #ifdef _WIN32
    SOCKET sock;
    #else
    int sock;
    #endif
    struct hostent *server;
    struct sockaddr_in serv_addr;

    char request[] =
    "GET /json/ HTTP/1.1\r\n"
    "Host: ip-api.com\r\n"
    "Connection: close\r\n\r\n";

    sock = socket(AF_INET, SOCK_STREAM, 0);
    server = gethostbyname("ip-api.com");

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(80);
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);

    connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    send(sock, request, strlen(request), 0);

    int total = 0, n;
    while ((n = recv(sock, response + total, BUFFER_SIZE - total - 1, 0)) > 0) {
        total += n;
    }
    response[total] = '\0';

    #ifdef _WIN32
    closesocket(sock);
    WSACleanup();
    #else
    close(sock);
    #endif
}

void extract(const char *json, const char *key, char *out) {
    char *start = strstr(json, key);
    if (start) {
        start = strchr(start, ':') + 2;
        char *end = strchr(start, '"');
        strncpy(out, start, end - start);
        out[end - start] = '\0';
    } else {
        strcpy(out, "N/A");
    }
}

void get_dns(char *dns) {
    #ifdef _WIN32
    FILE *fp = _popen("ipconfig /all", "r");
    char line[256];
    strcpy(dns, "");
    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, "DNS Servers")) {
            char *p = strchr(line, ':');
            if (p) strcat(dns, p + 2);
        }
    }
    _pclose(fp);
    #else
    FILE *fp = fopen("/etc/resolv.conf", "r");
    char line[256];
    strcpy(dns, "");
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "nameserver", 10) == 0) {
            strcat(dns, line + 11);
        }
    }
    fclose(fp);
    #endif
}

int main() {
    char host[256], local_ip[64];
    char response[BUFFER_SIZE];

    char public_ip[64], city[64], region[64], country[64];
    char zip[32], timezone[64], lat[32], lon[32];
    char isp[128], org[128], asn[128], reverse[128];
    char proxy[16], hosting[16], mobile[16];
    char dns[512];

    char choice;

    printf("Welcome to ipconfig+! Enter \"Y\" to get started. Otherwise, enter any other key.\nMade by bitetheapple\n> ");
    scanf(" %c", &choice);

    if (choice != 'Y' && choice != 'y') {
        printf("Goodbye!\n");
        return 0;
    }

    printf("\n--IP info--\n\n");

    get_local_ip(local_ip, host);
    get_dns(dns);
    fetch_api(response);

    char *json = strstr(response, "\r\n\r\n");
    if (json) json += 4;

    extract(json, "\"query\"", public_ip);
    extract(json, "\"city\"", city);
    extract(json, "\"regionName\"", region);
    extract(json, "\"country\"", country);
    extract(json, "\"zip\"", zip);
    extract(json, "\"timezone\"", timezone);
    extract(json, "\"lat\"", lat);
    extract(json, "\"lon\"", lon);

    extract(json, "\"isp\"", isp);
    extract(json, "\"org\"", org);
    extract(json, "\"as\"", asn);
    extract(json, "\"reverse\"", reverse);

    extract(json, "\"proxy\"", proxy);
    extract(json, "\"hosting\"", hosting);
    extract(json, "\"mobile\"", mobile);

    printf("Hostname: %s\n", host);
    printf("Local IP: %s\n\n", local_ip);

    printf("Public IP: %s\n\n", public_ip);

    printf("City: %s\nRegion: %s\nCountry: %s\nZIP: %s\n", city, region, country, zip);
    printf("Timezone: %s\nCoords: %s, %s\n\n", lat, lon);

    printf("ISP: %s\nOrg: %s\nASN: %s\nReverse DNS: %s\n\n", isp, org, asn, reverse);

    printf("Proxy/VPN: %s\nHosting: %s\nMobile: %s\n\n", proxy, hosting, mobile);

    printf("DNS:\n%s\n", dns);

    printf("\nProgram finished! Goodbye!\n");

    return 0;
}
