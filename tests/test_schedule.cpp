#include "../include/pis_schedule.h"
#include "../include/pis_config.h"
#include <iostream>
#include <cassert>
#include <string>
#include <vector>

using namespace pis;

static int g_test_count = 0, g_pass_count = 0;
#define TEST(name) do { ++g_test_count; std::cout << "  TEST " << g_test_count << ": " << name << "... "; } while(0)
#define PASS() do { ++g_pass_count; std::cout << "PASSED" << std::endl; } while(0)
#define FAIL(msg) do { std::cout << "FAILED: " << msg << std::endl; assert(false); } while(0)
#define ASSERT_EQ(a,b) do { if((a)!=(b)) FAIL("Expected " #a " == " #b " ("+std::to_string(a)+" vs "+std::to_string(b)+")"); } while(0)
#define ASSERT_GT(a,b) do { if(!((a)>(b))) FAIL("Expected " #a " > " #b); } while(0)

Route createRoute(int n) {
    Route r; r.id="TEST"; r.name="Test"; r.direction="cw";
    for(int i=0;i<n;++i){Station s;s.id="T"+std::to_string(i+1);s.name_en="S"+std::to_string(i+1);s.arrival_sec=i*60;r.stations.push_back(s);}
    return r;
}

void test_basic() { TEST("Basic 3 stations"); auto r=createRoute(3); ScheduleEngine e; e.initialize(r); ASSERT_GT(e.eventCount(),0); PASS(); }
void test_ordering() { TEST("Events ordered by time"); auto r=createRoute(3); ScheduleEngine e; std::vector<ScheduleEvent> v; e.onEvent([&](const ScheduleEvent& ev){v.push_back(ev);}); e.initialize(r); e.start(); e.stop(); for(size_t i=1;i<v.size();++i) ASSERT_GT(v[i].elapsed_sec,v[i-1].elapsed_sec); PASS(); }
void test_callback() { TEST("Multiple callbacks"); auto r=createRoute(2); ScheduleEngine e; int c1=0,c2=0; e.onEvent([&](const ScheduleEvent&){++c1;}); e.onEvent([&](const ScheduleEvent&){++c2;}); e.initialize(r); ASSERT_EQ(c1,0); ASSERT_EQ(c2,0); PASS(); }
void test_single() { TEST("Single station"); auto r=createRoute(1); ScheduleEngine e; e.initialize(r); PASS(); }
void test_custom() { TEST("Custom intervals"); Route r; r.name="X"; auto a=[&](auto id,auto nm,int s){Station st;st.id=id;st.name_en=nm;st.arrival_sec=s;r.stations.push_back(st);}; a("A","A",0);a("B","B",30);a("C","C",90);a("D","D",210); ScheduleEngine e; e.initialize(r); ASSERT_GT(e.eventCount(),0); PASS(); }
void test_state() { TEST("Engine state"); ScheduleEngine e; ASSERT_TRUE(!e.isRunning()); ASSERT_EQ(e.currentStationIndex(),-1); e.initialize(createRoute(2)); PASS(); }

int main() {
    std::cout << "\n=== PIS Schedule Engine Tests ===\n" << std::endl;
    test_basic(); test_ordering(); test_callback(); test_single(); test_custom(); test_state();
    std::cout << "\n=== Results: " << g_pass_count << "/" << g_test_count << " passed ===\n" << std::endl;
    return (g_pass_count==g_test_count)?0:1;
}
