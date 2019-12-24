/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#include "CConfig.h"
#include <string.h>

namespace solusek
{
#ifdef USE_YAML
	CConfig::CConfig(const char *fileName) : FOK(false)
	{
		FILE *f = fopen(fileName, "r");
		if(!f)
		{
			Log.print("ERROR: Config file not found.\n");
			return;
		}
		fseek(f, 0, SEEK_END);
		int len = ftell(f);
		fseek(f, 0, SEEK_SET);
		char *buf = new char[len+1];
		memset(buf, 0, len+1);
		size_t r = fread(buf, len, 1, f);
		fclose(f);

		BaseNode = YAML::Load(buf);
		YAML::Node dbd = BaseNode["database_driver"];
		if(!dbd.IsNull())
		{
			ConnectionString = dbd.as<std::string>();
			//Log.print("Database information read.\n");
		}

		delete[] buf;
		FOK = true;
	}

	CConfig::~CConfig()
	{

	}

	std::string CConfig::get(const std::string& path)
	{
		YAML::Node node = YAML::Clone(BaseNode);
		std::string acc = path;
		for(int n = acc.find('/'); n != std::string::npos; n = acc.find('/'))
		{
			std::string name = acc.substr(0,n);
			node = node[name];
			if(!node || node.IsNull())
				return std::string();
			acc = acc.substr(n+1);
		}
		if(!node || node.IsNull())
			return std::string();
		node = node[acc];
		return node.as<std::string>();
	}
#else
	void config_parse(CConfig::CNode* parent, std::string& body, int shift = 0)
	{
		//printf("Shift: %d\n", shift);
		int p, innerShift;
		std::string line;
		CConfig::CNode* lastNode = parent;
		while((p = body.find("\n")) != std::string::npos)
		{
			innerShift = 0;
			line = body.substr(0, p);
			while(line[0] == ' ' || line[0] == '\t')
			{
				innerShift++;
				line = line.substr(1);
			}

			if(innerShift > shift)
			{
				config_parse(lastNode, body, innerShift);
				continue;
			}
			else if(innerShift < shift)
				break;
			else
			{

				int pp = line.find(":");

				if(pp != std::string::npos)
				{
					std::string var = line.substr(0, pp);
					std::string val = line.substr(pp+1);

					while(var[0] == ' ')
						var = var.substr(1);
					while(var[var.size()-1] == ' ')
						var = var.substr(0, var.size()-1);

					while(val[0] == ' ')
						val = val.substr(1);
					while(val[val.size()-1] == ' ')
						val =val.substr(0, val.size()-1);

					CConfig::CNode* node = new CConfig::CNode();
					node->Parent = parent;
					node->Name = var;
					node->Value = val;
					parent->Children.push_back(node);
					lastNode = node;
				}
				body = body.substr(p+1);
			}
		}
		//printf("Exit shift: %d\n", shift);
	}

	CConfig::CConfig(const char *fileName) : FOK(false)
	{
		FILE *f = fopen(fileName, "r");
		if(!f)
		{
			Log.print("ERROR: Config file not found.\n");
			return;
		}
		fseek(f, 0, SEEK_END);
		int len = ftell(f);
		fseek(f, 0, SEEK_SET);
		char *buf = new char[len+1];
		memset(buf, 0, len+1);
		size_t r = fread(buf, len, 1, f);
		fclose(f);
		std::string body(buf);
		delete[] buf;

		BaseNode = new CNode();
		BaseNode->Parent = 0;
		config_parse(BaseNode, body, 0);

		FOK = true;
	}

	CConfig::~CConfig()
	{
		delete BaseNode;
	}

	CConfig::CNode* config_get(CConfig::CNode* node, std::string& path)
	{
		std::string var;
		int p = path.find("/");
		if(p != std::string::npos)
		{
			var = path.substr(0, p);
			path = path.substr(p+1);
		}
		else
		{
			var = path;
			path = "";
		}

		for(std::vector<CConfig::CNode*>::iterator it = node->Children.begin(); it != node->Children.end(); ++it)
		{
			if((*it)->Name == var)
			{
				if(path.size() > 0)
					return config_get((*it), path);
				else
					return (*it);
			}
		}
		return 0;
	}

	std::string CConfig::get(const std::string& path)
	{
		std::string _path(path);
		CNode* node = config_get(BaseNode, _path);
		if(node)
		{
			std::string val = node->Value;
			if(val[0] == '"' && val[val.size()-1] == '"')
				val = val.substr(1, val.size()-2);
			return val;
		}
		else
			return std::string();
	}

#endif
}
