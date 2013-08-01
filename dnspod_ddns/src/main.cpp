#include <iostream>
#include <map>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <syslog.h>
#include <boost/regex.hpp>

#include <dnspod.hpp>
#include "ddns_network.hpp"
#include "ddns_dnspod.hpp"
#include "ddns_error.hpp"

#define _DDNS_STRING_BUFLENGTH 256

std::map<const char*, char*> params;

void readConfigFile(char *filePath, std::map<const char*, char*> *params) {
	if (access(filePath, R_OK) != 0)
		ddnsHelp(1);

	boost::regex configReg("^\\s*(.*?)\\s*=\\s*(.*)\\n");
	boost::regex commentReg("^\\s* *#");
	boost::cmatch result;
	char stringBuffer[_DDNS_STRING_BUFLENGTH];
	FILE *file = fopen(filePath, "r");
	int flag = 0;
	while (!feof(file)) {
		fgets(stringBuffer, _DDNS_STRING_BUFLENGTH, file);
		if (boost::regex_match(stringBuffer, commentReg)) {
			continue;
		}

		if (boost::regex_search(stringBuffer, result, configReg)) {
			char *paramName = (char*)malloc(sizeof(char) * _DDNS_STRING_BUFLENGTH);
			char *paramValue = (char*)malloc(sizeof(char) * _DDNS_STRING_BUFLENGTH);
			sprintf(paramName, "%s", result[1].str().c_str());
			sprintf(paramValue, "%s", result[2].str().c_str());
			if (!strcmp(paramName, "domain")) {
				flag += (1 << 0);
				params->insert(std::pair<const char*, char*>("domain", paramValue));
			} else if (!strcmp(paramName, "subdomain")) {
				flag += (1 << 1);
				params->insert(std::pair<const char*, char*>("sub_domain", paramValue));
			} else if (!strcmp(paramName, "email")) {
				flag += (1 << 2);
				params->insert(std::pair<const char*, char*>("email", paramValue));
			} else if (!strcmp(paramName, "password")) {
				flag += (1 << 3);
				params->insert(std::pair<const char*, char*>("password", paramValue));
			}
			sprintf(stringBuffer, "");
		}
	}

	if (flag < 15)
		ddnsHelp(2);
}

std::map<const char*, char*> processParams(int argc, char* argv[]) {
	std::map<const char*, char*> params;
	int flag = 0;
	if (argc % 2 == 0) {
		ddnsHelp();
	} else {
		int counter = 1;
		while (counter < argc) {
			char* paramName = argv[counter++];
			char* paramValue = argv[counter++];
			if (!strcmp(paramName, "--config") || !strcmp(paramName, "-c")) {
				readConfigFile(paramValue, &params);
				flag = (1 << 4);
				break;
			} else if (!strcmp(paramName, "--domain") || !strcmp(paramName, "-d")) {
				flag += (1 << 0);
				params.insert(std::pair<const char*, char*>("domain", paramValue));
			} else if (!strcmp(paramName, "--subdomain") || !strcmp(paramName, "-s")) {
				flag += (1 << 1);
				params.insert(std::pair<const char*, char*>("sub_domain", paramValue));
			} else if (!strcmp(paramName, "--email") || !strcmp(paramName, "-e")) {
				flag += (1 << 2);
				params.insert(std::pair<const char*, char*>("email", paramValue));
			} else if (!strcmp(paramName, "--password") || !strcmp(paramName, "-p")) {
				flag += (1 << 3);
				params.insert(std::pair<const char*, char*>("password", paramValue));
			}
		}
	}

	if (flag < 15)
		ddnsHelp();

	return params;
}

int daemon_init(void) {
	pid_t pid;
	if ((pid = fork()) < 0)
		return(-1);
	else if (pid != 0)
		exit(0);

	setsid();
	chdir("/");
	umask(0);
	close(0);
	close(1);
	close(2);
	return(0);
}

void sig_term(int signo) {
	if(signo == SIGTERM) {
		std::map<const char*, char*>::iterator it;
		for (it = params.begin(); it != params.end(); it++) {
			free(it->second);
		}

		syslog(LOG_INFO, "Ddns daemon terminated.");
		closelog();
		exit(0);
	}
}

int main(int argc, char *argv[]) {
	params = processParams(argc, argv);

	std::map<const char*, char*>::iterator it;
	it = params.find("email");
	char* email = it->second;
	it = params.find("password");
	char* password = it->second;
	it = params.find("sub_domain");
	char* subdomain = it->second;
	it = params.find("domain");
	char* domain = it->second;


	dnspod *dp = new dnspod(email, password);
	dp->getUserDetail();

	openlog("Ddns", LOG_PID, LOG_USER);
	if (!strcmp(dp->response["status"]["code"].asCString(), "1")) {
		printf("Ddns daemon started.");
		syslog(LOG_INFO, "Ddns daemon started.\n");
	} else {
		ddnsError(dp->response["status"]["message"].asCString());
	}

	std::vector<char*> ips;
	getIpList(&ips);
	if (ips.empty()) {
		ddnsError("IP获取失败");
	}

	char* recordId = getSubdomainRecordId(dp, domain, subdomain);
	if (!strcmp(recordId, "")) {
		printf("Subdomain's A record unexist. Creating...\n");
		std::map<const char*, const char*> record;
		record.insert(std::pair<const char*, const char*>("sub_domain", subdomain));
		record.insert(std::pair<const char*, const char*>("record_type", "A"));
		record.insert(std::pair<const char*, const char*>("record_line", "默认"));
		record.insert(std::pair<const char*, const char*>("value", ips.front()));
		dp->setDomain(domain);
		dp->createRecord(record);

		if (strcmp(dp->response["status"]["code"].asCString(), "1")) {
			ddnsError(dp->response["status"]["message"].asCString());
		}

		printf("A record created, Ddns daemon start working.\n");
	}


	if (daemon_init() == -1) {
		printf("Fork self failed. Ddns daemon stopped.\n");
		exit(0);
	}

	signal(SIGTERM, sig_term);

	updateDdns(dp, ips.front(), domain, subdomain, recordId);
	char *curRecordIp = (char*)malloc(sizeof(char) * DDNS_IPADDRESS_LEN);
	while(1) {
		sleep(600);

		sprintf(curRecordIp, "%s", ips.front());
		getIpList(&ips);
		if (sizeof(ips) < 1) {
			ddnsError("IP获取失败");
		}
		if (strcmp(ips.front(), curRecordIp)) {
			updateDdns(dp, ips.front(), domain, subdomain, recordId);
		}
	}

	return 0;
}
