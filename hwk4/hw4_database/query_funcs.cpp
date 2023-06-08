#include "query_funcs.h"

void add_player(connection *C, int team_id, int jersey_num, string first_name, string last_name,
                int mpg, int ppg, int rpg, int apg, double spg, double bpg)
{
    work curTrans(*C);
    stringstream sql;
    sql << "INSERT INTO PLAYER (TEAM_ID, UNIFORM_NUM, FIRST_NAME, LAST_NAME, MPG, PPG, RPG, APG, SPG, BPG) "
        << "VALUES (" << team_id << ", " << jersey_num << ", " << curTrans.quote(first_name) << ", "
        << curTrans.quote(last_name) << ", " << mpg << ", " << ppg << ", " << rpg << ", " << apg << ", "
        << spg << ", " << bpg << ");";
    curTrans.exec(sql.str());
    curTrans.commit();
}

void add_team(connection *C, string name, int state_id, int color_id, int wins, int losses)
{
    work curTrans(*C);
    stringstream sql;
    sql << "INSERT INTO TEAM (NAME, STATE_ID, COLOR_ID, WINS, LOSSES) "
        << "VALUES (" << curTrans.quote(name) << ", " << state_id << ", " << color_id << ", "
        << wins << ", " << losses << ");";
    curTrans.exec(sql.str());
    curTrans.commit();
}

void add_state(connection *C, string name)
{
    work curTrans(*C);
    stringstream sql;
    sql << "INSERT INTO STATE (NAME) VALUES (" << curTrans.quote(name) << ");";
    curTrans.exec(sql.str());
    curTrans.commit();
}

void add_color(connection *C, string name)
{
    work curTrans(*C);
    stringstream sql;
    sql << "INSERT INTO COLOR (NAME) VALUES (" << curTrans.quote(name) << ");";
    curTrans.exec(sql.str());
    curTrans.commit();
}

/*
 * All use_ params are used as flags for corresponding attributes
 * a 1 for a use_ param means this attribute is enabled (i.e. a WHERE clause is needed)
 * a 0 for a use_ param means this attribute is disabled
 */
void query1(connection *C,
            int use_mpg, int min_mpg, int max_mpg,
            int use_ppg, int min_ppg, int max_ppg,
            int use_rpg, int min_rpg, int max_rpg,
            int use_apg, int min_apg, int max_apg,
            int use_spg, double min_spg, double max_spg,
            int use_bpg, double min_bpg, double max_bpg)
{
    nontransaction curNonTrans(*C);
    stringstream sql;
    sql << "SELECT * FROM PLAYER";
    bool where_pass = false;
    string stat_name[6] = {"MPG", "PPG", "RPG", "APG", "SPG", "BPG"};
    int use_params[6] = {use_mpg, use_ppg, use_rpg, use_apg, use_spg, use_bpg};
    int min_attrs_int[4] = {min_mpg, min_ppg, min_rpg, min_apg};
    int max_attrs_int[4] = {max_mpg, max_ppg, max_rpg, max_apg};
    double min_attrs_double[2] = {min_spg, min_bpg};
    double max_attrs_double[2] = {max_spg, max_bpg};
    for(int i = 0; i < 6; i++) {
        if(use_params[i]) {
            if(!where_pass) {
                sql << " WHERE ";
                where_pass = true;
            }
            else {
                sql << " AND ";
            }
            if(i < 4) {
                sql << "(" << stat_name[i] << " BETWEEN " << min_attrs_int[i]
                    << " AND " << max_attrs_int[i] << ")";
            }
            else {
                sql << "(" << stat_name[i] << " BETWEEN " << min_attrs_double[i-4]
                    << " AND " << max_attrs_double[i-4] << ")";               
            }
        }
    }
    sql << ";";
    result res(curNonTrans.exec(sql.str()));
    cout << fixed << setprecision(1);
    cout << "PLAYER_ID TEAM_ID UNIFORM_NUM FIRST_NAME LAST_NAME MPG PPG RPG APG SPG BPG" << endl;
    for (result::const_iterator row = res.begin(); row != res.end(); row++)
    {
        cout << row[0].as<int>() << ' ' << row[1].as<int>() << ' ' << row[2].as<int>()
            << ' ' << row[3].as<string>() << ' ' << row[4].as<string>() << ' ' << row[5].as<int>()
            << ' ' << row[6].as<int>() << ' ' << row[7].as<int>() << ' ' << row[8].as<int>()
            << ' ' << row[9].as<double>() << ' ' << row[10].as<double>() << endl;
    }
}

void query2(connection *C, string team_color)
{
    nontransaction curNonTrans(*C);
    stringstream sql;
    sql << "SELECT TEAM.NAME FROM TEAM, COLOR "
        << "WHERE TEAM.COLOR_ID = COLOR.COLOR_ID "
        << "AND COLOR.NAME = " << curNonTrans.quote(team_color) << ";";
    result res(curNonTrans.exec(sql.str()));
    cout << "NAME" << endl;
    for (result::const_iterator row = res.begin(); row != res.end(); row++)
    {
        cout << row[0].as<string>() << endl;
    }
}

void query3(connection *C, string team_name)
{
    nontransaction curNonTrans(*C);
    stringstream sql;
    sql << "SELECT FIRST_NAME, LAST_NAME FROM PLAYER, TEAM "
        << "WHERE PLAYER.TEAM_ID = TEAM.TEAM_ID AND "
        << "TEAM.NAME = " << curNonTrans.quote(team_name)
        << " ORDER BY PLAYER.PPG DESC;";
    result res(curNonTrans.exec(sql.str()));
    cout << "FIRST_NAME LAST_NAME" << endl;
    for (result::const_iterator row = res.begin(); row != res.end(); row++)
    {
        cout << row[0].as<string>() << ' ' << row[1].as<string>() << endl;
    }
}

void query4(connection *C, string team_state, string team_color)
{
    nontransaction curNonTrans(*C);
    stringstream sql;
    sql << "SELECT UNIFORM_NUM, FIRST_NAME, LAST_NAME "
        << "FROM PLAYER, TEAM, STATE, COLOR "
        << "WHERE PLAYER.TEAM_ID = TEAM.TEAM_ID AND "
        << "TEAM.STATE_ID = STATE.STATE_ID AND "
        << "TEAM.COLOR_ID = COLOR.COLOR_ID AND "
        << "STATE.NAME = " << curNonTrans.quote(team_state)
        << " AND COLOR.NAME = " << curNonTrans.quote(team_color) << ";";
    result res(curNonTrans.exec(sql.str()));
    cout << "UNIFORM_NUM FIRST_NAME LAST_NAME" << endl;
    for (result::const_iterator row = res.begin(); row != res.end(); row++)
    {
        cout << row[0].as<int>() << ' ' << row[1].as<string>() << ' ' << row[2].as<string>() << endl;
    }
}

void query5(connection *C, int num_wins)
{
    nontransaction curNonTrans(*C);
    stringstream sql;
    sql << "SELECT FIRST_NAME, LAST_NAME, NAME, WINS "
    << "FROM PLAYER, TEAM "
    << "WHERE PLAYER.TEAM_ID = TEAM.TEAM_ID AND "
    << "TEAM.WINS > " << num_wins << ";";
    result res(curNonTrans.exec(sql.str()));
    cout << "FIRST_NAME LAST_NAME NAME WINS" << endl;
    for (result::const_iterator row = res.begin(); row != res.end(); row++)
    {
        cout << row[0].as<string>() << ' ' << row[1].as<string>()
        << ' ' << row[2].as<string>() << ' ' << row[3].as<int>() << endl;
    }
}
