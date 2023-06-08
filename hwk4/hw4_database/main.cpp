#include <iostream>
#include <fstream>
#include <pqxx/pqxx>

#include "exerciser.h"

using namespace std;
using namespace pqxx;

void executeSQL(const string& sql, connection* C) {
  work curTrans(*C);
  curTrans.exec(sql);
  curTrans.commit();
}

void dropTable(const string& table_name, connection* C) {
  string sql;
  sql = "DROP TABLE IF EXISTS " + table_name + " CASCADE;";
  executeSQL(sql, C);
}

void createTable(const string& file_name, connection *C) {
  string sql;
  ifstream ifs(file_name.c_str(), ifstream::in);
  if(ifs.is_open()) {
    string line;
    while(getline(ifs, line)) {
      sql += line;
    }
    ifs.close();
    executeSQL(sql, C);
  }
  else {
    cerr << "Error: fail to open file: " << file_name << endl;
    exit(EXIT_FAILURE);
  }
}

void addColorValue(const string& file_name, connection* C) {
  int color_id;
  string color_name;
  ifstream ifs(file_name.c_str(), ifstream::in);
  if(ifs.is_open()) {
    string line;
    while(getline(ifs, line)) {
      stringstream spliter;
      spliter << line;
      spliter >> color_id >> color_name;
      add_color(C, color_name);
    }
    ifs.close();
  }
  else {
    cerr << "Error: fail to open file: " << file_name << endl;
    exit(EXIT_FAILURE);
  }
}

void addStateValue(const string& file_name, connection* C) {
  int state_id;
  string state_name;
  ifstream ifs(file_name.c_str(), ifstream::in);
  if(ifs.is_open()) {
    string line;
    while(getline(ifs, line)) {
      stringstream spliter;
      spliter << line;
      spliter >> state_id >> state_name;
      add_state(C, state_name);
    }
    ifs.close();
  }
  else {
    cerr << "Error: fail to open file: " << file_name << endl;
    exit(EXIT_FAILURE);
  }
}

void addTeamValue(const string& file_name, connection* C) {
  int team_id, state_id, color_id;
  int wins, losses;
  string team_name;
  ifstream ifs(file_name.c_str(), ifstream::in);
  if(ifs.is_open()) {
    string line;
    while(getline(ifs, line)) {
      stringstream spliter;
      spliter << line;
      spliter >> team_id >> team_name >> state_id >> color_id >> wins >> losses;
      add_team(C, team_name, state_id, color_id, wins, losses);
    }
    ifs.close();
  }
  else {
    cerr << "Error: fail to open file: " << file_name << endl;
    exit(EXIT_FAILURE);
  }
}

void addPlayerValue(const string& file_name, connection* C) {
  int player_id, team_id, jersey_num;
  int mpg, ppg, rpg, apg;
  double spg, bpg;
  string first_name, last_name;
  ifstream ifs(file_name.c_str(), ifstream::in);
  if(ifs.is_open()) {
    string line;
    while(getline(ifs, line)) {
      stringstream spliter;
      spliter << line;
      spliter >> player_id >> team_id >> jersey_num >> first_name >> last_name
              >> mpg >> ppg >> rpg >> apg >> spg >> bpg;
      add_player(C, team_id, jersey_num, first_name, last_name, mpg, ppg, rpg, apg, spg, bpg);
    }
    ifs.close();
  }
  else {
    cerr << "Error: fail to open file: " << file_name << endl;
    exit(EXIT_FAILURE);
  }
}

int main (int argc, char *argv[]) 
{

  //Allocate & initialize a Postgres connection object
  connection *C;

  try{
    //Establish a connection to the database
    //Parameters: database name, user name, user password
    C = new connection("dbname=ACC_BBALL user=postgres password=passw0rd");
    if (C->is_open()) {
      cout << "Opened database successfully: " << C->dbname() << endl;
    } else {
      cout << "Can't open database" << endl;
      return 1;
    }
  } catch (const std::exception &e){
    cerr << e.what() << std::endl;
    return 1;
  }


  //TODO: create PLAYER, TEAM, STATE, and COLOR tables in the ACC_BBALL database
  //      load each table with rows from the provided source txt files
  dropTable("PLAYER", C);
  dropTable("TEAM", C);
  dropTable("STATE", C);
  dropTable("COLOR", C);
  createTable("create_table.txt", C);
  addColorValue("color.txt", C);
  addStateValue("state.txt", C);
  addTeamValue("team.txt", C);
  addPlayerValue("player.txt", C);

  exercise(C);


  //Close database connection
  C->disconnect();

  return 0;
}


