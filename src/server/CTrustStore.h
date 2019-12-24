/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#ifndef CTRUSTSTORE_H
#define CTRUSTSTORE_H
#include <string>
#include <vector>

class CTrustCert
{
public:
	std::string PublicKeyHex;
	unsigned long int ID;
	int Type;
};

class CTrustStore
{
private:
	std::vector<CTrustCert> List;
	CTrustCert* Current = 0;
public:
	CTrustStore()
	{
	}
	~CTrustStore()
	{
	}

	const CTrustCert* getCurrent()
	{
		return Current;
	}

	void addKey(const std::string& hex)
	{
		CTrustCert cert;
		cert.ID = 0;
		cert.Type = 0;
		cert.PublicKeyHex = hex;
		List.push_back(cert);
	}

	void addKeyEx(const CTrustCert& cert)
	{
		List.push_back(cert);
	}

	bool validate(const std::string& hex)
	{
		for(unsigned int n = 0; n < List.size(); n++)
		{
			if(List[n].PublicKeyHex == hex)
			{
				Current = &List[n];
				return true;
			}
		}
		return false;
	}

};

#endif // CTRUSTSTORE_H
