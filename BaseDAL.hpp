#pragma once

#include <vector>
#include <iterator>
#include<string>
#include<iostream>
#include<sstream>
#include <time.h>

#define OTL_ODBC_MSSQL_2005 // Compile OTL 4.0/SNAC 10 or later
#define OTL_ODBC_TIME_ZONE
#define _CRT_SECURE_NO_WARNINGS
#define OTL_STL // Turn on STL features
#define OTL_ANSI_CPP // Turn on ANSI C++ typecasts


#include "otlv4.h"
#include<mutex>
#include "SqlServerSqlWhere.hpp"
#include "SqlServerHelper.hpp"
#include "SqlServerUtil.h"


template<typename T>
class BaseDAL {
private:
	otl_connect Db;
	mutex Mtx;
	bool Connected = false;
	const int FAIL_RETRY_TIME = 2;
	int RetryTime = 0;

	BaseDAL() {
	}

	BaseDAL(const BaseDAL&) {
	}

	BaseDAL& operator=(const BaseDAL&) {
	}

	~BaseDAL() {

	}

	typedef void(*ErrorMsgCall)(string msg);
	ErrorMsgCall Call;

protected:
	string ConnStr;

private:
	bool OtlStreamInput(otl_stream& o, ConditionVars& vars) {
		try {
			for (auto it = vars.GetVarInfos().begin(); it != vars.GetVarInfos().end(); it++) {
				if (it->Type == SqlServerSqlParameter::SqlParameterType::INT) {
					o << vars.GetInts().at(it->Index);
				}
				else if (it->Type == SqlServerSqlParameter::SqlParameterType::DOUBLE) {
					o << vars.GetDoubles().at(it->Index);
				}
				else if (it->Type == SqlServerSqlParameter::SqlParameterType::FLOAT) {
					o << vars.GetFloats().at(it->Index);
				}
				else if (it->Type == SqlServerSqlParameter::SqlParameterType::UNSIGNED) {
					o << vars.GetUnsignedInts().at(it->Index);
				}
				else if (it->Type == SqlServerSqlParameter::SqlParameterType::LONG) {
					o << vars.GetLongs().at(it->Index);
				}
				else if (it->Type == SqlServerSqlParameter::SqlParameterType::TIMESTAMP) {
					o << vars.GetOtlDateTimes().at(it->Index);
				}
				else if (it->Type == SqlServerSqlParameter::SqlParameterType::STRING) {
					o << vars.GetStrings().at(it->Index);
				}
			}
		}
		catch (otl_exception& p) {
			SetExcpetion(p);
			return false;
		}
		return true;
	}

public:
	static BaseDAL<T>& GetInstance() {
		static BaseDAL<T> instance;
		return instance;
	}

	bool Initial(string connStr) {
		this->ConnStr = connStr;
		return true;
	}

	void RegisterErrorMsgCall(ErrorMsgCall call) {
		this->Call = call;
	}

	bool Connect() {
		if (this->ConnStr.empty()) {
			return false;
		}
		if (Connected) return true;
		try {
			otl_connect::otl_initialize();
			Db.rlogon(this->ConnStr.c_str());
			if (Db.connected) {
				Connected = true;
			}
		}
		catch (otl_exception& p) {
			SetExcpetion(p);
			return false;
		}
		return true;
	}

	bool AllowConnect() {
		bool connected = Connect();
		DisConnect();
		return connected;
	}

	bool TryConnect() {
		if (!Connected) {
			if (!Connect()) {
				return false;
			}
		}
		return true;
	}

	bool GetModel(string tableName, T& model) {
		unique_lock<mutex> lckMtx(Mtx, defer_lock);
		lckMtx.lock();

		bool ret = true;

		if (ret = TryConnect()) {
			try {
				string sql;
				sql.append("select * from ");
				sql.append(tableName);
				sql.append(" ");
				sql.append(" where id=:id<int>");

				otl_stream o(50, sql.c_str(), Db);

				o << model.id;

				vector<T> v;

				copy(otl_input_iterator<T, ptrdiff_t>(o),
					otl_input_iterator<T, ptrdiff_t>(),
					back_inserter(v));

				o.close();

				if (v.size() > 0) {
					model = v.at(0);
				}
			}
			catch (otl_exception& p){
				ret = false;
				SetExcpetion(p);
			}
		}
		lckMtx.unlock();
		if (!Connected && (++RetryTime <= FAIL_RETRY_TIME)) {
			return GetModel(tableName, model);
		}
		else {
			RetryTime = 0;
		}
		return ret;
	}

