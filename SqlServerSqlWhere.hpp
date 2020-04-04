#pragma once

#include<string>
#include<vector>
#include<iterator>
#include<mutex>
#include<set>

#define OTL_ODBC_MSSQL_2005 // Compile OTL 4.0/SNAC 10 or later
#define _CRT_SECURE_NO_WARNINGS
// #define OTL_UNICODE
#define OTL_STL // Turn on STL features
#define OTL_ANSI_CPP // Turn on ANSI C++ typecasts

#include "otlv4.h"

using namespace std;

/// <summary>
/// where子句各个条件的组合方式
/// </summary>
enum SqlServerLogicalOperatorType
{
	/// <summary>
	/// and
	/// </summary>
	AND = 0,
	/// <summary>
	/// or
	/// </summary>
	OR,
	/// <summary>
	/// 无效运算符
	/// </summary>
	NONE
};

/// <summary>
/// where子句各个条件的条件判断方式
/// </summary>
enum SqlServerConditionOperatorType {
	/// <summary>
	/// is null
	/// </summary>
	IS_NULL,
	/// <summary>
	/// is not null
	/// </summary>
	NOT_NULL,
	/// <summary>
	/// =
	/// </summary>
	EQ,
	/// <summary>
	/// !=
	/// </summary>
	NOT_EQ,
	/// <summary>
	/// &lt;
	/// </summary>
	LT,
	/// <summary>
	/// &gt;
	/// </summary>
	GT,
	/// <summary>
	/// The less than or equal
	/// </summary>
	LE,
	/// <summary>
	/// The greater than or equal
	/// </summary>
	GE,
	/// <summary>
	/// in
	/// </summary>
	IS_IN,
	/// <summary>
	/// like
	/// </summary>
	LIKE,
	/// <summary>
	/// not in
	/// </summary>
	NOT_IN,
	/// <summary>
	/// not like
	/// </summary>
	NOT_LIKE
};

class SqlServerSqlParameter {
public:
	// doc:http://otl.sourceforge.net/otl4_mssql_examples.htm
	enum SqlParameterType
	{
		STRING,
		DOUBLE,
		FLOAT,
		INT,
		UNSIGNED,
		LONG,
		TIMESTAMP,
		NONE
	};

	string ParameterName;
	SqlParameterType ParameterType;
	int ParameterTypeSize;

	SqlServerSqlParameter() {
	}

	SqlServerSqlParameter(string parameterName, SqlParameterType parameterType,
		int parameterTypeSize) {
		this->ParameterName = parameterName;
		this->ParameterType = parameterType;
		this->ParameterTypeSize = parameterTypeSize;
	}
};

/// <summary>
/// sql where子句中的条件
/// </summary>
class SqlServerCondition
{
private:
	vector<SqlServerCondition> Child;

public:
	SqlServerLogicalOperatorType LogicalOperator = SqlServerLogicalOperatorType::AND;
	SqlServerConditionOperatorType ConditionOperator = SqlServerConditionOperatorType::EQ;
	string Sql = "";

	SqlServerSqlParameter Parameter;

	SqlServerCondition() {
	}

	SqlServerCondition(SqlServerLogicalOperatorType logicalOperator, string sql = "") {
		this->LogicalOperator = logicalOperator;
		this->Sql = sql;
	}

	SqlServerCondition(SqlServerLogicalOperatorType logicalOperator,
		SqlServerConditionOperatorType conditionOperator, SqlServerSqlParameter parameter) {
		this->LogicalOperator = logicalOperator;
		this->ConditionOperator = conditionOperator;
		this->Parameter = parameter;
	}

	void AddCondition(SqlServerCondition condition) {
		Child.push_back(condition);
	}

	void AddCondition(SqlServerLogicalOperatorType logicalOperator, string sql = "") {
		Child.push_back(SqlServerCondition(logicalOperator, sql));
	}

	void AddCondition(SqlServerLogicalOperatorType logicalOperator,
		SqlServerConditionOperatorType conditionOperator, SqlServerSqlParameter parameter) {
		Child.push_back(SqlServerCondition(logicalOperator, conditionOperator, parameter));
	}

	vector<SqlServerCondition>& GetChild() {
		return Child;
	}
};


class VarInfo {

public:
	SqlServerSqlParameter::SqlParameterType Type;
	int Index;

	VarInfo(SqlServerSqlParameter::SqlParameterType type, int index) {
		this->Type = type;
		this->Index = index;
	}
};

class ConditionVars {

private:
	vector<VarInfo> VarInfos;

	vector<int> Ints;
	vector<double> Doubles;
	vector<float> Floats;
	vector<unsigned int> UnsignedInts;
	vector<long> Longs;
	vector<otl_datetime> OtlDateTimes;
	vector<string> Strings;

public:

	ConditionVars() {
	}

	void AddIntValue(int val) {
		VarInfos.push_back(VarInfo(SqlServerSqlParameter::SqlParameterType::INT, Ints.size()));
		Ints.push_back(val);
	}

	void AddDoubleValue(double val) {
		VarInfos.push_back(VarInfo(SqlServerSqlParameter::SqlParameterType::DOUBLE, Doubles.size()));
		Doubles.push_back(val);
	}

	void AddFloatValue(float val) {
		VarInfos.push_back(VarInfo(SqlServerSqlParameter::SqlParameterType::FLOAT, Floats.size()));
		Floats.push_back(val);
	}

	void AddUnsignedIntValue(unsigned int val) {
		VarInfos.push_back(VarInfo(SqlServerSqlParameter::SqlParameterType::UNSIGNED, UnsignedInts.size()));
		UnsignedInts.push_back(val);
	}

	void AddLongValue(long val) {
		VarInfos.push_back(VarInfo(SqlServerSqlParameter::SqlParameterType::LONG, Longs.size()));
		Longs.push_back(val);
	}


	void AddOtlDateTimeValue(otl_datetime val) {
		VarInfos.push_back(VarInfo(SqlServerSqlParameter::SqlParameterType::TIMESTAMP, OtlDateTimes.size()));
		OtlDateTimes.push_back(val);
	}

	void AddStringValue(string val) {
		VarInfos.push_back(VarInfo(SqlServerSqlParameter::SqlParameterType::STRING, Strings.size()));
		Strings.push_back(val);
	}

	vector<int>& GetInts() {
		return Ints;
	}

	vector<double>& GetDoubles() {
		return Doubles;
	}

	vector<float>& GetFloats() {
		return Floats;
	}

	vector<unsigned int>& GetUnsignedInts() {
		return UnsignedInts;
	}

	vector<long>& GetLongs() {
		return Longs;
	}

	vector<otl_datetime>& GetOtlDateTimes() {
		return OtlDateTimes;
	}

	vector<string>& GetStrings() {
		return Strings;
	}

	vector<VarInfo>& GetVarInfos() {
		return VarInfos;
	}

	void Clear() {
		VarInfos.clear();
		Ints.clear();
		Doubles.clear();
		Floats.clear();
		UnsignedInts.clear();
		Longs.clear();
		OtlDateTimes.clear();
		Strings.clear();
	}
};

class SqlServerSqlWhere
{
private:
	mutex ConditionMtx;
	vector<SqlServerCondition> Conditions;
	bool EnableAutoFillWhere = true;

public:
	ConditionVars ConditionVars;

public:

	SqlServerSqlWhere() {
	}

	SqlServerSqlWhere(vector<SqlServerCondition> Conditions) {
		this->Conditions = Conditions;
	}

	std::vector<SqlServerCondition> GetConditions() {
		return Conditions;
	}

	/// <summary>
	/// 添加一个条件为‘与’且值为相等的条件
	/// </summary>
	/// <param name="SqlServerCondition">The SqlServerCondition.</param>
	void Add(SqlServerCondition Condition)
	{
		unique_lock<mutex> lckMtx(ConditionMtx, defer_lock);
		lckMtx.lock();
		Conditions.push_back(Condition);
		lckMtx.unlock();
	}

	void Add(SqlServerLogicalOperatorType logicalOperator, SqlServerConditionOperatorType conditionOperator,
		string columnName, SqlServerSqlParameter::SqlParameterType parameterType = SqlServerSqlParameter::SqlParameterType::NONE, int parameterTypeSize = -1
		) {
		Conditions.push_back(SqlServerCondition(logicalOperator, conditionOperator, SqlServerSqlParameter(columnName, parameterType, parameterTypeSize)));
	}

	/// <summary>
	/// 添加条件字符串，不安全
	/// </summary>
	/// <param name="sql">The SqlServerCondition SQL.</param>
	/// <param name="logicOperator">The logic operator.</param>
	void AddSql(SqlServerLogicalOperatorType logicOperator, string sql)
	{
		if (sql.empty())
		{
			return;
		}
		unique_lock<mutex> lckMtx(ConditionMtx, defer_lock);
		lckMtx.lock();
		Conditions.push_back(SqlServerCondition(logicOperator, sql));
		lckMtx.unlock();
	}

	/// <summary>
	/// 清空现有的条件
	/// </summary>
	void Clear()
	{
		Conditions.clear();
		ConditionVars.Clear();
	}

