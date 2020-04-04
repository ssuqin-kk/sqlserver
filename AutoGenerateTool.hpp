#pragma once

#include <vector>
#include <iterator>
#include<string>
#include<iostream>
#include "SqlServerUtil.h"

#define OTL_ODBC_MSSQL_2008 
#define _CRT_SECURE_NO_WARNINGS
#define OTL_STL 
#define OTL_ANSI_CPP

#include "otlv4.h"

using namespace std;

class Column {
public:
	string Name;
	int DbType = - 1;
	int DbSize;
	string CppType;
	string PlaceHolder; // 占位符

	Column() {
	}

	Column(string name, int dbType, int dbSize, string cppType, string placeHolder) {
		this->Name = name;
		this->DbType = dbType;
		this->DbSize = dbSize;
		this->CppType = cppType;
		this->PlaceHolder = placeHolder;
	}
};

class AutoGenerateTool {
private:
	const string DECLARE_TEMPLATE = "	@type @name;\n";
	const string INITIAL_TEMPLATE = "		this->@name = model.@name;\n";
	const string EQUAL_TEMPLATE = "		this->@name = model.@name;\n";
	const string RIGHT_TEMPLATE = "    s >> model.@name;\n";
	const string LEFT_TEMPLATE = "    s << model.@name;\n";
	const string IDENTITY_COL = "id";
	const string PRIMARY_KEY_COL = "id";

	otl_connect Db;

public:
	AutoGenerateTool() {
	}

	void Connect(string connStr) {
		otl_connect::otl_initialize(); // initialize OCI environment
		Db.rlogon(connStr.c_str());
		Db.auto_commit_off();
	}

	void GetTableNames(string connStr, vector<string>& tableNames) {
		try {
			Connect(connStr);
			otl_stream o(200, "select name from sys.tables", Db);

			string tableName;
			while (!o.eof()) {
				o >> tableName;
				tableNames.push_back(tableName);
			}
			o.close();
			Db.logoff();
		}
		catch (otl_exception& p){ // intercept OTL exceptions
			printf("insert Error: (%s) (%s) (%s)\n", p.msg, p.stm_text, p.var_info);
		}
	}

	bool GetColumns(string connStr, string tableName, vector<Column>& columns) {

		Connect(connStr);

		try {
			string columnDescSql;
			columnDescSql.append("select top 1 * from ");
			columnDescSql.append(tableName);

			otl_stream o(200, columnDescSql.c_str(), Db);

			otl_column_desc* desc;
			int desc_len;

			desc = o.describe_select(desc_len);

			for (int n = 0; n < desc_len; n++){
				string placeHolder;
				string cppType;

				placeHolder.append(":");
				placeHolder.append(desc[n].name);

				switch (desc[n].otl_var_dbtype)
				{
				case otl_var_char:
					placeHolder.append("<");
					placeHolder.append("char");
					placeHolder.append("[");
					placeHolder.append(std::to_string(desc[n].dbsize + 2));
					placeHolder.append("]");
					placeHolder.append(">");
					cppType = "string";
					break;

				case otl_var_double:
					placeHolder.append("<double>");
					cppType = "double";
					break;

				case otl_var_float:
					placeHolder.append("<float>");
					cppType = "float";
					break;

				case otl_var_int:
				case otl_var_short:
					placeHolder.append("<int>");
					cppType = "int";
					break;

				case otl_var_unsigned_int:
					placeHolder.append("<unsigned>");
					cppType = "unsigned int";
					break;

				case otl_var_long_int:
					placeHolder.append("<long>");
					cppType = "long";
					break;

				case otl_var_timestamp:
					placeHolder.append("<timestamp>");
					cppType = "otl_datetime";
					break;

				case otl_var_varchar_long:
					placeHolder.append("<varchar_long>");
					cppType = "otl_long_string";
					break;

				case otl_var_raw_long:
					placeHolder.append("<raw_long>");
					cppType = "otl_long_string";
					break;

				case otl_var_bigint:
					placeHolder.append("<bigint>");
					break;

				case otl_var_raw:
					placeHolder.append("<");
					placeHolder.append("raw");
					placeHolder.append("[");
					placeHolder.append(std::to_string(desc[n].dbsize));
					placeHolder.append("]");
					placeHolder.append(">");
					cppType = "string";
					break;

				case otl_var_ubigint:
					placeHolder.append("<ubigint>");
					break;
				}
				Column column((string)desc[n].name, desc[n].otl_var_dbtype, desc[n].dbsize, cppType, placeHolder);
				columns.push_back(column);
			}
			o.close();
			Db.logoff();
		}
		catch (otl_exception& p){ // intercept OTL exceptions
			printf("insert Error: (%s) (%s) (%s)\n", p.msg, p.stm_text, p.var_info);
			Db.logoff();
			return false;
		}
		return true;
	}

