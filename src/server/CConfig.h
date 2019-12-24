/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#ifndef __C_CONFIG_INCLUDED__
#define __C_CONFIG_INCLUDED__
#include <vector>
#include "CVarVal.h"
#include "CLog.h"
#include "../include/solusek/IConfig.h"
#if USE_YAML
#	include <yaml-cpp/yaml.h>
#endif

namespace solusek
{
	class CConfig : public IConfig
	{
	private:
		CLog Log;
		bool FOK;
		std::string ConnectionString;
	public:
		class CNode
		{
		public:
			CNode* Parent;
			std::vector<CNode*> Children;
			std::string Name;
			std::string Value;

			~CNode()
			{
				for(std::vector<CNode*>::iterator it = Children.begin(); it != Children.end(); ++it)
				{
					delete (*it);
				}
			}
		};

		CNode *BaseNode;
	public:
		CConfig(const char *fileName);
		~CConfig();

		bool isOK()
		{
			return FOK;
		}

		virtual std::string get(const std::string& path);
	};
}

#endif