	bool GetModelList(string tableName, vector<T>& models) {
		unique_lock<mutex> lckMtx(Mtx, defer_lock);
		lckMtx.lock();
		bool ret = true;
		if (ret = TryConnect()) {
			try {
				string sql;
				sql.append("select * from ");
				sql.append(tableName);

				otl_stream o(50, sql.c_str(), Db);

				copy(otl_input_iterator<T, ptrdiff_t>(o),
					otl_input_iterator<T, ptrdiff_t>(),
					back_inserter(models));
				o.close();
			}
			catch (otl_exception& p) {
				ret = false;
				SetExcpetion(p);
			}
		}
		lckMtx.unlock();
		if (!Connected && (++RetryTime <= FAIL_RETRY_TIME)) {
			return GetModelList(tableName, models);
		}
		else {
			RetryTime = 0;
		}
		return ret;
	}

	bool GetModels(SqlServerSqlWhere& sqlWhere, vector<T>& models) {
		unique_lock<mutex> lckMtx(Mtx, defer_lock);
		lckMtx.lock();
		bool ret = true;
		if (ret = TryConnect()) {
			try {
				otl_stream o(50, sqlWhere.GetSql().c_str(), Db);
				OtlStreamInput(o, sqlWhere.ConditionVars);

				copy(otl_input_iterator<T, ptrdiff_t>(o),
					otl_input_iterator<T, ptrdiff_t>(),
					back_inserter(models));

				o.close();
			}
			catch (otl_exception& p){
				ret = false;
				SetExcpetion(p);
			}
		}
		lckMtx.unlock();
		if (!Connected && (++RetryTime <= FAIL_RETRY_TIME)) {
			return GetModels(sqlWhere, models);
		}
		else {
			RetryTime = 0;
		}
		return ret;
	}

	bool GetModelList(string tableName, SqlServerSqlWhere& sqlWhere, vector<T>& models) {
		unique_lock<mutex> lckMtx(Mtx, defer_lock);
		lckMtx.lock();
		bool ret = true;
		if (ret = TryConnect()) {
			try {
				std::string sql;
				sql.append("select * from ");
				sql.append(tableName);
				sql.append(sqlWhere.GetSql());

				otl_stream o(50, sql.c_str(), Db);
				OtlStreamInput(o, sqlWhere.ConditionVars);

				copy(otl_input_iterator<T, ptrdiff_t>(o),
					otl_input_iterator<T, ptrdiff_t>(),
					back_inserter(models));

				o.close();
			}
			catch (otl_exception& p){
				ret = false;
				SetExcpetion(p);
			}
		}
		lckMtx.unlock();
		if (!Connected && (++RetryTime <= FAIL_RETRY_TIME)) {
			return GetModelList(tableName, sqlWhere, models);
		}
		else {
			RetryTime = 0;
		}
		return ret;
	}

	bool Add(string sql, T& model) {
		unique_lock<mutex> lckMtx(Mtx, defer_lock);
		lckMtx.lock();
		bool ret = true;
		if (ret = TryConnect()) {
			otl_stream o(50, sql.c_str(), Db);
			try {
				vector<T> vo;
				vo.push_back(model);
				copy(vo.begin(), vo.end(), otl_output_iterator<T>(o));
				o.close();
			}
			catch (otl_exception& p) {
				o.flush();
				o.close();
				ret = false;
				SetExcpetion(p);
			}
		}
		lckMtx.unlock();
		if (!Connected && (++RetryTime <= FAIL_RETRY_TIME)) {
			return Add(sql, model);
		}
		else {
			RetryTime = 0;
		}
		return ret;
	}

	bool DeleteSingle(string tableName, int id) {
		unique_lock<mutex> lckMtx(Mtx, defer_lock);
		lckMtx.lock();
		bool ret = true;
		if (ret = TryConnect()) {
			try {
				std::string sql;
				sql.append("delete from ");
				sql.append(tableName);
				sql.append(" where ");
				sql.append("id=:id<int>");
				otl_stream o(50, sql.c_str(), Db);
				o << id;
				o.close();
			}
			catch (otl_exception& p){
				SetExcpetion(p);
				ret = false;
			}
		}
		lckMtx.unlock();
		if (!Connected && (++RetryTime <= FAIL_RETRY_TIME)) {
			return DeleteSingle(tableName, id);
		}
		else {
			RetryTime = 0;
		}
		return true;
	}

	bool Delete(string tableName, SqlServerSqlWhere& sqlWhere) {
		unique_lock<mutex> lckMtx(Mtx, defer_lock);
		lckMtx.lock();
		bool ret = true;
		if (ret = TryConnect()) {
			try {
				std::string sql;
				sql.append("delete from ");
				sql.append(tableName);
				sql.append(sqlWhere.GetSql());
				otl_stream o(50, sql.c_str(), Db);
				OtlStreamInput(o, sqlWhere.ConditionVars);
				o.close();
			}
			catch (otl_exception& p){
				ret = false;
				SetExcpetion(p);
			}
		}
		lckMtx.unlock();
		if (!Connected && (++RetryTime <= FAIL_RETRY_TIME)) {
			return Delete(tableName, sqlWhere);
		}
		else {
			RetryTime = 0;
		}
		return ret;
	}