	void GenerateDeclare(vector<Column>& columns, string& declare) {
		int n = 0;
		string declareTemplate;
		for (; n < columns.size(); n++){
			declareTemplate = DECLARE_TEMPLATE;
			SqlServerUtil::Replace(declareTemplate, "@type", columns[n].CppType);
			SqlServerUtil::Replace(declareTemplate, "@name", columns[n].Name);
			if (n >= columns.size() - 1) {
				SqlServerUtil::Replace(declareTemplate, "\n", "");
			}
			declare.append(declareTemplate);
		}
	}

	void GenerateInitial(vector<Column>& columns, string& initial) {
		int n = 0;
		string initialTemplate;
		for (; n < columns.size(); n++){
			initialTemplate = INITIAL_TEMPLATE;
			SqlServerUtil::Replace(initialTemplate, "@name", columns[n].Name);
			if (n >= columns.size() - 1) {
				SqlServerUtil::Replace(initialTemplate, "\n", "");
			}
			initial.append(initialTemplate);
		}
	}

	void GenerateEqual(vector<Column> columns, string& equal) {
		int n = 0;
		string equalTemplate;
		for (; n < columns.size(); n++){
			equalTemplate = EQUAL_TEMPLATE;
			SqlServerUtil::Replace(equalTemplate, "@name", columns[n].Name);
			if (n >= columns.size() - 1) {
				SqlServerUtil::Replace(equalTemplate, "\n", "");
			}
			equal.append(equalTemplate);
		}
	}

	void GenerateRight(vector<Column> columns, string& right) {
		int n = 0;
		string rightTemplate;
		for (; n < columns.size(); n++){
			rightTemplate = RIGHT_TEMPLATE;
			SqlServerUtil::Replace(rightTemplate, "@name", columns[n].Name);
			if (n >= columns.size() - 1) {
				SqlServerUtil::Replace(rightTemplate, "\n", "");
			}
			right.append(rightTemplate);
		}
	}

	void GenerateLeft(vector<Column> columns, string& left) {
		int n = 0;
		string leftTemplate;
		for (; n < columns.size(); n++){
			leftTemplate = LEFT_TEMPLATE;
			SqlServerUtil::Replace(leftTemplate, "@name", columns[n].Name);
			if (n >= columns.size() - 1) {
				SqlServerUtil::Replace(leftTemplate, "\n", "");
			}
			else {
				if (columns[n + 1].Name == "id") {
					SqlServerUtil::Replace(leftTemplate, "\n", "");
				}
			}

			if (columns[n].Name == "id") {
				continue;
			}

			left.append(leftTemplate);
		}
	}

