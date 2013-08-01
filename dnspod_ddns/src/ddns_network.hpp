#ifndef DDNS_NETWORK_HPP_
#define DDNS_NETWORK_HPP_

#include <vector>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#define DDNS_IPADDRESS_LEN 32

int getIpList(std::vector<char*> *ip);

#endif
