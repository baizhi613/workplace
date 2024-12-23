#ifndef __M_DB_H_
#define __M_DB_H_
#include "util.hpp"
#include <cassert>
#include <mutex>

class user_table
{
private:
    MYSQL *_mysql;
    std::mutex _mutex;

public:
    user_table(const std::string &host,
               const std::string &username,
               const std::string &password,
               const std::string &dbname,
               uint16_t port = 3306)
    {
        _mysql = mysql_util::mysql_create(host, username, password, dbname, port);
        assert(_mysql != NULL);
    }
    ~user_table()
    {
        mysql_util::mysql_destroy(_mysql);
        _mysql = NULL;
    }
    bool insert(Json::Value &user)
    {
#define INSERT_USER "insert into user values(null,'%s',SHA2('%s',256),1000,0,0);"
        if (user["password"].isNull() || user["username"].isNull())
        {
            DLOG("INPUT PASSWORD OR USERNAME");
            return false;
        }
        char sql[4096] = {0};
        sprintf(sql, INSERT_USER, user["username"].asCString(), user["password"].asCString());
        bool ret = mysql_util::mysql_exec(_mysql, sql);
        if (ret == false)
        {
            DLOG("insert user info failed!!\n");
            return false;
        }
        return true;
    }
    bool login(Json::Value &user)
    {
        if (user["password"].isNull() || user["username"].isNull())
        {
            DLOG("INPUT PASSWORD OR USERNAME");
            return false;
        }
#define LOGIN_USER "select id,score,total_count,win_count from user where username='%s' and password=SHA2('%s',256);"
        char sql[4096] = {0};
        sprintf(sql, LOGIN_USER, user["username"].asCString(), user["password"].asCString());
        MYSQL_RES *res = NULL;
        {
            std::unique_lock<std::mutex> lock(_mutex);
            bool ret = mysql_util::mysql_exec(_mysql, sql);
            if (ret == false)
            {
                DLOG("user login failed!!\n");
                return false;
            }
            res = mysql_store_result(_mysql);
            if (res == NULL)
            {
                DLOG("have no login user info!!");
                return false;
            }
        }
        int row_num = mysql_num_rows(res);
        if (row_num != 1)
        {
            DLOG("the user information queried is not unique!!");
            return false;
        }
        MYSQL_ROW row = mysql_fetch_row(res);
        user["id"] = (Json::UInt64)std::stol(row[0]);
        user["score"] = (Json::UInt64)std::stol(row[1]);
        user["total_count"] = std::stoi(row[2]);
        user["win_count"] = std::stoi(row[3]);
        mysql_free_result(res);
        return true;
    }
    bool select_by_name(const std::string &name, Json::Value &user)
    {
#define USER_BY_NAME "select id,score,total_count,win_count from user where username='%s';"
        char sql[4096] = {0};
        sprintf(sql, USER_BY_NAME, name.c_str());
        MYSQL_RES *res = NULL;
        {
            std::unique_lock<std::mutex> lock(_mutex);
            bool ret = mysql_util::mysql_exec(_mysql, sql);
            if (ret == false)
            {
                DLOG("get user by name failed!!\n");
                return false;
            }
            res = mysql_store_result(_mysql);
            if (res == NULL)
            {
                DLOG("have no user info!!");
                return false;
            }
        }
        int row_num = mysql_num_rows(res);
        if (row_num != 1)
        {
            // 所以查不到，所以row_num为0，不等于1，所以走到这里了
            printf("%d\n", row_num);
            DLOG("the user information queried is not unique!!");
            return false;
        }
        MYSQL_ROW row = mysql_fetch_row(res);
        user["id"] = (Json::UInt64)std::stol(row[0]);
        user["username"] = name;
        user["score"] = (Json::UInt64)std::stol(row[1]);
        user["total_count"] = std::stoi(row[2]);
        user["win_count"] = std::stoi(row[3]);
        mysql_free_result(res);
        return true;
    }
    bool select_by_id(uint64_t id, Json::Value &user)
    {
#define USER_BY_ID "select username,score,total_count,win_count from user where id=%ld;"
        char sql[4096] = {0};
        sprintf(sql, USER_BY_ID, id);
        MYSQL_RES *res = NULL;
        {
            std::unique_lock<std::mutex> lock(_mutex);
            bool ret = mysql_util::mysql_exec(_mysql, sql);
            if (ret == false)
            {
                DLOG("get user by name failed!!\n");
                return false;
            }
            res = mysql_store_result(_mysql);
            if (res == NULL)
            {
                DLOG("have no user info!!");
                return false;
            }
        }
        int row_num = mysql_num_rows(res);
        if (row_num != 1)
        {
            DLOG("the user information queried is not unique!!");
            return false;
        }
        MYSQL_ROW row = mysql_fetch_row(res);
        user["id"] = (Json::UInt64)id;
        user["username"] = row[0];
        user["score"] = (Json::UInt64)std::stol(row[1]);
        user["total_count"] = std::stoi(row[2]);
        user["win_count"] = std::stoi(row[3]);
        mysql_free_result(res);
        return true;
    }
    bool win(uint64_t id)
    {
#define USER_WIN "update user set score=score+30,total_count=total_count+1,win_count=win_count+1 where id=%ld;"
        char sql[4096] = {0};
        sprintf(sql, USER_WIN, id);
        bool ret = mysql_util::mysql_exec(_mysql, sql);
        if (ret == false)
        {
            DLOG("update win user info failed!!\n");
            return false;
        }
        return true;
    }
    bool lose(uint64_t id)
    {
#define USER_LOSE "update user set score=score-30,total_count=total_count+1 where id=%ld;"
        char sql[4096] = {0};
        sprintf(sql, USER_LOSE, id);
        bool ret = mysql_util::mysql_exec(_mysql, sql);
        if (ret == false)
        {
            DLOG("update lose user info failed!!\n");
            return false;
        }
        return true;
    }
};
#endif