	void GetUpdateSql(string tableName, vector<Column>& columns, string& sql) {
		sql.append("update ");
		sql.append(tableName);
		sql.append(" set ");

		Column idColumn;

		int n = 0;
		for (; n < columns.size(); ++n){
			if (PRIMARY_KEY_COL != columns[n].Name) {
				sql.append(columns[n].Name);
				sql.append("=");

				sql.append(columns[n].PlaceHolder);

				if (n < columns.size() - 1) {
					sql.append(",");
				}
			}
			else {
				if (n >= columns.size() - 1) {
					if ("," == sql.substr(sql.length() - 2, sql.length() - 1)) {
						sql = sql.substr(0, sql.length() - 1);
					}
				}
			}

			if ("id" == columns[n].Name) {
				idColumn = columns[n];
			}
		}

		if (idColumn.Name == "id") {
			sql.append(" where ");
			sql.append(idColumn.Name);
			sql.append("=");
			sql.append(idColumn.PlaceHolder);
		}
	}

	void GetAddSql(string tableName, vector<Column>& columns, string& sql) {
		sql.append("insert into ");
		sql.append(tableName);
		sql.append(" values(");

		int n = 0;
		for (; n < columns.size(); ++n){

			if (columns[n].Name != IDENTITY_COL) {
				sql.append(columns[n].PlaceHolder);

				if (n < columns.size() - 1) {
					sql.append(",");
				}
				else {
					sql.append(")");
				}
			}
			else {
				if (n >= columns.size() - 1) {
					if ("," == sql.substr(sql.length() - 2, sql.length() - 1)) {
						sql = sql.substr(0, sql.length() - 1);
					}
					sql.append(")");
				}
			}
		}
	}

	string ChangeTableName(string& str)
	{
		string ret = "";
		for each (auto var in SqlServerUtil::Split(str, "_"))
		{
			var[0] = toupper(var[0]);
			ret.append(var);
		}
		return ret;
	}


	// 本函数需要传入两个参数，templatePath:模板文件路径 
	// connStrPath:连接字符串文件路径,此文件只有一行，
	// 例如:Driver={sql server};server=ip地址,1433;uid=登录名;pwd=密码;database=数据库名;
	// saveDir:根据模板生成的类的存放目录
	void GenerateModel(string templatePath, string connStrPath, string saveDir) {

        /**	
		char buffer[MAX_PATH];
		GetCurrentDirectoryA(MAX_PATH, buffer);
		string templatePath = string(buffer) + "\\Template\\Model.txt"; // 将当前目录下Model.txt
		string connPath = string(buffer) + "\\ConnConfig.txt";
		*/
		
		string connStr = SqlServerUtil::ReadFile(connStrPath.c_str());

		vector<string> tableNames;
		GetTableNames(connStr, tableNames);

		for (int i = 0; i < tableNames.size(); i++) {
			vector<Column> columns;

			if (GetColumns(connStr, tableNames.at(i), columns)) {
		
				string declare;
				GenerateDeclare(columns, declare);

				string initial;
				GenerateInitial(columns, initial);

				string equal;
				GenerateEqual(columns, equal);

				string right;
				GenerateRight(columns, right);

				string left;
				GenerateLeft(columns, left);

				string addSql;
				GetAddSql(tableNames.at(i), columns, addSql);

				string updateSql;
				GetUpdateSql(tableNames.at(i), columns, updateSql);

				string fileContent = SqlServerUtil::ReadFile(templatePath.c_str());
				string entityName = ChangeTableName(tableNames.at(i));

				SqlServerUtil::Replace(fileContent, "@daltblname", tableNames.at(i));
				SqlServerUtil::Replace(fileContent, "@declare", declare);
				SqlServerUtil::Replace(fileContent, "@initial", initial);
				SqlServerUtil::Replace(fileContent, "@equal", equal);
				SqlServerUtil::Replace(fileContent, "@right", right);
				SqlServerUtil::Replace(fileContent, "@left", left);
				SqlServerUtil::Replace(fileContent, "@add", addSql);
				SqlServerUtil::Replace(fileContent, "@tablename", entityName);
				SqlServerUtil::Replace(fileContent, "@update", updateSql);

				string newPath;
				newPath.append(saveDir);
				newPath.append(entityName);
				newPath.append(".hpp");

				SqlServerUtil::WriteFile(newPath.c_str(), fileContent);
			}
		}
	}
};

