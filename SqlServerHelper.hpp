#pragma once 

#include<string>
#include "SqlServerUtil.h"

using namespace std;

class SqlServerHelper {
private:
	SqlServerHelper() {
	}

	SqlServerHelper(const SqlServerHelper&) {
	}

	SqlServerHelper& operator=(const SqlServerHelper&) {
	}

	string ConnStr;

public:
	static SqlServerHelper& GetInstance() {
		static SqlServerHelper instance;
		return instance;
	}

	bool Initial(string connStr) {

		this->ConnStr = connStr;
	
		return true;
	}

	string GetConnStr() {
		return ConnStr;
	}
};