	bool UpdateSingle(const string sql, T& model) {
		unique_lock<mutex> lckMtx(Mtx, defer_lock);
		lckMtx.lock();
		bool ret = true;
		if (ret = TryConnect()) {
			try {
				otl_stream o(50, sql.c_str(), Db);
				vector<T> vo;
				vo.push_back(model);
				copy(vo.begin(), vo.end(), otl_output_iterator<T>(o));
				o << model.id;
				o.close();
			}
			catch (otl_exception& p){
				SetExcpetion(p);
				ret = false;
			}
		}
		lckMtx.unlock();
		if (!Connected && (++RetryTime <= FAIL_RETRY_TIME)) {
			return UpdateSingle(sql, model);
		}
		else {
			RetryTime = 0;
		}
		return ret;
	}

	bool Update(SqlServerSqlWhere& sqlWhere) {
		unique_lock<mutex> lckMtx(Mtx, defer_lock);
		lckMtx.lock();
		bool ret = true;
		if (ret = TryConnect()) {
			try {
				otl_stream o(50, sqlWhere.GetSql().c_str(), Db);
				OtlStreamInput(o, sqlWhere.ConditionVars);
				o.close();
			}
			catch (otl_exception& p){
				SetExcpetion(p);
				ret = false;
			}
		}
		lckMtx.unlock();
		if (!Connected && (++RetryTime <= FAIL_RETRY_TIME)) {
			return Update(sqlWhere);
		}
		else {
			RetryTime = 0;
		}
		return ret;
	}

	bool Update(SqlServerSqlWhere& sqlWhere, T& model) {
		unique_lock<mutex> lckMtx(Mtx, defer_lock);
		lckMtx.lock();
		bool ret = true;
		if (ret = TryConnect()) {
			try {
				otl_stream o(50, sqlWhere.GetSql().c_str(), Db);
				vector<T> vo;
				vo.push_back(model);
				copy(vo.begin(), vo.end(), otl_output_iterator<T>(o));
				OtlStreamInput(o, sqlWhere.ConditionVars);
				o.close();
			}
			catch (otl_exception& p){
				SetExcpetion(p);
				ret = false;
			}
		}
		lckMtx.unlock();
		if (!Connected && (++RetryTime <= FAIL_RETRY_TIME)) {
			return Update(sqlWhere, model);
		}
		else {
			RetryTime = 0;
		}
		return ret;
	}

	int Count(string tableName, SqlServerSqlWhere& sqlWhere) {
		unique_lock<mutex> lckMtx(Mtx, defer_lock);
		lckMtx.lock();
		int ret = -1;
		if (ret = TryConnect()) {
			try {
				std::string sql;
				sql.append("select COUNT(*) from ");
				sql.append(tableName);
				sql.append(sqlWhere.GetSql());
				otl_stream o(50, sql.c_str(), Db);
				OtlStreamInput(o, sqlWhere.ConditionVars);

				o >> ret;
				o.close();
			}
			catch (otl_exception& p){
				SetExcpetion(p);
			}
		}
		lckMtx.unlock();
		if (!Connected && (++RetryTime <= FAIL_RETRY_TIME)) {
			return Count(tableName, sqlWhere);
		}
		else {
			RetryTime = 0;
		}
		return ret;
	}

	std::string BoolToString(bool bval) {
		std::stringstream ss;
		ss << bval;
		return ss.str();
	}

	void SetExcpetion(otl_exception& p) {
		string sqlstate = (char*)p.sqlstate;

		if (!sqlstate.empty()) {
			if (sqlstate == "01000") {
				Connected = false;
			}
			else if (sqlstate == "01002") {
				Connected = false;
			}
			else if (sqlstate >= "08001" && sqlstate <= "08004") {
				Connected = false;
			}
			else if (sqlstate == "08007") {
				Connected = false;
			}
			else if (sqlstate == "08502") {
				Connected = false;
			}
			else if (sqlstate == "08504") {
				Connected = false;
			}
			else if (sqlstate == "08S01") {
				Connected = false;
			}
			else if (sqlstate == "23000") {
				Connected = false;
			}
		}

		string errorMsg;
		errorMsg.append("exception error,sqlsate[");
		errorMsg.append(sqlstate);
		errorMsg.append("]");
		errorMsg.append(",Connected[");
		errorMsg.append(BoolToString(Connected));
		errorMsg.append("]");

		if (!Connected) {
			Db.logoff();
		}

		if (Call != NULL) {
			Call(errorMsg);
		}
#ifdef _DEBUG
		//throw errorMsg;
#else
#endif
	}

	bool DisConnect() {
		unique_lock<mutex> lckMtx(Mtx, defer_lock);
		lckMtx.lock();
		bool ret = true;
		try {
			Db.logoff();
		}
		catch (otl_exception& p){
			SetExcpetion(p);
			ret = false;
		}
		lckMtx.unlock();
		return ret;
	}
};