	string GetSql(SqlServerCondition Condition) {

		string ConditionSql = "";

		if (Condition.LogicalOperator == SqlServerLogicalOperatorType::AND) {
			ConditionSql.append(" and ");
		}
		else if (Condition.LogicalOperator == SqlServerLogicalOperatorType::OR) {
			ConditionSql.append(" or ");
		}

		if (Condition.GetChild().size() > 0)
		{
			// 分组查询条件处理
			string childSql = "(";
			for (auto it = Condition.GetChild().begin(); it != Condition.GetChild().end(); it++) {
				childSql.append(GetSql(*it));
			}

			ConditionSql.append(childSql).append(")");
			return ConditionSql;
		}

		if (!Condition.Sql.empty()) { //直接指定sql的查询条件处理
			ConditionSql.append(Condition.Sql);
		}
		else {
			if (Condition.ConditionOperator == SqlServerConditionOperatorType::IS_NULL) {
				ConditionSql.append(Condition.Parameter.ParameterName).append(" is null");
			}
			else if (Condition.ConditionOperator == SqlServerConditionOperatorType::NOT_NULL) {
				ConditionSql.append(Condition.Parameter.ParameterName).append(" is not null");
			}
			else {
				ConditionSql.append(Condition.Parameter.ParameterName);

				// 默认使用参数的查询条件处理
				if (Condition.ConditionOperator == SqlServerConditionOperatorType::EQ) {
					ConditionSql.append("=:");
				}
				else if (Condition.ConditionOperator == SqlServerConditionOperatorType::NOT_EQ) {
					ConditionSql.append("!=:");
				}
				else if (Condition.ConditionOperator == SqlServerConditionOperatorType::LT) {
					ConditionSql.append("<:");
				}
				else if (Condition.ConditionOperator == SqlServerConditionOperatorType::GT) {
					ConditionSql.append(">:");
				}
				else if (Condition.ConditionOperator == SqlServerConditionOperatorType::LE) {
					ConditionSql.append("<=");
				}
				else if (Condition.ConditionOperator == SqlServerConditionOperatorType::GE) {
					ConditionSql.append(">=");
				}
				else if (Condition.ConditionOperator == SqlServerConditionOperatorType::IS_IN) {
					ConditionSql.append(" in (:");
				}
				else if (Condition.ConditionOperator == SqlServerConditionOperatorType::LIKE) {
					ConditionSql.append(" like :");
				}
				else if (Condition.ConditionOperator == SqlServerConditionOperatorType::NOT_IN) {
					ConditionSql.append(" not in (:");
				}
				else if (Condition.ConditionOperator == SqlServerConditionOperatorType::NOT_LIKE) {
					ConditionSql.append(" not like :");
				}

				ConditionSql.append(Condition.Parameter.ParameterName);

				switch (Condition.Parameter.ParameterType)
				{
				case SqlServerSqlParameter::SqlParameterType::DOUBLE:
					ConditionSql.append("<double>");
					break;

				case SqlServerSqlParameter::SqlParameterType::FLOAT:
					ConditionSql.append("<float>");
					break;

				case SqlServerSqlParameter::SqlParameterType::INT:
					ConditionSql.append("<int>");
					break;

				case SqlServerSqlParameter::SqlParameterType::UNSIGNED:
					ConditionSql.append("<unsigned int>");
					break;

				case SqlServerSqlParameter::SqlParameterType::LONG:
					ConditionSql.append("<long>");
					break;

				case SqlServerSqlParameter::SqlParameterType::TIMESTAMP:
					ConditionSql.append("<timestamp>");
					break;

				case SqlServerSqlParameter::SqlParameterType::STRING:
					if (Condition.Parameter.ParameterTypeSize >= 0) {
						ConditionSql.append("<char(")
							.append(std::to_string(Condition.Parameter.ParameterTypeSize)).append(")>");
					}
					else {
						ConditionSql.append("<char(1)>");
					}
					break;


				default:
					break;
				}

				if (Condition.ConditionOperator == SqlServerConditionOperatorType::IS_IN) {
					ConditionSql.append(")");
				}
				else if (Condition.ConditionOperator == SqlServerConditionOperatorType::NOT_IN) {
					ConditionSql.append(")");
				}
			}
		}

		return ConditionSql;
	}

	void SetEnableAutoFillWhere(bool isEnable){
		EnableAutoFillWhere = isEnable;
	}

	string GetSql() {
		unique_lock<mutex> lckMtx(ConditionMtx, defer_lock);
		lckMtx.lock();
		string sql;

		if (Conditions.size() > 0) {
			if (EnableAutoFillWhere) {
				sql.append(" where ");
			}
		}

		for (auto Condition = Conditions.begin(); Condition != Conditions.end(); Condition++) {
			sql.append(GetSql(*Condition));
		}

		lckMtx.unlock();
		return sql;
	}
};
