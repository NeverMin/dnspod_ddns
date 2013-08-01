#ifndef DDNS_ERROR_HPP_
#define DDNS_ERROR_HPP_

#include <stdlib.h>
#include <stdio.h>
#include <syslog.h>

void ddnsError(const char* errorInfo);
void ddnsHelp(int infoType = 0);

#endif /* DDNS_ERROR_HPP_ */
