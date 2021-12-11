#ifndef SHBK_COMMON_INICONFIG_H_
#define SHBK_COMMON_INICONFIG_H_

#include <string>
#include "configdef.h"

class Iniconfig
{
protected:
	Iniconfig();
	
public:
	static Iniconfig* getInstance();
	~Iniconfig();
	
	bool loadfile(const std::string& path);
	const st_env_config& getconfig();
	
private:
	st_env_config _config;
	static Iniconfig* config;
	bool _isloaded;
	
};


#endif