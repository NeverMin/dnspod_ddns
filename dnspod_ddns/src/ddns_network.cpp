#include "ddns_network.hpp"

#define GET_IP_CMD "/sbin/ifconfig | grep 'inet addr:'| grep -v '127.0.0.1' | cut -d: -f2 | awk '{ print $1}'"
int getIpList(std::vector<char*> *ips) {
	char buf[DDNS_IPADDRESS_LEN];

	FILE *output = popen(GET_IP_CMD, "r");

	while (!feof(output)) {
		fgets(buf, DDNS_IPADDRESS_LEN, output);
		if (!strcmp(buf, "")) {
			continue;
		}
		char *ip = (char *)malloc(sizeof(char) * DDNS_IPADDRESS_LEN);
		sprintf(ip, "%s", buf);
		if (*(ip + (strlen(ip)-1)) == '\n') {
			*(ip + (strlen(ip)-1)) = '\0';
		}

		ips->insert(ips->begin(), ip);
		sprintf(buf, "");
	}
	pclose(output);

	return 0;
}
