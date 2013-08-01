#include "ddns_error.hpp"

void ddnsError(const char* errorInfo) {
	fflush(stdout);
	fprintf(stderr, "Ddns daemon got an error. Error: %s\n", errorInfo);
	syslog(LOG_INFO, "Ddns daemon got an error. Error: %s\n", errorInfo);

	exit(-1);
}

void ddnsHelp(int infoType) {
	switch (infoType) {
	case 0:
		fprintf(stderr, "Run this program like:\n");
		fprintf(stderr, "ddns -s www -d example.com -e a@b.c -p my.Password\n\n");
		fprintf(stderr, "Options:\n");
		fprintf(stderr, "    --config, -c         Use a config file to set params. Other options will be skipped.\n");
		fprintf(stderr, "    --domain, -d         Set the domain to update.\n");
		fprintf(stderr, "    --subdomain, -s      Set the subdomain to update. Default will be \"@\"\n");
		fprintf(stderr, "    --email, -e          Your dnspod account's email address.\n");
		fprintf(stderr, "    --password, -p       Your dnspod account's password.\n");
		break;
	case 1:
		fprintf(stderr, "Config file does not exist or do not have read permission.\n");
		break;
	case 2:
		fprintf(stderr, "Config file does not have a correct format.\n");
		break;
	default:
		fprintf(stderr, "Unknow error.\n");
		break;
	}

	exit(-1);
}
