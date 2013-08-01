#ifndef DDNS_DNSPOD_HPP_
#define DDNS_DNSPOD_HPP_

#include <cstdlib>
#include <cstring>
#include <syslog.h>
#include <json/json.h>
#include <dnspod.hpp>
#include "ddns_error.hpp"

#define _DNSPOD_MAXLENGTH 3000
#define _DNSPOD_RECORDID_LENGTH 16

char* getSubdomainRecordId(dnspod* dp, const char* domain, const char* subdomain);
int updateDdns(dnspod* dp, const char* ip, const char* domain, const char* subdomain, const char* recordId);

#endif /* DDNS_DNSPOD_HPP_ */
