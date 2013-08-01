#include "ddns_dnspod.hpp"

char* getSubdomainRecordId(dnspod* dp, const char* domain, const char* subdomain) {
	dp->setDomain(domain);
	dp->getRecordList(0, _DNSPOD_MAXLENGTH);

	if (strcmp(dp->response["status"]["code"].asCString(), "1")) {
		ddnsError(dp->response["status"]["message"].asCString());
	}

	int counter;
	char* result = (char*)malloc(sizeof(char) * _DNSPOD_RECORDID_LENGTH);
	sprintf(result, "");
	for (counter = 0; counter < dp->response["records"].size(); counter++) {
		Json::Value record = dp->response["records"][counter];
		if (!strcmp(record["name"].asCString(), subdomain)) {
			sprintf(result, "%s",record["id"].asCString());
			break;
		}
	}

	return result;
}

int updateDdns(dnspod* dp, const char* ip, const char* domain, const char* subdomain, const char* recordId) {
	int record_id;
	sscanf(recordId, "%d", &record_id);

	dp->setDomain(domain);
	dp->updateDdnsRecord(record_id, subdomain, "д╛хо", ip);

	if (strcmp(dp->response["status"]["code"].asCString(), "1")) {
		syslog(LOG_INFO, "Ddns record update failed. Error: %s\n", dp->response["status"]["message"].asCString());
	} else {
		syslog(LOG_INFO, "Ddns record update succeed.");
	}

	return 0;
}
